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
