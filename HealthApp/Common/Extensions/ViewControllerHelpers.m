//
//  ViewControllerHelpers.m
//  HealthApp
//
//  Created by Christopher Ray on 3/21/21.
//

#import "ViewControllerHelpers.h"
#include "unordered_set.h"

gen_uset(char, unsigned short, ds_cmp_num_eq, DSDefault_addrOfVal, DSDefault_sizeOfVal, DSDefault_shallowCopy, DSDefault_shallowDelete)

static USet_char *validChars;

void viewControllerHelper_setupValidNumericChars(void) {
    validChars = uset_new_fromArray(char, ((unsigned short[]){'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'}), 10);
}

void viewControllerHelper_cleanupValidNumericChars(void) {
    uset_free(char, validChars);
}

void createToolbar(id target, SEL doneSelector, UITextField **fields) {
    UIToolbar *toolbar = [[UIToolbar alloc] initWithFrame:CGRectMake(0, 0, UIScreen.mainScreen.bounds.size.width, 50)];
    [toolbar sizeToFit];

    UIBarButtonItem *flexSpace = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace target:nil action:nil];
    UIBarButtonItem *doneButton = [[UIBarButtonItem alloc] initWithTitle:@"Done" style:UIBarButtonItemStylePlain target:target action:doneSelector];

    [toolbar setItems:@[flexSpace, doneButton] animated:false];
    [toolbar setUserInteractionEnabled:true];

    for (size_t i = 0; fields[i]; ++i) {
        fields[i].inputAccessoryView = toolbar;
    }

    [toolbar release];
    [flexSpace release];
    [doneButton release];
}

AlertDetails *alertDetails_init(CFStringRef title, CFStringRef message) {
    AlertDetails *details = calloc(1, sizeof(AlertDetails));
    if (!details) return NULL;

    details->style = UIAlertControllerStyleAlert;
    if (title) details->title = CFStringCreateCopy(NULL, title);
    if (message) details->message = CFStringCreateCopy(NULL, message);
    return details;
}

void viewController_showAlert(UIViewController *presenter, AlertDetails *details,
                              UIAlertAction *defaultAction, UIAlertAction *secondaryAction) {
    UIAlertController *ctrl = [UIAlertController alertControllerWithTitle:(__bridge NSString*)details->title
                                                                  message:(__bridge NSString*)details->message
                                                           preferredStyle:details->style];

    [ctrl addAction:defaultAction];
    if (secondaryAction) [ctrl addAction:secondaryAction];
    if (details->title) CFRelease(details->title);
    if (details->message) CFRelease(details->message);
    free(details);
    [presenter presentViewController:ctrl animated:true completion:nil];
}

bool viewController_validateNumericInput(CFStringRef str) {
    long len = CFStringGetLength(str);
    if (!len) return true;

    CFStringInlineBuffer buf;
    CFStringInitInlineBuffer(str, &buf, CFRangeMake(0, len));
    for (long i = 0; i < len; ++i) {
        if (!uset_contains(char, validChars, CFStringGetCharacterFromInlineBuffer(&buf, i))) return false;
    }
    return true;
}
