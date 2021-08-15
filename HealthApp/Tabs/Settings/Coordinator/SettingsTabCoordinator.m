//
//  SettingsTabCoordinator.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "SettingsTabCoordinator.h"
#import "SettingsViewController.h"

void settingsCoordinator_start(SettingsTabCoordinator *this) {
    UIViewController *vc = [[SettingsViewController alloc] init];
    [this->navigationController setViewControllers:@[vc]];
    [vc release];
}

void settingsCoordinator_updateWeightText(SettingsTabCoordinator *this) {
    SettingsViewController *vc = this->navigationController.viewControllers[0];
    if (vc) [vc updateWeightFields];
}
