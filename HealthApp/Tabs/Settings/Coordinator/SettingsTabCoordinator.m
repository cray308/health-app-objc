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

static inline SettingsViewController *getSettingsViewController(UINavigationController *controller) {
    return (SettingsViewController *) controller.viewControllers[0];
}

SettingsViewModel *settingsViewModel_init(SettingsTabCoordinator *delegate);
void settingsViewModel_free(SettingsViewModel *model);

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
    settingsViewModel_free(coordinator->viewModel);
    free(coordinator);
}

void settingsCoordinator_start(SettingsTabCoordinator *coordinator) {
    SettingsViewController *vc = [[SettingsViewController alloc] initWithViewModel:coordinator->viewModel];
    [coordinator->navigationController setViewControllers:@[vc]];
    [vc release];
}

void settingsCoordinator_updateNavBarTokens(SettingsTabCoordinator *coordinator, NSString *label) {
    SettingsViewController *vc = getSettingsViewController(coordinator->navigationController);
    [vc updateNavBarCoins:label];
}

SettingsViewModel *settingsViewModel_init(SettingsTabCoordinator *delegate) {
    SettingsViewModel *model = malloc(sizeof(SettingsViewModel));
    if (!model) return NULL;
    model->delegate = delegate;
    model->tokenGoalLabelFormat = [[NSString alloc] initWithString:@"Token Goal: %d"];
    return model;
}

void settingsViewModel_free(SettingsViewModel *model) {
    [model->tokenGoalLabelFormat release];
    free(model);
}
