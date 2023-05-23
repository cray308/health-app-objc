#ifndef DatabaseManager_h
#define DatabaseManager_h

#include "HistoryModel.h"

void createDB(void);

void runStartupJob(HistoryModel *historyModel, time_t weekStart, int tzDiff);

void deleteStoredData(void);
void saveWorkoutData(int duration, uint8_t type, int *weights);

#endif /* DatabaseManager_h */
