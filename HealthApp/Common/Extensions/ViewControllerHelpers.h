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

void viewControllerHelper_setupValidNumericChars(void);
void viewControllerHelper_cleanupValidNumericChars(void);

void createToolbar(id target, SEL doneSelector, UITextField **fields);

struct AlertDetails {
    CFStringRef title;
    CFStringRef message;
    UIAlertControllerStyle style;
};

AlertDetails *alertDetails_init(CFStringRef title, CFStringRef message);

void viewController_showAlert(UIViewController *presenter, AlertDetails *details, UIAlertAction *defaultAction, UIAlertAction *secondaryAction);
bool viewController_validateNumericInput(CFStringRef str);

#endif /* ViewControllerHelpers_h */
