//
//  WorkoutScreenHelpers.c
//  HealthApp
//
//  Created by Christopher Ray on 8/22/21.
//

#include "WorkoutScreenHelpers.h"
#include <unistd.h>
#include <dispatch/queue.h>
#include <objc/message.h>
#include "ViewControllerHelpers.h"

extern void workoutVC_finishedTimer(id, TimerType, unsigned, unsigned);

extern id UIApplicationDidBecomeActiveNotification;
extern id UIApplicationWillResignActiveNotification;

pthread_mutex_t timerLock;
static pthread_t *exerciseTimerThread;

static void handle_exercise_timer_interrupt(int n _U_) {}
static void handle_group_timer_interrupt(int n _U_) {}

static void *timer_loop(void *arg) {
    WorkoutTimer *t = arg;
    int res = 0, duration = 0;
    unsigned container = ExerciseTagNA, exercise = ExerciseTagNA;
    while (!t->info.stop) {
        pthread_mutex_lock(&t->lock);
        while (t->info.active != 1)
            pthread_cond_wait(&t->cond, &t->lock);
        duration = t->duration;
        container = t->container;
        exercise = t->exercise;
        pthread_mutex_unlock(&t->lock);

        if (!duration) continue;
        res = sleep(duration);
        t->info.active = 2;
        if (!res) {
            id parent = t->parent;
            if (t->info.type == 0 && t[1].info.active == 1)
                pthread_kill(*exerciseTimerThread, SignalExercise);
            dispatch_async(dispatch_get_main_queue(), ^(void) {
                workoutVC_finishedTimer(parent, t->info.type, container, exercise);
            });
        }
    }
    return NULL;
}

static id getDeviceNotificationCenter(void) {
    return staticMethod(objc_getClass("NSNotificationCenter"), sel_getUid("defaultCenter"));
}

static id createObserver(id center, id name, id queue, ObjectBlock block) {
    return ((id(*)(id,SEL,CFStringRef,id,id,void(^)(id)))objc_msgSend)
    (center, sel_getUid("addObserverForName:object:queue:usingBlock:"),
     (CFStringRef)name, nil, queue, block);
}

void setupTimers(Workout *w, id parent) {
    struct sigaction sa = {.sa_flags = 0, .sa_handler = handle_exercise_timer_interrupt};
    sigemptyset(&sa.sa_mask);
    sigaction(SignalExercise, &sa, NULL);
    sa.sa_handler = handle_group_timer_interrupt;
    sigaction(SignalGroup, &sa, NULL);
    exerciseTimerThread = &w->threads[TimerExercise];
    w->timers[0].parent = w->timers[1].parent = parent;
    pthread_create(&w->threads[1], NULL, timer_loop, &w->timers[1]);
    pthread_create(&w->threads[0], NULL, timer_loop, &w->timers[0]);
}

void setupDeviceEventNotifications(id *observers, ObjectBlock active, ObjectBlock resign) {
    id main = staticMethod(objc_getClass("NSOperationQueue"), sel_getUid("mainQueue"));
    id center = getDeviceNotificationCenter();
    observers[0] = createObserver(center, UIApplicationDidBecomeActiveNotification, main, active);
    observers[1] = createObserver(center, UIApplicationWillResignActiveNotification, main, resign);
}

void cleanupWorkoutNotifications(id *observers) {
    id center = getDeviceNotificationCenter();
    for (int i = 0; i < 2; ++i)
        setObject(center, sel_getUid("removeObserver:"), observers[i]);
    center = getNotificationCenter();
    voidFunc(center, sel_getUid("removeAllPendingNotificationRequests"));
    voidFunc(center, sel_getUid("removeAllDeliveredNotifications"));
    exerciseTimerThread = NULL;
}
