//
//  HistoryPieChartView.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef HistoryPieChartView_h
#define HistoryPieChartView_h

#import "HistoryPieChartViewModel.h"

@interface HistoryPieChartView: UIView

- (id) initWithViewModel: (HistoryPieChartViewModel *)model;
- (void) updateChart;

@end

#endif /* HistoryPieChartView_h */
