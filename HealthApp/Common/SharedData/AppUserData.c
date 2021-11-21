#include "AppUserData.h"
#include <CoreFoundation/CFNumber.h>
#include <stdlib.h>
#include <string.h>
#include "CocoaHelpers.h"

#define DaySeconds 86400

enum {
    WorkoutPlanNone = -1,
    WorkoutPlanBaseBuilding = 0,
    WorkoutPlanContinuation = 1
};

UserInfo *userData = NULL;
static CFStringRef const dictKey = CFSTR("userinfo");

static const void *keys[] = {
    CFSTR("planStart"), CFSTR("weekStart"), CFSTR("tzOffset"), CFSTR("currentPlan"),
    CFSTR("completedWorkouts"), CFSTR("squatMax"), CFSTR("pullUpMax"), CFSTR("benchMax"),
    CFSTR("deadliftMax"), CFSTR("darkMode")
};

static inline time_t getStartOfDay(time_t date, struct tm *info) {
    const int seconds = (info->tm_hour * 3600) + (info->tm_min * 60) + info->tm_sec;
    return date - seconds;
}

static int date_getOffsetFromGMT(time_t date) {
    struct tm gmtInfo;
    gmtime_r(&date, &gmtInfo);
    gmtInfo.tm_isdst = -1;
    return (int) (date - mktime(&gmtInfo));
}

#if DEBUG
time_t
#else
static time_t
#endif
date_calcStartOfWeek(time_t date) {
    struct tm localInfo;
    localtime_r(&date, &localInfo);
    int weekday = localInfo.tm_wday;

    if (weekday == 1) return getStartOfDay(date, &localInfo);

    date -= WeekSeconds;
    while (weekday != 1) {
        date += DaySeconds;
        weekday = weekday == 6 ? 0 : weekday + 1;
    }
    localtime_r(&date, &localInfo);
    return getStartOfDay(date, &localInfo);
}

static void saveData(void) {
    CFNumberRef values[10] = {
        CFNumberCreate(NULL, kCFNumberLongType, &userData->planStart),
        CFNumberCreate(NULL, kCFNumberLongType, &userData->weekStart),
        CFNumberCreate(NULL, kCFNumberIntType, &userData->tzOffset),
        CFNumberCreate(NULL, kCFNumberCharType, &userData->currentPlan),
        CFNumberCreate(NULL, kCFNumberCharType, &userData->completedWorkouts),
        CFNumberCreate(NULL, kCFNumberShortType, &userData->liftMaxes[0]),
        CFNumberCreate(NULL, kCFNumberShortType, &userData->liftMaxes[1]),
        CFNumberCreate(NULL, kCFNumberShortType, &userData->liftMaxes[2]),
        CFNumberCreate(NULL, kCFNumberShortType, &userData->liftMaxes[3]),
        CFNumberCreate(NULL, kCFNumberCharType, &userData->darkMode)
    };

    CFDictionaryRef dict = CFDictionaryCreate(NULL, keys, (const void **)values, 10,
                                              &kCFCopyStringDictionaryKeyCallBacks,
                                              &kCFTypeDictionaryValueCallBacks);

    id defaults = getUserDefaults();
    ((void(*)(id,SEL,CFDictionaryRef,CFStringRef))objc_msgSend)
    (defaults, sel_getUid("setObject:forKey:"), dict, dictKey);
    CFRelease(dict);
    for (int i = 0; i < 10; ++i) CFRelease(values[i]);
}

void userInfo_create(void) {
    time_t now = time(NULL);
    time_t weekStart = date_calcStartOfWeek(now);
    UserInfo info = {
        .currentPlan = WorkoutPlanNone,
        .weekStart = weekStart,
        .tzOffset = date_getOffsetFromGMT(now)
    };
    userData = malloc(sizeof(UserInfo));
    memcpy(userData, &info, sizeof(UserInfo));
    saveData();
}

int userInfo_initFromStorage(void) {
    const int planLengths[] = {8, 13};
    time_t now = time(NULL);
    time_t weekStart = date_calcStartOfWeek(now);
    id defaults = getUserDefaults();
    CFDictionaryRef savedInfo = getDict(defaults, sel_getUid("dictionaryForKey:"), dictKey);
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

    value = CFDictionaryGetValue(savedInfo, keys[9]);
    if (value != NULL) {
        CFNumberGetValue(value, kCFNumberCharType, &info->darkMode);
    } else {
        info->darkMode = false;
    }
    userData = info;

    int newOffset = date_getOffsetFromGMT(now);
    int tzDiff = userData->tzOffset - newOffset;
    bool madeChange = false;
    if (tzDiff) {
        madeChange = true;
        userData->weekStart += tzDiff;
        userData->tzOffset = newOffset;
    }

    if (weekStart != userData->weekStart) {
        madeChange = true;
        userData->completedWorkouts = 0;
        userData->weekStart = weekStart;

        if (userData->currentPlan != WorkoutPlanNone) {
            if ((appUserData_getWeekInPlan() / WeekSeconds) >= planLengths[userData->currentPlan]) {
                if (userData->currentPlan == WorkoutPlanBaseBuilding)
                    userData->currentPlan = WorkoutPlanContinuation;
                userData->planStart = weekStart;
            }
        }
    }

    if (madeChange) {
        saveData();
    }
    return tzDiff;
}

void appUserData_deleteSavedData(void) {
    userData->completedWorkouts = 0;
    saveData();
}

unsigned char appUserData_addCompletedWorkout(unsigned char day) {
    unsigned char total = 0;
    userData->completedWorkouts |= (1 << day);
    saveData();
    for (unsigned char i = 0; i < 7; ++i) {
        if ((1 << i) & userData->completedWorkouts)
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

bool appUserData_updateUserSettings(signed char plan, bool darkMode, short *weights) {
    bool rv = false;
    if (plan != WorkoutPlanNone && plan != userData->currentPlan) {
#if TARGET_OS_SIMULATOR
        userData->planStart = userData->weekStart;
#else
        userData->planStart = userData->weekStart + WeekSeconds;
#endif
    }
    userData->currentPlan = plan;

    if (osVersion == Version12) {
        rv = darkMode != userData->darkMode;
        userData->darkMode = darkMode;
    }
    appUserData_updateWeightMaxes(weights);
    return rv;
}
