#ifndef PET_ELEVATOR_H
#define PET_ELEVATOR_H

#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/types.h>

#define ELEVATOR_MIN_FLOOR 1
#define NUM_FLOORS 5
#define ELEVATOR_MAX_PETS 5
#define ELEVATOR_MAX_WEIGHT 50
#define ELEVATOR_TRAVEL_MS 2000
#define ELEVATOR_LOADING_MS 1000

enum elevator_state
{
    OFFLINE,
    IDLE,
    LOADING,
    UP,
    DOWN,
};

enum pet_type
{
    PET_CHIHUAHUA = 0,
    PET_PUG = 1,
    PET_PUGHUAHUA = 2,
    PET_DACHSHUND = 3,
};

struct pet
{
    struct list_head list;
    int start_floor;
    int dest_floor;
    enum pet_type type;
    int weight;
};

struct elevator
{
    struct mutex lock;
    enum elevator_state state;
    bool is_active;
    bool stopping;
    int current_floor;
    int load_weight;
    int num_pets;     // in elevator
    int num_waiting;  // alls pets waiting on all floor
    int num_serviced; // pets that have been serviced
    struct list_head pets;
    struct list_head floors[NUM_FLOORS];
};

extern struct elevator elevator;
extern struct mutex elevator_mutex;

int start_elevator(void);
int issue_request(int start_floor, int destination_floor, int type);
int stop_elevator(void);

int elevator_thread_start(void);
void elevator_thread_stop(void);

#endif /* PET_ELEVATOR_H */