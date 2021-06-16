//
//  AddWorkoutUpdateMaxesViewController.h
//  HealthApp
//
//  Created by Christopher Ray on 6/13/21.
//

#ifndef AddWorkoutUpdateMaxesViewController_h
#define AddWorkoutUpdateMaxesViewController_h

#import <UIKit/UIKit.h>

typedef struct AddWorkoutViewModel AddWorkoutViewModel;

@interface AddWorkoutUpdateMaxesViewController: UIViewController<UITextFieldDelegate>

- (id) initWithViewModel: (AddWorkoutViewModel *)model;

@end


#endif /* AddWorkoutUpdateMaxesViewController_h */
