//
//  HomeTabCoordinator.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "HomeTabCoordinator.h"
#import "AppCoordinator.h"
#import "AddWorkoutCoordinator.h"
#import "HomeViewModel.h"
#import "HomeViewController.h"
#import "CalendarDateHelpers.h"
#import "AppUserData.h"

#define UpdateGreeting 0x1
#define ResetWorkouts 0x2
#define UpdateWorkouts 0x4

HomeViewModel *homeViewModel_init(HomeTabCoordinator *delegate);
void homeViewModel_free(HomeViewModel *model);
void homeCoordinator_updateUI(HomeTabCoordinator *coordinator, unsigned char updates);

static inline HomeViewController *getHomeViewController(UINavigationController *controller) {
    return (HomeViewController *) controller.viewControllers[0];
}

HomeTabCoordinator *homeCoordinator_init(UINavigationController *navVC, AppCoordinator *delegate) {
    HomeTabCoordinator *coordinator = calloc(1, sizeof(HomeTabCoordinator));
    if (!coordinator) return NULL;
    if (!(coordinator->viewModel = homeViewModel_init(coordinator))) {
        free(coordinator);
        return NULL;
    }
    coordinator->delegate = delegate;
    coordinator->navigationController = navVC;
    return coordinator;
}

void homeCoordinator_free(HomeTabCoordinator *coordinator) {
    if (coordinator->childCoordinator) addWorkoutCoordinator_free(coordinator->childCoordinator);
    homeViewModel_free(coordinator->viewModel);
    free(coordinator);
}

void homeCoordinator_start(HomeTabCoordinator *coordinator) {
    HomeViewController *vc = [[HomeViewController alloc] initWithViewModel:coordinator->viewModel];
    [coordinator->navigationController setViewControllers:@[vc]];
    [vc release];
}

void homeCoordinator_navigateToAddWorkout(HomeTabCoordinator *coordinator, int index) {
    Array_workout *workouts = coordinator->viewModel->workouts;
    Workout *workout = array_at(workout, workouts, index);
    if (!workout) return;

    AddWorkoutCoordinator *child = addWorkoutCoordinator_init(coordinator->navigationController, coordinator, workout);
    if (!child) return;
    coordinator->childCoordinator = child;
    addWorkoutCoordinator_start(child);
}

void homeCoordinator_didFinishAddingWorkout(HomeTabCoordinator *coordinator, int totalCompletedWorkouts) {
    HomeViewController *homeVC = getHomeViewController(coordinator->navigationController);
    [homeVC updateWorkoutsList];

    unsigned char showConfetti = 0;
    Array_workout *workouts = coordinator->viewModel->workouts;
    if (workouts && array_size(workouts) == totalCompletedWorkouts) showConfetti = 1;

    addWorkoutCoordinator_free(coordinator->childCoordinator);
    coordinator->childCoordinator = NULL;
    [coordinator->navigationController popViewControllerAnimated:true];

    if (showConfetti) {
        dispatch_time_t endTime = dispatch_time(DISPATCH_TIME_NOW, (int64_t) NSEC_PER_SEC * 0.75);
        dispatch_after(endTime, dispatch_get_main_queue(), ^ (void) {
            [homeVC showConfetti];
        });
    }
}

void homeCoordinator_performForegroundUpdate(HomeTabCoordinator *coordinator) {
    homeCoordinator_updateUI(coordinator, UpdateGreeting);
}

void homeCoordinator_updateForNewWeek(HomeTabCoordinator *coordinator) {
    //homeViewModel_clear(coordinator->viewModel);
    homeCoordinator_updateUI(coordinator, ResetWorkouts | UpdateGreeting);
}

void homeCoordinator_updateForNewDay(HomeTabCoordinator *coordinator) {
    homeCoordinator_updateUI(coordinator, UpdateGreeting);
}

void homeCoordinator_handleUserInfoChange(HomeTabCoordinator *coordinator) {
    homeCoordinator_updateUI(coordinator, ResetWorkouts);
}

void homeCoordinator_handleDataDeletion(HomeTabCoordinator *coordinator) {
    homeCoordinator_updateUI(coordinator, UpdateWorkouts);
}

#pragma mark - Helpers

void homeCoordinator_updateUI(HomeTabCoordinator *coordinator, unsigned char updates) {
    HomeViewController *homeVC = getHomeViewController(coordinator->navigationController);
    if (updates & UpdateGreeting) {
        if (homeViewModel_updateTimeOfDay(coordinator->viewModel)) {
            [homeVC updateGreeting];
        }
    }
    if (updates & ResetWorkouts) {
        homeViewModel_fetchData(coordinator->viewModel);
        [homeVC createWorkoutsList];
    }
    if (updates & UpdateWorkouts) {
        [homeVC updateWorkoutsList];
    }
}

HomeViewModel *homeViewModel_init(HomeTabCoordinator *delegate) {
    HomeViewModel *model = malloc(sizeof(HomeViewModel));
    if (!model) return NULL;
    model->workouts = NULL;
    model->timeOfDay = 0;
    model->delegate = delegate;
    homeViewModel_updateTimeOfDay(model);
    return model;
}

void homeViewModel_free(HomeViewModel *model) {
    homeViewModel_clear(model);
    free(model);
}
