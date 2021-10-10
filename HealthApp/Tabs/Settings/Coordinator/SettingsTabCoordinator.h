//
//  SettingsTabCoordinator.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef SettingsTabCoordinator_h
#define SettingsTabCoordinator_h

#include <objc/objc.h>
#include "AppUserData.h"

typedef struct {
    id navVC;
} SettingsTabCoordinator;

void settingsCoordinator_handleSaveTap(SettingsTabCoordinator *this,
                                       short *weights, signed char plan);
void settingsCoordinator_handleDeleteTap(SettingsTabCoordinator *this);

#endif /* SettingsTabCoordinator_h */
