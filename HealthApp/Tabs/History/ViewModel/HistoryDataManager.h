//
//  HistoryDataManager.h
//  HealthApp
//
//  Created by Christopher Ray on 7/25/21.
//

#ifndef HistoryDataManager_h
#define HistoryDataManager_h

#include "HistoryViewModel.h"

void freeChartDataEntry(void *entry);
void historyDataManager_fetchData(HistoryViewModel *model);
void historyDataManager_createNewEntry(Array_chartData *arr, long x, int y);

#endif /* HistoryDataManager_h */
