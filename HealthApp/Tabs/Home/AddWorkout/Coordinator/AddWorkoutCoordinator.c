//
//  AddWorkoutCoordinator.c
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "AddWorkoutCoordinator.h"
#import "HomeTabCoordinator.h"
#include "AppCoordinator.h"
#include "AppUserData.h"
#import "PersistenceService.h"

void updateStoredData(AddWorkoutViewModel *model) {
    int16_t duration = (int16_t) ((model->stopTime - model->startTime) / 60.0);
    id data = persistenceService_getWeeklyDataForThisWeek();
    if (!(duration >= 15 && data)) return;

    addToWorkoutType(data, model->workout->type, duration);
    setWeekData(data, "setTotalWorkouts:", getTotalWorkouts(data) + 1);
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
    id vc = createVCWithDelegate("WorkoutViewController", this);
    ((void(*)(id,SEL,id,bool))objc_msgSend)(this->navVC,
                                            sel_getUid("pushViewController:animated:"), vc, true);
    releaseObj(vc);
}

void addWorkoutCoordinator_free(AddWorkoutCoordinator *this) {
    array_free(exGroup, this->viewModel.workout->activities);
    CFRelease(this->viewModel.workout->title);
    free(this->viewModel.workout);
    free(this);
}

void addWorkoutCoordinator_stoppedWorkout(AddWorkoutCoordinator *this) {
    updateStoredData(&this->viewModel);
    didFinishAddingWorkout(this, false, 0);
}

void addWorkoutCoordinator_completedWorkout(AddWorkoutCoordinator *this,
                                            bool dismissVC, bool showModalIfRequired) {
    CFStringRef title = this->viewModel.workout->title;
    if (showModalIfRequired &&
        CFStringCompareWithOptions(title, CFSTR("test day"),
                                   CFRangeMake(0, CFStringGetLength(title)),
                                   kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
        id modal = createVCWithDelegate("AddWorkoutUpdateMaxesViewController", this);
        presentVC(this->navVC, modal);
        return;
    }

    updateStoredData(&this->viewModel);
    const signed char day = this->viewModel.workout->day;
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
        setLiftingMaxes(data, weights);
        persistenceService_saveContext();
    }
    appUserData_updateWeightMaxes(weights);
    addWorkoutCoordinator_completedWorkout(this, true, false);
}

void addWorkoutCoordinator_stopWorkoutFromBackButtonPress(AddWorkoutCoordinator *this) {
    if (this->viewModel.startTime) {
        this->viewModel.stopTime = time(NULL) + 1;
        updateStoredData(&this->viewModel);
    }
}
