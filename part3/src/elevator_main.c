#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include "elevator.h"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Pet elevator kernel module");

#define ENTRY_NAME "elevator"
#define PERMS 0644
#define PROC_BUF_SIZE 10000

extern int (*STUB_start_elevator)(void);
extern int (*STUB_issue_request)(int, int, int);
extern int (*STUB_stop_elevator)(void);

struct elevator elevator;
DEFINE_MUTEX(elevator_mutex);

static struct proc_dir_entry *elevator_entry;

static const char *state_names[] = {"OFFLINE", "IDLE", "LOADING", "UP", "DOWN"};
static const char pet_chars[] = {'C', 'P', 'H', 'D'};

static ssize_t elevator_read(struct file *file, char __user *ubuf,
                             size_t count, loff_t *ppos)
{
    char *buf;
    int len = 0;
    int floor, waiting_here;
    ssize_t ret;
    struct pet *p;

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
        len += scnprintf(buf + len, PROC_BUF_SIZE - len, " %c%d",
                         pet_chars[p->type], p->dest_floor);
    len += scnprintf(buf + len, PROC_BUF_SIZE - len, "\n\n");

    for (floor = NUM_FLOORS; floor >= 1; floor--)
    {
        waiting_here = 0;
        list_for_each_entry(p, &elevator.floors[floor - 1], list)
            waiting_here++;

        len += scnprintf(buf + len, PROC_BUF_SIZE - len, "[%c] Floor %d: %d",
                         elevator.current_floor == floor ? '*' : ' ',
                         floor, waiting_here);
        list_for_each_entry(p, &elevator.floors[floor - 1], list)
            len += scnprintf(buf + len, PROC_BUF_SIZE - len, " %c%d",
                             pet_chars[p->type], p->dest_floor);
        len += scnprintf(buf + len, PROC_BUF_SIZE - len, "\n");
    }

    len += scnprintf(buf + len, PROC_BUF_SIZE - len, "\nNumber of pets: %d\n",
                     elevator.num_pets);
    len += scnprintf(buf + len, PROC_BUF_SIZE - len, "Number of pets waiting: %d\n",
                     elevator.num_waiting);
    len += scnprintf(buf + len, PROC_BUF_SIZE - len, "Number of pets serviced: %d\n",
                     elevator.num_serviced);

    mutex_unlock(&elevator_mutex);

    ret = simple_read_from_buffer(ubuf, count, ppos, buf, len);
    kfree(buf);
    return ret;
}

static const struct proc_ops elevator_fops = {
    .proc_read = elevator_read,
};

static int __init elevator_init(void)
{
    int i, ret;

    elevator.state = OFFLINE;
    elevator.current_floor = 1;
    INIT_LIST_HEAD(&elevator.pets);
    for (i = 0; i < NUM_FLOORS; i++)
        INIT_LIST_HEAD(&elevator.floors[i]);

    elevator_entry = proc_create(ENTRY_NAME, PERMS, NULL, &elevator_fops);
    if (!elevator_entry)
        return -ENOMEM;

    ret = elevator_thread_start();
    if (ret)
    {
        proc_remove(elevator_entry);
        return ret;
    }

    STUB_start_elevator = start_elevator;
    STUB_issue_request = issue_request;
    STUB_stop_elevator = stop_elevator;
    return 0;
}

static void free_all_pets(void)
{
    struct pet *pet, *next;
    int floor;

    list_for_each_entry_safe(pet, next, &elevator.pets, list)
    {
        list_del(&pet->list);
        kfree(pet);
    }

    for (floor = 0; floor < NUM_FLOORS; floor++)
    {
        list_for_each_entry_safe(pet, next, &elevator.floors[floor], list)
        {
            list_del(&pet->list);
            kfree(pet);
        }
    }

    elevator.num_pets = 0;
    elevator.num_waiting = 0;
    elevator.load_weight = 0;
}

static void __exit elevator_exit(void)
{
    STUB_start_elevator = NULL;
    STUB_issue_request = NULL;
    STUB_stop_elevator = NULL;

    elevator_thread_stop();
    proc_remove(elevator_entry);

    mutex_lock(&elevator_mutex);
    free_all_pets();
    mutex_unlock(&elevator_mutex);
}

module_init(elevator_init);
module_exit(elevator_exit);