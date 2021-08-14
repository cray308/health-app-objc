//
//  HomeTabCoordinator.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "HomeTabCoordinator.h"
#import "AppDelegate.h"
#import "AddWorkoutCoordinator.h"
#import "HomeViewController.h"
#import "HomeSetupWorkoutModalViewController.h"
#include "AppUserData.h"
#include "CalendarDateHelpers.h"

#define UpdateGreeting 0x1
#define ResetWorkouts 0x2
#define UpdateWorkouts 0x4

typedef enum {
    CustomWorkoutIndexTestMax,
    CustomWorkoutIndexEndurance,
    CustomWorkoutIndexStrength,
    CustomWorkoutIndexSE,
    CustomWorkoutIndexHIC
} CustomWorkoutIndex;

static inline HomeViewController *getHomeViewController(UINavigationController *controller) {
    return (HomeViewController *) controller.viewControllers[0];
}

void updateUI(HomeTabCoordinator *this, unsigned char updates) {
    HomeViewController *homeVC = getHomeViewController(this->navigationController);
    if (updates & ResetWorkouts) {
        homeViewModel_fetchData(&this->viewModel);
        [homeVC createWorkoutsList];
    }
    if (updates & UpdateWorkouts) {
        [homeVC updateWorkoutsList];
    }
}

void navigateToAddWorkout(HomeTabCoordinator *this, bool dismissVC, Workout *workout) {
    if (dismissVC) [this->navigationController.viewControllers[0]
                    dismissViewControllerAnimated:true completion:nil];
    AddWorkoutCoordinator *child = malloc(sizeof(AddWorkoutCoordinator));
    if (!child) return;

    child->navigationController = this->navigationController;
    child->parent = this;
    child->viewModel.workout = workout;
    this->childCoordinator = child;
    addWorkoutCoordinator_start(child);
}

void homeCoordinator_free(HomeTabCoordinator *this) {
    if (this->childCoordinator) addWorkoutCoordinator_free(this->childCoordinator);
    homeViewModel_free(&this->viewModel);
    free(this);
}

void homeCoordinator_start(HomeTabCoordinator *this) {
    homeViewModel_init(&this->viewModel);
    HomeViewController *vc = [[HomeViewController alloc] initWithDelegate:this];
    [this->navigationController setViewControllers:@[vc]];
    [vc release];
}

void homeCoordinator_didFinishAddingWorkout(HomeTabCoordinator *this, int totalCompletedWorkouts) {
    HomeViewController *homeVC = getHomeViewController(this->navigationController);
    [homeVC updateWorkoutsList];

    const bool showConfetti = homeViewModel_shouldShowConfetti(&this->viewModel,
                                                               totalCompletedWorkouts);

    addWorkoutCoordinator_free(this->childCoordinator);
    this->childCoordinator = NULL;
    [this->navigationController popViewControllerAnimated:true];

    if (showConfetti) {
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC * 0.75),
                       dispatch_get_main_queue(), ^ (void) {
            [homeVC showConfetti];
        });
    }
}

void homeCoordinator_addWorkoutFromPlan(HomeTabCoordinator *this, int index) {
    unsigned char plan = (unsigned char) appUserDataShared->currentPlan;
    Workout *w = exerciseManager_getWeeklyWorkoutAtIndex(plan, appUserData_getWeekInPlan(), index);
    if (w) navigateToAddWorkout(this, false, w);
}

void homeCoordinator_addWorkoutFromCustomButton(HomeTabCoordinator *this, int index) {
    unsigned char type = WorkoutTypeStrength;
    switch (index) {
        case CustomWorkoutIndexSE:
            type = WorkoutTypeSE;
            break;
        case CustomWorkoutIndexHIC:
            type = WorkoutTypeHIC;
            break;
        case CustomWorkoutIndexTestMax: ;
            Workout *w = exerciseManager_getWorkoutFromLibrary(WorkoutTypeStrength, 2, 1, 1, 100);
            if (w) navigateToAddWorkout(this, false, w);
            return;
        case CustomWorkoutIndexEndurance:
            type = WorkoutTypeEndurance;
        default:
            break;
    }

    int count = 0;
    CFStringRef *names = exerciseManager_getWorkoutNamesForType(type, &count);
    if (!names) return;
    else if (!count) {
        free(names);
        return;
    }

    UIViewController *modal = [[HomeSetupWorkoutModalViewController alloc]
                               initWithDelegate:this type:type names:names count:count];
    UINavigationController *container = [[UINavigationController alloc]
                                         initWithRootViewController:modal];
    [this->navigationController.viewControllers[0] presentViewController:container animated:true
                                                              completion:nil];
    [container release];
    [modal release];
}

void homeCoordinator_finishedSettingUpCustomWorkout(HomeTabCoordinator *this, unsigned char type,
                                                    int index, int sets, int reps, int weight) {
    Workout *w = exerciseManager_getWorkoutFromLibrary(type, index, reps, sets, weight);
    if (w) navigateToAddWorkout(this, true, w);
}

void homeCoordinator_checkForChildCoordinator(HomeTabCoordinator *this) {
    AddWorkoutCoordinator *child = this->childCoordinator;
    if (child) {
        addWorkoutCoordinator_stopWorkoutFromBackButtonPress(child);
        this->childCoordinator = NULL;
    }
}

void homeCoordinator_resetUI(HomeTabCoordinator *this) {
    HomeViewController *homeVC = getHomeViewController(this->navigationController);
    homeViewModel_fetchData(&this->viewModel);
    [homeVC createWorkoutsList];
}

void homeCoordinator_updateUI(HomeTabCoordinator *this) {
    HomeViewController *homeVC = getHomeViewController(this->navigationController);
    [homeVC updateWorkoutsList];
}
