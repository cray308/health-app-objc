//
//  HomeSetupWorkoutModalViewController.h
//  HealthApp
//
//  Created by Christopher Ray on 6/12/21.
//

#ifndef HomeSetupWorkoutModalViewController_h
#define HomeSetupWorkoutModalViewController_h

#include "Exercise.h"
#import <UIKit/UIKit.h>

@interface HomeSetupWorkoutModalViewController:
UIViewController<UIPickerViewDelegate, UIPickerViewDataSource, UITextFieldDelegate>
- (id) initWithDelegate: (void *)delegate type: (uchar)type names: (Array_str *)names;
@end

#endif /* HomeSetupWorkoutModalViewController_h */
