//
//  AddWorkoutCoordinator.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "AddWorkoutCoordinator.h"
#import "WorkoutViewController.h"
#import "AddWorkoutUpdateMaxesViewController.h"
#import "HomeTabCoordinator.h"
#import "AppDelegate.h"
#include "AppUserData.h"
#import "PersistenceService.h"
#import "WeeklyData+CoreDataClass.h"

void updateStoredData(AddWorkoutViewModel *model) {
    int duration = (int) ((model->stopTime - model->startTime) / 60.0);
    WeeklyData *data = persistenceService_getWeeklyDataForThisWeek();
    if (!(duration >= 15 && data)) return;

    switch (model->workout->type) {
        case WorkoutTypeSE:
            data.timeSE += duration;
            break;
        case WorkoutTypeHIC:
            data.timeHIC += duration;
            break;
        case WorkoutTypeStrength:
            data.timeStrength += duration;
            break;
        default:
            data.timeEndurance += duration;
            break;
    }
    data.totalWorkouts += 1;
    persistenceService_saveContext();
}

void didFinishAddingWorkout(AddWorkoutCoordinator *this, UIViewController *presenter, int totalCompletedWorkouts) {
    if (presenter) {
        [presenter dismissViewControllerAnimated:true completion:nil];
        AppDelegate *delegate = (AppDelegate *) UIApplication.sharedApplication.delegate;
        if (delegate) appCoordinator_updateMaxWeights(&delegate->coordinator);
    }
    homeCoordinator_didFinishAddingWorkout(this->parent, totalCompletedWorkouts);
}

void addWorkoutCoordinator_start(AddWorkoutCoordinator *this) {
    UIViewController *vc = [[WorkoutViewController alloc] initWithDelegate:this];
    [this->navigationController pushViewController:vc animated:true];
    [vc release];
}

void addWorkoutCoordinator_free(AddWorkoutCoordinator *this) {
    array_free(exGroup, this->viewModel.workout->activities);
    CFRelease(this->viewModel.workout->title);
    free(this->viewModel.workout);
    free(this);
}

void addWorkoutCoordinator_stoppedWorkout(AddWorkoutCoordinator *this) {
    updateStoredData(&this->viewModel);
    didFinishAddingWorkout(this, nil, 0);
}

void addWorkoutCoordinator_completedWorkout(AddWorkoutCoordinator *this, UIViewController *presenter,
                                            bool showModalIfRequired) {
    CFStringRef title = this->viewModel.workout->title;
    if (showModalIfRequired && CFStringCompareWithOptions(title, CFSTR("test day"),
                                                          CFRangeMake(0, CFStringGetLength(title)),
                                                          kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
        UIViewController *modal = [[AddWorkoutUpdateMaxesViewController alloc] initWithDelegate:this];
        UINavigationController *container = [[UINavigationController alloc] initWithRootViewController:modal];
        [this->navigationController presentViewController:container animated:true completion:nil];
        [container release];
        [modal release];
        return;
    }

    updateStoredData(&this->viewModel);
    const signed char day = this->viewModel.workout->day;
    if (day >= 0) {
        unsigned char totalCompleted = appUserData_addCompletedWorkout((unsigned char) day);
        didFinishAddingWorkout(this, presenter, totalCompleted);
        return;
    }
    didFinishAddingWorkout(this, presenter, 0);
}

void addWorkoutCoordinator_finishedAddingNewWeights(AddWorkoutCoordinator *this, UIViewController *presenter,
                                                    short *weights) {
    WeeklyData *data = persistenceService_getWeeklyDataForThisWeek();
    if (data) {
        data.bestSquat = weights[0];
        data.bestPullup = weights[1];
        data.bestBench = weights[2];
        data.bestDeadlift = weights[3];
        persistenceService_saveContext();
    }
    appUserData_updateWeightMaxes(weights);
    addWorkoutCoordinator_completedWorkout(this, presenter, 0);
}

void addWorkoutCoordinator_stopWorkoutFromBackButtonPress(AddWorkoutCoordinator *this) {
    if (this->viewModel.startTime) {
        this->viewModel.stopTime = CFAbsoluteTimeGetCurrent();
        updateStoredData(&this->viewModel);
    }
    addWorkoutCoordinator_free(this);
}
