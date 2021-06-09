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

struct AddWorkoutCoordinator {
    UINavigationController *navigationController;
    AddWorkoutViewModel *viewModel;
    HomeTabCoordinator *parent;
};

AddWorkoutCoordinator *addWorkoutCoordinator_init(UINavigationController *navigationController, HomeTabCoordinator *delegate);
void addWorkoutCoordinator_start(AddWorkoutCoordinator *coordinator);
void addWorkoutCoordinator_free(AddWorkoutCoordinator *coordinator);

void addWorkoutCoordinator_pushModalViewController(AddWorkoutCoordinator *coordinator);
void addWorkoutCoordinator_didFinishAddingActivity(AddWorkoutCoordinator *coordinator, UIViewController *presenter, int newIndex);

void addWorkoutCoordinator_didFinishAddingWorkouts(AddWorkoutCoordinator *coordinator);
void addWorkoutCoordinator_didDismissAlert(AddWorkoutCoordinator *coordinator);

#endif /* AddWorkoutCoordinator_h */
