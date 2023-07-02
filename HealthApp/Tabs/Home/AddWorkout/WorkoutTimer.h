#ifndef WorkoutTimer_h
#define WorkoutTimer_h

#include <sys/types.h>

typedef struct {
    time_t refTime;
    int identifier;
    int section;
    int row;
    int duration;
    uint8_t type;
    uint8_t active;
} WorkoutTimer;

#endif /* WorkoutTimer_h */
