#include "HistoryViewModel.h"
#import "UIKit/UIKit.h"
@import Charts;

@interface TotalWorkoutsChart: UIView @end
@interface TotalWorkoutsChart() {
    @public TotalWorkoutsChartViewModel *model;
    @public LineChartView *chartView;
    @public ChartLimitLine *limitLine;
}
@end

id totalWorkoutsChart_init(TotalWorkoutsChartViewModel *model, id formatter);
void totalWorkoutsChart_update(TotalWorkoutsChart *this, int count, bool isSmall);
