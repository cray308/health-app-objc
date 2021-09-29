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

@interface TotalWorkoutsChart: UIView @end
@interface TotalWorkoutsChart() {
    @public TotalWorkoutsChartViewModel *model;
    @public LineChartView *chartView;
    @public ChartLimitLine *limitLine;
}
@end

id totalWorkoutsChart_init(TotalWorkoutsChartViewModel *model, id formatter);
void totalWorkoutsChart_update(TotalWorkoutsChart *this, int count, bool isSmall);

#endif /* TotalWorkoutsChart_h */
