//
//  WorkoutViewController.h
//  HealthApp
//
//  Created by Christopher Ray on 6/9/21.
//

#ifndef WorkoutViewController_h
#define WorkoutViewController_h

#import <UIKit/UIKit.h>
#include "AddWorkoutCoordinator.h"

@interface WorkoutViewController: UIViewController
- (id) initWithDelegate: (AddWorkoutCoordinator *)delegate;
- (void) finishedWorkoutTimerForType: (uchar)type group: (uint)group entry: (uint)entry;
@end

#endif /* WorkoutViewController_h */
