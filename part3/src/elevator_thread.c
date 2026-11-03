#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/math.h>
#include "elevator.h"

static struct task_struct *elevator_kthread;
static void sleep_unlocked(unsigned int secs)
{
    mutex_unlock(&elevator_mutex);
    ssleep(secs);
    mutex_lock(&elevator_mutex);
}

static bool has_pets_to_unload(void)
{
    struct pet *pet;

    list_for_each_entry(pet, &elevator.pets, list)
    {
        if (pet->dest_floor == elevator.current_floor)
            return true;
    }
    return false;
}

static bool can_load_pet(void)
{
    struct list_head *queue = &elevator.floors[elevator.current_floor - 1];
    struct pet *pet;

    if (elevator.stopping || elevator.num_pets >= ELEVATOR_MAX_PETS || list_empty(queue))
        return false;

    pet = list_first_entry(queue, struct pet, list);

    return (elevator.load_weight + pet->weight <= ELEVATOR_MAX_WEIGHT);
}

static void unload_pets(void)
{
    struct pet *pet, *next;

    list_for_each_entry_safe(pet, next, &elevator.pets, list)
    {
        if (pet->dest_floor != elevator.current_floor)
            continue;
        
        list_del(&pet->list);
        elevator.num_pets--;
        elevator.load_weight -= pet->weight;
        elevator.num_serviced++;
        kfree(pet);
    }
}

static void load_pets(void)
{
    struct list_head *queue = &elevator.floors[elevator.current_floor - 1];
    struct pet *pet;

    while (can_load_pet())
    {
        pet = list_first_entry(queue, struct pet, list);
        list_move_tail(&pet->list, &elevator.pets);
        elevator.num_waiting--;
        elevator.num_pets++;
        elevator.load_weight += pet->weight;
    }
}

static int next_direction(void)
{
    struct pet *pet;
    int current_floor = elevator.current_floor;
    int target = current_floor;
    int best_distance = NUM_FLOORS;
    int floor, distance;

    list_for_each_entry(pet, &elevator.pets, list) 
    {
        distance = abs(pet->dest_floor - current_floor);
        if (distance > 0 && distance < best_distance) {
            best_distance = distance;
            target = pet->dest_floor;
        }
    }

    if (elevator.num_pets == 0 && !elevator.stopping) 
    {
        for (floor = 1; floor <= NUM_FLOORS; floor++) 
        {
            if (floor == current_floor ||
                list_empty(&elevator.floors[floor - 1]))
                continue;

            distance = abs(floor - current_floor);
            if (distance < best_distance) 
            {
                best_distance = distance;
                target = floor;
            }
        }
    }

    if (target > current_floor)
        return 1;
    if (target < current_floor)
        return -1;
    return 0;
}

static int elevator_run(void *data)
{
    int direction;
    bool rest;

    while (!kthread_should_stop())
    {
        rest = false;
        mutex_lock(&elevator_mutex);

        if (elevator.state == OFFLINE)
        {
            rest = true;
        }

        else if (has_pets_to_unload() || (!elevator.stopping && can_load_pet()))
        {
            elevator.state = LOADING;
            sleep_unlocked(1);
            unload_pets();
            if (!elevator.stopping)
                load_pets();
        }

        else if (elevator.stopping && elevator.num_pets == 0)
        {
            elevator.state = OFFLINE;
            elevator.stopping = false;
        }

        else
        {
            direction = next_direction();
            
            if (direction > 0 && elevator.current_floor < NUM_FLOORS)
            {
                elevator.state = UP;
                sleep_unlocked(2);
                elevator.current_floor++;
            }
            
            else if (direction < 0 && elevator.current_floor > 1)
            {
                elevator.state = DOWN;
                sleep_unlocked(2);
                elevator.current_floor--;
            }
            
            else
            {
                elevator.state = IDLE;
                rest = true;
            }
        }

        mutex_unlock(&elevator_mutex);
        if (rest)
            msleep(100);
    }
    return 0;
}

int elevator_thread_start(void)
{
    elevator_kthread = kthread_run(elevator_run, NULL, "elevator_thread");
    if (IS_ERR(elevator_kthread))
        return PTR_ERR(elevator_kthread);
    return 0;
}

void elevator_thread_stop(void)
{
    kthread_stop(elevator_kthread);
}