//
//  WorkoutTimer.h
//  HealthApp
//
//  Created by Christopher Ray on 10/4/21.
//

#ifndef WorkoutTimer_h
#define WorkoutTimer_h

#include <pthread.h>
#include <objc/objc.h>
#include "AppTypes.h"

enum {
    TimerGroup,
    TimerExercise
};

typedef struct {
    id parent;
    struct _TimerInfo {
        const byte type : 2;
        byte active : 2;
        byte stop : 4;
    } info;
    unsigned container;
    unsigned exercise;
    int duration;
    time_t refTime;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} WorkoutTimer;

void scheduleNotification(int secondsFromNow, byte type);
void startWorkoutTimer(WorkoutTimer *t, int duration);

#endif /* WorkoutTimer_h */
