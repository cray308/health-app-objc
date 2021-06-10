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
#import "CalendarDateHelpers.h"
#import "ActivityEntry+CoreDataClass.h"
#import "WorkoutFinder.h"

void homeViewModel_clear(HomeViewModel *model) {
    if (model->workouts) {
        array_free(workout, model->workouts);
        model->workouts = NULL;
    }
}

void homeViewModel_fetchData(HomeViewModel *model) {
    homeViewModel_clear(model);

    if (!appUserData_hasWorkoutPlan()) return;
    model->workouts = workoutFinder_get_weekly_workouts(appUserDataShared->currentPlan, appUserData_getWeekInPlan());
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
