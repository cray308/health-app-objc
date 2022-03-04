#include "PersistenceService.h"
#include "AppUserData.h"
#include "CocoaHelpers.h"

#define deleteWeekData(c, d) msg1(void, id, c, sel_getUid("deleteObject:"), d)

#define weekData_setWeekStart(_d, _val) msg1(void, int64_t, _d, sel_getUid("setWeekStart:"), _val)

#define createWeekData(c)\
 msg1(id,id,allocClass(objc_getClass("WeeklyData")),sel_getUid("initWithContext:"),c)

id backgroundContext = nil;

void weekData_getLiftingLimits(id data, int16_t *output) {
    static char const *const getters[] = {"bestSquat", "bestPullup", "bestBench", "bestDeadlift"};
    for (int i = 0; i < 4; ++i) {
        output[i] = msg0(int16_t, data, sel_getUid(getters[i]));
    }
}

int16_t weekData_getWorkoutTimeForType(id data, int type) {
    static char const *const getters[] = {"timeStrength", "timeSE", "timeEndurance", "timeHIC"};
    return msg0(int16_t, data, sel_getUid(getters[type]));
}

void weekData_setWorkoutTimeForType(id data, int type, int16_t duration) {
    static char const *const setters[] = {
        "setTimeStrength:", "setTimeSE:", "setTimeEndurance:", "setTimeHIC:"
    };
    msg1(void, int16_t, data, sel_getUid(setters[type]), duration);
}

void weekData_setLiftingMaxArray(id data, int16_t *weights) {
    static char const *const setters[] = {
        "setBestSquat:", "setBestPullup:", "setBestBench:", "setBestDeadlift:"
    };
    for (int i = 0; i < 4; ++i) {
        msg1(void, int16_t, data, sel_getUid(setters[i]), weights[i]);
    }
}

void persistenceService_saveContext(id context) {
    if (msg0(bool, context, sel_getUid("hasChanges")))
        msg1(void, id, context, sel_getUid("save:"), nil);
}

#if DEBUG
#include <stdlib.h>

static time_t _psCalcStartOfWeek(time_t date) {
    struct tm info;
    localtime_r(&date, &info);
    int weekday = info.tm_wday;

    if (weekday == 1) {
        int seconds = (info.tm_hour * 3600) + (info.tm_min * 60) + info.tm_sec;
        return date - seconds;
    }

    date -= WeekSeconds;
    while (weekday != 1) {
        date += 86400;
        weekday = weekday == 6 ? 0 : weekday + 1;
    }
    localtime_r(&date, &info);
    int seconds = ((info.tm_hour * 3600) + (info.tm_min * 60) + info.tm_sec);
    return date - seconds;
}

void persistenceService_create(void) {
    id context = backgroundContext;
    msg1(void, void(^)(void), context, sel_getUid("performBlock:"), (^{
        int16_t lifts[] = {300, 20, 185, 235};
        int i = 0, r = 0;
        unsigned char plan = 0;
        time_t start = _psCalcStartOfWeek(time(NULL) - 126489600);
        time_t end = _psCalcStartOfWeek(time(NULL) - 2678400);

        while (start < end) {
            int16_t totalWorkouts = 0;
            int16_t times[4] = {0};
            id data = createWeekData(context);
            int addnlTime = 0, addnlWk = 1;
            if (r == 0 || (i < 24)) {
                addnlTime = rand() % 40;
                addnlWk += rand() % 5;
            }
            weekData_setWeekStart(data, start);

            if (plan == 0) {
                for (int j = 0; j < 6; ++j) {
                    int extra = 10;
                    bool didSE = true;
                    switch (j) {
                        case 1:
                        case 2:
                        case 5:
                            times[2] += ((rand() % 30) + 30 + addnlTime);
                            totalWorkouts += addnlWk;
                            break;
                        case 4:
                            if ((didSE = (rand() % 10 >= 5))) extra = 0;
                        case 0:
                        case 3:
                            if (didSE) {
                                times[1] += ((rand() % 20) + extra + addnlTime);
                                totalWorkouts += addnlWk;
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
                            times[0] += ((rand() % 20) + 20 + addnlTime);
                            totalWorkouts += addnlWk;
                            break;
                        case 1:
                        case 3:
                            times[3] += ((rand() % 20) + 15 + addnlTime);
                            totalWorkouts += addnlWk;
                            break;
                        case 5:
                            times[2] += ((rand() % 30) + 60 + addnlTime);
                            totalWorkouts += addnlWk;
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
                r += 1;
            }
            start += WeekSeconds;
        }
        persistenceService_saveContext(context);
    }));
}
#endif

void persistenceService_init(void) {
    id _container = allocClass(objc_getClass("NSPersistentContainer"));
    id svc = msg1(id, CFStringRef, _container, sel_getUid("initWithName:"), CFSTR("HealthApp"));
    msg1(void, void(^)(id,id), svc,
         sel_getUid("loadPersistentStoresWithCompletionHandler:"), ^(id desc _U_, id err _U_){});
    id context = msg0(id, svc, sel_getUid("newBackgroundContext"));
    msg1(void, bool, context, sel_getUid("setAutomaticallyMergesChangesFromParent:"), true);
    backgroundContext = context;
}

void persistenceService_start(int tzOffset, time_t weekStart,
                              void (*completion)(void*), void *receiver) {
    id context = backgroundContext;
    msg1(void, void(^)(void), context, sel_getUid("performBlock:"), ^{
        const time_t endPt = weekStart - 63244800;
        time_t start;
        int count = 0;
        CFArrayRef data = persistenceService_fetchData(context, 1, &count);
        if (!data) {
            id first = createWeekData(context);
            weekData_setWeekStart(first, weekStart);
            releaseObj(first);
            persistenceService_saveContext(context);
            goto cleanup;
        }

        if (tzOffset) {
            for (int i = 0; i < count; ++i) {
                id d = (id) CFArrayGetValueAtIndex(data, i);
                start = weekData_getWeekStart(d) + tzOffset;
                weekData_setWeekStart(d, start);
            }
        }

        int16_t lastLifts[4];
        id last = (id) CFArrayGetValueAtIndex(data, count - 1);
        weekData_getLiftingLimits(last, lastLifts);
        start = weekData_getWeekStart(last);
        if (start != weekStart) {
            id currWeek = createWeekData(context);
            weekData_setWeekStart(currWeek, weekStart);
            weekData_setLiftingMaxArray(currWeek, lastLifts);
            releaseObj(currWeek);
        }

        for (int i = 0; i < count; ++i) {
            id d = (id) CFArrayGetValueAtIndex(data, i);
            start = weekData_getWeekStart(d);
            if (start < endPt)
                deleteWeekData(context, d);
        }

        start = weekData_getWeekStart(last) + WeekSeconds;
        for (; start < weekStart; start += WeekSeconds) {
            id curr = createWeekData(context);
            weekData_setWeekStart(curr, start);
            weekData_setLiftingMaxArray(curr, lastLifts);
            releaseObj(curr);
        }

        persistenceService_saveContext(context);

    cleanup:
        completion(receiver);
    });
}

void persistenceService_deleteUserData(void) {
    id context = backgroundContext;
    msg1(void, void(^)(void), context, sel_getUid("performBlock:"), ^{
        int count = 0;
        CFArrayRef data = persistenceService_fetchData(context, 1, &count);

        if (data) {
            int end = count - 1;
            for (int i = 0; i < end; ++i) {
                id d = (id) CFArrayGetValueAtIndex(data, i);
                deleteWeekData(context, d);
            }

            id currWeek = (id) CFArrayGetValueAtIndex(data, end);
            weekData_setTotalWorkouts(currWeek, 0);
            for (int i = 0; i < 4; ++i) {
                weekData_setWorkoutTimeForType(currWeek, i, 0);
            }
            persistenceService_saveContext(context);
        }
    });
}

CFArrayRef persistenceService_fetchData(id context, int options, int *count) {
    id req = clsF1(id, CFStringRef, objc_getClass("NSFetchRequest"),
                   sel_getUid("fetchRequestWithEntityName:"), CFSTR("WeeklyData"));
    if (options & 0x2)
        msg1(void, unsigned long, req, sel_getUid("setFetchLimit:"), 1);
    CFArrayRef descriptorArr = createSortDescriptors(CFSTR("weekStart"), options & 0x1);
    msg1(void, CFArrayRef, req, sel_getUid("setSortDescriptors:"), descriptorArr);
    CFRelease(descriptorArr);

    int len = 0;
    CFArrayRef data = msg2(CFArrayRef, id, id, context,
                           sel_getUid("executeFetchRequest:error:"), req, nil);
    if (!(data && (len = (int)(CFArrayGetCount(data)))))
        data = NULL;
    *count = len;
    return data;
}
