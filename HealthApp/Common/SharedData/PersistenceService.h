//
//  PersistenceService.h
//  HealthApp
//
//  Created by Christopher Ray on 3/21/21.
//

#ifndef PersistenceService_h
#define PersistenceService_h

#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>

@class WeeklyData;

extern NSPersistentContainer *persistenceService_sharedContainer;

void persistenceService_setup(void);
void persistenceService_free(void);
void persistenceService_saveContext(void);
void persistenceService_performForegroundUpdate(void);
void persistenceService_deleteUserData(void);

WeeklyData *persistenceService_getWeeklyDataForThisWeek(void);


#endif /* PersistenceService_h */
