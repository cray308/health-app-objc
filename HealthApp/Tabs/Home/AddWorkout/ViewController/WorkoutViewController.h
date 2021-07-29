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
- (id) initWithDelegate: (AddWorkoutCoordinator *)_delegate;
- (void) finishedExerciseGroup;
- (void) finishedWorkoutTimerForType: (unsigned char)type container: (int)container exercise: (int)exercise;
@end

#endif /* WorkoutViewController_h */
