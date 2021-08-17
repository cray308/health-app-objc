//
//  HomeTabCoordinator.c
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#include "HomeTabCoordinator.h"
#include "AddWorkoutCoordinator.h"
#include "AppUserData.h"
#include "CalendarDateHelpers.h"

typedef enum {
    CustomWorkoutIndexTestMax,
    CustomWorkoutIndexEndurance,
    CustomWorkoutIndexStrength,
    CustomWorkoutIndexSE,
    CustomWorkoutIndexHIC
} CustomWorkoutIndex;

static inline id getHomeViewController(HomeTabCoordinator *this) {
    id ctrls = ((id (*)(id, SEL)) objc_msgSend)(this->navVC, sel_getUid("viewControllers"));
    return ((id (*)(id, SEL, int)) objc_msgSend)(ctrls, sel_getUid("objectAtIndex:"), 0);
}

void navigateToAddWorkout(HomeTabCoordinator *this, bool dismissVC, Workout *workout) {
    if (dismissVC) {
        ((void (*)(id, SEL, bool, id)) objc_msgSend)
            (getHomeViewController(this), sel_getUid("dismissViewControllerAnimated:completion:"),
             true, nil);
    }
    AddWorkoutCoordinator *child = malloc(sizeof(AddWorkoutCoordinator));

    child->navVC = this->navVC;
    child->parent = this;
    child->viewModel.workout = workout;
    this->childCoordinator = child;
    addWorkoutCoordinator_start(child);
}

void homeCoordinator_start(HomeTabCoordinator *this) {
    homeViewModel_init(&this->viewModel);
    id vc = ((id (*)(id, SEL, HomeTabCoordinator *)) objc_msgSend)
        (objc_staticMethod(objc_getClass("HomeViewController"), sel_getUid("alloc")),
         sel_getUid("initWithDelegate:"), this);
    id array = ((id (*)(Class, SEL, id, ...)) objc_msgSend)
        (objc_getClass("NSArray"), sel_getUid("arrayWithObjects:"), vc, nil);
    ((void (*)(id, SEL, id)) objc_msgSend)(this->navVC, sel_getUid("setViewControllers:"), array);
    objc_singleArg(vc, sel_getUid("release"));
}

void homeCoordinator_didFinishAddingWorkout(HomeTabCoordinator *this, int totalCompletedWorkouts) {
    id homeVC = getHomeViewController(this);
    objc_singleArg(homeVC, sel_getUid("updateWorkoutsList"));

    const bool showConfetti = homeViewModel_shouldShowConfetti(&this->viewModel,
                                                               totalCompletedWorkouts);

    addWorkoutCoordinator_free(this->childCoordinator);
    this->childCoordinator = NULL;
    ((id (*)(id, SEL, bool)) objc_msgSend)
        (this->navVC, sel_getUid("popViewControllerAnimated:"), true);

    if (showConfetti) {
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC * 0.75),
                       dispatch_get_main_queue(), ^ (void) {
            objc_singleArg(homeVC, sel_getUid("showConfetti"));
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

    id modal = ((id (*)(id, SEL, HomeTabCoordinator *, unsigned char, CFStringRef *, int))
                objc_msgSend)
        (objc_staticMethod(objc_getClass("HomeSetupWorkoutModalViewController"),
                           sel_getUid("alloc")),
         sel_getUid("initWithDelegate:type:names:count:"), this, type, names, count);

    id container = ((id (*)(id, SEL, id)) objc_msgSend)
        (objc_staticMethod(objc_getClass("UINavigationController"), sel_getUid("alloc")),
         sel_getUid("initWithRootViewController:"), modal);
    ((void (*)(id, SEL, id, bool, id)) objc_msgSend)
        (getHomeViewController(this), sel_getUid("presentViewController:animated:completion:"),
         container, true, nil);

    objc_singleArg(container, sel_getUid("release"));
    objc_singleArg(modal, sel_getUid("release"));
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
        addWorkoutCoordinator_free(this->childCoordinator);
        this->childCoordinator = NULL;
    }
}

void homeCoordinator_resetUI(HomeTabCoordinator *this) {
    homeViewModel_fetchData(&this->viewModel);
    objc_singleArg(getHomeViewController(this), sel_getUid("createWorkoutsList"));
}

void homeCoordinator_updateUI(HomeTabCoordinator *this) {
    objc_singleArg(getHomeViewController(this), sel_getUid("updateWorkoutsList"));
}
