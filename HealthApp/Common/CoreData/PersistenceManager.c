#include "PersistenceManager.h"
#include "CocoaHelpers.h"
#include "UserData.h"

#define oldestDate(time) (time - (WeekSeconds * 104) - (DaySeconds << 2))

enum {
    FetchSorted = 1,
    FetchLimited
};

static const char *const liftGets[] = {"bestSquat", "bestPullup", "bestBench", "bestDeadlift"};
static const char *const timeGets[] = {"timeStrength", "timeSE", "timeEndurance", "timeHIC"};
static char const *const timeSets[] = {
    "setTimeStrength:", "setTimeSE:", "setTimeEndurance:", "setTimeHIC:"
};

static void saveContext(id context) {
    if (msg0(bool, context, sel_getUid("hasChanges")))
        msg1(void, id, context, sel_getUid("save:"), nil);
}

static CFArrayRef fetchData(id context, int options, int *count) {
    id req = clsF1(id, CFStringRef, objc_getClass("NSFetchRequest"),
                   sel_getUid("fetchRequestWithEntityName:"), CFSTR("WeeklyData"));
    if (options & FetchLimited) msg1(void, u_long, req, sel_getUid("setFetchLimit:"), 1);
    CFArrayRef descriptorArr = createSortDescriptors(CFSTR("weekStart"), options & FetchSorted);
    msg1(void, CFArrayRef, req, sel_getUid("setSortDescriptors:"), descriptorArr);
    CFRelease(descriptorArr);

    int len = 0;
    CFArrayRef data = msg2(CFArrayRef, id, id, context,
                           sel_getUid("executeFetchRequest:error:"), req, nil);
    if (!(data && (len = (int)(CFArrayGetCount(data))))) data = NULL;
    *count = len;
    return data;
}

static void setLifts(id data, int16_t *lifts) {
    static char const *const liftSets[] = {
        "setBestSquat:", "setBestPullup:", "setBestBench:", "setBestDeadlift:"
    };
    for (int i = 0; i < 4; ++i) {
        msg1(void, int16_t, data, sel_getUid(liftSets[i]), lifts[i]);
    }
}

static void addWeeklyData(id context, int64_t weekStart, int16_t *lifts) {
    id data = msg1(id, id, clsF0(id, objc_getClass("WeeklyData"), Sels.alo),
                   sel_getUid("initWithContext:"), context);
    msg1(void, int64_t, data, sel_getUid("setWeekStart:"), weekStart);
    if (lifts) setLifts(data, lifts);
    msg0(void, data, Sels.rel);
}

id context_init(void) {
    static id container;
    id _c = alloc(objc_getClass("NSPersistentContainer"));
    container = msg1(id, CFStringRef, _c, sel_getUid("initWithName:"), CFSTR("HealthApp"));
    msg1(void, void(^)(id,id), container,
         sel_getUid("loadPersistentStoresWithCompletionHandler:"), ^(id desc _U_, id err _U_){});
    id context = msg0(id, container, sel_getUid("newBackgroundContext"));
    msg1(void, bool, context, sel_getUid("setAutomaticallyMergesChangesFromParent:"), true);
    return context;
}

#if DEBUG
static void createDummyData(id context) {
    ((void(*)(id,SEL,Callback))objc_msgSend)(context, sel_getUid("performBlock:"), ^{
        int16_t lifts[] = {300, 20, 185, 235};
        int i = 0, r = 0;
        unsigned char plan = 0;
        time_t endpts[] = {time(NULL) - 126489600, time(NULL) - 2678400};
        for (int x = 0; x < 2; ++x) {
            time_t date = endpts[x];
            struct tm tmInfo;
            localtime_r(&date, &tmInfo);
            if (tmInfo.tm_wday != 1) {
                date = date - WeekSeconds + (((8 - tmInfo.tm_wday) % 7) * DaySeconds);
                localtime_r(&date, &tmInfo);
            }
            endpts[x] = date - ((tmInfo.tm_hour * 3600) + (tmInfo.tm_min * 60) + tmInfo.tm_sec);
        }
        while (endpts[0] < endpts[1]) {
            int16_t totalWorkouts = 0;
            int16_t times[4] = {0};
            id data = msg1(id, id, clsF0(id, objc_getClass("WeeklyData"), Sels.alo),
                           sel_getUid("initWithContext:"), context);
            int addnlTime = 0, addnlWk = 1;
            if (r == 0 || (i < 24)) {
                addnlTime = rand() % 40;
                addnlWk += rand() % 5;
            }
            msg1(void, int64_t, data, sel_getUid("setWeekStart:"), endpts[0]);

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
            msg1(void, int16_t, data, sel_getUid("setTotalWorkouts:"), totalWorkouts);
            setLifts(data, lifts);
            for (int x = 0; x < 4; ++x) {
                msg1(void, int16_t, data, sel_getUid(timeSets[x]), times[x]);
            }
            msg0(void, data, Sels.rel);

            if (++i == 52) {
                i = 0;
                plan = 0;
                r += 1;
            }
            endpts[0] += WeekSeconds;
        }
        saveContext(context);
    });
}

id context_create(void) {
    id context = context_init();
    createDummyData(context);
    return context;
}
#endif

static void fetchHistory(id context, HistoryModel *model, bool ltr) {
    int count = 0;
    CFArrayRef data = fetchData(context, FetchSorted, &count);
    if (data && count > 1) {
        SEL liftSels[4], timeSels[4];
        SEL getTotal = sel_getUid("totalWorkouts"), getStart = sel_getUid("weekStart");
        for (int i = 0; i < 4; ++i) {
            liftSels[i] = sel_getUid(liftGets[i]);
            timeSels[i] = sel_getUid(timeGets[i]);
        }

        CFLocaleRef locale = CFLocaleCopyCurrent();
        CFDateFormatterRef fmt = CFDateFormatterCreate(NULL, locale, kCFDateFormatterShortStyle, 0);
        CFRelease(locale);
        CFMutableArrayRef strs = CFArrayCreateMutable(NULL, --count, &kCFTypeArrayCallBacks);
        WeekDataModel *results = malloc((unsigned)count * sizeof(WeekDataModel));
        customAssert(count > 0)
        for (int i = 0; i < count; ++i) {
            id d = (id)CFArrayGetValueAtIndex(data, i);
            WeekDataModel *r = &results[i];
            int64_t date = ToAppleTime(msg0(int64_t, d, getStart));
            CFStringRef str = CFDateFormatterCreateStringWithAbsoluteTime(NULL, fmt, date);
            CFArrayAppendValue(strs, str);
            r->totalWorkouts = msg0(int16_t, d, getTotal);
            for (int j = 0; j < 4; ++j) {
                r->durationByType[j] = msg0(int16_t, d, timeSels[j]);
                r->weightArray[j] = msg0(int16_t, d, liftSels[j]);
            }

            r->cumulativeDuration[0] = r->durationByType[0];
            for (int j = 1; j < 4; ++j) {
                r->cumulativeDuration[j] = r->cumulativeDuration[j - 1] + r->durationByType[j];
            }
            CFRelease(str);
        }
        if (!ltr) {
            for (int i = 0, j = count - 1; i < j; ++i, --j) {
                CFArrayExchangeValuesAtIndices(strs, i, j);
            }
        }
        CFRelease(fmt);
        historyModel_populate(model, strs, results, count, ltr);
    }
}

void runStartupJob(id context, HistoryModel *model, time_t weekStart, int tzDiff, bool ltr) {
    ((void(*)(id,SEL,Callback))objc_msgSend)(context, sel_getUid("performBlock:"), ^{
        SEL getStart = sel_getUid("weekStart"), delObj = sel_getUid("deleteObject:");
        SEL setStart = sel_getUid("setWeekStart:");
        const time_t endPt = oldestDate(weekStart);
        int count = 0;
        CFArrayRef data = fetchData(context, FetchSorted, &count);
        if (!data) {
            addWeeklyData(context, weekStart, NULL);
            saveContext(context);
            return;
        }

        if (tzDiff) {
            for (int i = 0; i < count; ++i) {
                id d = (id)CFArrayGetValueAtIndex(data, i);
                msg1(void, int64_t, d, setStart, msg0(int64_t, d, getStart) + tzDiff);
            }
        }

        id last = (id)CFArrayGetValueAtIndex(data, count - 1);
        int16_t lastLifts[4];
        for (int i = 0; i < 4; ++i) {
            lastLifts[i] = msg0(int16_t, last, sel_getUid(liftGets[i]));
        }
        time_t start = msg0(int64_t, last, getStart);
        if (start != weekStart) addWeeklyData(context, weekStart, lastLifts);

        for (int i = 0; i < count; ++i) {
            id d = (id)CFArrayGetValueAtIndex(data, i);
            if (msg0(int64_t, d, getStart) < endPt) msg1(void, id, context, delObj, d);
        }

        start += WeekSeconds;
        for (; start < weekStart; start += WeekSeconds) {
            addWeeklyData(context, start, lastLifts);
        }

        saveContext(context);
        fetchHistory(context, model, ltr);
    });
}

void deleteStoredData(id context) {
    ((void(*)(id,SEL,Callback))objc_msgSend)(context, sel_getUid("performBlock:"), ^{
        SEL delObj = sel_getUid("deleteObject:");
        int count = 0;
        CFArrayRef data = fetchData(context, FetchSorted, &count);
        if (data) {
            int end = count - 1;
            for (int i = 0; i < end; ++i) {
                msg1(void, id, context, delObj, (id)CFArrayGetValueAtIndex(data, i));
            }
            id currWeek = (id)CFArrayGetValueAtIndex(data, end);
            msg1(void, int16_t, currWeek, sel_getUid("setTotalWorkouts:"), 0);
            for (int i = 0; i < 4; ++i) {
                msg1(void, int16_t, currWeek, sel_getUid(timeSets[i]), 0);
            }
            saveContext(context);
        }
    });
}

void saveWorkoutData(id context, int16_t duration, unsigned char type, short *weights) {
    ((void(*)(id,SEL,Callback))objc_msgSend)(context, sel_getUid("performBlock:"), ^{
        id data = (id)CFArrayGetValueAtIndex(fetchData(context, FetchLimited, &(int){0}), 0);
        int16_t newDuration = duration + msg0(int16_t, data, sel_getUid(timeGets[type]));
        msg1(void, int16_t, data, sel_getUid(timeSets[type]), newDuration);
        int16_t totalWorkouts = msg0(int16_t, data, sel_getUid("totalWorkouts")) + 1;
        msg1(void, int16_t, data, sel_getUid("setTotalWorkouts:"), totalWorkouts);
        if (weights) {
            setLifts(data, weights);
            free(weights);
        }
        saveContext(context);
    });
}
