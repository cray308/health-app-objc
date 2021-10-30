#include "HistoryViewModel.h"
#import "UIKit/UIKit.h"
@import Charts;

@interface LiftingChart: UIView @end
@interface LiftingChart() {
    @public LineChartView *chartView;
    @public LiftChartViewModel *model;
}
@end

id liftingChart_init(LiftChartViewModel *model, id formatter);
void liftingChart_update(LiftingChart *this, int count, bool isSmall);
