//
//  WorkoutTypeChart.h
//  HealthApp
//
//  Created by Christopher Ray on 8/24/21.
//

#ifndef WorkoutTypeChart_h
#define WorkoutTypeChart_h

#include "HistoryViewModel.h"
#import "UIKit/UIKit.h"
@import Charts;

@interface WorkoutTypeChart: UIView<AxisValueFormatter, ValueFormatter>
- (id) initWithViewModel: (HistoryWorkoutTypeChartViewModel *)viewModel
               formatter: (id<AxisValueFormatter>)xAxisFormatter;
- (void) updateWithCount: (int)count isSmall: (bool)isSmall;
@end

@interface WorkoutTypeChart() {
    @public LineChartView *chartView;
    HistoryWorkoutTypeChartViewModel *viewModel;
}
@end

#endif /* WorkoutTypeChart_h */
