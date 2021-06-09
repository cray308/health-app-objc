//
//  PersistenceService.m
//  HealthApp
//
//  Created by Christopher Ray on 3/21/21.
//

#import "PersistenceService.h"
#import "Constants.h"
#import "CalendarDateHelpers.h"
#import "WeekStats+CoreDataClass.h"
#import "ActivityEntry+CoreDataClass.h"
#import "WeeklyData+CoreDataClass.h"

NSPersistentContainer *persistenceService_sharedContainer = nil;

void removeOldData(NSFetchRequest *fetchRequest);
void downsample(CFCalendarRef calendar, NSFetchRequest *fetchRequest);
void deleteAllActivityEntries(NSFetchRequest *fetchRequest);
void deleteAllWeekStats(NSFetchRequest *fetchRequest);
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

void persistenceService_downsample(void) {
    CFCalendarRef calendar = CFCalendarCopyCurrent();

    NSFetchRequest *fetchRequest = [WeekStats fetchRequest];
    fetchRequest.predicate = [NSPredicate predicateWithFormat:@"end < %f", date_lastYear(calendar)];
    removeOldData(fetchRequest);

    double date = date_calcStartOfWeek(CFAbsoluteTimeGetCurrent(), calendar, DateSearchDirection_Previous, true);
    fetchRequest = [ActivityEntry fetchRequest];
    NSSortDescriptor *descriptor = [[NSSortDescriptor alloc] initWithKey:@"timestamp" ascending:true];
    fetchRequest.predicate = [NSPredicate predicateWithFormat:@"timestamp < %f", date];
    fetchRequest.sortDescriptors = @[descriptor];
    downsample(calendar, fetchRequest);
    [descriptor release];

    CFRelease(calendar);
}

void persistenceService_deleteUserData(void) {
    //WeeklyData *currWeek = persistenceService_getWeeklyDataForThisWeek(void);
    /*

     @property (nonatomic) int16_t bestBench;
     @property (nonatomic) int16_t bestPullup;
     @property (nonatomic) int16_t bestSquat;
     @property (nonatomic) int16_t timeEndurance;
     @property (nonatomic) int16_t timeHIC;
     @property (nonatomic) int16_t timeSE;
     @property (nonatomic) int16_t timeStrength;
     @property (nonatomic) int16_t totalWorkouts;
     @property (nonatomic) double weekEnd;
     @property (nonatomic) double weekStart;


     */
//    deleteAllActivityEntries([ActivityEntry fetchRequest]);
//    deleteAllWeekStats([WeekStats fetchRequest]);
//    persistenceService_saveContext();
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

void downsample(CFCalendarRef calendar, NSFetchRequest *fetchRequest) {
    size_t count = 0;
    NSArray<ActivityEntry*> *oldActivities = [persistenceService_sharedContainer.viewContext executeFetchRequest:fetchRequest error:nil];
    if (!oldActivities) return;
    else if (!(count = (oldActivities.count))) return;

    WeeklyActivitySummaryModel model = {0};
    date_calcWeekEndpoints(oldActivities[0].timestamp, calendar, DateSearchDirection_Previous, true,
                           &model.weekStart, &model.weekEnd);

    for (size_t i = 0; i < count; ++i) {
        ActivityEntry *object = oldActivities[i];
        double date = object.timestamp;

        if (date > model.weekEnd) { // past the end date, save empty data for these weeks
            WeekStats *weekStats = [[WeekStats alloc] initWithContext:persistenceService_sharedContainer.viewContext];
            [weekStats setProperties:&model];

            model.tokensEarned = 0;
            memset(model.durationByDay, 0, 7 * sizeof(int));
            memset(model.durationByIntensity, 0, 3 * sizeof(int));
            model.weekStart = model.weekEnd + 1;
            model.weekEnd += WeekSeconds;
            [weekStats release];

            while (date > model.weekEnd) {
                weekStats = [[WeekStats alloc] initWithContext:persistenceService_sharedContainer.viewContext];
                [weekStats setProperties:&model];
                model.weekStart += 1;
                model.weekEnd += WeekSeconds;
                [weekStats release];
            }
        }

        int dayIdx = date_getDayOfWeek(date, calendar) - 1;
        model.tokensEarned += object.tokens;
        model.durationByIntensity[object.type] += object.duration;
        model.durationByDay[dayIdx] += object.duration;
        [persistenceService_sharedContainer.viewContext deleteObject:object];
    }

    // save WeekStats from the end of the loop (most recent)
    WeekStats *weekStats = [[WeekStats alloc] initWithContext:persistenceService_sharedContainer.viewContext];
    [weekStats setProperties:&model];
    [weekStats release];
    persistenceService_saveContext();
}

void removeOldData(NSFetchRequest *fetchRequest) {
    size_t count = 0;
    NSArray<WeekStats*> *stats = [persistenceService_sharedContainer.viewContext executeFetchRequest:fetchRequest error:nil];
    if (!stats) return;
    else if (!(count = (stats.count))) return;

    for (size_t i = 0; i < count; ++i) {
        [persistenceService_sharedContainer.viewContext deleteObject:stats[i]];
    }
    persistenceService_saveContext();
}

void deleteAllActivityEntries(NSFetchRequest *fetchRequest) {
    size_t count = 0;
    NSArray<ActivityEntry*> *data = [persistenceService_sharedContainer.viewContext executeFetchRequest:fetchRequest error:nil];
    if (!data) return;
    else if (!(count = (data.count))) return;

    for (size_t i = 0; i < count; ++i) {
        [persistenceService_sharedContainer.viewContext deleteObject:data[i]];
    }
}

void deleteAllWeekStats(NSFetchRequest *fetchRequest) {
    size_t count = 0;
    NSArray<WeekStats*> *data = [persistenceService_sharedContainer.viewContext executeFetchRequest:fetchRequest error:nil];
    if (!data) return;
    else if (!(count = (data.count))) return;

    for (size_t i = 0; i < count; ++i) {
        [persistenceService_sharedContainer.viewContext deleteObject:data[i]];
    }
}
