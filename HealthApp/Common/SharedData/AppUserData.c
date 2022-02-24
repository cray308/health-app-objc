#include "AppUserData.h"
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CFPreferences.h>
#include <stdlib.h>
#include <string.h>
#include "CocoaHelpers.h"

#define DaySeconds 86400

#if TARGET_OS_SIMULATOR
extern void exerciseManager_setCurrentWeek(int);
#endif

enum {
    WorkoutPlanBaseBuilding, WorkoutPlanContinuation
};

enum {
    IWeekStart, IPlanStart, ITzOffset, ICurrentPlan, ICompletedWorkouts, IDarkMode, ILiftArray
};

UserInfo *userData = NULL;
static CFStringRef const dictKey = CFSTR("userinfo");

static const void *DictKeys[] = {
    CFSTR("weekStart"), CFSTR("planStart"), CFSTR("tzOffset"),
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

static CFMutableDictionaryRef createMutableDict(void) {
    CFDictionaryRef saved = CFPreferencesCopyAppValue(dictKey, kCFPreferencesCurrentApplication);
    CFMutableDictionaryRef newDict = CFDictionaryCreateMutableCopy(NULL, 10, saved);
    CFRelease(saved);
    return newDict;
}

static void saveChanges(CFMutableDictionaryRef dict CF_CONSUMED,
                        CFStringRef *keys, CFNumberRef *values, int count) {
    for (int i = 0; i < count; ++i) {
        CFDictionaryReplaceValue(dict, keys[i], values[i]);
    }
    CFPreferencesSetAppValue(dictKey, dict, kCFPreferencesCurrentApplication);
    CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
    CFRelease(dict);
    for (int i = 0; i < count; ++i) {
        CFRelease(values[i]);
    }
}

void userInfo_create(bool legacy, UserInfo const **dataOut) {
    time_t now = time(NULL);
    time_t weekStart = calcStartOfWeek(now);
    int tzOffset = getOffsetFromGMT(now);
    UserInfo data = {weekStart, weekStart, {0}, legacy ? 0 : 0xff, 0xff, 0};

    CFNumberRef values[] = {
        CFNumberCreate(NULL, kCFNumberLongType, &weekStart),
        CFNumberCreate(NULL, kCFNumberLongType, &weekStart),
        CFNumberCreate(NULL, kCFNumberIntType, &tzOffset),
        CFNumberCreate(NULL, kCFNumberCharType, &(unsigned char){0xff}),
        CFNumberCreate(NULL, kCFNumberCharType, &(unsigned char){0}),
        CFNumberCreate(NULL, kCFNumberCharType, &data.darkMode),
        CFNumberCreate(NULL, kCFNumberShortType, &(short){0}),
        CFNumberCreate(NULL, kCFNumberShortType, &(short){0}),
        CFNumberCreate(NULL, kCFNumberShortType, &(short){0}),
        CFNumberCreate(NULL, kCFNumberShortType, &(short){0})
    };
    CFDictionaryRef dict = CFDictionaryCreate(NULL, DictKeys, (const void **)values, 10,
                                              &kCFCopyStringDictionaryKeyCallBacks,
                                              &kCFTypeDictionaryValueCallBacks);
    CFPreferencesSetAppValue(dictKey, dict, kCFPreferencesCurrentApplication);
    CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
    CFRelease(dict);
    for (int i = 0; i < 10; ++i) {
        CFRelease(values[i]);
    }

    userData = malloc(sizeof(UserInfo));
    memcpy(userData, &data, sizeof(UserInfo));
    *dataOut = userData;
}

int userInfo_initFromStorage(bool legacy, int *weekInPlan, UserInfo const **dataOut) {
    const int planLengths[] = {8, 13};
    int tzOffset;
    time_t now = time(NULL);
    time_t weekStart = calcStartOfWeek(now), savedWeekStart, planStart;
    unsigned char changes = 0, plan, completed, savedDM;
    CFDictionaryRef savedInfo = CFPreferencesCopyAppValue(dictKey, kCFPreferencesCurrentApplication);

    CFNumberRef value = CFDictionaryGetValue(savedInfo, DictKeys[0]);
    CFNumberGetValue(value, kCFNumberLongType, &savedWeekStart);
    value = CFDictionaryGetValue(savedInfo, DictKeys[1]);
    CFNumberGetValue(value, kCFNumberLongType, &planStart);
    value = CFDictionaryGetValue(savedInfo, DictKeys[2]);
    CFNumberGetValue(value, kCFNumberIntType, &tzOffset);
    value = CFDictionaryGetValue(savedInfo, DictKeys[3]);
    CFNumberGetValue(value, kCFNumberCharType, &plan);
    value = CFDictionaryGetValue(savedInfo, DictKeys[4]);
    CFNumberGetValue(value, kCFNumberCharType, &completed);
    value = CFDictionaryGetValue(savedInfo, DictKeys[5]);
    CFNumberGetValue(value, kCFNumberCharType, &savedDM);

    int newOffset = getOffsetFromGMT(now);
    int tzDiff = tzOffset - newOffset;
    if (tzDiff) {
        changes = 7;
        savedWeekStart += tzDiff;
        planStart += tzDiff;
        tzOffset = newOffset;
    }

    int week = (int) ((weekStart - planStart) / WeekSeconds);
    if (weekStart != savedWeekStart) {
        changes |= 17;
        completed = 0;
        savedWeekStart = weekStart;

        if (!(plan & 128) && week >= planLengths[plan]) {
            if (plan == WorkoutPlanBaseBuilding) {
                plan = WorkoutPlanContinuation;
                changes |= 8;
            }
            planStart = weekStart;
            changes |= 2;
            week = 0;
        }
    }

    if (!(savedDM & 128) && !legacy) {
        savedDM = 0xff;
        changes |= 32;
    }

    if (changes) {
        CFStringRef keys[6];
        CFNumberRef values[6];
        int nChanges = 0;

        if (changes & 1) {
            keys[0] = DictKeys[IWeekStart];
            values[nChanges++] = CFNumberCreate(NULL, kCFNumberLongType, &weekStart);
        }
        if (changes & 2) {
            keys[nChanges] = DictKeys[IPlanStart];
            values[nChanges++] = CFNumberCreate(NULL, kCFNumberLongType, &planStart);
        }
        if (changes & 4) {
            keys[nChanges] = DictKeys[ITzOffset];
            values[nChanges++] = CFNumberCreate(NULL, kCFNumberIntType, &tzOffset);
        }
        if (changes & 8) {
            keys[nChanges] = DictKeys[ICurrentPlan];
            values[nChanges++] = CFNumberCreate(NULL, kCFNumberCharType, &plan);
        }
        if (changes & 16) {
            keys[nChanges] = DictKeys[ICompletedWorkouts];
            values[nChanges++] = CFNumberCreate(NULL, kCFNumberCharType, &completed);
        }
        if (changes & 32) {
            keys[nChanges] = DictKeys[IDarkMode];
            values[nChanges++] = CFNumberCreate(NULL, kCFNumberCharType, &savedDM);
        }
        CFMutableDictionaryRef dict = CFDictionaryCreateMutableCopy(NULL, 10, savedInfo);
        saveChanges(dict, keys, values, nChanges);
    }

    UserInfo data = {planStart, weekStart, {0}, savedDM, plan, completed};
    for (int i = 0; i < 4; ++i) {
        value = CFDictionaryGetValue(savedInfo, DictKeys[6 + i]);
        CFNumberGetValue(value, kCFNumberShortType, &data.liftMaxes[i]);
    }

    userData = malloc(sizeof(UserInfo));
    memcpy(userData, &data, sizeof(UserInfo));
    *dataOut = userData;
    *weekInPlan = week;
    CFRelease(savedInfo);
    return tzDiff;
}

bool appUserData_deleteSavedData(void) {
    if (userData->completedWorkouts) {
        userData->completedWorkouts = 0;
        CFStringRef keys[] = {DictKeys[ICompletedWorkouts]};
        CFNumberRef values[] = {CFNumberCreate(NULL, kCFNumberCharType, &(unsigned char){0})};
        saveChanges(createMutableDict(), keys, values, 1);
        return true;
    }
    return false;
}

unsigned char appUserData_addCompletedWorkout(unsigned char day) {
    unsigned char completed = userData->completedWorkouts;
    completed |= (1 << day);
    userData->completedWorkouts = completed;
    CFStringRef keys[] = {DictKeys[ICompletedWorkouts]};
    CFNumberRef values[] = {CFNumberCreate(NULL, kCFNumberCharType, &completed)};
    saveChanges(createMutableDict(), keys, values, 1);
    return completed;
}

static int updateWeight(short *weights, short *output, CFStringRef *keys, CFNumberRef *values) {
    int nChanges = 0;
    short *lifts = userData->liftMaxes;
    for (int i = 0, keyIndex = ILiftArray; i < 4; ++i, ++keyIndex) {
        short new = weights[i];
        short old = lifts[i];
        if (new > old) {
            lifts[i] = new;
            output[i] = new;
            keys[nChanges] = DictKeys[keyIndex];
            values[nChanges++] = CFNumberCreate(NULL, kCFNumberShortType, &new);
        } else {
            output[i] = old;
        }
    }
    return nChanges;
}

bool appUserData_updateWeightMaxes(short *weights, short *output) {
    CFStringRef keys[4];
    CFNumberRef values[4];
    int nChanges = updateWeight(weights, output, keys, values);
    if (nChanges)
        saveChanges(createMutableDict(), keys, values, nChanges);
    return nChanges;
}

unsigned char appUserData_updateUserSettings(unsigned char plan,
                                             unsigned char darkMode, short *weights) {
    int nChanges = 0;
    CFStringRef keys[7];
    CFNumberRef values[7];
    unsigned char changes = plan != userData->currentPlan ? 1 : 0;
    if (changes) {
        keys[0] = DictKeys[ICurrentPlan];
        values[nChanges++] = CFNumberCreate(NULL, kCFNumberCharType, &plan);
        userData->currentPlan = plan;
        if (!(plan & 128)) {
#if TARGET_OS_SIMULATOR
            time_t newPlanStart = userData->weekStart;
            exerciseManager_setCurrentWeek(0);
#else
            time_t newPlanStart = userData->weekStart + WeekSeconds;
#endif
            keys[1] = DictKeys[IPlanStart];
            values[nChanges++] = CFNumberCreate(NULL, kCFNumberLongType, &newPlanStart);
            userData->planStart = newPlanStart;
        }
    }

    if (darkMode != userData->darkMode) {
        changes |= 2;
        keys[nChanges] = DictKeys[IDarkMode];
        values[nChanges++] = CFNumberCreate(NULL, kCFNumberCharType, &darkMode);
        userData->darkMode = darkMode;
    }

    nChanges += updateWeight(weights, (short[]){0,0,0,0}, &keys[nChanges], &values[nChanges]);
    if (nChanges)
        saveChanges(createMutableDict(), keys, values, nChanges);
    return changes;
}
