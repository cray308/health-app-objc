#include "UserData.h"
#include "BaseMacros.h"

#define NKeys 10

#if TARGET_OS_SIMULATOR
void exerciseManager_setCurrentWeek(int week);
#define setCurrentWeek() exerciseManager_setCurrentWeek(0);
#define getNewPlanStart(s) (s)
#else
#define setCurrentWeek()
#define getNewPlanStart(s) getWeekStart(s + WeekSeconds + HourSeconds, &(int){0}, &(bool){0})
#endif

#if DEBUG
int _HATestIsMetric = 0;
#endif

enum {
    IndexWeekStart,
    IndexPlanStart,
    IndexTzOffset,
    IndexDST,
    IndexCurrentPlan,
    IndexCompletedWorkouts,
    IndexLifts
};

enum {
    PlanBaseBuilding,
    PlanContinuation
};

static const void *UDKeys[] = {
    CFSTR("weekStart"),
    CFSTR("planStart"),
    CFSTR("tzOffset"),
    CFSTR("isDST"),
    CFSTR("currentPlan"),
    CFSTR("completedWorkouts"),
    CFSTR("squatMax"),
    CFSTR("pullUpMax"),
    CFSTR("benchMax"),
    CFSTR("deadliftMax")
};

static CFMutableDictionaryRef createMutableDict(void) CF_RETURNS_RETAINED {
    CFDictionaryRef prefs = CFPreferencesCopyAppValue(PrefsKey, kCFPreferencesCurrentApplication);
    CFMutableDictionaryRef res = CFDictionaryCreateMutableCopy(NULL, 11, prefs);
    CFRelease(prefs);
    return res;
}

static void saveChanges(CFMutableDictionaryRef dict CF_CONSUMED,
                        CFStringRef const *keys, CFNumberRef *values, int count) {
    for (int i = 0; i < count; ++i) {
        CFDictionaryReplaceValue(dict, keys[i], values[i]);
        CFRelease(values[i]);
    }
    CFPreferencesSetAppValue(PrefsKey, dict, kCFPreferencesCurrentApplication);
    CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
    CFRelease(dict);
}

static int updateWeights(int *lifts, int const *newWeights,
                         CFStringRef *keys, CFNumberRef *values, bool canDecrease) {
    int nChanges = 0;
    for (int i = 0, keyIndex = IndexLifts; i < 4; ++i, ++keyIndex) {
        int new = newWeights[i];
        if (new > lifts[i] || (canDecrease && new < lifts[i])) {
            lifts[i] = new;
            keys[nChanges] = UDKeys[keyIndex];
            values[nChanges++] = CFNumberCreate(NULL, kCFNumberIntType, &new);
        }
    }
    return nChanges;
}

static time_t getWeekStart(time_t date, int *tzOffset, bool *isDST) {
    struct tm tmInfo = {.tm_isdst = -1};
    localtime_r(&date, &tmInfo);
    if (tmInfo.tm_wday != 1) {
        date = date - WeekSeconds + (((8 - tmInfo.tm_wday) % 7) * DaySeconds);
        localtime_r(&date, &tmInfo);
    }
    *tzOffset = (int)tmInfo.tm_gmtoff;
    *isDST = tmInfo.tm_isdst;
    return date - (tmInfo.tm_hour * HourSeconds) - (tmInfo.tm_min * 60) - tmInfo.tm_sec;
}

void userData_create(UserData *res) {
    int tzOffset;
    bool isDST;
    time_t weekStart = getWeekStart(time(NULL), &tzOffset, &isDST);
    res->weekStart = res->planStart = weekStart;
    res->plan = UCHAR_MAX;

    const void *values[] = {
        CFNumberCreate(NULL, kCFNumberLongType, &weekStart),
        CFNumberCreate(NULL, kCFNumberLongType, &weekStart),
        CFNumberCreate(NULL, kCFNumberIntType, &tzOffset),
        CFNumberCreate(NULL, kCFNumberCharType, &isDST),
        CFNumberCreate(NULL, kCFNumberCharType, &(uint8_t){UCHAR_MAX}),
        CFNumberCreate(NULL, kCFNumberCharType, &(uint8_t){0}),
        CFNumberCreate(NULL, kCFNumberIntType, &(int){0}),
        CFNumberCreate(NULL, kCFNumberIntType, &(int){0}),
        CFNumberCreate(NULL, kCFNumberIntType, &(int){0}),
        CFNumberCreate(NULL, kCFNumberIntType, &(int){0})
    };
    CFDictionaryRef dict = CFDictionaryCreate(NULL, UDKeys, values, NKeys,
                                              &kCFCopyStringDictionaryKeyCallBacks, NULL);
    CFPreferencesSetAppValue(PrefsKey, dict, kCFPreferencesCurrentApplication);
    CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
    CFRelease(dict);
    for (int i = 0; i < NKeys; ++i) {
        CFRelease(values[i]);
    }
}

int userData_init(UserData *res, CFDictionaryRef prefs, int *week) {
    int tzOffset, savedOffset;
    bool isDST, wasDST;
    res->weekStart = getWeekStart(time(NULL), &tzOffset, &isDST);
    time_t savedWeekStart;
    getDictValue(prefs, UDKeys[IndexWeekStart], kCFNumberLongType, &savedWeekStart);
    getDictValue(prefs, UDKeys[IndexPlanStart], kCFNumberLongType, &res->planStart);
    getDictValue(prefs, UDKeys[IndexTzOffset], kCFNumberIntType, &savedOffset);
    getDictValue(prefs, UDKeys[IndexDST], kCFNumberCharType, &wasDST);
    getDictValue(prefs, UDKeys[IndexCurrentPlan], kCFNumberCharType, &res->plan);
    getDictValue(prefs, UDKeys[IndexCompletedWorkouts], kCFNumberCharType, &res->completedWorkouts);

    uint8_t changes = 0;
    int tzDiff = savedOffset - tzOffset, dstChange = isDST - wasDST;
    if (dstChange) {
        changes = MaskDST;
        if (abs(tzDiff) != HourSeconds) {
            tzDiff += dstChange * HourSeconds;
        } else {
            tzDiff = 0;
            changes |= MaskTzOffset;
        }
    }

    if (tzDiff) {
        res->planStart += tzDiff;
        changes |= (MaskTzOffset | MaskPlanStart);
        if (res->weekStart != savedWeekStart) {
            changes |= MaskWeekStart;
            savedWeekStart += tzDiff;
        }
    }

    *week = (int)((res->weekStart - res->planStart + HourSeconds) / WeekSeconds);
    if (res->weekStart != savedWeekStart) {
        changes |= (MaskCompletedWorkouts | MaskWeekStart);
        res->completedWorkouts = 0;

        if (isCharValid(res->plan) && *week >= (int []){8, 13}[res->plan]) {
            if (res->plan == PlanBaseBuilding) {
                res->plan = PlanContinuation;
                changes |= MaskCurrentPlan;
            }
            res->planStart = res->weekStart;
            changes |= MaskPlanStart;
            *week = 0;
        }
    }

    if (changes) {
        CFStringRef keys[IndexLifts];
        CFNumberRef values[IndexLifts];
        long keyTypes[] = {
            kCFNumberLongType,
            kCFNumberLongType,
            kCFNumberIntType,
            kCFNumberCharType,
            kCFNumberCharType,
            kCFNumberCharType
        };
        void *valueRefs[] = {
            &res->weekStart, &res->planStart, &tzOffset, &isDST, &res->plan, &res->completedWorkouts
        };
        int nChanges = 0;

        for (uint8_t i = 0; i < IndexLifts; ++i) {
            if (changes & (1 << i)) {
                keys[nChanges] = UDKeys[i];
                values[nChanges++] = CFNumberCreate(NULL, keyTypes[i], valueRefs[i]);
            }
        }
        saveChanges(CFDictionaryCreateMutableCopy(NULL, 11, prefs), keys, values, nChanges);
    }

    for (int i = 0; i < 4; ++i) {
        getDictValue(prefs, UDKeys[IndexLifts + i], kCFNumberIntType, &res->lifts[i]);
    }
    CFRelease(prefs);
    return tzDiff;
}

bool userData_update(UserData *m, uint8_t plan, int const *weights) {
    CFStringRef keys[IndexLifts];
    CFNumberRef values[IndexLifts];
    int nChanges = 0;
    bool res = plan != m->plan;
    if (res) {
        keys[0] = UDKeys[IndexCurrentPlan];
        values[nChanges++] = CFNumberCreate(NULL, kCFNumberCharType, &plan);
        m->plan = plan;
        if (isCharValid(plan)) {
            setCurrentWeek()
            m->planStart = getNewPlanStart(m->weekStart);
            keys[1] = UDKeys[IndexPlanStart];
            values[nChanges++] = CFNumberCreate(NULL, kCFNumberLongType, &m->planStart);
        }
    }

    nChanges += updateWeights(m->lifts, weights, &keys[nChanges], &values[nChanges], true);
    if (nChanges) saveChanges(createMutableDict(), keys, values, nChanges);
    return res;
}

bool userData_clear(UserData *m) {
    if (m->completedWorkouts) {
        m->completedWorkouts = 0;
        CFNumberRef values[] = {CFNumberCreate(NULL, kCFNumberCharType, &(uint8_t){0})};
        saveChanges(createMutableDict(), (CFStringRef []){UDKeys[IndexCompletedWorkouts]}, values, 1);
        return true;
    }
    return false;
}

uint8_t userData_addWorkoutData(UserData *m, uint8_t day, int const *weights, bool *updatedWeights) {
    CFStringRef keys[5];
    CFNumberRef values[5];
    int nChanges = 0;
    if (weights) *updatedWeights = (nChanges = updateWeights(m->lifts, weights, keys, values, false));

    uint8_t completed = 0;
    if (day != UCHAR_MAX) {
        completed = (uint8_t)(m->completedWorkouts | (1 << day));
        m->completedWorkouts = completed;
        keys[nChanges] = UDKeys[IndexCompletedWorkouts];
        values[nChanges++] = CFNumberCreate(NULL, kCFNumberCharType, &completed);
    }

    if (nChanges) saveChanges(createMutableDict(), keys, values, nChanges);
    return completed;
}
