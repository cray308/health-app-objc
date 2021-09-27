//
//  PersistenceService.c
//  HealthApp
//
//  Created by Christopher Ray on 3/21/21.
//

#include "PersistenceService.h"
#include "AppUserData.h"
#include "CalendarDateHelpers.h"

static id persistenceService = nil;
id backgroundContext = nil;

static id createWeekData(void) {
    return ((id(*)(id,SEL,id))objc_msgSend)(allocClass("WeeklyData"),
                                            sel_getUid("initWithContext:"), backgroundContext);
}

static void deleteWeekData(id data) {
    ((void(*)(id,SEL,id))objc_msgSend)(backgroundContext, sel_getUid("deleteObject:"), data);
}

static void setWeekData(id data, const char *setter, int16_t value) {
    ((void(*)(id,SEL,int16_t))objc_msgSend)(data, sel_getUid(setter), value);
}

static int16_t getWeekData(id data, const char *getter) {
    return ((int16_t(*)(id,SEL))objc_msgSend)(data, sel_getUid(getter));
}

int16_t weekData_getLiftingLimitForType(id data, unsigned char type) {
    static char const *getterStrs[] = {"bestSquat", "bestPullup", "bestBench", "bestDeadlift"};
    return getWeekData(data, getterStrs[type]);
}

int16_t weekData_getWorkoutTimeForType(id data, unsigned char type) {
    static char const *getterStrs[] = {"timeStrength", "timeSE", "timeEndurance", "timeHIC"};
    return getWeekData(data, getterStrs[type]);
}

int16_t weekData_getTotalWorkouts(id data) {
    return getWeekData(data, "totalWorkouts");
}

int64_t weekData_getWeekStart(id data) {
    return ((int64_t(*)(id,SEL))objc_msgSend)(data, sel_getUid("weekStart"));
}

void weekData_setWorkoutTimeForType(id data, unsigned char type, int16_t duration) {
    static char const *setterStrs[] = {
        "setTimeStrength:", "setTimeSE:", "setTimeEndurance:", "setTimeHIC:"
    };
    setWeekData(data, setterStrs[type], duration + weekData_getWorkoutTimeForType(data, type));
}

void weekData_setLiftingMaxForType(id data, unsigned char type, int16_t weight) {
    static char const *setterStrs[] = {
        "setBestSquat:", "setBestPullup:", "setBestBench:", "setBestDeadlift:"
    };
    setWeekData(data, setterStrs[type], weight);
}

void weekData_setTotalWorkouts(id data, int16_t value) {
    ((void(*)(id,SEL,int16_t))objc_msgSend)(data, sel_getUid("setTotalWorkouts:"), value);
}

void weekData_setWeekStart(id data, int64_t value) {
    ((void(*)(id,SEL,int64_t))objc_msgSend)(data, sel_getUid("setWeekStart:"), value);
}

void persistenceService_saveContext(void) {
    if (((bool(*)(id,SEL))objc_msgSend)(backgroundContext, sel_getUid("hasChanges"))) {
        ((bool(*)(id,SEL,id))objc_msgSend)(backgroundContext, sel_getUid("save:"), nil);
    }
}

id createSortDescriptor(void) {
    return ((id(*)(id,SEL,CFStringRef,bool))objc_msgSend)
    (allocClass("NSSortDescriptor"),
     sel_getUid("initWithKey:ascending:"), CFSTR("weekStart"), true);
}

id fetchRequest(void) {
    return objc_staticMethod(objc_getClass("WeeklyData"), sel_getUid("fetchRequest"));
}

void setPredicate(id request, id pred) {
    ((void(*)(id,SEL,id))objc_msgSend)(request, sel_getUid("setPredicate:"), pred);
}

void setDescriptors(id request, CFArrayRef descriptors) {
    ((void(*)(id,SEL,CFArrayRef))objc_msgSend)(request,
                                               sel_getUid("setSortDescriptors:"), descriptors);
}

void persistenceService_create(void) {
#if DEBUG
    ((void(*)(id,SEL,void(^)(void)))objc_msgSend)(backgroundContext, sel_getUid("performBlock:"), ^{
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
            for (int l = 0; l < 4; ++l) {
                weekData_setWorkoutTimeForType(data, l, times[l]);
                weekData_setLiftingMaxForType(data, l, lifts[l]);
            }
            releaseObj(data);

            if (++i == 52) {
                i = 0;
                plan = 0;
            }
            start += WeekSeconds;
        }
        persistenceService_saveContext();
    });
#else

#endif
}

void persistenceService_init(void) {
    persistenceService = ((id(*)(id,SEL,CFStringRef))objc_msgSend)
    (allocClass("NSPersistentContainer"), sel_getUid("initWithName:"), CFSTR("HealthApp"));
    ((void(*)(id,SEL,void(^)(id,id)))objc_msgSend)
    (persistenceService, sel_getUid("loadPersistentStoresWithCompletionHandler:"),
     ^(id description _U_, id error _U_) {});
    backgroundContext = ((id(*)(id,SEL))objc_msgSend)(persistenceService,
                                                      sel_getUid("newBackgroundContext"));
    ((void(*)(id,SEL,bool))objc_msgSend)
    (backgroundContext, sel_getUid("setAutomaticallyMergesChangesFromParent:"), true);
}

void persistenceService_start(int tzOffset) {
    ((void(*)(id,SEL,void(^)(void)))objc_msgSend)(backgroundContext, sel_getUid("performBlock:"), ^{
        int count = 0;
        CFArrayRef data;
        id request = fetchRequest();
        if (tzOffset) {
            data = persistenceService_executeFetchRequest(request, &count);
            if (data && count) {
                for (int i = 0; i < count; ++i) {
                    id d = (id) CFArrayGetValueAtIndex(data, i);
                    weekData_setWeekStart(d, weekData_getWeekStart(d) + tzOffset);
                }
            }
        }

        setPredicate(request, createPredicate(CFSTR("weekStart < %lld"), date_twoYears));
        data = persistenceService_executeFetchRequest(request, &count);
        if (data && count) {
            for (int i = 0; i < count; ++i) {
                id d = (id) CFArrayGetValueAtIndex(data, i);
                deleteWeekData(d);
            }
            persistenceService_saveContext();
        }

        id descriptor = createSortDescriptor();
        id _descriptors[] = {descriptor};
        CFArrayRef array = CFArrayCreate(NULL, (const void **)_descriptors, 1, &kCocoaArrCallbacks);
        setPredicate(request, createPredicate(CFSTR("weekStart < %lld"), userData->weekStart));
        setDescriptors(request, array);
        data = persistenceService_executeFetchRequest(request, &count);
        id currWeek = persistenceService_getCurrentWeek();
        bool newEntryForCurrentWeek = false;
        if (!currWeek) {
            newEntryForCurrentWeek = true;
            currWeek = createWeekData();
            weekData_setWeekStart(currWeek, userData->weekStart);
        }

        releaseObj(descriptor);
        CFRelease(array);

        if (!(data && count)) {
            if (newEntryForCurrentWeek)
                releaseObj(currWeek);
            goto cleanup;
        }

        id last = (id) CFArrayGetValueAtIndex(data, count - 1);
        int16_t lastLifts[4];
        for (int i = 0; i < 4; ++i)
            lastLifts[i] = weekData_getLiftingLimitForType(last, i);

        for (time_t currStart = weekData_getWeekStart(last) + WeekSeconds;
             currStart < userData->weekStart;
             currStart += WeekSeconds) {
            id curr = createWeekData();
            weekData_setWeekStart(curr, currStart);
            for (int i = 0; i < 4; ++i)
                weekData_setLiftingMaxForType(curr, i, lastLifts[i]);
            releaseObj(curr);
        }

        if (newEntryForCurrentWeek) {
            for (int i = 0; i < 4; ++i)
                weekData_setLiftingMaxForType(currWeek, i, lastLifts[i]);
            releaseObj(currWeek);
        }

    cleanup:
        persistenceService_saveContext();
    });
}

void persistenceService_deleteUserData(void) {
    ((void(*)(id,SEL,void(^)(void)))objc_msgSend)(backgroundContext, sel_getUid("performBlock:"), ^{
        int count = 0;
        id request = fetchRequest();
        id predicate = createPredicate(CFSTR("weekStart < %lld"), userData->weekStart);
        setPredicate(request, predicate);
        CFArrayRef data = persistenceService_executeFetchRequest(fetchRequest(), &count);

        if (data && count) {
            for (int i = 0; i < count; ++i) {
                id d = (id) CFArrayGetValueAtIndex(data, i);
                deleteWeekData(d);
            }
        }

        id currWeek = persistenceService_getCurrentWeek();
        if (currWeek) {
            weekData_setTotalWorkouts(currWeek, 0);
            for (int i = 0; i < 4; ++i)
                weekData_setWorkoutTimeForType(currWeek, i, 0);
        }
        persistenceService_saveContext();
    });
}

id persistenceService_getCurrentWeek(void) {
    int count = 0;
    id request = fetchRequest();
    id predicate = createPredicate(CFSTR("weekStart == %lld"), userData->weekStart);
    setPredicate(request, predicate);
    CFArrayRef currentWeeks = persistenceService_executeFetchRequest(request, &count);
    if (!(currentWeeks && count)) return nil;
    return (id) CFArrayGetValueAtIndex(currentWeeks, 0);
}

CFArrayRef persistenceService_executeFetchRequest(id req, int *count) {
    int len = 0;
    CFArrayRef data = ((CFArrayRef(*)(id,SEL,id,id))objc_msgSend)
    (backgroundContext, sel_getUid("executeFetchRequest:error:"), req, nil);
    if (!(data && (len = (int)(CFArrayGetCount(data))))) return NULL;
    *count = len;
    return data;
}
