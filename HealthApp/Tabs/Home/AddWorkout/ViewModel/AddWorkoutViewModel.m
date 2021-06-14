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

void updateStoredData(AddWorkoutViewModel *model, unsigned char type) {
    unsigned int duration = (unsigned int) ((model->stopTime - model->startTime) / 60.0);
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

AddWorkoutViewModel *addWorkoutViewModel_init(Workout *w) {
    AddWorkoutViewModel *model = calloc(1, sizeof(AddWorkoutViewModel));
    if (!model) return NULL;
    model->workout = w;
    return model;
}

void addWorkoutViewModel_free(AddWorkoutViewModel *model) {
    workout_free(model->workout);
    free(model);
}

void addWorkoutViewModel_stoppedWorkout(AddWorkoutViewModel *model) {
    updateStoredData(model, model->workout->type);
    addWorkoutCoordinator_didFinishAddingWorkout(model->delegate, nil, 0);
}

void addWorkoutViewModel_completedWorkout(AddWorkoutViewModel *model, UIViewController *presenter) {
    updateStoredData(model, model->workout->type);
    const signed char day = model->workout->day;
    if (day >= 0) {
        unsigned char totalCompleted = appUserData_addCompletedWorkout((unsigned char) day);
        addWorkoutCoordinator_didFinishAddingWorkout(model->delegate, presenter, totalCompleted);
        return;
    }
    addWorkoutCoordinator_didFinishAddingWorkout(model->delegate, presenter, 0);
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
    addWorkoutViewModel_completedWorkout(model, presenter);
}
