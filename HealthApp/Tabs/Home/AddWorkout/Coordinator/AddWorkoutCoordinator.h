//
//  AddWorkoutCoordinator.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef AddWorkoutCoordinator_h
#define AddWorkoutCoordinator_h

#import <UIKit/UIKit.h>
#import "Exercise.h"

typedef struct {
    Workout *workout;
    double startTime, stopTime;
} AddWorkoutViewModel;

typedef struct {
    UINavigationController *navigationController;
    AddWorkoutViewModel viewModel;
    void *parent;
} AddWorkoutCoordinator;

void addWorkoutCoordinator_start(AddWorkoutCoordinator *this);
void addWorkoutCoordinator_free(AddWorkoutCoordinator *this);

void addWorkoutCoordinator_stoppedWorkout(AddWorkoutCoordinator *this);
void addWorkoutCoordinator_completedWorkout(AddWorkoutCoordinator *this, UIViewController *presenter,
                                            bool showModalIfRequired);
void addWorkoutCoordinator_finishedAddingNewWeights(AddWorkoutCoordinator *this, UIViewController *presenter,
                                                    short *weights);
void addWorkoutCoordinator_stopWorkoutFromBackButtonPress(AddWorkoutCoordinator *this);

#endif /* AddWorkoutCoordinator_h */
