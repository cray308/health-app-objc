//
//  WorkoutScreenHelpers.h
//  HealthApp
//
//  Created by Christopher Ray on 8/22/21.
//

#ifndef WorkoutScreenHelpers_h
#define WorkoutScreenHelpers_h

#include "CocoaHelpers.h"
#include <pthread.h>

typedef enum {
    TimerTypeGroup,
    TimerTypeExercise
} TimerType;

typedef enum {
    WorkoutNotificationExerciseCompleted,
    WorkoutNotificationAMRAPCompleted
} WorkoutNotification;

typedef struct {
    id parent;
    struct info {
        const unsigned char type : 2;
        unsigned char active : 2;
        unsigned char stop : 4;
    } info;
    int container;
    int exercise;
    int duration;
    time_t refTime;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} WorkoutTimer;

extern pthread_mutex_t timerLock;

void scheduleNotification(int secondsFromNow, CFStringRef message);
void startWorkoutTimer(WorkoutTimer *t, int duration, int container, int exercise);

#endif /* WorkoutScreenHelpers_h */
