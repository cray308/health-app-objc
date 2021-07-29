//
//  PersistenceService.m
//  HealthApp
//
//  Created by Christopher Ray on 3/21/21.
//

#import "PersistenceService.h"
#include "AppUserData.h"
#include "CalendarDateHelpers.h"

#define _U_ __attribute__((__unused__))

NSPersistentContainer *persistenceServiceShared = nil;

void persistenceService_saveContext(void) {
    NSManagedObjectContext *context = persistenceServiceShared.viewContext;
    if (context.hasChanges) [context save:NULL];
}

void persistenceService_performForegroundUpdate(void) {
    int count = 0;

    NSFetchRequest *request = WeeklyData.fetchRequest;
    request.predicate = [NSPredicate predicateWithFormat:@"weekStart < %lld", date_twoYears];

    NSArray<WeeklyData *> *data = [persistenceServiceShared.viewContext executeFetchRequest:request error:nil];
    if (data && (count = (int) data.count)) {
        for (int i = 0; i < count; ++i) {
            [persistenceServiceShared.viewContext deleteObject:data[i]];
        }
        persistenceService_saveContext();
    }

    NSSortDescriptor *descriptor = [[NSSortDescriptor alloc] initWithKey:@"weekStart" ascending:true];
    request.predicate = [NSPredicate predicateWithFormat:@"weekStart < %lld", appUserDataShared->weekStart];
    request.sortDescriptors = @[descriptor];
    data = [persistenceServiceShared.viewContext executeFetchRequest:request error:nil];
    bool createEntryForCurrentWeek = persistenceService_getWeeklyDataForThisWeek() == nil;

    [descriptor release];

    if (!(data && (count = (int) data.count))) {
        if (createEntryForCurrentWeek) {
            WeeklyData *curr = [[WeeklyData alloc] initWithContext:persistenceServiceShared.viewContext];
            curr.weekStart = appUserDataShared->weekStart;
            [curr release];
        }
        persistenceService_saveContext();
        return;
    }

    WeeklyData *last = data[count - 1];

    for (long currStart = last.weekStart + WeekSeconds; currStart < appUserDataShared->weekStart; currStart += WeekSeconds) {
        WeeklyData *curr = [[WeeklyData alloc] initWithContext:persistenceServiceShared.viewContext];
        curr.weekStart = currStart;
        curr.bestBench = last.bestBench;
        curr.bestPullup = last.bestPullup;
        curr.bestSquat = last.bestSquat;
        curr.bestDeadlift = last.bestDeadlift;
        [curr release];
    }

    if (createEntryForCurrentWeek) {
        WeeklyData *curr = [[WeeklyData alloc] initWithContext:persistenceServiceShared.viewContext];
        curr.weekStart = appUserDataShared->weekStart;
        curr.bestBench = last.bestBench;
        curr.bestPullup = last.bestPullup;
        curr.bestSquat = last.bestSquat;
        curr.bestDeadlift = last.bestDeadlift;
        [curr release];
    }

    persistenceService_saveContext();
}

void persistenceService_deleteUserData(void) {
    int count = 0;
    NSFetchRequest *request = WeeklyData.fetchRequest;
    request.predicate = [NSPredicate predicateWithFormat:@"weekStart < %lld", appUserDataShared->weekStart];

    NSArray<WeeklyData *> *data = [persistenceServiceShared.viewContext executeFetchRequest:request error:nil];
    if (data && (count = (int) data.count)) {
        for (int i = 0; i < count; ++i) {
            [persistenceServiceShared.viewContext deleteObject:data[i]];
        }
    }

    WeeklyData *currWeek = persistenceService_getWeeklyDataForThisWeek();
    if (currWeek) {
        currWeek.timeEndurance = 0;
        currWeek.timeHIC = 0;
        currWeek.timeSE = 0;
        currWeek.timeStrength = 0;
        currWeek.totalWorkouts = 0;
    }

    persistenceService_saveContext();
}

void persistenceService_changeTimestamps(int difference) {
    int count = 0;
    NSArray<WeeklyData *> *data = [persistenceServiceShared.viewContext executeFetchRequest:WeeklyData.fetchRequest error:nil];
    if (!(data && (count = (int) data.count))) return;
    for (int i = 0; i < count; ++i) {
        data[i].weekStart += difference;
    }
    persistenceService_saveContext();
}

WeeklyData *persistenceService_getWeeklyDataForThisWeek(void) {
    NSFetchRequest *request = WeeklyData.fetchRequest;
    request.predicate = [NSPredicate predicateWithFormat:@"weekStart == %lld", appUserDataShared->weekStart];
    NSArray<WeeklyData *> *currentWeeks = [persistenceServiceShared.viewContext executeFetchRequest:request error:nil];
    if (!(currentWeeks && currentWeeks.count)) return nil;
    return currentWeeks[0];
}

NSArray<id> *persistenceService_executeFetchRequest(NSFetchRequest *req, NSPredicate *pred,
                                                    NSSortDescriptor *descriptor, int *count) {
    if (pred) req.predicate = pred;
    if (descriptor) req.sortDescriptors = @[descriptor];

    int len = 0;
    NSArray<id> *data = [persistenceServiceShared.viewContext executeFetchRequest:req error:nil];
    if (!(data && (len = (int)(data.count)))) return nil;
    *count = len;
    return data;
}
