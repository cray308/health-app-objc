//
//  HomeSetupWorkoutModalViewController.h
//  HealthApp
//
//  Created by Christopher Ray on 6/12/21.
//

#ifndef HomeSetupWorkoutModalViewController_h
#define HomeSetupWorkoutModalViewController_h

#include "HomeTabCoordinator.h"

@interface HomeSetupWorkoutModalViewController:
UIViewController<UIPickerViewDelegate, UIPickerViewDataSource, UITextFieldDelegate>
- (id) initWithDelegate: (HomeTabCoordinator *)_delegate
                   type: (unsigned char)_type names: (CFStringRef *)_names count: (int)_count;
@end

#endif /* HomeSetupWorkoutModalViewController_h */
