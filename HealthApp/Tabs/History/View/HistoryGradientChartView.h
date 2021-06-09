//
//  HistoryGradientChartView.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef HistoryGradientChartView_h
#define HistoryGradientChartView_h

#import "HistoryGradientChartViewModel.h"

@interface HistoryGradientChartView: UIView

- (id) initWithViewModel: (HistoryGradientChartViewModel *)model;
- (void) updateChart: (bool)shouldAnimate;

@end

#endif /* HistoryGradientChartView_h */
