//
//  SettingsViewModel.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "SettingsViewModel.h"
#import "ViewControllerHelpers.h"

AlertDetails *settingsViewModel_getAlertDetailsForSaveButton(int goal, NSString *name) {
    NSString *message = [[NSString alloc] initWithFormat:@"Press 'Save' to save %d as your new token goal and \"%@\" as your new username, or 'Cancel' to cancel.", goal, name];
    AlertDetails *details = alertDetails_init(@"Are you sure?", message);
    [message release];
    return details;
}

AlertDetails *settingsViewModel_getAlertDetailsForDeleteButton(void) {
    AlertDetails *details = alertDetails_init(@"Are you sure?", @"Are you sure? This action cannot be undone.");
    return details;
}
