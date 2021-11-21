#include "PersistenceService.h"
#if DEBUG
#include <stdlib.h>
extern time_t date_calcStartOfWeek(time_t date);
#endif
#include "AppUserData.h"
#include "CocoaHelpers.h"

#define deleteWeekData(_d) setObject(backgroundContext, sel_getUid("deleteObject:"), _d)

#define setPredicate(_req, _pred) setObject(_req, sel_getUid("setPredicate:"), _pred)

#define weekData_setWeekStart(_d, _val) setInt64(_d, sel_getUid("setWeekStart:"), _val)

static id persistenceService = nil;
id backgroundContext = nil;

static inline id createWeekData(void) {
    id _obj = allocClass(objc_getClass("WeeklyData"));
    return getObjectWithObject(_obj, sel_getUid("initWithContext:"), backgroundContext);
}

void weekData_getLiftingLimits(id data, int16_t *output) {
    static char const *getterStrs[] = {"bestSquat", "bestPullup", "bestBench", "bestDeadlift"};
    for (int i = 0; i < 4; ++i)
        output[i] = getInt16(data, sel_getUid(getterStrs[i]));
}

int16_t weekData_getWorkoutTimeForType(id data, unsigned char type) {
    static char const *getterStrs[] = {"timeStrength", "timeSE", "timeEndurance", "timeHIC"};
    return getInt16(data, sel_getUid(getterStrs[type]));
}

void weekData_setWorkoutTimeForType(id data, unsigned char type, int16_t duration) {
    static char const *setters[] = {
        "setTimeStrength:", "setTimeSE:", "setTimeEndurance:", "setTimeHIC:"
    };
    int16_t newDuration = weekData_getWorkoutTimeForType(data,type) + duration;
    setInt16(data, sel_getUid(setters[type]), newDuration);
}

void weekData_setLiftingMaxArray(id data, int16_t *weights) {
    static char const *setterStrs[] = {
        "setBestSquat:", "setBestPullup:", "setBestBench:", "setBestDeadlift:"
    };
    for (int i = 0; i < 4; ++i)
        setInt16(data, sel_getUid(setterStrs[i]), weights[i]);
}

void persistenceService_saveContext(void) {
    if (getBool(backgroundContext, sel_getUid("hasChanges")))
        setObject(backgroundContext, sel_getUid("save:"), nil);
}

id fetchRequest(id predicate) {
    id request = staticMethodWithString(objc_getClass("NSFetchRequest"),
                                        sel_getUid("fetchRequestWithEntityName:"),
                                        CFSTR("WeeklyData"));
    if (predicate)
        setPredicate(request, predicate);
    return request;
}

#if DEBUG
void persistenceService_create(void) {
    runInBackground((^{
        int16_t lifts[] = {300, 20, 185, 235};
        int i = 0;
        unsigned char plan = 0;
        time_t start = date_calcStartOfWeek(time(NULL) - 126489600);
        time_t end = date_calcStartOfWeek(time(NULL) - 2678400);

        while (start < end) {
            int16_t totalWorkouts = 0;
            int16_t times[4] = {0};
            id data = createWeekData();
            weekData_setWeekStart(data, start);

            if (plan == 0) {
                for (int j = 0; j < 6; ++j) {
                    int extra = 10;
                    bool didSE = true;
                    switch (j) {
                        case 1:
                        case 2:
                        case 5:
                            times[2] += ((rand() % 30) + 30);
                            totalWorkouts += 1;
                            break;
                        case 4:
                            if ((didSE = (rand() % 10 >= 5))) extra = 0;
                        case 0:
                        case 3:
                            if (didSE) {
                                times[1] += ((rand() % 20) + extra);
                                totalWorkouts += 1;
                            }
                        default:
                            break;
                    }
                }
            } else {
                for (int j = 0; j < 6; ++j) {
                    switch (j) {
                        case 0:
                        case 2:
                        case 4:
                            times[0] += ((rand() % 20) + 20);
                            totalWorkouts += 1;
                            break;
                        case 1:
                        case 3:
                            times[3] += ((rand() % 20) + 15);
                            totalWorkouts += 1;
                            break;
                        case 5:
                            times[2] += ((rand() % 30) + 60);
                            totalWorkouts += 1;
                        default:
                            break;
                    }
                }
            }

            if (i == 7) {
                plan = 1;
            } else if (i == 20 || i == 32 || i == 44) {
                lifts[2] = 185 + (rand() % 50);
                lifts[1] = 20 + (rand() % 20);
                lifts[0] = 300 + (rand() % 80);
                lifts[3] = 235 + (rand() % 50);
            }
            weekData_setTotalWorkouts(data, totalWorkouts);
            weekData_setLiftingMaxArray(data, lifts);
            for (int l = 0; l < 4; ++l) {
                weekData_setWorkoutTimeForType(data, l, times[l]);
            }
            releaseObj(data);

            if (++i == 52) {
                i = 0;
                plan = 0;
            }
            start += WeekSeconds;
        }
        persistenceService_saveContext();
    }));
}
#endif

void persistenceService_init(void) {
    id _container = allocClass(objc_getClass("NSPersistentContainer"));
    persistenceService = ((id(*)(id,SEL,CFStringRef))objc_msgSend)
    (_container, sel_getUid("initWithName:"), CFSTR("HealthApp"));
    ((void(*)(id,SEL,void(^)(id,id)))objc_msgSend)
    (persistenceService, sel_getUid("loadPersistentStoresWithCompletionHandler:"),
     ^(id description _U_, id error _U_) {});
    backgroundContext = getObject(persistenceService, sel_getUid("newBackgroundContext"));
    setBool(backgroundContext, sel_getUid("setAutomaticallyMergesChangesFromParent:"), true);
}

void persistenceService_start(int tzOffset, void (*completion)(void*), void *receiver) {
    runInBackground((^{
        int count = 0;
        CFArrayRef data;
        id request = fetchRequest(nil);
        if (tzOffset) {
            data = persistenceService_executeFetchRequest(request, &count, false);
            if (data) {
                for (int i = 0; i < count; ++i) {
                    id d = (id) CFArrayGetValueAtIndex(data, i);
                    int64_t start = weekData_getWeekStart(d);
                    start += tzOffset;
                    weekData_setWeekStart(d, start);
                }
            }
        }

        long endPt = userData->weekStart - 63244800;
        id pred = createPredicate(CFSTR("weekStart < %lld"), endPt);
        setPredicate(request, pred);
        data = persistenceService_executeFetchRequest(request, &count, false);
        if (data) {
            for (int i = 0; i < count; ++i) {
                id d = (id) CFArrayGetValueAtIndex(data, i);
                deleteWeekData(d);
            }
            persistenceService_saveContext();
        }

        pred = createPredicate(CFSTR("weekStart < %lld"), userData->weekStart);
        setPredicate(request, pred);
        data = persistenceService_executeFetchRequest(request, &count, true);
        id currWeek = persistenceService_getCurrentWeek();
        bool newEntryForCurrentWeek = false;
        if (!currWeek) {
            newEntryForCurrentWeek = true;
            currWeek = createWeekData();
            weekData_setWeekStart(currWeek, userData->weekStart);
        }

        if (!data) {
            if (newEntryForCurrentWeek)
                releaseObj(currWeek);
            goto cleanup;
        }

        id last = (id) CFArrayGetValueAtIndex(data, count - 1);
        int16_t lastLifts[4];
        weekData_getLiftingLimits(last, lastLifts);

        time_t currStart = weekData_getWeekStart(last);
        currStart += WeekSeconds;
        for (; currStart < userData->weekStart; currStart += WeekSeconds) {
            id curr = createWeekData();
            weekData_setWeekStart(curr, currStart);
            weekData_setLiftingMaxArray(curr, lastLifts);
            releaseObj(curr);
        }

        if (newEntryForCurrentWeek) {
            weekData_setLiftingMaxArray(currWeek, lastLifts);
            releaseObj(currWeek);
        }

    cleanup:
        persistenceService_saveContext();
        completion(receiver);
    }));
}

void persistenceService_deleteUserData(void) {
    runInBackground((^{
        int count = 0;
        id request = fetchRequest(createPredicate(CFSTR("weekStart < %lld"), userData->weekStart));
        CFArrayRef data = persistenceService_executeFetchRequest(request, &count, false);

        if (data) {
            for (int i = 0; i < count; ++i) {
                id d = (id) CFArrayGetValueAtIndex(data, i);
                deleteWeekData(d);
            }
        }

        id currWeek = persistenceService_getCurrentWeek();
        weekData_setTotalWorkouts(currWeek, 0);
        for (int i = 0; i < 4; ++i)
        weekData_setWorkoutTimeForType(currWeek, i, 0);
        persistenceService_saveContext();
    }));
}

id persistenceService_getCurrentWeek(void) {
    id request = fetchRequest(createPredicate(CFSTR("weekStart == %lld"), userData->weekStart));
    CFArrayRef currentWeeks = persistenceService_executeFetchRequest(request, &(int){0}, false);
    if (!currentWeeks) return nil;
    return (id) CFArrayGetValueAtIndex(currentWeeks, 0);
}

CFArrayRef persistenceService_executeFetchRequest(id req, int *count, bool sorted) {
    int len = 0;
    if (sorted) {
        id _obj = allocClass(objc_getClass("NSSortDescriptor"));
        id descriptor = ((id(*)(id,SEL,CFStringRef,bool))objc_msgSend)
        (_obj, sel_getUid("initWithKey:ascending:"), CFSTR("weekStart"), true);
        CFArrayRef descriptorArr = CFArrayCreate(NULL, (const void *[]){descriptor},
                                                 1, &retainedArrCallbacks);
        setArray(req, sel_getUid("setSortDescriptors:"), descriptorArr);
        releaseObj(descriptor);
        CFRelease(descriptorArr);
    }
    CFArrayRef data = ((CFArrayRef(*)(id,SEL,id,id))objc_msgSend)
    (backgroundContext, sel_getUid("executeFetchRequest:error:"), req, nil);
    if (!(data && (len = (int)(CFArrayGetCount(data)))))
        data = NULL;
    *count = len;
    return data;
}
