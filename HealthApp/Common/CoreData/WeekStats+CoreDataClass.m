//
//  WeekStats+CoreDataClass.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//
//

#import "WeekStats+CoreDataClass.h"

@implementation WeekStats

@end

@implementation WeekStats (CoreDataProperties)

+ (NSFetchRequest<WeekStats *> *) fetchRequest {
    return [NSFetchRequest fetchRequestWithEntityName:@"WeekStats"];
}

@dynamic end;
@dynamic start;
@dynamic timeFriday;
@dynamic timeHigh;
@dynamic timeLow;
@dynamic timeMedium;
@dynamic timeMonday;
@dynamic timeSaturday;
@dynamic timeSunday;
@dynamic timeThursday;
@dynamic timeTuesday;
@dynamic timeWednesday;
@dynamic tokens;

- (void) populateIntensities: (int *)array {
    array[0] = self.timeLow;
    array[1] = self.timeMedium;
    array[2] = self.timeHigh;
}

- (void) populateDurationByDay: (int *)array {
    array[0] = self.timeSunday;
    array[1] = self.timeMonday;
    array[2] = self.timeTuesday;
    array[3] = self.timeWednesday;
    array[4] = self.timeThursday;
    array[5] = self.timeFriday;
    array[6] = self.timeSaturday;
}

- (void) setProperties: (WeeklyActivitySummaryModel *)model {
    self.start = model->weekStart;
    self.end = model->weekEnd;
    self.tokens = model->tokensEarned;
    self.timeLow = model->durationByIntensity[0];
    self.timeMedium = model->durationByIntensity[1];
    self.timeHigh = model->durationByIntensity[2];
    self.timeSunday = model->durationByDay[0];
    self.timeMonday = model->durationByDay[1];
    self.timeTuesday = model->durationByDay[2];
    self.timeWednesday = model->durationByDay[3];
    self.timeThursday = model->durationByDay[4];
    self.timeFriday = model->durationByDay[5];
    self.timeSaturday = model->durationByDay[6];
}

@end
