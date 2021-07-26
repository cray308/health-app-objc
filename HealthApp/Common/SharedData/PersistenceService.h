//
//  PersistenceService.h
//  HealthApp
//
//  Created by Christopher Ray on 3/21/21.
//

#ifndef PersistenceService_h
#define PersistenceService_h

#import "WeeklyData+CoreDataClass.h"

extern NSPersistentContainer *persistenceServiceShared;

void persistenceService_saveContext(void);
void persistenceService_performForegroundUpdate(void);
void persistenceService_deleteUserData(void);
WeeklyData *persistenceService_getWeeklyDataForThisWeek(void);
NSArray<id> *persistenceService_executeFetchRequest(NSFetchRequest *req, NSPredicate *pred,
                                                    NSSortDescriptor *descriptor, int *count);

#endif /* PersistenceService_h */
