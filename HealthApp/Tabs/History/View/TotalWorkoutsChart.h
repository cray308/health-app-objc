//
//  TotalWorkoutsChart.h
//  HealthApp
//
//  Created by Christopher Ray on 8/24/21.
//

#ifndef TotalWorkoutsChart_h
#define TotalWorkoutsChart_h

#include "HistoryViewModel.h"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#import "HealthApp-Swift.h"
#pragma clang diagnostic pop

@interface TotalWorkoutsChart: UIView
- (id) initWithViewModel: (HistoryTotalWorkoutsChartViewModel *)viewModel
               formatter: (id<ChartAxisValueFormatter>) xAxisFormatter;
- (void) updateWithCount: (int)count isSmall: (bool)isSmall;
@end

@interface TotalWorkoutsChart() {
    HistoryTotalWorkoutsChartViewModel *viewModel;
    @public LineChartView *chartView;
    ChartDefaultValueFormatter *valueFormatter;
    ChartLimitLine *limitLine;
}
@end

#endif /* TotalWorkoutsChart_h */
