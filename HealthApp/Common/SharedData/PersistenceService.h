#ifndef PersistenceService_h
#define PersistenceService_h

#include <objc/objc.h>
#include <CoreFoundation/CFArray.h>

#define weekData_getTotalWorkouts(d) msg0(int16_t,d,sel_getUid("totalWorkouts"))
#define weekData_setTotalWorkouts(d, val) msg1(void,int16_t,d,sel_getUid("setTotalWorkouts:"),val)
#define weekData_getWeekStart(d) msg0(int64_t,d,sel_getUid("weekStart"))

extern id backgroundContext;

void weekData_getLiftingLimits(id weekData, int16_t *output);
int16_t weekData_getWorkoutTimeForType(id weekData, int type);

void weekData_setWorkoutTimeForType(id weekData, int type, int16_t duration);
void weekData_setLiftingMaxArray(id weekData, int16_t *weights);

void persistenceService_init(void);
void persistenceService_start(int tzOffset, time_t weekStart,
                              void (*completion)(void*), void *receiver);

void persistenceService_saveContext(id context);
void persistenceService_deleteUserData(void);
CFArrayRef persistenceService_fetchData(id context, int options, int *count);

#endif /* PersistenceService_h */
