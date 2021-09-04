//
//  HistoryViewController.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef HistoryViewController_h
#define HistoryViewController_h

#include "HistoryTabCoordinator.h"
#import "UIKit/UIKit.h"
@import Charts;

@interface HistoryViewController: UIViewController<AxisValueFormatter>
- (id) initWithDelegate: (HistoryTabCoordinator *)_delegate;
- (void) performForegroundUpdate;
- (void) updateCharts;
@end

#endif /* HistoryViewController_h */
