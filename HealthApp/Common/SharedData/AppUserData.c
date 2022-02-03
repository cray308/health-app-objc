#include "AppUserData.h"
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CFSet.h>
#include <stdlib.h>
#include "CocoaHelpers.h"

#define DaySeconds 86400

extern id HKQuantityTypeIdentifierBodyMass;
extern id HKSampleSortIdentifierStartDate;

enum {
    WorkoutPlanBaseBuilding = 0,
    WorkoutPlanContinuation = 1
};

UserInfo *userData = NULL;
static CFStringRef const dictKey = CFSTR("userinfo");

static const void *keys[] = {
    CFSTR("planStart"), CFSTR("weekStart"), CFSTR("tzOffset"),
    CFSTR("darkMode"), CFSTR("currentPlan"), CFSTR("completedWorkouts"),
    CFSTR("squatMax"), CFSTR("pullUpMax"), CFSTR("benchMax"), CFSTR("deadliftMax")
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

static time_t date_calcStartOfWeek(time_t date) {
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
    CFNumberRef values[] = {
        CFNumberCreate(NULL, kCFNumberLongType, &userData->planStart),
        CFNumberCreate(NULL, kCFNumberLongType, &userData->weekStart),
        CFNumberCreate(NULL, kCFNumberIntType, &userData->tzOffset),
        CFNumberCreate(NULL, kCFNumberCharType, &userData->darkMode),
        CFNumberCreate(NULL, kCFNumberCharType, &userData->currentPlan),
        CFNumberCreate(NULL, kCFNumberCharType, &userData->completedWorkouts),
        CFNumberCreate(NULL, kCFNumberShortType, &userData->liftMaxes[0]),
        CFNumberCreate(NULL, kCFNumberShortType, &userData->liftMaxes[1]),
        CFNumberCreate(NULL, kCFNumberShortType, &userData->liftMaxes[2]),
        CFNumberCreate(NULL, kCFNumberShortType, &userData->liftMaxes[3])
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

static void getCurrentWeight(id store, id weightType) {
    CFArrayRef sortArr = createSortDescriptors((CFStringRef)HKSampleSortIdentifierStartDate, false);
    id _obj = allocClass(objc_getClass("HKSampleQuery"));
    id req = (((id(*)(id,SEL,id,id,unsigned long,CFArrayRef,void(^)(id,CFArrayRef,id)))objc_msgSend)
              (_obj, sel_getUid("initWithSampleType:predicate:limit:sortDescriptors:resultsHandler:"),
               weightType, nil, 1, sortArr, ^(id query _U_, CFArrayRef data, id error _U_) {
        if (data && CFArrayGetCount(data)) {
            id unit = staticMethod(objc_getClass("HKUnit"), sel_getUid("poundUnit"));
            id sample = (id) CFArrayGetValueAtIndex(data, 0);
            id quantity = getObject(sample, sel_getUid("quantity"));
            userData->bodyweight = (short) (((double(*)(id,SEL,id))objc_msgSend)
                                            (quantity, sel_getUid("doubleValueForUnit:"), unit));
        }
        releaseObj(store);
    }));
    setObject(store, sel_getUid("executeQuery:"), req);
    CFRelease(sortArr);
    releaseObj(req);
}

static void getHealthData(void) {
    userData->bodyweight = -1;
    Class storeClass = objc_getClass("HKHealthStore");
    if (!((bool(*)(Class,SEL))objc_msgSend)(storeClass, sel_getUid("isHealthDataAvailable")))
        return;

    id store = createNew(storeClass);
    id weightType = staticMethodWithString(objc_getClass("HKSampleType"),
                                           sel_getUid("quantityTypeForIdentifier:"),
                                           (CFStringRef) HKQuantityTypeIdentifierBodyMass);
    CFSetRef set = CFSetCreate(NULL, (const void *[]){weightType}, 1, NULL);

    (((void(*)(id,SEL,CFSetRef,CFSetRef,void(^)(bool,id)))objc_msgSend)
     (store, sel_getUid("requestAuthorizationToShareTypes:readTypes:completion:"),
      NULL, set, ^(bool granted, id error _U_) {
        if (granted) {
            getCurrentWeight(store, weightType);
        } else {
            releaseObj(store);
        }
    }));
    CFRelease(set);
}

void userInfo_create(bool darkMode) {
    time_t now = time(NULL);
    userData = calloc(1, sizeof(UserInfo));
    userData->currentPlan = -1;
    userData->weekStart = date_calcStartOfWeek(now);
    userData->planStart = userData->weekStart;
    userData->tzOffset = date_getOffsetFromGMT(now);
    userData->darkMode = darkMode ? 0 : -1;
    saveData();
    getHealthData();
}

int userInfo_initFromStorage(void) {
    const int planLengths[] = {8, 13};
    time_t now = time(NULL);
    time_t weekStart = date_calcStartOfWeek(now);
    id defaults = getUserDefaults();
    CFDictionaryRef savedInfo = (((CFDictionaryRef(*)(id,SEL,CFStringRef))objc_msgSend)
                                 (defaults, sel_getUid("dictionaryForKey:"), dictKey));
    userData = malloc(sizeof(UserInfo));

    CFNumberRef value = CFDictionaryGetValue(savedInfo, keys[0]);
    CFNumberGetValue(value, kCFNumberLongType, &userData->planStart);
    value = CFDictionaryGetValue(savedInfo, keys[1]);
    CFNumberGetValue(value, kCFNumberLongType, &userData->weekStart);
    value = CFDictionaryGetValue(savedInfo, keys[2]);
    CFNumberGetValue(value, kCFNumberIntType, &userData->tzOffset);
    value = CFDictionaryGetValue(savedInfo, keys[3]);
    CFNumberGetValue(value, kCFNumberCharType, &userData->darkMode);
    value = CFDictionaryGetValue(savedInfo, keys[4]);
    CFNumberGetValue(value, kCFNumberCharType, &userData->currentPlan);
    value = CFDictionaryGetValue(savedInfo, keys[5]);
    CFNumberGetValue(value, kCFNumberCharType, &userData->completedWorkouts);
    for (int i = 0; i < 4; ++i) {
        value = CFDictionaryGetValue(savedInfo, keys[6 + i]);
        CFNumberGetValue(value, kCFNumberShortType, &userData->liftMaxes[i]);
    }

    int newOffset = date_getOffsetFromGMT(now);
    int tzDiff = userData->tzOffset - newOffset;
    bool madeChange = false;
    if (tzDiff) {
        madeChange = true;
        userData->weekStart += tzDiff;
        userData->planStart += tzDiff;
        userData->tzOffset = newOffset;
    }

    userData->week = (short) ((weekStart - userData->planStart) / WeekSeconds);
    if (weekStart != userData->weekStart) {
        madeChange = true;
        userData->completedWorkouts = 0;
        userData->weekStart = weekStart;

        if (userData->currentPlan != -1 && userData->week >= planLengths[userData->currentPlan]) {
            if (userData->currentPlan == WorkoutPlanBaseBuilding)
                userData->currentPlan = WorkoutPlanContinuation;
            userData->planStart = weekStart;
            userData->week = 0;
        }
    }

    if (madeChange)
        saveData();
    getHealthData();
    return tzDiff;
}

void appUserData_deleteSavedData(void) {
    if (userData->completedWorkouts) {
        userData->completedWorkouts = 0;
        saveData();
    }
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

bool appUserData_updateWeightMaxes(short *weights) {
    bool madeChanges = false;
    for (int i = 0; i < 4; ++i) {
        if (weights[i] > userData->liftMaxes[i]) {
            madeChanges = true;
            userData->liftMaxes[i] = weights[i];
        }
    }
    if (madeChanges)
        saveData();
    return madeChanges;
}

bool appUserData_updateUserSettings(signed char plan, signed char darkMode, short *weights) {
    bool madeChanges = plan != userData->currentPlan;
    if (plan != -1 && madeChanges) {
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
        saveData();
    return rv;
}
