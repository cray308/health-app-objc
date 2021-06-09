//
//  SettingsTabCoordinator.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef SettingsTabCoordinator_h
#define SettingsTabCoordinator_h

#import <UIKit/UIKit.h>

@class SettingsViewController;
typedef struct AppCoordinator AppCoordinator;
typedef struct SettingsTabCoordinator SettingsTabCoordinator;
typedef struct SettingsViewModel SettingsViewModel;

struct SettingsTabCoordinator {
    UINavigationController *navigationController;
    AppCoordinator *delegate;
    SettingsViewModel *viewModel;
};

SettingsTabCoordinator *settingsCoordinator_init(UINavigationController *navVC, AppCoordinator *delegate);
void settingsCoordinator_free(SettingsTabCoordinator *coordinator);
void settingsCoordinator_start(SettingsTabCoordinator *coordinator);

void settingsCoordinator_updateNavBarTokens(SettingsTabCoordinator *coordinator, NSString *label);

#endif /* SettingsTabCoordinator_h */
