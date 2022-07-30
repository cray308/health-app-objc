#include "UserData.h"
#include <CoreFoundation/CoreFoundation.h>

#define NKeys 9

#if TARGET_OS_SIMULATOR
void exerciseManager_setCurrentWeek(int week);
#define SET_CURR_WEEK() exerciseManager_setCurrentWeek(0);
#define NEW_PLAN_START(val) val
#else
#define SET_CURR_WEEK()
#define NEW_PLAN_START(val) val + WeekSeconds
#endif

float toSavedMass = 1;

enum {
    IWeekStart,
    IPlanStart,
    ITzOffset,
    ICurrentPlan,
    ICompletedWorkouts,
    ILiftArray
};

enum {
    PlanBaseBuilding,
    PlanContinuation
};

static CFStringRef const dictKey = CFSTR("userinfo");
static const void *DictKeys[] = {
    CFSTR("weekStart"), CFSTR("planStart"), CFSTR("tzOffset"),
    CFSTR("currentPlan"), CFSTR("completedWorkouts"),
    CFSTR("squatMax"), CFSTR("pullUpMax"), CFSTR("benchMax"), CFSTR("deadliftMax")
};

static CFMutableDictionaryRef createMutableDict(void) CF_RETURNS_RETAINED {
    CFDictionaryRef prefs = CFPreferencesCopyAppValue(dictKey, kCFPreferencesCurrentApplication);
    CFMutableDictionaryRef res = CFDictionaryCreateMutableCopy(NULL, 10, prefs);
    CFRelease(prefs);
    return res;
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

static int updateWeight(short *curr, short *weights, CFStringRef *keys, CFNumberRef *values) {
    int nChanges = 0;
    for (int i = 0, keyIndex = ILiftArray; i < 4; ++i, ++keyIndex) {
        short new = weights[i];
        if (new > curr[i]) {
            curr[i] = new;
            keys[nChanges] = DictKeys[keyIndex];
            values[nChanges++] = CFNumberCreate(NULL, kCFNumberShortType, &new);
        }
    }
    return nChanges;
}

static time_t getWeekStart(int *tzOffset) {
    time_t now = time(NULL);
    struct tm tmInfo;
    localtime_r(&now, &tmInfo);
    *tzOffset = (int)tmInfo.tm_gmtoff;
    if (tmInfo.tm_wday != 1) {
        now = now - WeekSeconds + (((8 - tmInfo.tm_wday) % 7) * DaySeconds);
        localtime_r(&now, &tmInfo);
    }
    return now - ((tmInfo.tm_hour * 3600) + (tmInfo.tm_min * 60) + tmInfo.tm_sec);
}

void userInfo_create(UserInfo *out, time_t *weekStart) {
    int tzOffset = 0;
    time_t start = getWeekStart(&tzOffset);
    *weekStart = start;

    UserInfo data = {start, start, {0}, UCHAR_MAX, 0};
    const void *values[] = {
        CFNumberCreate(NULL, kCFNumberLongType, &start),
        CFNumberCreate(NULL, kCFNumberLongType, &start),
        CFNumberCreate(NULL, kCFNumberIntType, &tzOffset),
        CFNumberCreate(NULL, kCFNumberCharType, &(unsigned char){UCHAR_MAX}),
        CFNumberCreate(NULL, kCFNumberCharType, &(unsigned char){0}),
        CFNumberCreate(NULL, kCFNumberShortType, &(short){0}),
        CFNumberCreate(NULL, kCFNumberShortType, &(short){0}),
        CFNumberCreate(NULL, kCFNumberShortType, &(short){0}),
        CFNumberCreate(NULL, kCFNumberShortType, &(short){0})
    };
    CFDictionaryRef dict = CFDictionaryCreate(
      NULL, DictKeys, values, NKeys, &kCFCopyStringDictionaryKeyCallBacks, NULL);
    CFNumberRef hasLaunched = CFNumberCreate(NULL, kCFNumberCharType, &(bool){true});
    CFPreferencesSetAppValue(HasLaunchedKey, hasLaunched, kCFPreferencesCurrentApplication);
    CFPreferencesSetAppValue(dictKey, dict, kCFPreferencesCurrentApplication);
    CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
    CFRelease(hasLaunched);
    CFRelease(dict);
    for (int i = 0; i < NKeys; ++i) {
        CFRelease(values[i]);
    }
    memcpy(out, &data, sizeof(UserInfo));
}

int userInfo_init(UserInfo *out, time_t *weekStart, int *week) {
    int tzOffset = 0;
    time_t start = getWeekStart(&tzOffset);

    const int planLengths[] = {8, 13};
    int savedOffset;
    time_t savedWeekStart, planStart;
    unsigned char changes = 0, plan, completed;
    CFDictionaryRef dict = CFPreferencesCopyAppValue(dictKey, kCFPreferencesCurrentApplication);
    CFNumberGetValue(CFDictionaryGetValue(dict, DictKeys[IWeekStart]),
                     kCFNumberLongType, &savedWeekStart);
    CFNumberGetValue(CFDictionaryGetValue(dict, DictKeys[IPlanStart]), kCFNumberLongType, &planStart);
    CFNumberGetValue(CFDictionaryGetValue(dict, DictKeys[ITzOffset]), kCFNumberIntType, &savedOffset);
    CFNumberGetValue(CFDictionaryGetValue(dict, DictKeys[ICurrentPlan]), kCFNumberCharType, &plan);
    CFNumberGetValue(CFDictionaryGetValue(dict, DictKeys[ICompletedWorkouts]),
                     kCFNumberCharType, &completed);

    int tzDiff = savedOffset - tzOffset;
    if (tzDiff) {
        planStart += tzDiff;
        changes = (MaskTzOffset | MaskPlanStart);
        if (start != savedWeekStart) {
            changes |= MaskWeekStart;
            savedWeekStart += tzDiff;
        } else {
            tzDiff = 0;
        }
    }

    int weekInPlan = (int)((start - planStart) / WeekSeconds);
    if (start != savedWeekStart) {
        changes |= (MaskCompletedWorkouts | MaskWeekStart);
        completed = 0;

        if (isCharValueValid(plan) && weekInPlan >= planLengths[plan]) {
            if (plan == PlanBaseBuilding) {
                plan = PlanContinuation;
                changes |= MaskCurrentPlan;
            }
            planStart = start;
            changes |= MaskPlanStart;
            weekInPlan = 0;
        }
    }

    if (changes) {
        CFStringRef keys[ILiftArray];
        CFNumberRef values[ILiftArray];
        int nChanges = 0;

        if (changes & MaskWeekStart) {
            keys[0] = DictKeys[IWeekStart];
            values[nChanges++] = CFNumberCreate(NULL, kCFNumberLongType, &start);
        }
        if (changes & MaskPlanStart) {
            keys[nChanges] = DictKeys[IPlanStart];
            values[nChanges++] = CFNumberCreate(NULL, kCFNumberLongType, &planStart);
        }
        if (changes & MaskTzOffset) {
            keys[nChanges] = DictKeys[ITzOffset];
            values[nChanges++] = CFNumberCreate(NULL, kCFNumberIntType, &tzOffset);
        }
        if (changes & MaskCurrentPlan) {
            keys[nChanges] = DictKeys[ICurrentPlan];
            values[nChanges++] = CFNumberCreate(NULL, kCFNumberCharType, &plan);
        }
        if (changes & MaskCompletedWorkouts) {
            keys[nChanges] = DictKeys[ICompletedWorkouts];
            values[nChanges++] = CFNumberCreate(NULL, kCFNumberCharType, &completed);
        }
        CFMutableDictionaryRef updates = CFDictionaryCreateMutableCopy(NULL, 10, dict);
        saveChanges(updates, keys, values, nChanges);
    }

    UserInfo data = {planStart, start, {0}, plan, completed};
    for (int i = 0; i < 4; ++i) {
        const void *value = CFDictionaryGetValue(dict, DictKeys[ILiftArray + i]);
        CFNumberGetValue(value, kCFNumberShortType, &data.liftMaxes[i]);
    }
    memcpy(out, &data, sizeof(UserInfo));
    CFRelease(dict);
    *weekStart = start;
    *week = weekInPlan;
    return tzDiff;
}

bool userInfo_update(UserInfo *m, unsigned char plan, short *weights) {
    int nChanges = 0;
    bool res = plan != m->currentPlan;
    CFStringRef keys[ILiftArray + 1];
    CFNumberRef values[ILiftArray + 1];
    if (res) {
        keys[0] = DictKeys[ICurrentPlan];
        values[nChanges++] = CFNumberCreate(NULL, kCFNumberCharType, &plan);
        m->currentPlan = plan;
        if (isCharValueValid(plan)) {
            SET_CURR_WEEK()
            time_t newPlanStart = NEW_PLAN_START(m->weekStart);
            keys[1] = DictKeys[IPlanStart];
            values[nChanges++] = CFNumberCreate(NULL, kCFNumberLongType, &newPlanStart);
            m->planStart = newPlanStart;
        }
    }

    nChanges += updateWeight(m->liftMaxes, weights, &keys[nChanges], &values[nChanges]);
    if (nChanges) saveChanges(createMutableDict(), keys, values, nChanges);
    return res;
}

bool userInfo_deleteData(UserInfo *m) {
    if (m->completedWorkouts) {
        m->completedWorkouts = 0;
        CFNumberRef vals[] = {CFNumberCreate(NULL, kCFNumberCharType, &(unsigned char){0})};
        saveChanges(createMutableDict(), (CFStringRef[]){DictKeys[ICompletedWorkouts]}, vals, 1);
        return true;
    }
    return false;
}

unsigned char userInfo_addWorkoutData(UserInfo *m,
                                      unsigned char day, short *weights, bool *updatedWeights) {
    unsigned char completed = 0;
    CFStringRef keys[5];
    CFNumberRef values[5];
    int nChanges = 0;
    *updatedWeights = (weights && (nChanges = updateWeight(m->liftMaxes, weights, keys, values)));

    if (day != UCHAR_MAX) {
        completed = (unsigned char)(m->completedWorkouts | (1 << day));
        m->completedWorkouts = completed;
        keys[nChanges] = DictKeys[ICompletedWorkouts];
        values[nChanges++] = CFNumberCreate(NULL, kCFNumberCharType, &completed);
    }
    if (nChanges) saveChanges(createMutableDict(), keys, values, nChanges);
    return completed;
}
