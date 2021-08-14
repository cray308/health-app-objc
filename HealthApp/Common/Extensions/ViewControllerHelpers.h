//
//  ViewControllerHelpers.h
//  HealthApp
//
//  Created by Christopher Ray on 3/21/21.
//

#ifndef ViewControllerHelpers_h
#define ViewControllerHelpers_h

#import <UIKit/UIKit.h>

typedef struct {
    CFStringRef title;
    CFStringRef message;
} AlertDetails;

void createToolbar(id target, SEL doneSelector, UITextField **fields);
void viewController_showAlert(UIViewController *presenter, AlertDetails const* details,
                              UIAlertAction *defaultAction, UIAlertAction *secondaryAction);

#endif /* ViewControllerHelpers_h */
