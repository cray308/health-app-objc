//
//  PersistenceService.h
//  HealthApp
//
//  Created by Christopher Ray on 3/21/21.
//

#ifndef PersistenceService_h
#define PersistenceService_h

#include "CocoaHelpers.h"

extern id backgroundContext;

id createSortDescriptor(void);
id fetchRequest(void);
void setPredicate(id request, id pred);
void setDescriptors(id request, CFArrayRef descriptors);

int16_t weekData_getLiftingLimitForType(id weekData, unsigned char type);
int16_t weekData_getWorkoutTimeForType(id weekData, unsigned char type);
int16_t weekData_getTotalWorkouts(id weekData);
int64_t weekData_getWeekStart(id data);

void weekData_setWorkoutTimeForType(id weekData, unsigned char type, int16_t duration);
void weekData_setLiftingMaxForType(id weekData, unsigned char type, int16_t weight);
void weekData_setTotalWorkouts(id weekData, int16_t value);

#define createPredicate(format, ...)                                                        \
((id(*)(Class,SEL,CFStringRef,...))objc_msgSend)                                            \
 (objc_getClass("NSPredicate"), sel_getUid("predicateWithFormat:"), format, ##__VA_ARGS__)  \

void persistenceService_create(void);
void persistenceService_saveContext(void);
void persistenceService_init(void);
void persistenceService_start(int tzOffset);
void persistenceService_deleteUserData(void);
id persistenceService_getCurrentWeek(void);
CFArrayRef persistenceService_executeFetchRequest(id req, int *count);

#endif /* PersistenceService_h */
