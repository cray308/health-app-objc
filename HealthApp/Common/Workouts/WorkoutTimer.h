//
//  WorkoutTimer.h
//  HealthApp
//
//  Created by Christopher Ray on 10/4/21.
//

#ifndef WorkoutTimer_h
#define WorkoutTimer_h

#include <pthread.h>
#include "CocoaHelpers.h"

enum {
    SignalGroup = SIGUSR2,
    SignalExercise = SIGUSR1
};

enum {
    TimerGroup,
    TimerExercise
};

typedef struct {
    id parent;
    struct _TimerInfo {
        const unsigned char type : 2;
        unsigned char active : 2;
        unsigned char stop : 4;
    } info;
    unsigned container;
    unsigned exercise;
    int duration;
    time_t refTime;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} WorkoutTimer;

extern const unsigned ExerciseTagNA;

void scheduleNotification(int secondsFromNow, int type);
void startWorkoutTimer(WorkoutTimer *t, int duration);

#endif /* WorkoutTimer_h */
