//
//  SettingsTabCoordinator.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "SettingsTabCoordinator.h"
#import "SettingsViewController.h"

void settingsCoordinator_free(SettingsTabCoordinator *coordinator) {
    free(coordinator);
}

void settingsCoordinator_start(SettingsTabCoordinator *coordinator) {
    UIViewController *vc = [[SettingsViewController alloc] init];
    [coordinator->navigationController setViewControllers:@[vc]];
    [vc release];
}

void settingsCoordinator_updateWeightText(SettingsTabCoordinator *coordinator) {
    SettingsViewController *vc = (SettingsViewController *) coordinator->navigationController.viewControllers[0];
    if (vc) [vc updateWeightFields];
}
