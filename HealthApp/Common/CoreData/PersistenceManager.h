#ifndef PersistenceManager_h
#define PersistenceManager_h

#include "HistoryModel.h"

id context_init(void);

#if DEBUG
id context_create(void);
#define context_init_first_launch() context_create()
#else
#define context_init_first_launch() context_init()
#endif

void runStartupJob(id context, HistoryModel *model, time_t weekStart, int tzDiff, bool ltr);

void deleteStoredData(id context);
void saveWorkoutData(id context, int16_t duration, unsigned char type, short *weights);

#endif /* PersistenceManager_h */
