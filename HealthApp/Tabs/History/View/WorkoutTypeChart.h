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

@interface WorkoutTypeChart: UIView<AxisValueFormatter, ValueFormatter> @end
@interface WorkoutTypeChart() {
    @public LineChartView *chartView;
    @public WorkoutTypeChartViewModel *model;
}
@end

id workoutTypeChart_init(WorkoutTypeChartViewModel *model, id formatter);
void workoutTypeChart_update(WorkoutTypeChart *this, int count, bool isSmall);

#endif /* WorkoutTypeChart_h */
