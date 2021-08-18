//
//  SettingsTabCoordinator.c
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#include "SettingsTabCoordinator.h"

void settingsCoordinator_start(SettingsTabCoordinator *this) {
    id vc = ((id(*)(id,SEL))objc_msgSend)(allocClass("SettingsViewController"), sel_getUid("init"));
    setupNavVC(this->navVC, vc);
}

void settingsCoordinator_updateWeightText(SettingsTabCoordinator *this) {
    objc_singleArg(getFirstVC(this->navVC), sel_getUid("updateWeightFields"));
}
