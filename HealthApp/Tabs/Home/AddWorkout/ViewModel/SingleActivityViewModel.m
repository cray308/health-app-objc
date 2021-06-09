//
//  SingleActivityViewModel.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "SingleActivityViewModel.h"
#import "AddWorkoutCoordinator.h"
#import "AddWorkoutViewModel.h"
#import "ActivityType.h"

int calcTokens(ActivityType intensity, uint8_t duration);

void addSingleActivityViewModel_tappedAddActivity(AddSingleActivityViewModel *model, UIViewController *presenter) {
    SingleActivityModel activityModel = {
        .intensity = model->intensityIndex,
        .duration = model->duration,
        .tokens = calcTokens(model->intensityIndex, model->duration)
    };
    array_push_back(SingleActivityModel, model->parentViewModel->workouts, activityModel);
    int newRow = array_size(model->parentViewModel->workouts) - 1;
    addWorkoutCoordinator_didFinishAddingActivity(model->delegate, presenter, newRow);
}

int calcTokens(ActivityType intensity, uint8_t duration) {
    uint8_t increment = 0, limit = 0;
    int tokens = 1;

    switch (intensity) {
        case ActivityType_Low:
            limit = 40;
            increment = 20;
            break;
        case ActivityType_Medium:
            limit = 30;
            increment = 15;
            break;
        case ActivityType_High:
            limit = 20;
            increment = 10;
            break;
    }

    if (duration <= limit) {
        return duration >= increment ? 1 : 0;
    }

    limit += increment;
    tokens += 1;
    while (duration > limit) {
        limit += increment;
        tokens += 1;
    }

    uint8_t difference = limit - duration;
    if (difference >= 0 && difference <= (increment >> 1)) {
        return tokens;
    }
    return tokens - 1;
}
