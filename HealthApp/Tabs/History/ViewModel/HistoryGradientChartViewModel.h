//
//  HistoryGradientChartViewModel.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef HistoryGradientChartViewModel_h
#define HistoryGradientChartViewModel_h

#import "HistoryViewModel.h"

struct HistoryGradientChartViewModel {
    Array_chartData *entries;
    NSString *legendLabelFormats[2];
    int totalTokens;
    int maxTokens;
    double avgTokens;

};

void historyGradientChartViewModel_updateLeftAxisData(HistoryGradientChartViewModel *model, ChartLimitLine **limitLines,
                                                      NSArray<ChartLegendEntry *> *legendEntries, double *axisMax);

#endif /* HistoryGradientChartViewModel_h */
