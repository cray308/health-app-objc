//
//  WeekStats+CoreDataClass.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//
//

#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>
#import "Constants.h"

NS_ASSUME_NONNULL_BEGIN

@interface WeekStats : NSManagedObject

@end

@interface WeekStats (CoreDataProperties)

+ (NSFetchRequest<WeekStats *> *) fetchRequest;

@property (nonatomic) double end;
@property (nonatomic) double start;
@property (nonatomic) int16_t timeFriday;
@property (nonatomic) int16_t timeHigh;
@property (nonatomic) int16_t timeLow;
@property (nonatomic) int16_t timeMedium;
@property (nonatomic) int16_t timeMonday;
@property (nonatomic) int16_t timeSaturday;
@property (nonatomic) int16_t timeSunday;
@property (nonatomic) int16_t timeThursday;
@property (nonatomic) int16_t timeTuesday;
@property (nonatomic) int16_t timeWednesday;
@property (nonatomic) int16_t tokens;

- (void) populateIntensities: (int *)array;
- (void) populateDurationByDay: (int *)array;
- (void) setProperties: (WeeklyActivitySummaryModel *)model;

@end

NS_ASSUME_NONNULL_END
