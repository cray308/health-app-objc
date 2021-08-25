//
//  WorkoutTypeChart.h
//  HealthApp
//
//  Created by Christopher Ray on 8/24/21.
//

#ifndef WorkoutTypeChart_h
#define WorkoutTypeChart_h

#include "HistoryViewModel.h"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#import "HealthApp-Swift.h"
#pragma clang diagnostic pop

@interface WorkoutTypeChart: UIView<ChartAxisValueFormatter, ChartValueFormatter>
- (id) initWithViewModel: (HistoryWorkoutTypeChartViewModel *)viewModel
               formatter: (id<ChartAxisValueFormatter>)xAxisFormatter;
- (void) updateWithCount: (int)count isSmall: (bool)isSmall;
@end

@interface WorkoutTypeChart() {
    @public LineChartView *chartView;
    HistoryWorkoutTypeChartViewModel *viewModel;
}
@end

#endif /* WorkoutTypeChart_h */
