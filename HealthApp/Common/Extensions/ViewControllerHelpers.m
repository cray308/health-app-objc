//
//  ViewControllerHelpers.m
//  HealthApp
//
//  Created by Christopher Ray on 3/21/21.
//

#import "ViewControllerHelpers.h"

void alertDetails_free(AlertDetails *details);

void viewController_createToolbar(UIViewController *vc, SEL doneSelector, UITextField **fields) {
    UIToolbar *toolbar = [[UIToolbar alloc] initWithFrame:CGRectZero];
    [toolbar sizeToFit];

    UIBarButtonItem *flexSpace = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace target:nil action:nil];
    UIBarButtonItem *doneButton = [[UIBarButtonItem alloc] initWithTitle:@"Done" style:UIBarButtonItemStylePlain target:vc action:doneSelector];

    [toolbar setItems:@[flexSpace, doneButton] animated:false];
    [toolbar setUserInteractionEnabled:true];

    for (size_t i = 0; fields[i]; ++i) {
        fields[i].inputAccessoryView = toolbar;
    }

    [toolbar release];
    [flexSpace release];
    [doneButton release];
}

AlertDetails *alertDetails_init(NSString *title, NSString *message) {
    AlertDetails *details = calloc(1, sizeof(AlertDetails));
    if (!details) return NULL;

    details->style = UIAlertControllerStyleAlert;
    if (title) details->title = [[NSString alloc] initWithString:title];
    if (message) details->message = [[NSString alloc] initWithString:message];
    return details;
}

void alertDetails_free(AlertDetails *details) {
    if (!details) return;
    if (details->title)  [details->title release];
    if (details->message) [details->message release];
    free(details);
}

void viewController_showAlert(UIViewController *presenter, AlertDetails *details,
                              UIAlertAction *defaultAction, UIAlertAction *secondaryAction) {
    UIAlertController *ctrl = [UIAlertController alertControllerWithTitle:details->title
                                                                  message:details->message
                                                           preferredStyle:details->style];

    [ctrl addAction:defaultAction];
    if (secondaryAction) [ctrl addAction:secondaryAction];
    alertDetails_free(details);

    [presenter presentViewController:ctrl animated:true completion:nil];
}
