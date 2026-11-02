#ifndef PET_ELEVATOR_H
#define PET_ELEVATOR_H

#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/types.h>

#define ELEVATOR_MIN_FLOOR 1
#define ELEVATOR_MAX_FLOOR 5
#define ELEVATOR_MAX_PETS 5
#define ELEVATOR_MAX_WEIGHT 50
#define ELEVATOR_TRAVEL_MS 2000
#define ELEVATOR_LOADING_MS 1000

enum elevator_state
{
    ELEVATOR_OFFLINE,
    ELEVATOR_IDLE,
    ELEVATOR_LOADING,
    ELEVATOR_UP,
    ELEVATOR_DOWN,
};

enum pet_type
{
    PET_CHIHUAHUA,
    PET_PUG,
    PET_PUGHUAHUA,
    PET_DACKSHUND,
};

struct pet
{
    struct list_head node;
    int start_floor;
    int destination_floor;
    enum pet_type type;
    int weight;
};

struct elevator
{
    struct mutex lock;
    enum elevator_state state;
    bool is_moving;
    bool is_stopping;
    int current_floor;
    int current_weight;
    int onboard_pet_count;  // in elevator
    int waiting_pet_count;  // waiting on a floor
    int serviced_pet_count; // pets that have been serviced
    struct list_head pets_onboard;
    struct list_head waiting[ELEVATOR_MAX_FLOOR];
}