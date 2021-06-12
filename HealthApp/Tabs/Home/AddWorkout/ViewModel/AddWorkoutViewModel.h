//
//  AddWorkoutViewModel.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef AddWorkoutViewModel_h
#define AddWorkoutViewModel_h

#import "Exercise.h"
#import <UIKit/UIKit.h>

typedef struct AddWorkoutViewModel AddWorkoutViewModel;
typedef struct AddWorkoutCoordinator AddWorkoutCoordinator;

struct AddWorkoutViewModel {
    Workout *workout;
    AddWorkoutCoordinator *delegate;
};

void addWorkoutViewModel_stoppedWorkout(AddWorkoutViewModel *model, unsigned int duration);
void addWorkoutViewModel_completedWorkout(AddWorkoutViewModel *model, unsigned int duration);
void addWorkoutViewModel_finishedAddingNewWeights(AddWorkoutViewModel *model, UIViewController *presenter, unsigned short *weights);

#endif /* AddWorkoutViewModel_h */
