//
//  HomeViewModel.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "HomeViewModel.h"
#import "HomeTabCoordinator.h"
#import "AppCoordinator.h"
#import "AppUserData.h"
#import "PersistenceService.h"
#include "CalendarDateHelpers.h"
#import "WorkoutFinder.h"

typedef enum {
    CustomWorkoutIndexTestMax,
    CustomWorkoutIndexEndurance,
    CustomWorkoutIndexStrength,
    CustomWorkoutIndexSE,
    CustomWorkoutIndexHIC
} CustomWorkoutIndex;

void clearNames(CFStringRef *names) {
    for (int i = 0; i < 7; ++i) {
        if (names[i]) CFRelease(names[i]);
        names[i] = NULL;
    }
}

HomeViewModel *homeViewModel_init(void) {
    HomeViewModel *model = calloc(1, sizeof(HomeViewModel));
    if (!model) return NULL;
    homeViewModel_updateTimeOfDay(model);
    return model;
}

void homeViewModel_free(HomeViewModel *model) {
    clearNames(model->workoutNames);
    free(model);
}

void homeViewModel_fetchData(HomeViewModel *model) {
    clearNames(model->workoutNames);
    if (!appUserData_hasWorkoutPlan()) return;
    unsigned char plan = (unsigned char) appUserDataShared->currentPlan;
    workoutFinder_setWeeklyWorkoutNames(plan, appUserData_getWeekInPlan(), model->workoutNames);
}

bool homeViewModel_updateTimeOfDay(HomeViewModel *model) {
    CFCalendarRef calendar = CFCalendarCopyCurrent();
    int timeOfDay = model->timeOfDay;
    int hour = (int) CFCalendarGetOrdinalityOfUnit(calendar, kCFCalendarUnitHour, kCFCalendarUnitDay, CFAbsoluteTimeGetCurrent());
    CFRelease(calendar);
    if (hour >= 6 && hour < 13 && timeOfDay != Morning) {
        model->timeOfDay = Morning;
        return true;
    } else if (hour < 18 && timeOfDay != Afternoon) {
        model->timeOfDay = Afternoon;
        return true;
    } else if ((hour < 6 || hour >= 18) && timeOfDay != Evening) {
        model->timeOfDay = Evening;
        return true;
    }
    return false;
}

NSString *homeViewModel_getGreeting(HomeViewModel *model) {
    NSString *greet;
    switch (model->timeOfDay) {
        case Morning:
            greet = @"morning";
            break;
        case Afternoon:
            greet = @"afternoon";
            break;
        default:
            greet = @"evening";
            break;
    }
    return [[NSString alloc] initWithFormat:@"Good %@!", greet];
}

void homeViewModel_handleDayWorkoutButtonTap(HomeViewModel *model, int index) {
    unsigned char plan = (unsigned char) appUserDataShared->currentPlan;
    Workout *w = workoutFinder_getWeeklyWorkoutAtIndex(plan, appUserData_getWeekInPlan(), index);
    if (w) homeCoordinator_navigateToAddWorkout(model->delegate, nil, w);
}

unsigned char homeViewModel_hasWorkoutsForThisWeek(HomeViewModel *model) {
    CFStringRef *names = model->workoutNames;
    for (int i = 0; i < 7; ++i) {
        if (names[i]) return 1;
    }
    return 0;
}

unsigned char homeViewModel_shouldShowConfetti(HomeViewModel *model, int totalCompletedWorkouts) {
    if (!totalCompletedWorkouts) return 0;

    int nWorkouts = 0;
    CFStringRef *names = model->workoutNames;
    for (int i = 0; i < 7; ++i) {
        if (names[i]) ++nWorkouts;
    }
    return nWorkouts == totalCompletedWorkouts;
}

void homeViewModel_handleCustomWorkoutButtonTap(HomeViewModel *model, int index) {
    unsigned char type = WorkoutTypeStrength;
    switch (index) {
        case CustomWorkoutIndexSE:
            type = WorkoutTypeSE;
            break;
        case CustomWorkoutIndexHIC:
            type = WorkoutTypeHIC;
            break;
        case CustomWorkoutIndexTestMax: ;
            Workout *w = workoutFinder_get_workout_from_library(WorkoutTypeStrength, 2, 1, 1, 100);
            if (w) homeCoordinator_navigateToAddWorkout(model->delegate, nil, w);
            //homeCoordinator_navigateToAddWorkoutWithCustom(model->delegate, nil, w);
            return;
        case CustomWorkoutIndexEndurance:
            type = WorkoutTypeEndurance;
            break;
        default:
            break;
    }

    unsigned int count = 0;
    CFStringRef *names = workoutFinder_get_workout_names(type, &count);
    if (!names) return;
    else if (!count) {
        free(names);
        return;
    }
    homeCoordinator_showWorkoutPickerVC(model->delegate, type, names, count);
}

void homeViewModel_finishedSettingUpCustomWorkout(HomeViewModel *model, UIViewController *presenter, unsigned char type, unsigned int index, unsigned int sets, unsigned int reps, unsigned int weight) {
    Workout *w = workoutFinder_get_workout_from_library(type, index, reps, sets, weight);
    if (w) homeCoordinator_navigateToAddWorkout(model->delegate, presenter, w);
}

void homeViewModel_cancelCustomWorkout(UIViewController *presenter) {
    [presenter dismissViewControllerAnimated:true completion:nil];
}
