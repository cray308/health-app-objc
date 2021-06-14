//
//  AppUserData.m
//  HealthApp
//
//  Created by Christopher Ray on 3/20/21.
//

#import <Foundation/Foundation.h>
#import "AppUserData.h"
#include "CalendarDateHelpers.h"

UserInfo *appUserDataShared = NULL;

static CFStringRef const keys[] = {CFSTR("planStart"), CFSTR("weekStart"), CFSTR("currentPlan"), CFSTR("completedWorkouts"), CFSTR("squatMax"), CFSTR("pullUpMax"), CFSTR("benchMax"), CFSTR("deadliftMax")};

UserInfo *userInfo_initFromStorage(void) {
    CFDictionaryRef savedInfo = (__bridge CFDictionaryRef)[NSUserDefaults.standardUserDefaults dictionaryForKey:@"userinfo"];
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
    value = CFDictionaryGetValue(savedInfo, keys[4]);
    CFNumberGetValue(value, kCFNumberShortType, &info->squatMax);
    value = CFDictionaryGetValue(savedInfo, keys[5]);
    CFNumberGetValue(value, kCFNumberShortType, &info->pullUpMax);
    value = CFDictionaryGetValue(savedInfo, keys[6]);
    CFNumberGetValue(value, kCFNumberShortType, &info->benchMax);
    value = CFDictionaryGetValue(savedInfo, keys[7]);
    CFNumberGetValue(value, kCFNumberShortType, &info->deadliftMax);
    return info;
}

void userInfo_saveData(UserInfo *info) {
    CFNumberRef values[] = {CFNumberCreate(NULL, kCFNumberDoubleType, &info->planStart), CFNumberCreate(NULL, kCFNumberDoubleType, &info->weekStart), CFNumberCreate(NULL, kCFNumberCharType, &info->currentPlan), CFNumberCreate(NULL, kCFNumberCharType, &info->completedWorkouts), CFNumberCreate(NULL, kCFNumberShortType, &info->squatMax), CFNumberCreate(NULL, kCFNumberShortType, &info->pullUpMax), CFNumberCreate(NULL, kCFNumberShortType, &info->benchMax), CFNumberCreate(NULL, kCFNumberShortType, &info->deadliftMax)};

    CFDictionaryRef dict = CFDictionaryCreate(NULL, (const void **)keys, (const void **)values, 8, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    [NSUserDefaults.standardUserDefaults setObject:(__bridge NSDictionary*)dict forKey:@"userinfo"];
    [NSUserDefaults.standardUserDefaults synchronize];
    CFRelease(dict);
    for (int i = 0; i < 8; ++i) CFRelease(values[i]);
}

void appUserData_free(void) {
    if (appUserDataShared) free(appUserDataShared);
    appUserDataShared = NULL;
}

void appUserData_setWorkoutPlan(signed char plan) {
    if (plan >= 0 && plan != appUserDataShared->currentPlan) {
        CFCalendarRef calendar = CFCalendarCopyCurrent();
        appUserDataShared->planStart = date_calcStartOfWeek(CFAbsoluteTimeGetCurrent(), calendar, DateSearchDirection_Next, 1);
        CFRelease(calendar);
    }
    appUserDataShared->currentPlan = plan;
    userInfo_saveData(appUserDataShared);
}

unsigned char appUserData_hasWorkoutPlan(void) {
    if (appUserDataShared->currentPlan < 0) return 0;
    return ((int) appUserDataShared->weekStart <= (int) CFAbsoluteTimeGetCurrent());
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
        unsigned int difference = (unsigned int) (weekStart - appUserDataShared->planStart);
        const unsigned int nWeeks = plan == 0 ? 8 : 13;
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

unsigned int appUserData_getWeekInPlan(void) {
    return ((unsigned int) (appUserDataShared->weekStart - appUserDataShared->planStart)) / WeekSeconds;
}

void appUserData_updateWeightMaxes(unsigned short *weights) {
    if (weights[0] > appUserDataShared->squatMax) {
        appUserDataShared->squatMax = weights[0];
    }
    if (weights[1] > appUserDataShared->pullUpMax) {
        appUserDataShared->pullUpMax = weights[1];
    }
    if (weights[2] > appUserDataShared->benchMax) {
        appUserDataShared->benchMax = weights[2];
    }
    if (weights[3] > appUserDataShared->deadliftMax) {
        appUserDataShared->deadliftMax = weights[3];
    }
    userInfo_saveData(appUserDataShared);
}
