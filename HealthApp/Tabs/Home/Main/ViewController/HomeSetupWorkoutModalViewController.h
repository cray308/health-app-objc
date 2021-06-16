//
//  HomeSetupWorkoutModalViewController.h
//  HealthApp
//
//  Created by Christopher Ray on 6/12/21.
//

#ifndef HomeSetupWorkoutModalViewController_h
#define HomeSetupWorkoutModalViewController_h

#import <UIKit/UIKit.h>

typedef struct HomeViewModel HomeViewModel;

@interface HomeSetupWorkoutModalViewController: UIViewController<UIPickerViewDelegate, UIPickerViewDataSource, UITextFieldDelegate>

- (id) initWithViewModel: (HomeViewModel *)model type: (unsigned char)_type names: (CFStringRef *)_names count: (unsigned int)_count;

@end


#endif /* HomeSetupWorkoutModalViewController_h */
