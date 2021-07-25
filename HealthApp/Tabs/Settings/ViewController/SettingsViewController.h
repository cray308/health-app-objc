//
//  SettingsViewController.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef SettingsViewController_h
#define SettingsViewController_h

#include "SettingsTabCoordinator.h"

@interface SettingsViewController: UIViewController<UITextFieldDelegate>
- (id) init;
- (void) updateWeightFields;
@end

#endif /* SettingsViewController_h */
