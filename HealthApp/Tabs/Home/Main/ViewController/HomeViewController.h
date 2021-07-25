//
//  HomeViewController.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef HomeViewController_h
#define HomeViewController_h

#import <UIKit/UIKit.h>
#include "HomeTabCoordinator.h"

@interface HomeViewController: UIViewController
- (id) initWithDelegate: (HomeTabCoordinator *)_delegate;
- (void) showConfetti;
- (void) updateGreeting;
- (void) createWorkoutsList;
- (void) updateWorkoutsList;
@end

#endif /* HomeViewController_h */
