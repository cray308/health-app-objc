//
//  HomeTabCoordinator.c
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#include "HomeTabCoordinator.h"
#include <CoreFoundation/CFString.h>
#include <dispatch/queue.h>
#include <objc/message.h>
#include "AddWorkoutCoordinator.h"
#include "AppUserData.h"
#include "CalendarDateHelpers.h"
#include "ViewControllerHelpers.h"
#include "ConfettiView.h"
#include "ExerciseManager.h"

extern id homeVC_init(void *);
extern void homeVC_createWorkoutsList(id);
extern void homeVC_updateWorkoutsList(id);
extern id setupWorkoutVC_init(void *, byte, Array_str *);

enum {
    CustomWorkoutIndexTestMax,
    CustomWorkoutIndexEndurance,
    CustomWorkoutIndexStrength,
    CustomWorkoutIndexSE,
    CustomWorkoutIndexHIC
};

static void navigateToAddWorkout(HomeTabCoordinator *this, bool dismissVC, Workout *workout) {
    HomeTabCoordinator *coord = this;
    AddWorkoutCoordinator *child = malloc(sizeof(AddWorkoutCoordinator));
    child->navVC = this->navVC;
    child->parent = this;
    child->workout = workout;

    if (dismissVC) {
        dismissPresentedVC(getFirstVC(this->navVC), ^{
            coord->childCoordinator = child;
            addWorkoutCoordinator_start(child);
        });
    } else {
        coord->childCoordinator = child;
        addWorkoutCoordinator_start(child);
    }
}

static void showConfetti(id vc) {
    id view = getView(vc);
    CGRect frame;
    getRect(view, &frame, 0);
    id confettiView = createConfettiView((CGRect){{0}, frame.size});
    addSubview(view, confettiView);

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC * 5),
                   dispatch_get_main_queue(), ^(void) {
        removeView(confettiView);
        releaseObj(confettiView);
        id ctrl = createAlertController(localize(CFSTR("homeAlertTitle")),
                                        localize(CFSTR("homeAlertMessage")));
        addAlertAction(ctrl, localize(CFSTR("ok")), 0, NULL);
        presentVC(vc, ctrl);
    });
}

void homeCoordinator_start(HomeTabCoordinator *this) {
    homeViewModel_init(&this->model);
    setupNavVC(this->navVC, homeVC_init(this));
}

void homeCoordinator_didFinishAddingWorkout(HomeTabCoordinator *this, int totalCompletedWorkouts) {
    id homeVC = getFirstVC(this->navVC);
    homeVC_updateWorkoutsList(homeVC);

    setBool(this->navVC, sel_getUid("popViewControllerAnimated:"), true);
    this->childCoordinator = NULL;

    if (homeViewModel_shouldShowConfetti(&this->model, totalCompletedWorkouts)) {
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC * 0.75),
                       dispatch_get_main_queue(), ^(void) {
            showConfetti(homeVC);
        });
    }
}

void homeCoordinator_addWorkoutFromPlan(HomeTabCoordinator *this, int index) {
    Workout *w = exerciseManager_getWeeklyWorkoutAtIndex(userData->currentPlan,
                                                         appUserData_getWeekInPlan(), index);
    if (w)
        navigateToAddWorkout(this, false, w);
}

void homeCoordinator_addWorkoutFromCustomButton(HomeTabCoordinator *this, int index) {
    byte type = WorkoutStrength;
    switch (index) {
        case CustomWorkoutIndexSE:
            type = WorkoutSE;
            break;
        case CustomWorkoutIndexHIC:
            type = WorkoutHIC;
            break;
        case CustomWorkoutIndexTestMax: ;
            WorkoutParams params = {-1, WorkoutStrength, 2, 1, 1, 100};
            Workout *w = exerciseManager_getWorkoutFromLibrary(&params);
            if (w)
                navigateToAddWorkout(this, false, w);
            return;
        case CustomWorkoutIndexEndurance:
            type = WorkoutEndurance;
        default:
            break;
    }

    Array_str *names = exerciseManager_getWorkoutNamesForType(type);
    if (!names) return;
    else if (!names->size) {
        free(names);
        return;
    }

    presentModalVC(getFirstVC(this->navVC), setupWorkoutVC_init(this, type, names));
}

void homeCoordinator_finishedSettingUpCustomWorkout(HomeTabCoordinator *this, void *params) {
    Workout *w = exerciseManager_getWorkoutFromLibrary(params);
    if (w)
        navigateToAddWorkout(this, true, w);
}

void homeCoordinator_checkForChildCoordinator(HomeTabCoordinator *this) {
    AddWorkoutCoordinator *child = this->childCoordinator;
    if (child) {
        addWorkoutCoordinator_stopWorkoutFromBackButtonPress(child);
        this->childCoordinator = NULL;
    }
}

void homeCoordinator_resetUI(HomeTabCoordinator *this) {
    homeViewModel_fetchData(&this->model);
    homeVC_createWorkoutsList(getFirstVC(this->navVC));
}

void homeCoordinator_updateUI(HomeTabCoordinator *this) {
    homeVC_updateWorkoutsList(getFirstVC(this->navVC));
}
