//
//  PersistenceService.c
//  HealthApp
//
//  Created by Christopher Ray on 3/21/21.
//

#include "PersistenceService.h"
#include "AppUserData.h"
#include "CalendarDateHelpers.h"

id persistenceServiceShared = nil;

static id getContext(void) {
    return ((id(*)(id,SEL))objc_msgSend)(persistenceServiceShared, sel_getUid("viewContext"));
}

static id createWeekData(id context) {
    return ((id(*)(id,SEL,id))objc_msgSend)(allocClass("WeeklyData"),
                                            sel_getUid("initWithContext:"), context);
}

static void deleteWeekData(id context, id data) {
    ((void(*)(id,SEL,id))objc_msgSend)(context, sel_getUid("deleteObject:"), data);
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
    id context = getContext();
    if (((bool(*)(id,SEL))objc_msgSend)(context, sel_getUid("hasChanges"))) {
        ((bool(*)(id,SEL,id))objc_msgSend)(context, sel_getUid("save:"), nil);
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

void persistenceService_performForegroundUpdate(void) {
    int count = 0;
    id context = getContext();
    id request = fetchRequest();
    setPredicate(request, createPredicate(CFSTR("weekStart < %lld"), date_twoYears));

    CFArrayRef data = persistenceService_executeFetchRequest(request, &count);
    if (data && count) {
        for (int i = 0; i < count; ++i) {
            id d = (id) CFArrayGetValueAtIndex(data, i);
            deleteWeekData(context, d);
        }
        persistenceService_saveContext();
    }

    id descriptor = createSortDescriptor();
    id _descriptors[] = {descriptor};
    CFArrayRef array = CFArrayCreate(NULL, (const void **)_descriptors, 1, kCocoaArrCallbacks);
    setPredicate(request, createPredicate(CFSTR("weekStart < %lld"), appUserDataShared->weekStart));
    setDescriptors(request, array);
    data = persistenceService_executeFetchRequest(request, &count);
    bool createEntryForCurrentWeek = persistenceService_getWeeklyDataForThisWeek() == nil;

    releaseObj(descriptor);
    CFRelease(array);

    if (!(data && count)) {
        if (createEntryForCurrentWeek) {
            id curr = createWeekData(context);
            weekData_setWeekStart(curr, appUserDataShared->weekStart);
            releaseObj(curr);
        }
        persistenceService_saveContext();
        return;
    }

    id last = (id) CFArrayGetValueAtIndex(data, count - 1);
    int16_t lastLifts[4];
    for (int i = 0; i < 4; ++i) {
        lastLifts[i] = weekData_getLiftingLimitForType(last, i);
    }

    for (time_t currStart = weekData_getWeekStart(last) + WeekSeconds;
         currStart < appUserDataShared->weekStart;
         currStart += WeekSeconds) {
        id curr = createWeekData(context);
        weekData_setWeekStart(curr, currStart);
        for (int i = 0; i < 4; ++i) {
            weekData_setLiftingMaxForType(curr, i, lastLifts[i]);
        }
        releaseObj(curr);
    }

    if (createEntryForCurrentWeek) {
        id curr = createWeekData(context);
        weekData_setWeekStart(curr, appUserDataShared->weekStart);
        for (int i = 0; i < 4; ++i) {
            weekData_setLiftingMaxForType(curr, i, lastLifts[i]);
        }
        releaseObj(curr);
    }

    persistenceService_saveContext();
}

void persistenceService_deleteUserData(void) {
    int count = 0;
    id context = getContext();
    id request = fetchRequest();
    id predicate = createPredicate(CFSTR("weekStart < %lld"), appUserDataShared->weekStart);
    setPredicate(request, predicate);
    CFArrayRef data = persistenceService_executeFetchRequest(fetchRequest(), &count);

    if (data && count) {
        for (int i = 0; i < count; ++i) {
            id d = (id) CFArrayGetValueAtIndex(data, i);
            deleteWeekData(context, d);
        }
    }

    id currWeek = persistenceService_getWeeklyDataForThisWeek();
    if (currWeek) {
        weekData_setTotalWorkouts(currWeek, 0);
        for (int i = 0; i < 4; ++i) {
            weekData_setWorkoutTimeForType(currWeek, i, 0);
        }
    }

    persistenceService_saveContext();
}

void persistenceService_changeTimestamps(int difference) {
    int count = 0;
    CFArrayRef data = persistenceService_executeFetchRequest(fetchRequest(), &count);
    if (!(data && count)) return;
    for (int i = 0; i < count; ++i) {
        id d = (id) CFArrayGetValueAtIndex(data, i);
        weekData_setWeekStart(d, weekData_getWeekStart(d) + difference);
    }
    persistenceService_saveContext();
}

id persistenceService_getWeeklyDataForThisWeek(void) {
    int count = 0;
    id request = fetchRequest();
    id predicate = createPredicate(CFSTR("weekStart == %lld"), appUserDataShared->weekStart);
    setPredicate(request, predicate);
    CFArrayRef currentWeeks = persistenceService_executeFetchRequest(request, &count);
    if (!(currentWeeks && count)) return nil;
    return (id) CFArrayGetValueAtIndex(currentWeeks, 0);
}

CFArrayRef persistenceService_executeFetchRequest(id req, int *count) {
    int len = 0;
    CFArrayRef data = ((CFArrayRef(*)(id,SEL,id,id))objc_msgSend)
    (getContext(), sel_getUid("executeFetchRequest:error:"), req, nil);
    if (!(data && (len = (int)(CFArrayGetCount(data))))) return NULL;
    *count = len;
    return data;
}