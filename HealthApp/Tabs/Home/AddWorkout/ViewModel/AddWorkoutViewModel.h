//
//  AddWorkoutViewModel.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef AddWorkoutViewModel_h
#define AddWorkoutViewModel_h

#import <UIKit/UIKit.h>
#import "Exercise.h"

typedef struct AddWorkoutViewModel AddWorkoutViewModel;
typedef struct AddWorkoutCoordinator AddWorkoutCoordinator;

struct AddWorkoutViewModel {
    Workout *workout;
    AddWorkoutCoordinator *delegate;
    double startTime, stopTime;
};

AddWorkoutViewModel *addWorkoutViewModel_init(Workout *w);
void addWorkoutViewModel_free(AddWorkoutViewModel *model);

void addWorkoutViewModel_stoppedWorkout(AddWorkoutViewModel *model);
void addWorkoutViewModel_completedWorkout(AddWorkoutViewModel *model, UIViewController *presenter, unsigned char showModalIfRequired);
void addWorkoutViewModel_finishedAddingNewWeights(AddWorkoutViewModel *model, UIViewController *presenter, unsigned short *weights);

#endif /* AddWorkoutViewModel_h */
