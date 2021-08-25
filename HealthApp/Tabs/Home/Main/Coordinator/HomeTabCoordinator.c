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
#include "ViewControllerHelpers.h"
#include "ConfettiView.h"

typedef enum {
    CustomWorkoutIndexTestMax,
    CustomWorkoutIndexEndurance,
    CustomWorkoutIndexStrength,
    CustomWorkoutIndexSE,
    CustomWorkoutIndexHIC
} CustomWorkoutIndex;

static void navigateToAddWorkout(HomeTabCoordinator *this, bool dismissVC, Workout *workout) {
    if (dismissVC) {
        dismissPresentedVC(getFirstVC(this->navVC));
    }
    AddWorkoutCoordinator *child = malloc(sizeof(AddWorkoutCoordinator));

    child->navVC = this->navVC;
    child->parent = this;
    child->viewModel.workout = workout;
    this->childCoordinator = child;
    addWorkoutCoordinator_start(child);
}

static void showConfetti(id vc) {
    id ctrl = createAlertController(CFSTR("Nicely done!"),
                                    CFSTR("Great job meeting your workout goal this week."));
    addAlertAction(ctrl, createAlertAction(CFSTR("OK"), 0, NULL));

    id view = getRootView(vc);
    CGRect frame;
    getViewFrame(view, &frame);
    id confettiView = createConfettiView((CGRect){{0}, frame.size});
    addSubview(view, confettiView);

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC * 5),
                   dispatch_get_main_queue(), ^(void) {
        objc_singleArg(confettiView, sel_getUid("removeFromSuperview"));
        releaseObj(confettiView);
        presentVC(vc, ctrl);
    });
}

void homeCoordinator_start(HomeTabCoordinator *this) {
    homeViewModel_init(&this->viewModel);
    id vc = createVCWithDelegate("HomeViewController", this);
    setupNavVC(this->navVC, vc);
}

void homeCoordinator_didFinishAddingWorkout(HomeTabCoordinator *this, int totalCompletedWorkouts) {
    id homeVC = getFirstVC(this->navVC);
    objc_singleArg(homeVC, sel_getUid("updateWorkoutsList"));

    const bool confetti = homeViewModel_shouldShowConfetti(&this->viewModel,
                                                           totalCompletedWorkouts);

    addWorkoutCoordinator_free(this->childCoordinator);
    this->childCoordinator = NULL;
    ((id(*)(id,SEL,bool))objc_msgSend)(this->navVC, sel_getUid("popViewControllerAnimated:"), true);

    if (confetti) {
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC * 0.75),
                       dispatch_get_main_queue(), ^(void) {
            showConfetti(homeVC);
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

    Array_str *names = exerciseManager_getWorkoutNamesForType(type);
    if (!names) return;
    else if (!names->size) {
        free(names);
        return;
    }

    id modal = ((id(*)(id,SEL,HomeTabCoordinator*,unsigned char,Array_str*))objc_msgSend)
    (allocClass("HomeSetupWorkoutModalViewController"),
     sel_getUid("initWithDelegate:type:names:"), this, type, names);
    presentModalVC(getFirstVC(this->navVC), modal);
}

void homeCoordinator_finishedSettingUpCustomWorkout(HomeTabCoordinator *this, unsigned char type,
                                                    int index, short *params) {
    Workout *w = exerciseManager_getWorkoutFromLibrary(type, index,
                                                       params[0], params[1], params[2]);
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
    objc_singleArg(getFirstVC(this->navVC), sel_getUid("createWorkoutsList"));
}

void homeCoordinator_updateUI(HomeTabCoordinator *this) {
    objc_singleArg(getFirstVC(this->navVC), sel_getUid("updateWorkoutsList"));
}
