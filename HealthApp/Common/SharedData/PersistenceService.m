//
//  PersistenceService.m
//  HealthApp
//
//  Created by Christopher Ray on 3/21/21.
//

#import "PersistenceService.h"
#include "CalendarDateHelpers.h"
#import "WeeklyData+CoreDataClass.h"

#define _U_ __attribute__((__unused__))

NSPersistentContainer *persistenceService_sharedContainer = nil;

WeeklyData *getCurrentWeeklyData(double weekStart);

void persistenceService_setup(void) {
    persistenceService_sharedContainer = [[NSPersistentContainer alloc] initWithName:@"HealthApp"];
    [persistenceService_sharedContainer loadPersistentStoresWithCompletionHandler:
     ^(NSPersistentStoreDescription *description _U_, NSError *error _U_) {}];
}

void persistenceService_free(void) {
    if (persistenceService_sharedContainer) [persistenceService_sharedContainer release];
}

void persistenceService_saveContext(void) {
    NSManagedObjectContext *context = persistenceService_sharedContainer.viewContext;
    if (context.hasChanges) [context save:NULL];
}

void persistenceService_performForegroundUpdate(void) {
    size_t count = 0;
    CFCalendarRef calendar = CFCalendarCopyCurrent();

    NSFetchRequest *request = WeeklyData.fetchRequest;
    request.predicate = [NSPredicate predicateWithFormat:@"weekEnd < %f", date_twoYears(calendar)];

    NSArray<WeeklyData *> *data = [persistenceService_sharedContainer.viewContext executeFetchRequest:request error:nil];
    if ((data && (count = (data.count)) != 0)) {
        for (size_t i = 0; i < count; ++i) {
            [persistenceService_sharedContainer.viewContext deleteObject:data[i]];
        }
        persistenceService_saveContext();
    }

    double weekStart = 0, weekEnd = 0;
    date_calcWeekEndpoints(CFAbsoluteTimeGetCurrent(), calendar, DateSearchDirection_Previous, true, &weekStart, &weekEnd);

    NSSortDescriptor *descriptor = [[NSSortDescriptor alloc] initWithKey:@"weekStart" ascending:true];
    request.predicate = [NSPredicate predicateWithFormat:@"weekStart < %f", weekStart - 2];
    request.sortDescriptors = @[descriptor];
    data = [persistenceService_sharedContainer.viewContext executeFetchRequest:request error:nil];
    bool createEntryForCurrentWeek = getCurrentWeeklyData(weekStart) == nil;

    [descriptor release];
    CFRelease(calendar);

    if (!(data && (count = (data.count)) != 0)) {
        if (createEntryForCurrentWeek) {
            WeeklyData *curr = [[WeeklyData alloc] initWithContext:persistenceService_sharedContainer.viewContext];
            curr.weekStart = weekStart;
            curr.weekEnd = weekEnd;
            [curr release];
        }
        persistenceService_saveContext();
        return;
    }

    WeeklyData *last = data[count - 1];

    for (double currStart = last.weekEnd + 1, currEnd = last.weekEnd + WeekSeconds; (int) currStart < (int) weekStart; currStart = currEnd + 1, currEnd += WeekSeconds) {
        WeeklyData *curr = [[WeeklyData alloc] initWithContext:persistenceService_sharedContainer.viewContext];
        curr.weekStart = currStart;
        curr.weekEnd = currEnd;
        curr.bestBench = last.bestBench;
        curr.bestPullup = last.bestPullup;
        curr.bestSquat = last.bestSquat;
        curr.bestDeadlift = last.bestDeadlift;
        [curr release];
    }

    if (createEntryForCurrentWeek) {
        WeeklyData *curr = [[WeeklyData alloc] initWithContext:persistenceService_sharedContainer.viewContext];
        curr.weekStart = weekStart;
        curr.weekEnd = weekEnd;
        curr.bestBench = last.bestBench;
        curr.bestPullup = last.bestPullup;
        curr.bestSquat = last.bestSquat;
        curr.bestDeadlift = last.bestDeadlift;
        [curr release];
    }

    persistenceService_saveContext();
}

void persistenceService_deleteUserData(void) {
    size_t count = 0;
    CFCalendarRef calendar = CFCalendarCopyCurrent();
    double weekStart = date_calcStartOfWeek(CFAbsoluteTimeGetCurrent(), calendar, DateSearchDirection_Previous, true);
    CFRelease(calendar);
    NSFetchRequest *request = WeeklyData.fetchRequest;
    request.predicate = [NSPredicate predicateWithFormat:@"weekStart < %f", weekStart - 2];

    NSArray<WeeklyData *> *data = [persistenceService_sharedContainer.viewContext executeFetchRequest:request error:nil];
    if ((data && (count = (data.count)) != 0)) {
        for (size_t i = 0; i < count; ++i) {
            [persistenceService_sharedContainer.viewContext deleteObject:data[i]];
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

WeeklyData *persistenceService_getWeeklyDataForThisWeek(void) {
    CFCalendarRef calendar = CFCalendarCopyCurrent();
    double weekStart = date_calcStartOfWeek(CFAbsoluteTimeGetCurrent(), calendar, DateSearchDirection_Previous, true);
    CFRelease(calendar);
    return getCurrentWeeklyData(weekStart);
}

#pragma mark - Helper Functions

WeeklyData *getCurrentWeeklyData(double weekStart) {
    NSFetchRequest *request = WeeklyData.fetchRequest;
    request.predicate = [NSPredicate predicateWithFormat:@"weekStart > %f", weekStart - 2];
    NSArray<WeeklyData *> *currentWeeks = [persistenceService_sharedContainer.viewContext executeFetchRequest:request error:nil];
    if (!(currentWeeks && currentWeeks.count)) return nil;
    return currentWeeks[0];
}
