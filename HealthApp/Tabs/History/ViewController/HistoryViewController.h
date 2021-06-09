//
//  HistoryViewController.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef HistoryViewController_h
#define HistoryViewController_h

#import <UIKit/UIKit.h>

typedef struct HistoryViewModel HistoryViewModel;

@interface HistoryViewController: UIViewController

- (id) initWithViewModel: (HistoryViewModel *)model;
- (void) performForegroundUpdate;
- (void) updateChartsWithAnimation: (bool)shouldAnimate;
- (void) updateNavBarCoins: (NSString *)text;

@end

#endif /* HistoryViewController_h */
