//
//  SettingsTabCoordinator.c
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#include "SettingsTabCoordinator.h"
#include "AppUserData.h"
#include "AppCoordinator.h"
#import "PersistenceService.h"
#include "ViewControllerHelpers.h"

static id cancelAction(void) {
    return createAlertAction(CFSTR("Cancel"), 1, NULL);
}

void settingsCoordinator_start(SettingsTabCoordinator *this) {
    setupNavVC(this->navVC, createVCWithDelegate("SettingsViewController", this));
}

void settingsCoordinator_updateWeightText(SettingsTabCoordinator *this) {
    objc_singleArg(getFirstVC(this->navVC), sel_getUid("updateWeightFields"));
}

void settingsCoordinator_handleSaveTap(SettingsTabCoordinator *this,
                                       short *weights, signed char plan) {
    id ctrl = createAlertController(CFSTR("Are you sure?"),
                                    CFSTR("This will save the currently entered data."));
    addAlertAction(ctrl, createAlertAction(CFSTR("Save"), 0, ^{
        appUserData_updateWeightMaxes(weights);
        appUserData_setWorkoutPlan(plan);
        appCoordinator_updatedUserInfo(appCoordinatorShared);
    }));
    addAlertAction(ctrl, cancelAction());
    presentVC(getFirstVC(this->navVC), ctrl);
}

void settingsCoordinator_handleDeleteTap(SettingsTabCoordinator *this) {
    id ctrl = createAlertController(CFSTR("Are you sure?"),
                                    CFSTR("This will delete all workout history."));
    addAlertAction(ctrl, createAlertAction(CFSTR("Delete"), 2, ^{
        persistenceService_deleteUserData();
        appUserData_deleteSavedData();
        appCoordinator_deletedAppData(appCoordinatorShared);
    }));
    addAlertAction(ctrl, cancelAction());
    presentVC(getFirstVC(this->navVC), ctrl);
}
