//
//  HomeBarChartView.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef HomeBarChartView_h
#define HomeBarChartView_h

#import "HomeBarChartViewModel.h"

@interface HomeBarChartView: UIView

- (id) initWithViewModel: (HomeBarChartViewModel *)model;
- (void) updateChart;

@end

#endif /* HomeBarChartView_h */
