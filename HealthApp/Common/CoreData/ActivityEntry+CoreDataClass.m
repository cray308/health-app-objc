//
//  ActivityEntry+CoreDataClass.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//
//

#import "ActivityEntry+CoreDataClass.h"

@implementation ActivityEntry

@end

@implementation ActivityEntry (CoreDataProperties)

+ (NSFetchRequest<ActivityEntry *> *) fetchRequest {
    return [NSFetchRequest fetchRequestWithEntityName:@"ActivityEntry"];
}

@dynamic duration;
@dynamic timestamp;
@dynamic tokens;
@dynamic type;

- (void) setProperties: (SingleActivityModel *)model {
    self.timestamp = model->date;
    self.tokens = model->tokens;
    self.duration = model->duration;
    self.type = model->intensity;
}

@end
