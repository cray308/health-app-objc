//
//  TotalWorkoutsChart.h
//  HealthApp
//
//  Created by Christopher Ray on 8/24/21.
//

#ifndef TotalWorkoutsChart_h
#define TotalWorkoutsChart_h

#include "HistoryViewModel.h"
#import "UIKit/UIKit.h"
@import Charts;

@interface TotalWorkoutsChart: UIView
- (id) initWithViewModel: (HistoryTotalWorkoutsChartViewModel *)viewModel
               formatter: (id<AxisValueFormatter>)xAxisFormatter;
- (void) updateWithCount: (int)count isSmall: (bool)isSmall;
@end

@interface TotalWorkoutsChart() {
    HistoryTotalWorkoutsChartViewModel *viewModel;
    @public LineChartView *chartView;
    ChartLimitLine *limitLine;
}
@end

#endif /* TotalWorkoutsChart_h */
