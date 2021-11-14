//
//  SettingsTabCoordinator.c
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#include "SettingsTabCoordinator.h"
#include <CoreFoundation/CFString.h>
#import "PersistenceService.h"
#include "ViewControllerHelpers.h"

extern id settingsVC_init(void *delegate);
extern void settingsVC_updateWeightFields(id vc);
extern void appCoordinator_updatedUserInfo(bool reloadScreens);
extern void appCoordinator_deletedAppData(void);
extern bool appUserData_updateUserSettings(signed char, bool, short *);
extern void appUserData_deleteSavedData(void);
extern void persistenceService_deleteUserData(void);

static inline void addCancelAction(id ctrl) {
    addAlertAction(ctrl, localize(CFSTR("cancel")), 1, NULL);
}

void settingsCoordinator_start(SettingsTabCoordinator *this) {
    setupNavVC(this->navVC, settingsVC_init(this));
}

void settingsCoordinator_updateWeightText(SettingsTabCoordinator *this) {
    settingsVC_updateWeightFields(getFirstVC(this->navVC));
}

void settingsCoordinator_handleSaveTap(SettingsTabCoordinator *this,
                                       short *weights, signed char plan, bool dark) {
    id ctrl = createAlertController(localize(CFSTR("settingsAlertTitle")),
                                    localize(CFSTR("settingsAlertMessageSave")));
    addAlertAction(ctrl, localize(CFSTR("save")), 0, ^{
        bool rv = appUserData_updateUserSettings(plan, dark, weights);
        appCoordinator_updatedUserInfo(rv);
    });
    addCancelAction(ctrl);
    presentVC(getFirstVC(this->navVC), ctrl);
}

void settingsCoordinator_handleDeleteTap(SettingsTabCoordinator *this) {
    id ctrl = createAlertController(localize(CFSTR("settingsAlertTitle")),
                                    localize(CFSTR("settingsAlertMessageDelete")));
    addAlertAction(ctrl, localize(CFSTR("delete")), 2, ^{
        appUserData_deleteSavedData();
        appCoordinator_deletedAppData();
        persistenceService_deleteUserData();
    });
    addCancelAction(ctrl);
    presentVC(getFirstVC(this->navVC), ctrl);
}
