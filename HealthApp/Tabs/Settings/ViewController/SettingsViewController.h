//
//  SettingsViewController.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef SettingsViewController_h
#define SettingsViewController_h

#import <UIKit/UIKit.h>

typedef struct SettingsViewModel SettingsViewModel;

@interface SettingsViewController: UIViewController<UITextFieldDelegate>

- (id) initWithViewModel: (SettingsViewModel *)model;
- (void) updateNavBarCoins: (NSString *)text;

@end

#endif /* SettingsViewController_h */
