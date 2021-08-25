//
//  LiftingChart.h
//  HealthApp
//
//  Created by Christopher Ray on 8/24/21.
//

#ifndef LiftingChart_h
#define LiftingChart_h

#include "HistoryViewModel.h"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#import "HealthApp-Swift.h"
#pragma clang diagnostic pop

@interface LiftingChart: UIView
- (id) initWithViewModel: (HistoryLiftChartViewModel *)viewModel
               formatter: (id<ChartAxisValueFormatter>) xAxisFormatter;
- (void) updateWithCount: (int)count isSmall: (bool)isSmall;
@end

@interface LiftingChart() {
    @public LineChartView *chartView;
    HistoryLiftChartViewModel *viewModel;
}
@end

#endif /* LiftingChart_h */
