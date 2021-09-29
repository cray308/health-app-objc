//
//  LiftingChart.h
//  HealthApp
//
//  Created by Christopher Ray on 8/24/21.
//

#ifndef LiftingChart_h
#define LiftingChart_h

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

#endif /* LiftingChart_h */
