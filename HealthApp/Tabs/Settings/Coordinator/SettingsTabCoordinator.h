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

void settingsCoordinator_start(SettingsTabCoordinator *this);
void settingsCoordinator_updateWeightText(SettingsTabCoordinator *this);

#endif /* SettingsTabCoordinator_h */
