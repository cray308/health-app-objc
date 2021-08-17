//
//  PersistenceService.h
//  HealthApp
//
//  Created by Christopher Ray on 3/21/21.
//

#ifndef PersistenceService_h
#define PersistenceService_h

#import "CocoaBridging.h"

extern id persistenceServiceShared;

void persistenceService_saveContext(void);
void persistenceService_performForegroundUpdate(void);
void persistenceService_deleteUserData(void);
void persistenceService_changeTimestamps(int difference);
id persistenceService_getWeeklyDataForThisWeek(void);
id persistenceService_executeFetchRequest(id req, int *count);

#endif /* PersistenceService_h */
