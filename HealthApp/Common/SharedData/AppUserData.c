//
//  AppUserData.c
//  HealthApp
//
//  Created by Christopher Ray on 3/20/21.
//

#include "AppUserData.h"
#include "CocoaHelpers.h"
#include "CalendarDateHelpers.h"

UserInfo *userData = NULL;
static CFStringRef const userInfoKey = CFSTR("userinfo");

static CFStringRef const keys[] = {
    CFSTR("planStart"), CFSTR("weekStart"), CFSTR("tzOffset"), CFSTR("currentPlan"),
    CFSTR("completedWorkouts"), CFSTR("squatMax"), CFSTR("pullUpMax"), CFSTR("benchMax"),
    CFSTR("deadliftMax")
};

static void saveData(void) {
    CFNumberRef values[] = {
        CFNumberCreate(NULL, kCFNumberLongType, &userData->planStart),
        CFNumberCreate(NULL, kCFNumberLongType, &userData->weekStart),
        CFNumberCreate(NULL, kCFNumberIntType, &userData->tzOffset),
        CFNumberCreate(NULL, kCFNumberCharType, &userData->currentPlan),
        CFNumberCreate(NULL, kCFNumberCharType, &userData->completedWorkouts),
        CFNumberCreate(NULL, kCFNumberShortType, &userData->liftMaxes[0]),
        CFNumberCreate(NULL, kCFNumberShortType, &userData->liftMaxes[1]),
        CFNumberCreate(NULL, kCFNumberShortType, &userData->liftMaxes[2]),
        CFNumberCreate(NULL, kCFNumberShortType, &userData->liftMaxes[3])
    };

    CFDictionaryRef dict = CFDictionaryCreate(NULL, (const void **)keys, (const void **)values, 9,
                                              &kCFCopyStringDictionaryKeyCallBacks,
                                              &kCFTypeDictionaryValueCallBacks);

    id defaults = getUserDefaults();
    ((void(*)(id,SEL,CFDictionaryRef,CFStringRef))objc_msgSend)
    (defaults, sel_getUid("setObject:forKey:"), dict, userInfoKey);
    CFRelease(dict);
    for (int i = 0; i < 9; ++i) CFRelease(values[i]);
}

void userInfo_create(void) {
    time_t now = time(NULL);
    time_t weekStart = date_calcStartOfWeek(now);
    UserInfo info = {
        .currentPlan = -1, .weekStart = weekStart, .tzOffset = date_getOffsetFromGMT(now)
    };
    userData = malloc(sizeof(UserInfo));
    memcpy(userData, &info, sizeof(UserInfo));
    saveData();
}

int userInfo_initFromStorage(void) {
    time_t now = time(NULL);
    time_t weekStart = date_calcStartOfWeek(now);
    CFDictionaryRef savedInfo = ((CFDictionaryRef(*)(id,SEL,CFStringRef))objc_msgSend)
    (getUserDefaults(), sel_getUid("dictionaryForKey:"), userInfoKey);
    CFNumberRef value;

    UserInfo *info = malloc(sizeof(UserInfo));

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
    userData = info;

    int newOffset = date_getOffsetFromGMT(now);
    int tzDiff = userData->tzOffset - newOffset;
    if (tzDiff) {
        userData->weekStart += tzDiff;
        userData->tzOffset = newOffset;
        saveData();
    }

    if (weekStart != userData->weekStart) {
        userData->completedWorkouts = 0;
        userData->weekStart = weekStart;

        signed char plan = userData->currentPlan;
        if (plan >= 0) {
            const int nWeeks = plan == 0 ? 8 : 13;
            if ((appUserData_getWeekInPlan() / WeekSeconds) >= nWeeks) {
                if (plan == 0)
                    userData->currentPlan = 1;
                userData->planStart = weekStart;
            }
        }
        saveData();
    }
    return tzDiff;
}

void appUserData_setWorkoutPlan(signed char plan) {
    if (plan >= 0 && plan != userData->currentPlan) {
#if DEBUG
        userData->planStart = userData->weekStart;
#else
        userData->planStart = userData->weekStart + WeekSeconds;
#endif
    }
    userData->currentPlan = plan;
    saveData();
}

void appUserData_deleteSavedData(void) {
    userData->completedWorkouts = 0;
    saveData();
}

unsigned char appUserData_addCompletedWorkout(unsigned char day) {
    unsigned char total = 0;
    userData->completedWorkouts |= (1 << day);
    saveData();
    const unsigned char completedMask = userData->completedWorkouts;
    for (unsigned char i = 0; i < 7; ++i) {
        if ((1 << i) & completedMask)
            ++total;
    }
    return total;
}

int appUserData_getWeekInPlan(void) {
    return ((int) (userData->weekStart - userData->planStart)) / WeekSeconds;
}

void appUserData_updateWeightMaxes(short *weights) {
    for (int i = 0; i < 4; ++i) {
        if (weights[i] > userData->liftMaxes[i])
            userData->liftMaxes[i] = weights[i];
    }
    saveData();
}
