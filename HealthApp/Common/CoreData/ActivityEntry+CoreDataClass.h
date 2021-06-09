//
//  ActivityEntry+CoreDataClass.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//
//

#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>
#import "Constants.h"

NS_ASSUME_NONNULL_BEGIN

@interface ActivityEntry : NSManagedObject

@end

@interface ActivityEntry (CoreDataProperties)

+ (NSFetchRequest<ActivityEntry *> *) fetchRequest;

@property (nonatomic) int16_t duration;
@property (nonatomic) double timestamp;
@property (nonatomic) int16_t tokens;
@property (nonatomic) int16_t type;

- (void) setProperties:(SingleActivityModel*)model;

@end

NS_ASSUME_NONNULL_END
