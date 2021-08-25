//
//  SettingsViewController.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef SettingsViewController_h
#define SettingsViewController_h

#include "SettingsTabCoordinator.h"
#import <UIKit/UIKit.h>

@interface SettingsViewController: UIViewController<UITextFieldDelegate>
- (id) initWithDelegate: (SettingsTabCoordinator *)delegate;
- (void) updateWeightFields;
@end

#endif /* SettingsViewController_h */
