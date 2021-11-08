//
//  PersistenceService.h
//  HealthApp
//
//  Created by Christopher Ray on 3/21/21.
//

#ifndef PersistenceService_h
#define PersistenceService_h

#include <objc/objc.h>
#include <CoreFoundation/CFArray.h>
#include "AppTypes.h"

extern id backgroundContext;

id fetchRequest(id predicate);

void weekData_getLiftingLimits(id weekData, int16_t *output);
int16_t weekData_getWorkoutTimeForType(id weekData, byte type);
int16_t weekData_getTotalWorkouts(id weekData);
int64_t weekData_getWeekStart(id data);

void weekData_setWorkoutTimeForType(id weekData, byte type, int16_t duration);
void weekData_setLiftingMaxArray(id weekData, int16_t *weights);
void weekData_setTotalWorkouts(id weekData, int16_t value);

#define runInBackground(block) ((void(*)(id,SEL,void(^)(void)))objc_msgSend)\
(backgroundContext, sel_getUid("performBlock:"), block);

#define createPredicate(format, ...)                                                        \
((id(*)(Class,SEL,CFStringRef,...))objc_msgSend)                                            \
 (objc_getClass("NSPredicate"), sel_getUid("predicateWithFormat:"), format, ##__VA_ARGS__)  \

void persistenceService_saveContext(void);
id persistenceService_getCurrentWeek(void);
CFArrayRef persistenceService_executeFetchRequest(id req, int *count, bool sorted);

#endif /* PersistenceService_h */
