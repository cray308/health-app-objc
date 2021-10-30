#import <CoreData/CoreData.h>

NS_ASSUME_NONNULL_BEGIN

@interface WeeklyData: NSManagedObject @end
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

@implementation WeeklyData @end
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
@dynamic weekStart;
@end
