//
//  AddWorkoutCoordinator.c
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#include "AddWorkoutCoordinator.h"
#include "AppUserData.h"
#include "PersistenceService.h"
#include "ViewControllerHelpers.h"
#include "WorkoutScreenHelpers.h"

extern id workoutVC_init(void *delegate);
extern id updateMaxesVC_init(void *delegate);
extern void homeCoordinator_didFinishAddingWorkout(void *, int);
extern void appCoordinator_updateMaxWeights(void);
extern unsigned char appUserData_addCompletedWorkout(unsigned char);

static void updateStoredData(AddWorkoutCoordinator *this) {
    if (this->workout->timers[TimerGroup].info.active == 1)
        pthread_kill(this->workout->threads[TimerGroup], SignalGroup);
    if (this->workout->timers[TimerExercise].info.active == 1)
        pthread_kill(this->workout->threads[TimerExercise], SignalExercise);
    startWorkoutTimer(&this->workout->timers[TimerGroup], 0);
    startWorkoutTimer(&this->workout->timers[TimerExercise], 0);
    pthread_join(this->workout->threads[1], NULL);
    pthread_join(this->workout->threads[0], NULL);
    signal(SIGUSR1, SIG_DFL);
    signal(SIGUSR2, SIG_DFL);
    for (int i = 0; i < 2; ++i) {
        pthread_cond_destroy(&this->workout->timers[i].cond);
        pthread_mutex_destroy(&this->workout->timers[i].lock);
    }
    pthread_mutex_destroy(&timerLock);
    array_free(circuit, this->workout->activities);
    CFRelease(this->workout->title);

    runInBackground((^{
        Workout *w = this->workout;
        if (w->duration >= 15) {
            id data = persistenceService_getCurrentWeek();

            int16_t duration = w->duration + weekData_getWorkoutTimeForType(data, w->type);
            weekData_setWorkoutTimeForType(data, w->type, duration);
            weekData_setTotalWorkouts(data, weekData_getTotalWorkouts(data) + 1);
            if (w->newLifts) {
                for (int i = 0; i < 4; ++i)
                    weekData_setLiftingMaxForType(data, i, w->newLifts[i]);
            }
            persistenceService_saveContext();
        }
        if (w->newLifts)
            free(w->newLifts);
        free(w);
        free(this);
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

    const byte day = this->workout->day;
    ubyte totalCompleted = 0;
    bool longEnough = this->workout->duration >= 15;
    if (longEnough) {
        if (day >= 0)
            totalCompleted = appUserData_addCompletedWorkout((ubyte) day);
        if (this->workout->newLifts)
            appUserData_updateWeightMaxes(this->workout->newLifts);
    }

    if (dismissVC) {
        dismissPresentedVC(this->navVC);
        appCoordinator_updateMaxWeights();
    }
    void *parent = this->parent;
    updateStoredData(this);
    homeCoordinator_didFinishAddingWorkout(parent, totalCompleted);
}

void addWorkoutCoordinator_stopWorkoutFromBackButtonPress(AddWorkoutCoordinator *this) {
    if (this->workout->startTime) {
        workout_setDuration(this->workout);
        updateStoredData(this);
    }
}
