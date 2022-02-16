#include "AppUserData.h"
#include <CoreFoundation/CFNumber.h>
#include <stdlib.h>
#include <string.h>
#include "CocoaHelpers.h"

#define DaySeconds 86400

enum {
    WorkoutPlanBaseBuilding = 0,
    WorkoutPlanContinuation = 1
};

UserInfo *userData = NULL;
static CFStringRef const dictKey = CFSTR("userinfo");

static const void *keys[] = {
    CFSTR("planStart"), CFSTR("weekStart"), CFSTR("tzOffset"),
    CFSTR("currentPlan"), CFSTR("completedWorkouts"), CFSTR("darkMode"),
    CFSTR("squatMax"), CFSTR("pullUpMax"), CFSTR("benchMax"), CFSTR("deadliftMax")
};

static inline time_t getStartOfDay(time_t date, struct tm *info) {
    const int seconds = (info->tm_hour * 3600) + (info->tm_min * 60) + info->tm_sec;
    return date - seconds;
}

static int getOffsetFromGMT(time_t date) {
    struct tm gmtInfo;
    gmtime_r(&date, &gmtInfo);
    gmtInfo.tm_isdst = -1;
    return (int) (date - mktime(&gmtInfo));
}

static time_t calcStartOfWeek(time_t date) {
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

static void saveData(UserInfo *data) {
    CFNumberRef values[] = {
        CFNumberCreate(NULL, kCFNumberLongType, &data->planStart),
        CFNumberCreate(NULL, kCFNumberLongType, &data->weekStart),
        CFNumberCreate(NULL, kCFNumberIntType, &data->tzOffset),
        CFNumberCreate(NULL, kCFNumberCharType, &data->currentPlan),
        CFNumberCreate(NULL, kCFNumberCharType, &data->completedWorkouts),
        CFNumberCreate(NULL, kCFNumberCharType, &data->darkMode),
        CFNumberCreate(NULL, kCFNumberShortType, &data->liftMaxes[0]),
        CFNumberCreate(NULL, kCFNumberShortType, &data->liftMaxes[1]),
        CFNumberCreate(NULL, kCFNumberShortType, &data->liftMaxes[2]),
        CFNumberCreate(NULL, kCFNumberShortType, &data->liftMaxes[3])
    };

    CFDictionaryRef dict = CFDictionaryCreate(NULL, keys, (const void **)values, 10,
                                              &kCFCopyStringDictionaryKeyCallBacks,
                                              &kCFTypeDictionaryValueCallBacks);

    id defaults = getUserDefaults();
    (((void(*)(id,SEL,CFDictionaryRef,CFStringRef))objc_msgSend)
     (defaults, sel_getUid("setObject:forKey:"), dict, dictKey));
    CFRelease(dict);
    for (int i = 0; i < 10; ++i) {
        CFRelease(values[i]);
    }
}

void userInfo_create(bool darkMode, time_t *startOfWeek) {
    time_t now = time(NULL);
    time_t weekStart = calcStartOfWeek(now);
    UserInfo data = {
        .currentPlan = 0xff, .weekStart = weekStart, .planStart = weekStart,
        .tzOffset = getOffsetFromGMT(now), .darkMode = darkMode ? 0 : 0xff
    };
    *startOfWeek = weekStart;
    userData = malloc(sizeof(UserInfo));
    memcpy(userData, &data, sizeof(UserInfo));
    saveData(&data);
}

int userInfo_initFromStorage(time_t *startOfWeek) {
    const int planLengths[] = {8, 13};
    time_t now = time(NULL);
    time_t weekStart = calcStartOfWeek(now);
    id defaults = getUserDefaults();
    CFDictionaryRef savedInfo = (((CFDictionaryRef(*)(id,SEL,CFStringRef))objc_msgSend)
                                 (defaults, sel_getUid("dictionaryForKey:"), dictKey));
    UserInfo data;

    CFNumberRef value = CFDictionaryGetValue(savedInfo, keys[0]);
    CFNumberGetValue(value, kCFNumberLongType, &data.planStart);
    value = CFDictionaryGetValue(savedInfo, keys[1]);
    CFNumberGetValue(value, kCFNumberLongType, &data.weekStart);
    value = CFDictionaryGetValue(savedInfo, keys[2]);
    CFNumberGetValue(value, kCFNumberIntType, &data.tzOffset);
    value = CFDictionaryGetValue(savedInfo, keys[3]);
    CFNumberGetValue(value, kCFNumberCharType, &data.currentPlan);
    value = CFDictionaryGetValue(savedInfo, keys[4]);
    CFNumberGetValue(value, kCFNumberCharType, &data.completedWorkouts);
    value = CFDictionaryGetValue(savedInfo, keys[5]);
    CFNumberGetValue(value, kCFNumberCharType, &data.darkMode);
    for (int i = 0; i < 4; ++i) {
        value = CFDictionaryGetValue(savedInfo, keys[6 + i]);
        CFNumberGetValue(value, kCFNumberShortType, &data.liftMaxes[i]);
    }

    int newOffset = getOffsetFromGMT(now);
    int tzDiff = data.tzOffset - newOffset;
    bool madeChange = false;
    if (tzDiff) {
        madeChange = true;
        data.weekStart += tzDiff;
        data.planStart += tzDiff;
        data.tzOffset = newOffset;
    }

    data.week = (int) ((weekStart - data.planStart) / WeekSeconds);
    if (weekStart != data.weekStart) {
        madeChange = true;
        data.completedWorkouts = 0;
        data.weekStart = weekStart;

        if (data.currentPlan != 0xff && data.week >= planLengths[data.currentPlan]) {
            if (data.currentPlan == WorkoutPlanBaseBuilding)
                data.currentPlan = WorkoutPlanContinuation;
            data.planStart = weekStart;
            data.week = 0;
        }
    }

    *startOfWeek = weekStart;
    userData = malloc(sizeof(UserInfo));
    memcpy(userData, &data, sizeof(UserInfo));
    if (madeChange)
        saveData(&data);
    return tzDiff;
}

bool appUserData_deleteSavedData(void) {
    if (userData->completedWorkouts) {
        userData->completedWorkouts = 0;
        saveData(userData);
        return true;
    }
    return false;
}

int appUserData_addCompletedWorkout(unsigned char day) {
    int total = 0;
    unsigned char completed = userData->completedWorkouts;
    completed |= (1 << day);
    userData->completedWorkouts = completed;
    saveData(userData);
    for (int i = 0; i < 7; ++i) {
        if ((1 << i) & completed)
            ++total;
    }
    return total;
}

bool appUserData_updateWeightMaxes(short *weights) {
    bool madeChanges = false;
    for (int i = 0; i < 4; ++i) {
        if (weights[i] > userData->liftMaxes[i]) {
            madeChanges = true;
            userData->liftMaxes[i] = weights[i];
        }
    }
    if (madeChanges)
        saveData(userData);
    return madeChanges;
}

bool appUserData_updateUserSettings(unsigned char plan, unsigned char darkMode, short *weights) {
    bool madeChanges = plan != userData->currentPlan;
    if (plan != 0xff && madeChanges) {
#if TARGET_OS_SIMULATOR
        userData->planStart = userData->weekStart;
        userData->week = 0;
#else
        userData->planStart = userData->weekStart + WeekSeconds;
#endif
    }
    userData->currentPlan = plan;

    bool rv = false;
    if (darkMode != userData->darkMode) {
        madeChanges = rv = true;
        userData->darkMode = darkMode;
    }

    if (!appUserData_updateWeightMaxes(weights) && madeChanges)
        saveData(userData);
    return rv;
}
