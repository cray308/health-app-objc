//
//  ViewControllerHelpers.h
//  HealthApp
//
//  Created by Christopher Ray on 3/21/21.
//

#ifndef ViewControllerHelpers_h
#define ViewControllerHelpers_h

#import <UIKit/UIKit.h>

typedef struct AlertDetails AlertDetails;

void viewController_createToolbar(UIViewController *vc, SEL doneSelector, UITextField **fields);

struct AlertDetails {
    NSString *title;
    NSString *message;
    UIAlertControllerStyle style;
};

AlertDetails *alertDetails_init(NSString *title, NSString *message);

void viewController_showAlert(UIViewController *presenter, AlertDetails *details,
                              UIAlertAction *defaultAction, UIAlertAction *secondaryAction);

#endif /* ViewControllerHelpers_h */
