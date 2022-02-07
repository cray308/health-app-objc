#ifndef PersistenceService_h
#define PersistenceService_h

#include <objc/objc.h>
#include <CoreFoundation/CFArray.h>

#define setInt16(_obj, _cmd, _arg) (((void(*)(id,SEL,int16_t))objc_msgSend)((_obj), (_cmd), (_arg)))

#define getInt16(_obj, _cmd) (((int16_t(*)(id,SEL))objc_msgSend)((_obj), (_cmd)))

#define weekData_getTotalWorkouts(_d) getInt16(_d, sel_getUid("totalWorkouts"))

#define weekData_getWeekStart(_d) getInt(_d, sel_getUid("weekStart"))

#define weekData_setTotalWorkouts(_d, _val) setInt16(_d, sel_getUid("setTotalWorkouts:"), _val)

extern id backgroundContext;

id fetchRequest(id predicate);

void weekData_getLiftingLimits(id weekData, int16_t *output);
int16_t weekData_getWorkoutTimeForType(id weekData, unsigned char type);

void weekData_setWorkoutTimeForType(id weekData, unsigned char type, int16_t duration);
void weekData_setLiftingMaxArray(id weekData, int16_t *weights);

#define runInBackground(block) ((void(*)(id,SEL,void(^)(void)))objc_msgSend)\
(backgroundContext, sel_getUid("performBlock:"), block);

#define createPredicate(format, ...)                                                        \
((id(*)(Class,SEL,CFStringRef,...))objc_msgSend)                                            \
 (objc_getClass("NSPredicate"), sel_getUid("predicateWithFormat:"), format, ##__VA_ARGS__)  \

void persistenceService_init(void);
void persistenceService_start(int tzOffset, void (*completion)(void*), void *receiver);

void persistenceService_saveContext(void);
void persistenceService_deleteUserData(void);
id persistenceService_getCurrentWeek(void);
CFArrayRef persistenceService_executeFetchRequest(id req, int *count, bool sorted);

#endif /* PersistenceService_h */
