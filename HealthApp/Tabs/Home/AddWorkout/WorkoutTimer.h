#ifndef WorkoutTimer_h
#define WorkoutTimer_h

#include <time.h>

typedef struct {
    time_t refTime;
    int identifier;
    int container;
    int exercise;
    int duration;
    const unsigned char type;
    unsigned char active;
} WorkoutTimer;

#endif /* WorkoutTimer_h */
