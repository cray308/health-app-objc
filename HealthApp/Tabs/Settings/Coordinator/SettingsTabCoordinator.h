//
//  SettingsTabCoordinator.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef SettingsTabCoordinator_h
#define SettingsTabCoordinator_h

#import <UIKit/UIKit.h>

typedef struct {
    UINavigationController *navigationController;
} SettingsTabCoordinator;

void settingsCoordinator_free(SettingsTabCoordinator *coordinator);
void settingsCoordinator_start(SettingsTabCoordinator *coordinator);
void settingsCoordinator_updateWeightText(SettingsTabCoordinator *coordinator);

#endif /* SettingsTabCoordinator_h */
