//
//  SettingsViewModel.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef SettingsViewModel_h
#define SettingsViewModel_h

#import <Foundation/Foundation.h>

typedef struct SettingsViewModel SettingsViewModel;
typedef struct SettingsTabCoordinator SettingsTabCoordinator;
typedef struct AlertDetails AlertDetails;

struct SettingsViewModel {
    SettingsTabCoordinator *delegate;
    NSString *tokenGoalLabelFormat;
};

AlertDetails *settingsViewModel_getAlertDetailsForSaveButton(int goal, NSString *name);
AlertDetails *settingsViewModel_getAlertDetailsForDeleteButton(void);

#endif /* SettingsViewModel_h */
