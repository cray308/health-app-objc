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

extern id settingsVC_init(void *delegate);
extern void settingsVC_updateWeightFields(id vc);

static id cancelAction(void) {
    return createAlertAction(localize(CFSTR("cancel")), 1, NULL);
}

void settingsCoordinator_start(SettingsTabCoordinator *this) {
    setupNavVC(this->navVC, settingsVC_init(this));
}

void settingsCoordinator_updateWeightText(SettingsTabCoordinator *this) {
    settingsVC_updateWeightFields(getFirstVC(this->navVC));
}

void settingsCoordinator_handleSaveTap(SettingsTabCoordinator *this,
                                       short *weights, signed char plan) {
    id ctrl = createAlertController(localize(CFSTR("settingsAlertTitle")),
                                    localize(CFSTR("settingsAlertMessageSave")));
    addAlertAction(ctrl, createAlertAction(localize(CFSTR("save")), 0, ^{
        appUserData_updateWeightMaxes(weights);
        appUserData_setWorkoutPlan(plan);
        appCoordinator_updatedUserInfo();
    }));
    addAlertAction(ctrl, cancelAction());
    presentVC(getFirstVC(this->navVC), ctrl);
}

void settingsCoordinator_handleDeleteTap(SettingsTabCoordinator *this) {
    id ctrl = createAlertController(localize(CFSTR("settingsAlertTitle")),
                                    localize(CFSTR("settingsAlertMessageDelete")));
    addAlertAction(ctrl, createAlertAction(localize(CFSTR("delete")), 2, ^{
        appUserData_deleteSavedData();
        appCoordinator_deletedAppData();
        persistenceService_deleteUserData();
    }));
    addAlertAction(ctrl, cancelAction());
    presentVC(getFirstVC(this->navVC), ctrl);
}
