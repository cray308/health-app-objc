//
//  AddWorkoutCoordinator.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef AddWorkoutCoordinator_h
#define AddWorkoutCoordinator_h

#import <UIKit/UIKit.h>

@class AddWorkoutViewController;
typedef struct AddWorkoutCoordinator AddWorkoutCoordinator;
typedef struct HomeTabCoordinator HomeTabCoordinator;
typedef struct AddWorkoutViewModel AddWorkoutViewModel;
typedef struct Workout Workout;

struct AddWorkoutCoordinator {
    UINavigationController *navigationController;
    AddWorkoutViewModel *viewModel;
    HomeTabCoordinator *parent;
};

AddWorkoutCoordinator *addWorkoutCoordinator_init(UINavigationController *navigationController, HomeTabCoordinator *delegate, Workout *workout);
void addWorkoutCoordinator_start(AddWorkoutCoordinator *coordinator);
void addWorkoutCoordinator_free(AddWorkoutCoordinator *coordinator);

void addWorkoutCoordinator_didFinishAddingWorkout(AddWorkoutCoordinator *coordinator, int totalCompletedWorkouts);
void addWorkoutCoordinator_finishedUpdatingWeights(AddWorkoutCoordinator *coordinator, UIViewController *presenter);

#endif /* AddWorkoutCoordinator_h */
