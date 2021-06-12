//
//  SettingsViewModel.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef SettingsViewModel_h
#define SettingsViewModel_h

typedef struct SettingsViewModel SettingsViewModel;
typedef struct SettingsTabCoordinator SettingsTabCoordinator;

struct SettingsViewModel {
    SettingsTabCoordinator *delegate;
};

#endif /* SettingsViewModel_h */
