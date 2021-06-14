//
//  SettingsTabCoordinator.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "SettingsTabCoordinator.h"
#import "SettingsViewModel.h"
#import "SettingsViewController.h"
#import "AppUserData.h"

SettingsViewModel *settingsViewModel_init(SettingsTabCoordinator *delegate);

SettingsTabCoordinator *settingsCoordinator_init(UINavigationController *navVC, AppCoordinator *delegate) {
    SettingsTabCoordinator *coordinator = calloc(1, sizeof(SettingsTabCoordinator));
    if (!coordinator) return NULL;
    if (!(coordinator->viewModel = settingsViewModel_init(coordinator))) {
        free(coordinator);
        return NULL;
    }
    coordinator->delegate = delegate;
    coordinator->navigationController = navVC;
    return coordinator;
}

void settingsCoordinator_free(SettingsTabCoordinator *coordinator) {
    free(coordinator->viewModel);
    free(coordinator);
}

void settingsCoordinator_start(SettingsTabCoordinator *coordinator) {
    SettingsViewController *vc = [[SettingsViewController alloc] initWithViewModel:coordinator->viewModel];
    [coordinator->navigationController setViewControllers:@[vc]];
    [vc release];
}

SettingsViewModel *settingsViewModel_init(SettingsTabCoordinator *delegate) {
    SettingsViewModel *model = malloc(sizeof(SettingsViewModel));
    if (!model) return NULL;
    model->delegate = delegate;
    return model;
}

void settingsCoordinator_updateWeightText(SettingsTabCoordinator *coordinator) {
    SettingsViewController *vc = (SettingsViewController *) coordinator->navigationController.viewControllers[0];
    if (vc) [vc updateWeightFields];
}
