#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/kthread.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Pet elevator kernel module");

#define ENTRY_NAME "elevator"
#define PERMS 0644
#define PARENT NULL
#define PROC_BUF_SIZE 16384

#define NUM_FLOORS 5
#define MAX_PETS 5
#define MAX_WEIGHT 50
#define NUM_PET_TYPES 4
#define MIN_PET_WEIGHT 3

enum elevator_state { OFFLINE, IDLE, LOADING, UP, DOWN };

/* Pet types: 0 = Chihuahua, 1 = Pug, 2 = Pughuahua, 3 = Dachshund */
static const int pet_weights[NUM_PET_TYPES] = {3, 14, 10, 16};
static const char pet_symbols[NUM_PET_TYPES] = {'C', 'P', 'H', 'D'};
static const char *state_names[] = {"OFFLINE", "IDLE", "LOADING", "UP", "DOWN"};

struct pet {
    int type;
    int weight;
    int start_floor;
    int dest_floor;
    struct list_head list;
};

struct elevator {
    enum elevator_state state;
    int current_floor;
    int direction;                  /* +1 = up, -1 = down */
    int num_pets;
    int load_weight;
    int stopping;
    struct list_head pets;          /* pets on board */
    struct task_struct *kthread;
};

static struct elevator elevator;
static struct list_head floors[NUM_FLOORS];   /* FIFO waiting line per floor */
static int floor_waiting[NUM_FLOORS];
static int total_waiting;
static int total_serviced;
static DEFINE_MUTEX(elevator_mutex);
static struct proc_dir_entry *elevator_entry;

extern int (*STUB_start_elevator)(void);
extern int (*STUB_issue_request)(int, int, int);
extern int (*STUB_stop_elevator)(void);

/* ===== helpers: caller must hold elevator_mutex ===== */

static int has_unloads(int floor) {
    struct pet *p;
    list_for_each_entry(p, &elevator.pets, list)
        if (p->dest_floor == floor)
            return 1;
    return 0;
}

/* FIFO: only the pet at the front of the line may board */
static int can_board(int floor) {
    struct pet *p;
    if (elevator.stopping || list_empty(&floors[floor - 1]))
        return 0;
    if (elevator.num_pets >= MAX_PETS)
        return 0;
    p = list_first_entry(&floors[floor - 1], struct pet, list);
    return elevator.load_weight + p->weight <= MAX_WEIGHT;
}

static void unload_pets(int floor) {
    struct pet *p, *tmp;
    list_for_each_entry_safe(p, tmp, &elevator.pets, list) {
        if (p->dest_floor == floor) {
            list_del(&p->list);
            elevator.num_pets--;
            elevator.load_weight -= p->weight;
            total_serviced++;
            kfree(p);
        }
    }
}

static void load_pets(int floor) {
    struct pet *p;
    while (can_board(floor)) {
        p = list_first_entry(&floors[floor - 1], struct pet, list);
        list_move_tail(&p->list, &elevator.pets);
        elevator.num_pets++;
        elevator.load_weight += p->weight;
        floor_waiting[floor - 1]--;
        total_waiting--;
    }
}

/* ===== 3f: LOOK scheduler ===== */

static int floor_has_work(int floor) {
    if (has_unloads(floor))
        return 1;
    if (!elevator.stopping && floor_waiting[floor - 1] > 0 &&
        elevator.num_pets < MAX_PETS &&
        elevator.load_weight + MIN_PET_WEIGHT <= MAX_WEIGHT)
        return 1;
    return 0;
}

static int work_in_direction(int dir) {
    int f;
    for (f = elevator.current_floor + dir; f >= 1 && f <= NUM_FLOORS; f += dir)
        if (floor_has_work(f))
            return 1;
    return 0;
}

/* keep going while there is work ahead, otherwise reverse; 0 = nothing to do */
static int choose_direction(void) {
    if (work_in_direction(elevator.direction))
        return elevator.direction;
    if (work_in_direction(-elevator.direction))
        return -elevator.direction;
    return 0;
}

/* ===== 3c: elevator thread ===== */

static int elevator_run(void *data) {
    int dir;

    while (!kthread_should_stop()) {
        mutex_lock(&elevator_mutex);

        if (elevator.state == OFFLINE) {
            mutex_unlock(&elevator_mutex);
            msleep(100);
            continue;
        }

        /* unload and load at the current floor (1 second) */
        if (has_unloads(elevator.current_floor) || can_board(elevator.current_floor)) {
            elevator.state = LOADING;
            mutex_unlock(&elevator_mutex);
            ssleep(1);
            mutex_lock(&elevator_mutex);
            unload_pets(elevator.current_floor);
            load_pets(elevator.current_floor);
        }

        /* finish stopping once everyone has been delivered */
        if (elevator.stopping && elevator.num_pets == 0) {
            elevator.state = OFFLINE;
            elevator.stopping = 0;
            mutex_unlock(&elevator_mutex);
            continue;
        }

        dir = choose_direction();
        if (dir == 0) {
            elevator.state = IDLE;
            mutex_unlock(&elevator_mutex);
            msleep(100);
            continue;
        }

        /* move one floor (2 seconds) */
        elevator.direction = dir;
        elevator.state = (dir > 0) ? UP : DOWN;
        mutex_unlock(&elevator_mutex);
        ssleep(2);
        mutex_lock(&elevator_mutex);
        elevator.current_floor += dir;
        mutex_unlock(&elevator_mutex);
    }
    return 0;
}

/* ===== system call handlers ===== */

static int start_elevator(void) {
    mutex_lock(&elevator_mutex);
    if (elevator.state != OFFLINE) {
        mutex_unlock(&elevator_mutex);
        return 1;                   /* already active */
    }
    elevator.state = IDLE;
    elevator.stopping = 0;
    mutex_unlock(&elevator_mutex);
    return 0;
}

static int issue_request(int start_floor, int destination_floor, int type) {
    struct pet *p;

    if (start_floor < 1 || start_floor > NUM_FLOORS ||
        destination_floor < 1 || destination_floor > NUM_FLOORS ||
        start_floor == destination_floor ||
        type < 0 || type >= NUM_PET_TYPES)
        return 1;                   /* invalid request */

    p = kmalloc(sizeof(*p), GFP_KERNEL);
    if (!p)
        return -ENOMEM;

    p->type = type;
    p->weight = pet_weights[type];
    p->start_floor = start_floor;
    p->dest_floor = destination_floor;

    mutex_lock(&elevator_mutex);
    list_add_tail(&p->list, &floors[start_floor - 1]);
    floor_waiting[start_floor - 1]++;
    total_waiting++;
    mutex_unlock(&elevator_mutex);
    return 0;
}

static int stop_elevator(void) {
    mutex_lock(&elevator_mutex);
    if (elevator.state == OFFLINE || elevator.stopping) {
        mutex_unlock(&elevator_mutex);
        return 1;                   /* already stopped or stopping */
    }
    elevator.stopping = 1;
    mutex_unlock(&elevator_mutex);
    return 0;
}

/* ===== /proc/elevator ===== */

static ssize_t elevator_read(struct file *file, char __user *ubuf,
                             size_t count, loff_t *ppos) {
    char *buf;
    int len = 0, i;
    struct pet *p;
    ssize_t ret;

    buf = kmalloc(PROC_BUF_SIZE, GFP_KERNEL);
    if (!buf)
        return -ENOMEM;

    mutex_lock(&elevator_mutex);
    len += scnprintf(buf + len, PROC_BUF_SIZE - len, "Elevator state: %s\n",
                     state_names[elevator.state]);
    len += scnprintf(buf + len, PROC_BUF_SIZE - len, "Current floor: %d\n",
                     elevator.current_floor);
    len += scnprintf(buf + len, PROC_BUF_SIZE - len, "Current load: %d lbs\n",
                     elevator.load_weight);
    len += scnprintf(buf + len, PROC_BUF_SIZE - len, "Elevator status:");
    list_for_each_entry(p, &elevator.pets, list)
        len += scnprintf(buf + len, PROC_BUF_SIZE - len, " %c", pet_symbols[p->type]);
    len += scnprintf(buf + len, PROC_BUF_SIZE - len, "\n\n");

    for (i = NUM_FLOORS; i >= 1; i--) {
        len += scnprintf(buf + len, PROC_BUF_SIZE - len, "[%c] Floor %d: %d",
                         i == elevator.current_floor ? '*' : ' ', i, floor_waiting[i - 1]);
        list_for_each_entry(p, &floors[i - 1], list)
            len += scnprintf(buf + len, PROC_BUF_SIZE - len, " %c", pet_symbols[p->type]);
        len += scnprintf(buf + len, PROC_BUF_SIZE - len, "\n");
    }

    len += scnprintf(buf + len, PROC_BUF_SIZE - len, "\nNumber of pets: %d\n",
                     elevator.num_pets);
    len += scnprintf(buf + len, PROC_BUF_SIZE - len, "Number of pets waiting: %d\n",
                     total_waiting);
    len += scnprintf(buf + len, PROC_BUF_SIZE - len, "Number of pets serviced: %d\n",
                     total_serviced);
    mutex_unlock(&elevator_mutex);

    ret = simple_read_from_buffer(ubuf, count, ppos, buf, len);
    kfree(buf);
    return ret;
}

static const struct proc_ops elevator_fops = {
    .proc_read = elevator_read,
};

/* ===== init / exit ===== */

static void free_pet_list(struct list_head *head) {
    struct pet *p, *tmp;
    list_for_each_entry_safe(p, tmp, head, list) {
        list_del(&p->list);
        kfree(p);
    }
}

static int __init elevator_init(void) {
    int i;

    for (i = 0; i < NUM_FLOORS; i++) {
        INIT_LIST_HEAD(&floors[i]);
        floor_waiting[i] = 0;
    }
    INIT_LIST_HEAD(&elevator.pets);
    elevator.state = OFFLINE;
    elevator.current_floor = 1;
    elevator.direction = 1;
    elevator.num_pets = 0;
    elevator.load_weight = 0;
    elevator.stopping = 0;
    total_waiting = 0;
    total_serviced = 0;

    elevator_entry = proc_create(ENTRY_NAME, PERMS, PARENT, &elevator_fops);
    if (!elevator_entry)
        return -ENOMEM;

    elevator.kthread = kthread_run(elevator_run, NULL, "elevator_thread");
    if (IS_ERR(elevator.kthread)) {
        proc_remove(elevator_entry);
        return PTR_ERR(elevator.kthread);
    }

    STUB_start_elevator = start_elevator;
    STUB_issue_request = issue_request;
    STUB_stop_elevator = stop_elevator;
    return 0;
}

static void __exit elevator_exit(void) {
    int i;

    STUB_start_elevator = NULL;
    STUB_issue_request = NULL;
    STUB_stop_elevator = NULL;

    kthread_stop(elevator.kthread);
    proc_remove(elevator_entry);

    mutex_lock(&elevator_mutex);
    for (i = 0; i < NUM_FLOORS; i++)
        free_pet_list(&floors[i]);
    free_pet_list(&elevator.pets);
    mutex_unlock(&elevator_mutex);
}

module_init(elevator_init);
module_exit(elevator_exit);
