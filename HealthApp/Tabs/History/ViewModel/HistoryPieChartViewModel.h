//
//  HistoryPieChartViewModel.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef HistoryPieChartViewModel_h
#define HistoryPieChartViewModel_h

#import "HistoryViewModel.h"

struct HistoryPieChartViewModel {
    int entryCount;
    int totalDailyActivity[7];
    int activitySum;
    NSString *legendLabelFormats[7];
    PieChartDataEntry *entries[7];
};

void historyPieChartViewModel_updateLegend(HistoryPieChartViewModel *model, NSArray<ChartLegendEntry*> *legendEntries);

#endif /* HistoryPieChartViewModel_h */
