//
//  ViewControllerHelpers.h
//  HealthApp
//
//  Created by Christopher Ray on 3/21/21.
//

#ifndef ViewControllerHelpers_h
#define ViewControllerHelpers_h

#import <UIKit/UIKit.h>

void viewControllerHelper_setupValidNumericChars(void);
void viewControllerHelper_cleanupValidNumericChars(void);

void createToolbar(id target, SEL doneSelector, UITextField **fields);

typedef struct {
    CFStringRef title;
    CFStringRef message;
} AlertDetails;

void viewController_showAlert(UIViewController *presenter, AlertDetails const* details, UIAlertAction *defaultAction,
                              UIAlertAction *secondaryAction);
bool viewController_validateNumericInput(CFStringRef str);

#endif /* ViewControllerHelpers_h */
