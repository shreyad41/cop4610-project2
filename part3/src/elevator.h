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
    ELEVATOR_MOVING,
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