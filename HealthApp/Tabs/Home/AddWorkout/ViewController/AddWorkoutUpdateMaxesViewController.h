//
//  AddWorkoutUpdateMaxesViewController.h
//  HealthApp
//
//  Created by Christopher Ray on 6/13/21.
//

#ifndef AddWorkoutUpdateMaxesViewController_h
#define AddWorkoutUpdateMaxesViewController_h

#import <UIKit/UIKit.h>
#include "AddWorkoutCoordinator.h"
#include "CocoaHelpers.h"

@interface AddWorkoutUpdateMaxesViewController: UIViewController<UITextFieldDelegate>
- (id) initWithDelegate: (AddWorkoutCoordinator *)_delegate;
@end

#endif /* AddWorkoutUpdateMaxesViewController_h */
