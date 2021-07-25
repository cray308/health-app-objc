//
//  HistoryViewController.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef HistoryViewController_h
#define HistoryViewController_h

#include "HistoryTabCoordinator.h"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#import "HealthApp-Swift.h"
#pragma clang diagnostic pop

@interface HistoryViewController: UIViewController<ChartAxisValueFormatter>
- (id) initWithDelegate: (HistoryTabCoordinator *)_delegate;
- (void) performForegroundUpdate;
- (void) updateCharts;
@end

#endif /* HistoryViewController_h */
