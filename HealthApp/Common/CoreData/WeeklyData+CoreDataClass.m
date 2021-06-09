//
//  WeeklyData+CoreDataClass.m
//  HealthApp
//
//  Created by Christopher Ray on 6/7/21.
//
//

#import "WeeklyData+CoreDataClass.h"

@implementation WeeklyData

@end

@implementation WeeklyData (CoreDataProperties)

+ (NSFetchRequest<WeeklyData *> *) fetchRequest {
    return [NSFetchRequest fetchRequestWithEntityName:@"WeeklyData"];
}

@dynamic bestBench;
@dynamic bestDeadlift;
@dynamic bestPullup;
@dynamic bestSquat;
@dynamic timeEndurance;
@dynamic timeHIC;
@dynamic timeSE;
@dynamic timeStrength;
@dynamic totalWorkouts;
@dynamic weekEnd;
@dynamic weekStart;

@end
