//
//  AddWorkoutCoordinator.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "AddWorkoutCoordinator.h"
#import "AddWorkoutViewModel.h"
#import "SingleActivityViewModel.h"
#import "AddSingleActivityModalViewController.h"
#import "AddWorkoutViewController.h"
#import "HomeTabCoordinator.h"
#import "ActivityType.h"

AddWorkoutViewController *getAddWorkoutViewController(UINavigationController *navVC) {
    return (AddWorkoutViewController *) [navVC topViewController];
}

AddWorkoutCoordinator *addWorkoutCoordinator_init(UINavigationController *navigationController, HomeTabCoordinator *delegate) {
    AddWorkoutCoordinator *coordinator = malloc(sizeof(AddWorkoutCoordinator));
    if (!coordinator) return NULL;

    AddWorkoutViewModel *model = calloc(1, sizeof(AddWorkoutViewModel));
    if (!model) { free(coordinator); return NULL; }
    model->delegate = coordinator;
    model->workouts = array_new(SingleActivityModel);
    for (int i = 0; i < 3; ++i) {
        model->intensityStrings[i] = activityType_getString(i, true);
    }

    coordinator->viewModel = model;
    coordinator->navigationController = navigationController;
    coordinator->parent = delegate;
    return coordinator;
}

void addWorkoutCoordinator_start(AddWorkoutCoordinator *coordinator) {
    AddWorkoutViewController *vc = [[AddWorkoutViewController alloc] initWithViewModel:coordinator->viewModel];
    [coordinator->navigationController pushViewController:vc animated:true];
    [vc release];
}

void addWorkoutCoordinator_free(AddWorkoutCoordinator *coordinator) {
    AddWorkoutViewModel *viewModel = coordinator->viewModel;
    array_free(SingleActivityModel, viewModel->workouts);
    for (int i = 0; i < 3; ++i) { [viewModel->intensityStrings[i] release]; }
    free(viewModel);
    free(coordinator);
}

void addWorkoutCoordinator_pushModalViewController(AddWorkoutCoordinator *coordinator) {
    AddSingleActivityViewModel *newModel = calloc(1, sizeof(AddSingleActivityViewModel));
    if (!newModel) return;
    newModel->delegate = coordinator;
    newModel->parentViewModel = coordinator->viewModel;

    AddSingleActivityModalViewController *vc = [[AddSingleActivityModalViewController alloc] initWithViewModel:newModel];
    UINavigationController *container = [[UINavigationController alloc] initWithRootViewController:vc];
    [coordinator->navigationController presentViewController:container animated:true completion:nil];
    [container release];
    [vc release];
}

void addWorkoutCoordinator_didFinishAddingActivity(AddWorkoutCoordinator *coordinator, UIViewController *presenter, int newIndex) {
    AddWorkoutViewController *vc = getAddWorkoutViewController(coordinator->navigationController);
    if (vc) {
        [vc insertEntryInRow:newIndex];
    }
    [presenter dismissViewControllerAnimated:true completion:nil];
}

void addWorkoutCoordinator_didFinishAddingWorkouts(AddWorkoutCoordinator *coordinator) {
    if (coordinator->viewModel->newTokens) { // show custom tokens alert
        AddWorkoutViewController *vc = getAddWorkoutViewController(coordinator->navigationController);
        if (vc) [vc showTokenPopup];
    } else { // newTokens portion will be ignored
        AddWorkoutViewModel *viewModel = coordinator->viewModel;
        homeCoordinator_didFinishAddingWorkouts(coordinator->parent, viewModel->newTokens, &viewModel->durations[0]);
    }
}

void addWorkoutCoordinator_didDismissAlert(AddWorkoutCoordinator *coordinator) {
    AddWorkoutViewModel *viewModel = coordinator->viewModel;
    homeCoordinator_didFinishAddingWorkouts(coordinator->parent, viewModel->newTokens, &viewModel->durations[0]);
}
