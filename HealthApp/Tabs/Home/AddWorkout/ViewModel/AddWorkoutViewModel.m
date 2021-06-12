//
//  AddWorkoutViewModel.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "AddWorkoutViewModel.h"
#import "AppUserData.h"
#import "WeeklyData+CoreDataClass.h"
#import "PersistenceService.h"
#import "AddWorkoutCoordinator.h"

void updateStoredData(unsigned int duration, unsigned char type) {
    WeeklyData *data = persistenceService_getWeeklyDataForThisWeek();
    if (!(duration && data)) return;

    switch (type) {
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

void addWorkoutViewModel_stoppedWorkout(AddWorkoutViewModel *model, unsigned int duration) {
    updateStoredData(duration, model->workout->type);
    addWorkoutCoordinator_didFinishAddingWorkout(model->delegate, 0);
}

void addWorkoutViewModel_completedWorkout(AddWorkoutViewModel *model, unsigned int duration) {
    updateStoredData(duration, model->workout->type);
    const signed char day = model->workout->day;
    if (day >= 0) {
        unsigned char totalCompleted = appUserData_addCompletedWorkout((unsigned char) day);
        addWorkoutCoordinator_didFinishAddingWorkout(model->delegate, totalCompleted);
        return;
    }
    addWorkoutCoordinator_didFinishAddingWorkout(model->delegate, 0);
}

void addWorkoutViewModel_finishedAddingNewWeights(AddWorkoutViewModel *model, UIViewController *presenter, unsigned short *weights) {
    WeeklyData *data = persistenceService_getWeeklyDataForThisWeek();
    if (data) {
        data.bestSquat = weights[0];
        data.bestPullup = weights[1];
        data.bestBench = weights[2];
        data.bestDeadlift = weights[3];
        persistenceService_saveContext();
    }

    appUserData_updateWeightMaxes(weights);
    addWorkoutCoordinator_finishedUpdatingWeights(model->delegate, presenter);
}
