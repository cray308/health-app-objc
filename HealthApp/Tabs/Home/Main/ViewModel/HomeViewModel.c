//
//  HomeViewModel.c
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#include "HomeViewModel.h"
#include "AppUserData.h"
#include "CalendarDateHelpers.h"
#include "Exercise.h"

void homeViewModel_init(HomeViewModel *model) {
    memcpy(model->weekdays, (CFStringRef []){
        CFSTR("Monday"), CFSTR("Tuesday"), CFSTR("Wednesday"), CFSTR("Thursday"), CFSTR("Friday"),
        CFSTR("Saturday"), CFSTR("Sunday")}, 7 * sizeof(CFStringRef));
    memcpy(model->timeNames, (char [][10]){"morning", "afternoon", "evening"}, 30);

    homeViewModel_updateTimeOfDay(model);
}

void homeViewModel_fetchData(HomeViewModel *model) {
    for (int i = 0; i < 7; ++i) {
        if (model->workoutNames[i]) CFRelease(model->workoutNames[i]);
        model->workoutNames[i] = NULL;
    }
    if (appUserDataShared->currentPlan >= 0 && appUserDataShared->planStart <= time(NULL)) {
        unsigned char plan = (unsigned char) appUserDataShared->currentPlan;
        exerciseManager_setWeeklyWorkoutNames(plan,
                                              appUserData_getWeekInPlan(), model->workoutNames);
    }
}

bool homeViewModel_updateTimeOfDay(HomeViewModel *model) {
    struct tm localInfo;
    time_t now = time(NULL);
    localtime_r(&now, &localInfo);
    int timeOfDay = model->timeOfDay;
    int hour = localInfo.tm_hour;

    if (hour >= 5 && hour < 12 && timeOfDay != Morning) {
        model->timeOfDay = Morning;
        return true;
    } else if (hour >= 12 && hour < 17 && timeOfDay != Afternoon) {
        model->timeOfDay = Afternoon;
        return true;
    } else if ((hour < 5 || hour >= 17) && timeOfDay != Evening) {
        model->timeOfDay = Evening;
        return true;
    }
    return false;
}

bool homeViewModel_hasWorkoutsForThisWeek(HomeViewModel *model) {
    CFStringRef *names = model->workoutNames;
    for (int i = 0; i < 7; ++i) {
        if (names[i]) return true;
    }
    return false;
}

bool homeViewModel_shouldShowConfetti(HomeViewModel *model, int totalCompletedWorkouts) {
    if (!totalCompletedWorkouts) return false;

    int nWorkouts = 0;
    CFStringRef *names = model->workoutNames;
    for (int i = 0; i < 7; ++i) {
        if (names[i]) ++nWorkouts;
    }
    return nWorkouts == totalCompletedWorkouts;
}
