//
//  HistoryAreaChartView.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef HistoryAreaChartView_h
#define HistoryAreaChartView_h

#import "HistoryAreaChartViewModel.h"

@interface HistoryAreaChartView: UIView<ChartAxisValueFormatter, ChartValueFormatter>

- (id) initWithViewModel: (HistoryAreaChartViewModel *)model;
- (void) updateChart: (bool)shouldAnimate;

@end

#endif /* HistoryAreaChartView_h */
