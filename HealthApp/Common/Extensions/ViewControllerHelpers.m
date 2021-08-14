//
//  ViewControllerHelpers.m
//  HealthApp
//
//  Created by Christopher Ray on 3/21/21.
//

#import "ViewControllerHelpers.h"

void createToolbar(id target, SEL doneSelector, UITextField **fields) {
    UIToolbar *toolbar = [[UIToolbar alloc] initWithFrame:(CGRect){.size = {
        .width = UIScreen.mainScreen.bounds.size.width, .height = 50
    }}];
    [toolbar sizeToFit];

    UIBarButtonItem *flexSpace = [[UIBarButtonItem alloc]
                                  initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace
                                  target:nil action:nil];
    UIBarButtonItem *doneButton = [[UIBarButtonItem alloc]
                                   initWithTitle:@"Done" style:UIBarButtonItemStylePlain
                                   target:target action:doneSelector];

    [toolbar setItems:@[flexSpace, doneButton] animated:false];
    [toolbar setUserInteractionEnabled:true];

    for (int i = 0; fields[i]; ++i) {
        fields[i].inputAccessoryView = toolbar;
    }

    [toolbar release];
    [flexSpace release];
    [doneButton release];
}

void viewController_showAlert(UIViewController *presenter, AlertDetails const* details,
                              UIAlertAction *defaultAction, UIAlertAction *secondaryAction) {
    UIAlertController *ctrl = [UIAlertController
                               alertControllerWithTitle:(__bridge NSString*)details->title
                               message:(__bridge NSString*)details->message
                               preferredStyle:UIAlertControllerStyleAlert];

    [ctrl addAction:defaultAction];
    if (secondaryAction) [ctrl addAction:secondaryAction];
    [presenter presentViewController:ctrl animated:true completion:nil];
}
