//
//  HomeViewController.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef HomeViewController_h
#define HomeViewController_h

#import <UIKit/UIKit.h>

typedef struct HomeViewModel HomeViewModel;

@interface HomeViewController: UIViewController

- (id) initWithViewModel: (HomeViewModel *)model;
- (void) showConfetti;
- (void) updateGreeting;
- (void) createWorkoutsList;
- (void) updateWorkoutsList;

@end

#endif /* HomeViewController_h */
