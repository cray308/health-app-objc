//
//  HistoryAreaChartViewModel.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef HistoryAreaChartViewModel_h
#define HistoryAreaChartViewModel_h

#import "HistoryViewModel.h"

struct HistoryAreaChartViewModel {
    Array_chartData *entries[4];
    NSString *legendLabelPrefixes[3];
    int maxActivityTime;
    int totalByIntensity[3];
};

void historyAreaChartViewModel_updateLegend(HistoryAreaChartViewModel *model, NSArray<ChartLegendEntry*> *entries);

#endif /* HistoryAreaChartViewModel_h */
