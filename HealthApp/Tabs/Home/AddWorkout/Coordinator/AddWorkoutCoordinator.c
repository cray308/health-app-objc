//
//  AddWorkoutCoordinator.c
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#include "AddWorkoutCoordinator.h"
#include <CoreFoundation/CFString.h>
#include <objc/message.h>
#include "PersistenceService.h"
#include "ViewControllerHelpers.h"
#include "WorkoutScreenHelpers.h"

extern id workoutVC_init(void *delegate);
extern id updateMaxesVC_init(void *delegate);
extern void homeCoordinator_didFinishAddingWorkout(void *, int);
extern void appCoordinator_updateMaxWeights(void);
extern void appUserData_updateWeightMaxes(short *weights);
extern byte appUserData_addCompletedWorkout(byte);

static void cleanupTimers(Workout *w) {
    if (w->timers[TimerGroup].info.active == 1)
        pthread_kill(w->threads[TimerGroup], SignalGroup);
    if (w->timers[TimerExercise].info.active == 1)
        pthread_kill(w->threads[TimerExercise], SignalExercise);
    startWorkoutTimer(&w->timers[TimerGroup], 0);
    startWorkoutTimer(&w->timers[TimerExercise], 0);
    pthread_join(w->threads[1], NULL);
    pthread_join(w->threads[0], NULL);
    signal(SIGUSR1, SIG_DFL);
    signal(SIGUSR2, SIG_DFL);
    for (int i = 0; i < 2; ++i) {
        pthread_cond_destroy(&w->timers[i].cond);
        pthread_mutex_destroy(&w->timers[i].lock);
    }
    pthread_mutex_destroy(&timerLock);
}

static void cleanupCoordinator(AddWorkoutCoordinator *this) {
    Workout *w = this->workout;
    CFRelease(w->title);
    array_free(circuit, w->activities);
    if (w->newLifts)
        free(w->newLifts);
    free(w);
    free(this);
}

static void updateStoredData(AddWorkoutCoordinator *this) {
    cleanupTimers(this->workout);
    AddWorkoutCoordinator *coordinator = this;

    runInBackground((^{
        Workout *w = this->workout;
        if (w->duration >= 15) {
            id data = persistenceService_getCurrentWeek();

            int16_t duration = w->duration + weekData_getWorkoutTimeForType(data, w->type);
            weekData_setWorkoutTimeForType(data, w->type, duration);
            weekData_setTotalWorkouts(data, weekData_getTotalWorkouts(data) + 1);
            if (w->newLifts) {
                weekData_setLiftingMaxArray(data, w->newLifts);
            }
            persistenceService_saveContext();
        }
        cleanupCoordinator(coordinator);
    }));
}

void addWorkoutCoordinator_start(AddWorkoutCoordinator *this) {
    pthread_mutex_init(&timerLock, NULL);
    id vc = workoutVC_init(this);
    ((void(*)(id,SEL,id,bool))objc_msgSend)(this->navVC,
                                            sel_getUid("pushViewController:animated:"), vc, true);
    releaseObj(vc);
}

void addWorkoutCoordinator_stoppedWorkout(AddWorkoutCoordinator *this) {
    void *parent = this->parent;
    updateStoredData(this);
    homeCoordinator_didFinishAddingWorkout(parent, 0);
}

void addWorkoutCoordinator_completedWorkout(AddWorkoutCoordinator *this,
                                            bool dismissVC, bool showModalIfRequired) {
    CFStringRef title = this->workout->title;
    if (showModalIfRequired &&
        CFStringCompareWithOptions(title, localize(CFSTR("workoutTitleTestDay")),
                                   CFRangeMake(0, CFStringGetLength(title)),
                                   kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
        presentModalVC(this->navVC, updateMaxesVC_init(this));
        return;
    }

    const signed char day = this->workout->day;
    byte totalCompleted = 0;
    bool longEnough = this->workout->duration >= 15;
    if (longEnough) {
        if (day >= 0)
            totalCompleted = appUserData_addCompletedWorkout(day);
        if (this->workout->newLifts)
            appUserData_updateWeightMaxes(this->workout->newLifts);
    }

    id navVC = this->navVC;
    void *parent = this->parent;
    updateStoredData(this);
    if (dismissVC) {
        dismissPresentedVC(navVC, ^{
            appCoordinator_updateMaxWeights();
            homeCoordinator_didFinishAddingWorkout(parent, totalCompleted);
        });
    } else {
        homeCoordinator_didFinishAddingWorkout(parent, totalCompleted);
    }
}

void addWorkoutCoordinator_stopWorkoutFromBackButtonPress(AddWorkoutCoordinator *this) {
    if (this->workout->startTime) {
        workout_setDuration(this->workout);
        updateStoredData(this);
    } else {
        cleanupTimers(this->workout);
        cleanupCoordinator(this);
    }
}
