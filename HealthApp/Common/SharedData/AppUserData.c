//
//  AppUserData.m
//  HealthApp
//
//  Created by Christopher Ray on 3/20/21.
//

#include "AppUserData.h"
#include "CocoaBridging.h"
#include "CalendarDateHelpers.h"

UserInfo *appUserDataShared = NULL;
static CFStringRef const userInfoKey = CFSTR("userinfo");

static CFStringRef const keys[] = {
    CFSTR("planStart"), CFSTR("weekStart"), CFSTR("tzOffset"), CFSTR("currentPlan"),
    CFSTR("completedWorkouts"), CFSTR("squatMax"), CFSTR("pullUpMax"), CFSTR("benchMax"),
    CFSTR("deadliftMax")
};

id getUserDefaults(void) {
    return objc_staticMethod(objc_getClass("NSUserDefaults"), sel_getUid("standardUserDefaults"));
}

UserInfo *userInfo_initFromStorage(void) {
    CFDictionaryRef savedInfo = ((CFDictionaryRef (*)(id, SEL, CFStringRef)) objc_msgSend)
        (getUserDefaults(), sel_getUid("dictionaryForKey:"), userInfoKey);
    if (!savedInfo) return NULL;
    CFNumberRef value;

    UserInfo *info = malloc(sizeof(UserInfo));
    if (!info) return NULL;

    value = CFDictionaryGetValue(savedInfo, keys[0]);
    CFNumberGetValue(value, kCFNumberLongType, &info->planStart);
    value = CFDictionaryGetValue(savedInfo, keys[1]);
    CFNumberGetValue(value, kCFNumberLongType, &info->weekStart);
    value = CFDictionaryGetValue(savedInfo, keys[2]);
    CFNumberGetValue(value, kCFNumberIntType, &info->tzOffset);
    value = CFDictionaryGetValue(savedInfo, keys[3]);
    CFNumberGetValue(value, kCFNumberCharType, &info->currentPlan);
    value = CFDictionaryGetValue(savedInfo, keys[4]);
    CFNumberGetValue(value, kCFNumberCharType, &info->completedWorkouts);
    for (int i = 0; i < 4; ++i) {
        value = CFDictionaryGetValue(savedInfo, keys[5 + i]);
        CFNumberGetValue(value, kCFNumberShortType, &info->liftMaxes[i]);
    }
    return info;
}

void userInfo_saveData(UserInfo *info) {
    CFNumberRef values[] = {
        CFNumberCreate(NULL, kCFNumberLongType, &info->planStart),
        CFNumberCreate(NULL, kCFNumberLongType, &info->weekStart),
        CFNumberCreate(NULL, kCFNumberIntType, &info->tzOffset),
        CFNumberCreate(NULL, kCFNumberCharType, &info->currentPlan),
        CFNumberCreate(NULL, kCFNumberCharType, &info->completedWorkouts),
        CFNumberCreate(NULL, kCFNumberShortType, &info->liftMaxes[0]),
        CFNumberCreate(NULL, kCFNumberShortType, &info->liftMaxes[1]),
        CFNumberCreate(NULL, kCFNumberShortType, &info->liftMaxes[2]),
        CFNumberCreate(NULL, kCFNumberShortType, &info->liftMaxes[3])
    };

    CFDictionaryRef dict = CFDictionaryCreate(NULL, (const void **)keys, (const void **)values, 9,
                                              &kCFCopyStringDictionaryKeyCallBacks,
                                              &kCFTypeDictionaryValueCallBacks);

    id defaults = getUserDefaults();
    ((void (*)(id, SEL, CFDictionaryRef, CFStringRef)) objc_msgSend)
        (defaults, sel_getUid("setObject:forKey:"), dict, userInfoKey);
    ((bool (*)(id, SEL)) objc_msgSend)(defaults, sel_getUid("synchronize"));
    CFRelease(dict);
    for (int i = 0; i < 9; ++i) CFRelease(values[i]);
}

void appUserData_setWorkoutPlan(signed char plan) {
    if (plan >= 0 && plan != appUserDataShared->currentPlan) {
        appUserDataShared->planStart = appUserDataShared->weekStart + WeekSeconds;
    }
    appUserDataShared->currentPlan = plan;
    userInfo_saveData(appUserDataShared);
}

int appUserData_checkTimezone(time_t now) {
    int newOffset = date_getOffsetFromGMT(now);
    int diff = newOffset - appUserDataShared->tzOffset;
    if (diff) {
        appUserDataShared->weekStart += diff;
        appUserDataShared->tzOffset = newOffset;
        userInfo_saveData(appUserDataShared);
    }
    return diff;
}

void appUserData_deleteSavedData(void) {
    appUserDataShared->completedWorkouts = 0;
    userInfo_saveData(appUserDataShared);
}

void appUserData_handleNewWeek(time_t weekStart) {
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
