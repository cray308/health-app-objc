//
//  AddWorkoutCoordinator.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "AddWorkoutCoordinator.h"
#import "AddWorkoutViewModel.h"
#import "WorkoutViewController.h"
#import "HomeTabCoordinator.h"
#import "AppDelegate.h"
#import "AppCoordinator.h"

AddWorkoutCoordinator *addWorkoutCoordinator_init(UINavigationController *navigationController, HomeTabCoordinator *delegate, Workout *workout) {
    AddWorkoutCoordinator *coordinator = malloc(sizeof(AddWorkoutCoordinator));
    if (!coordinator) return NULL;

    coordinator->viewModel = addWorkoutViewModel_init(workout);
    if (!coordinator->viewModel) {
        free(coordinator);
        return NULL;
    }
    coordinator->viewModel->delegate = coordinator;
    coordinator->navigationController = navigationController;
    coordinator->parent = delegate;
    return coordinator;
}

void addWorkoutCoordinator_start(AddWorkoutCoordinator *coordinator) {
    WorkoutViewController *vc = [[WorkoutViewController alloc] initWithViewModel:coordinator->viewModel];
    [coordinator->navigationController pushViewController:vc animated:true];
    [vc release];
}

void addWorkoutCoordinator_free(AddWorkoutCoordinator *coordinator) {
    addWorkoutViewModel_free(coordinator->viewModel);
    free(coordinator);
}

void addWorkoutCoordinator_didFinishAddingWorkout(AddWorkoutCoordinator *coordinator, UIViewController *presenter, int totalCompletedWorkouts) {
    if (presenter) {
        [presenter dismissViewControllerAnimated:true completion:nil];
        AppDelegate *delegate = (AppDelegate *) UIApplication.sharedApplication.delegate;
        if (delegate) {
            appCoordinator_updateMaxWeights([delegate getAppCoordinator]);
        }
    }
    homeCoordinator_didFinishAddingWorkout(coordinator->parent, totalCompletedWorkouts);
}
