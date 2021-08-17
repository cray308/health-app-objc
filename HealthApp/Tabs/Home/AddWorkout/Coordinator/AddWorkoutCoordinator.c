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

    switch (model->workout->type) {
        case WorkoutTypeSE:
            duration += ((int16_t (*)(id, SEL)) objc_msgSend)(data, sel_getUid("timeSE"));
            ((void (*)(id, SEL, int16_t)) objc_msgSend)(data, sel_getUid("setTimeSE:"), duration);
            break;
        case WorkoutTypeHIC:
            duration += ((int16_t (*)(id, SEL)) objc_msgSend)(data, sel_getUid("timeHIC"));
            ((void (*)(id, SEL, int16_t)) objc_msgSend)(data, sel_getUid("setTimeHIC:"), duration);
            break;
        case WorkoutTypeStrength:
            duration += ((int16_t (*)(id, SEL)) objc_msgSend)(data, sel_getUid("timeStrength"));
            ((void (*)(id, SEL, int16_t)) objc_msgSend)(data, sel_getUid("setTimeStrength:"),
                                                        duration);
            break;
        default:
            duration += ((int16_t (*)(id, SEL)) objc_msgSend)(data, sel_getUid("timeEndurance"));
            ((void (*)(id, SEL, int16_t)) objc_msgSend)(data, sel_getUid("setTimeEndurance:"),
                                                        duration);
            break;
    }
    int16_t total = ((int16_t (*)(id, SEL)) objc_msgSend)(data, sel_getUid("totalWorkouts")) + 1;
    ((void (*)(id, SEL, int16_t)) objc_msgSend)(data, sel_getUid("setTotalWorkouts:"), total);
    persistenceService_saveContext();
}

void didFinishAddingWorkout(AddWorkoutCoordinator *this,
                            bool dismissVC, int totalCompletedWorkouts) {
    if (dismissVC) {
        ((void (*)(id, SEL, bool, id)) objc_msgSend)
            (this->navVC, sel_getUid("dismissViewControllerAnimated:completion:"), true, nil);
        appCoordinator_updateMaxWeights(appCoordinatorShared);
    }
    homeCoordinator_didFinishAddingWorkout(this->parent, totalCompletedWorkouts);
}

void addWorkoutCoordinator_start(AddWorkoutCoordinator *this) {
    id vc = ((id (*)(id, SEL, AddWorkoutCoordinator *)) objc_msgSend)
        (objc_staticMethod(objc_getClass("WorkoutViewController"), sel_getUid("alloc")),
         sel_getUid("initWithDelegate:"), this);
    ((void (*)(id, SEL, id, bool)) objc_msgSend)
        (this->navVC, sel_getUid("pushViewController:animated:"), vc, true);
    objc_singleArg(vc, sel_getUid("release"));
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
        id modal = ((id (*)(id, SEL, AddWorkoutCoordinator *)) objc_msgSend)
            (objc_staticMethod(objc_getClass("AddWorkoutUpdateMaxesViewController"),
                               sel_getUid("alloc")),
             sel_getUid("initWithDelegate:"), this);

        id container = ((id (*)(id, SEL, id)) objc_msgSend)
            (objc_staticMethod(objc_getClass("UINavigationController"), sel_getUid("alloc")),
             sel_getUid("initWithRootViewController:"), modal);

        ((void (*)(id, SEL, id, bool, id)) objc_msgSend)
            (this->navVC, sel_getUid("presentViewController:animated:completion:"),
             container, true, nil);
        objc_singleArg(container, sel_getUid("release"));
        objc_singleArg(modal, sel_getUid("release"));
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
        ((void (*)(id, SEL, int16_t)) objc_msgSend)(data, sel_getUid("setBestSquat:"), weights[0]);
        ((void (*)(id, SEL, int16_t)) objc_msgSend)(data, sel_getUid("setBestPullup:"), weights[1]);
        ((void (*)(id, SEL, int16_t)) objc_msgSend)(data, sel_getUid("setBestBench:"), weights[2]);
        ((void (*)(id, SEL, int16_t)) objc_msgSend)(data, sel_getUid("setBestDeadlift:"),
                                                    weights[3]);
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
