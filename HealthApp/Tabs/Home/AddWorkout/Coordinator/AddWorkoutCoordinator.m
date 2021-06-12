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

AddWorkoutCoordinator *addWorkoutCoordinator_init(UINavigationController *navigationController, HomeTabCoordinator *delegate, Workout *workout) {
    AddWorkoutCoordinator *coordinator = malloc(sizeof(AddWorkoutCoordinator));
    if (!coordinator) return NULL;

    AddWorkoutViewModel *model = malloc(sizeof(AddWorkoutViewModel));
    if (!model) { free(coordinator); return NULL; }
    model->delegate = coordinator;
    model->workout = workout;

    coordinator->viewModel = model;
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
    free(coordinator->viewModel);
    free(coordinator);
}

void addWorkoutCoordinator_didFinishAddingWorkout(AddWorkoutCoordinator *coordinator, int totalCompletedWorkouts) {
    homeCoordinator_didFinishAddingWorkout(coordinator->parent, totalCompletedWorkouts);
}

void addWorkoutCoordinator_finishedUpdatingWeights(AddWorkoutCoordinator *coordinator, UIViewController *presenter) {
    [presenter dismissViewControllerAnimated:true completion:nil];
    WorkoutViewController *vc = (WorkoutViewController *) [coordinator->navigationController topViewController];
    if (vc) [vc finishedAddingNewWeights];
}
