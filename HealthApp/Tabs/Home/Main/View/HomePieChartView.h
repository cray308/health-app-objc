//
//  HomePieChartView.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef HomePieChartView_h
#define HomePieChartView_h

#import "HomePieChartViewModel.h"

@interface HomePieChartView: UIView

- (id) initWithViewModel: (HomePieChartViewModel *)model;
- (void) updateChart;

@end

#endif /* HomePieChartView_h */
