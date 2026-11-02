#include <linux/slab.h>
#include <linux/errno.h>
#include "elevator.h"

static const int pet_weights[] = { 3, 14, 10, 16 };

int start_elevator(void)
{
    mutex_lock(&elevator_mutex);

    if (elevator.state != OFFLINE && !elevator.stopping) {
        mutex_unlock(&elevator_mutex);
        return 1;
    }

    if (elevator.state == OFFLINE)
        elevator.state = IDLE;
    elevator.stopping = false;

    mutex_unlock(&elevator_mutex);
    return 0;
}

int issue_request(int start_floor, int destination_floor, int type)
{
    struct pet *new_pet;

    if (start_floor < 1 || start_floor > NUM_FLOORS ||
        destination_floor < 1 || destination_floor > NUM_FLOORS ||
        start_floor == destination_floor || type < 0 || type > 3)
        return 1;
    new_pet = kmalloc(sizeof(*new_pet), GFP_KERNEL);
    if (!new_pet)
        return -ENOMEM;

    new_pet->type = type;
    new_pet->weight = pet_weights[type];
    new_pet->start_floor = start_floor;
    new_pet->dest_floor = destination_floor;

    mutex_lock(&elevator_mutex);
    list_add_tail(&new_pet->list, &elevator.floors[start_floor - 1]);
    elevator.num_waiting++;
    mutex_unlock(&elevator_mutex);

    return 0;
}


int stop_elevator(void)
{
    mutex_lock(&elevator_mutex);

    if (elevator.stopping) {
        mutex_unlock(&elevator_mutex);
        return 1;
    }
    if (elevator.state != OFFLINE)
        elevator.stopping = true; 
    
    mutex_unlock(&elevator_mutex);
    return 0;
}