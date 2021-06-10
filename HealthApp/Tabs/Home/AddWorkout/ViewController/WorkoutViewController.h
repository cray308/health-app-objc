//
//  WorkoutViewController.h
//  HealthApp
//
//  Created by Christopher Ray on 6/9/21.
//

#ifndef WorkoutViewController_h
#define WorkoutViewController_h

#import <UIKit/UIKit.h>

typedef struct AddWorkoutViewModel AddWorkoutViewModel;

@interface WorkoutViewController: UIViewController

- (id) initWithViewModel: (AddWorkoutViewModel *)model;
- (void) finishedExerciseGroup;

@end

#endif /* WorkoutViewController_h */
