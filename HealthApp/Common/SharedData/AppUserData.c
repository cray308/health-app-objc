//
//  AppUserData.m
//  HealthApp
//
//  Created by Christopher Ray on 3/20/21.
//

#include <CoreFoundation/CoreFoundation.h>
#include "AppUserData.h"
#include "CocoaBridging.h"
#include "Exercise.h"
#include "CalendarDateHelpers.h"

UserInfo *appUserDataShared = NULL;

static CFStringRef const keys[] = {CFSTR("planStart"), CFSTR("weekStart"), CFSTR("currentPlan"),
    CFSTR("completedWorkouts"), CFSTR("squatMax"), CFSTR("pullUpMax"), CFSTR("benchMax"), CFSTR("deadliftMax")};

UserInfo *userInfo_initFromStorage(void) {
    CFDictionaryRef savedInfo = getUserInfoDictionary();
    if (!savedInfo) return NULL;
    CFNumberRef value;

    UserInfo *info = malloc(sizeof(UserInfo));
    if (!info) return NULL;

    value = CFDictionaryGetValue(savedInfo, keys[0]);
    CFNumberGetValue(value, kCFNumberDoubleType, &info->planStart);
    value = CFDictionaryGetValue(savedInfo, keys[1]);
    CFNumberGetValue(value, kCFNumberDoubleType, &info->weekStart);
    value = CFDictionaryGetValue(savedInfo, keys[2]);
    CFNumberGetValue(value, kCFNumberCharType, &info->currentPlan);
    value = CFDictionaryGetValue(savedInfo, keys[3]);
    CFNumberGetValue(value, kCFNumberCharType, &info->completedWorkouts);
    for (int i = 0; i < 4; ++i) {
        value = CFDictionaryGetValue(savedInfo, keys[4 + i]);
        CFNumberGetValue(value, kCFNumberShortType, &info->liftMaxes[i]);
    }
    return info;
}

void userInfo_saveData(UserInfo *info) {
    CFNumberRef values[] = {CFNumberCreate(NULL, kCFNumberDoubleType, &info->planStart),
        CFNumberCreate(NULL, kCFNumberDoubleType, &info->weekStart),
        CFNumberCreate(NULL, kCFNumberCharType, &info->currentPlan),
        CFNumberCreate(NULL, kCFNumberCharType, &info->completedWorkouts),
        CFNumberCreate(NULL, kCFNumberShortType, &info->liftMaxes[LiftTypeSquat]),
        CFNumberCreate(NULL, kCFNumberShortType, &info->liftMaxes[LiftTypePullup]),
        CFNumberCreate(NULL, kCFNumberShortType, &info->liftMaxes[LiftTypeBench]),
        CFNumberCreate(NULL, kCFNumberShortType, &info->liftMaxes[LiftTypeDeadlift])};

    CFDictionaryRef dict = CFDictionaryCreate(NULL, (const void **)keys, (const void **)values, 8,
                                              &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    writeUserInfoDictionary(dict);
    CFRelease(dict);
    for (int i = 0; i < 8; ++i) CFRelease(values[i]);
}

void appUserData_setWorkoutPlan(signed char plan) {
    if (plan >= 0 && plan != appUserDataShared->currentPlan) {
        CFCalendarRef calendar = CFCalendarCopyCurrent();
        appUserDataShared->planStart = date_calcStartOfWeek(CFAbsoluteTimeGetCurrent(), calendar,
                                                            DateSearchDirectionNext, 1);
        CFRelease(calendar);
    }
    appUserDataShared->currentPlan = plan;
    userInfo_saveData(appUserDataShared);
}

void appUserData_deleteSavedData(void) {
    appUserDataShared->completedWorkouts = 0;
    userInfo_saveData(appUserDataShared);
}

void appUserData_handleNewWeek(double weekStart) {
    appUserDataShared->completedWorkouts = 0;
    appUserDataShared->weekStart = weekStart;

    signed char plan = appUserDataShared->currentPlan;
    if (plan >= 0) {
        int difference = (int) (weekStart - appUserDataShared->planStart);
        const int nWeeks = plan == 0 ? 8 : 13;
        if ((difference / WeekSeconds) >= nWeeks) {
            if (plan == 0) {
                appUserDataShared->currentPlan = 1;
            }
            appUserDataShared->planStart = weekStart;
        }
    }
    userInfo_saveData(appUserDataShared);
}

unsigned char appUserData_addCompletedWorkout(unsigned char day) {
    unsigned char total = 0;
    appUserDataShared->completedWorkouts |= (1 << day);
    userInfo_saveData(appUserDataShared);
    const unsigned char completedMask = appUserDataShared->completedWorkouts;
    for (unsigned char i = 0; i < 7; ++i) {
        if ((1 << i) & completedMask) ++total;
    }
    return total;
}

int appUserData_getWeekInPlan(void) {
    return ((int) (appUserDataShared->weekStart - appUserDataShared->planStart)) / WeekSeconds;
}

void appUserData_updateWeightMaxes(short *weights) {
    for (int i = 0; i < 4; ++i) {
        if (weights[i] > appUserDataShared->liftMaxes[i]) {
            appUserDataShared->liftMaxes[i] = weights[i];
        }
    }
    userInfo_saveData(appUserDataShared);
}
