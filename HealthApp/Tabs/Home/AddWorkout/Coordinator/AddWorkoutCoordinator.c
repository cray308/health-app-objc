//
//  AddWorkoutCoordinator.c
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#include "AddWorkoutCoordinator.h"
#include "HomeTabCoordinator.h"
#include "AppCoordinator.h"
#include "AppUserData.h"
#include "PersistenceService.h"
#include "ViewControllerHelpers.h"
#include "WorkoutScreenHelpers.h"

void updateStoredData(Workout *w) {
    int16_t duration = (int16_t) ((w->stopTime - w->startTime) / 60.0);
    id data = persistenceService_getWeeklyDataForThisWeek();
    if (!(duration >= 15 && data)) return;

    duration += weekData_getWorkoutTimeForType(data, w->type);
    weekData_setWorkoutTimeForType(data, w->type, duration);
    weekData_setTotalWorkouts(data, weekData_getTotalWorkouts(data) + 1);
    persistenceService_saveContext();
}

void didFinishAddingWorkout(AddWorkoutCoordinator *this,
                            bool dismissVC, int totalCompletedWorkouts) {
    if (dismissVC) {
        dismissPresentedVC(this->navVC);
        appCoordinator_updateMaxWeights(appCoordinatorShared);
    }
    homeCoordinator_didFinishAddingWorkout(this->parent, totalCompletedWorkouts);
}

void addWorkoutCoordinator_start(AddWorkoutCoordinator *this) {
    pthread_mutex_init(&timerLock, NULL);
    id vc = createVCWithDelegate("WorkoutViewController", this);
    ((void(*)(id,SEL,id,bool))objc_msgSend)(this->navVC,
                                            sel_getUid("pushViewController:animated:"), vc, true);
    releaseObj(vc);
}

void addWorkoutCoordinator_free(AddWorkoutCoordinator *this) {
    if (this->workout->timers[TimerTypeGroup].info.active == 1)
        pthread_kill(this->workout->threads[TimerTypeGroup], TimerSignalGroup);
    if (this->workout->timers[TimerTypeExercise].info.active == 1)
        pthread_kill(this->workout->threads[TimerTypeExercise], TimerSignalExercise);
    startWorkoutTimer(&this->workout->timers[TimerTypeGroup], 0, 0, 255);
    startWorkoutTimer(&this->workout->timers[TimerTypeExercise], 0, 255, 0);
    pthread_join(this->workout->threads[1], NULL);
    pthread_join(this->workout->threads[0], NULL);
    signal(SIGUSR1, SIG_DFL);
    signal(SIGUSR2, SIG_DFL);
    for (int i = 0; i < 2; ++i) {
        pthread_cond_destroy(&this->workout->timers[i].cond);
        pthread_mutex_destroy(&this->workout->timers[i].lock);
    }
    pthread_mutex_destroy(&timerLock);
    array_free(exGroup, this->workout->activities);
    CFRelease(this->workout->title);
    free(this->workout);
    free(this);
}

void addWorkoutCoordinator_stoppedWorkout(AddWorkoutCoordinator *this) {
    updateStoredData(this->workout);
    didFinishAddingWorkout(this, false, 0);
}

void addWorkoutCoordinator_completedWorkout(AddWorkoutCoordinator *this,
                                            bool dismissVC, bool showModalIfRequired) {
    CFStringRef title = this->workout->title;
    if (showModalIfRequired &&
        CFStringCompareWithOptions(title, CFSTR("test day"),
                                   CFRangeMake(0, CFStringGetLength(title)),
                                   kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
        id modal = createVCWithDelegate("AddWorkoutUpdateMaxesViewController", this);
        presentModalVC(this->navVC, modal);
        return;
    }

    updateStoredData(this->workout);
    const signed char day = this->workout->day;
    if (day >= 0) {
        unsigned char totalCompleted = appUserData_addCompletedWorkout((unsigned char) day);
        didFinishAddingWorkout(this, dismissVC, totalCompleted);
        return;
    }
    didFinishAddingWorkout(this, dismissVC, 0);
}

void addWorkoutCoordinator_finishedAddingNewWeights(AddWorkoutCoordinator *this, short *weights) {
    id data = persistenceService_getWeeklyDataForThisWeek();
    if (data) {
        for (int i = 0; i < 4; ++i)
            weekData_setLiftingMaxForType(data, i, weights[i]);
        persistenceService_saveContext();
    }
    appUserData_updateWeightMaxes(weights);
    addWorkoutCoordinator_completedWorkout(this, true, false);
}

void addWorkoutCoordinator_stopWorkoutFromBackButtonPress(AddWorkoutCoordinator *this) {
    if (this->workout->startTime) {
        this->workout->stopTime = time(NULL) + 1;
        updateStoredData(this->workout);
    }
}
