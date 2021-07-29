//
//  WeeklyData+CoreDataClass.h
//  HealthApp
//
//  Created by Christopher Ray on 6/7/21.
//
//

#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>

NS_ASSUME_NONNULL_BEGIN

@interface WeeklyData : NSManagedObject

@end

@interface WeeklyData (CoreDataProperties)

+ (NSFetchRequest<WeeklyData *> *) fetchRequest;

@property (nonatomic) int16_t bestBench;
@property (nonatomic) int16_t bestDeadlift;
@property (nonatomic) int16_t bestPullup;
@property (nonatomic) int16_t bestSquat;
@property (nonatomic) int16_t timeEndurance;
@property (nonatomic) int16_t timeHIC;
@property (nonatomic) int16_t timeSE;
@property (nonatomic) int16_t timeStrength;
@property (nonatomic) int16_t totalWorkouts;
@property (nonatomic) int64_t weekStart;

@end

NS_ASSUME_NONNULL_END
