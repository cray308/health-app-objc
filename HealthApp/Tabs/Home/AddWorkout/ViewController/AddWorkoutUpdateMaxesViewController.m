//
//  AddWorkoutUpdateMaxesViewController.m
//  HealthApp
//
//  Created by Christopher Ray on 6/13/21.
//

#import "AddWorkoutUpdateMaxesViewController.h"
#include "AddWorkoutCoordinator.h"
#include "ViewControllerHelpers.h"

@interface UpdateMaxesSheet() {
    @public AddWorkoutCoordinator *delegate;
    @public Validator validator;
}
@end

id updateMaxesVC_init(AddWorkoutCoordinator *delegate) {
    UpdateMaxesSheet *this = [[UpdateMaxesSheet alloc] initWithNibName:nil bundle:nil];
    this->delegate = delegate;
    return this;
}

@implementation UpdateMaxesSheet
- (void) dealloc {
    textValidator_free(&validator);
    [super dealloc];
}

- (void) viewDidLoad {
    [super viewDidLoad];
    setBackground(self.view, UIColor.secondarySystemBackgroundColor);
    textValidator_setup(&validator);

    UIToolbar *toolbar = createToolbar(self, @selector(dismissKeyboard));
    UIView *views[4];
    UILayoutGuide *guide = self.view.safeAreaLayoutGuide;

    for (int i = 0; i < 4; ++i) {
        CFStringRef key = CFStringCreateWithFormat(NULL, NULL, CFSTR("maxWeight%d"), i);
        views[i] = validator_addChild(&validator, self, localize(key), 1, 999, toolbar);
        [self.view addSubview:views[i]];
        pin(views[i], guide, (Padding){0}, EdgeTop | EdgeBottom);
        CFRelease(key);
    }

    UIButton *finishButton = createButton(localize(CFSTR("finish")), UIColor.systemBlueColor, 0,
                                          0, self, @selector(didPressFinish));
    setNavButton(self.navigationItem, false, finishButton, self.view.frame.size.width);
    enableButton(finishButton, false);
    validator.button = finishButton;

    pinTopToTop(views[0], self.view.safeAreaLayoutGuide, 20);
    for (int i = 1; i < 4; ++i)
        pinTopToBottom(views[i], views[i - 1], 0);

    [toolbar release];
}

- (void) didPressFinish {
    delegate->workout->newLifts = malloc(4 * sizeof(short));
    for (int i = 0; i < 4; ++i)
        delegate->workout->newLifts[i] = validator.children[i].result;
    addWorkoutCoordinator_completedWorkout(delegate, true, false);
}

- (void) dismissKeyboard { [self.view endEditing:true]; }

- (BOOL) textField: (UITextField *)textField
shouldChangeCharactersInRange: (NSRange)range replacementString: (NSString *)string {
    return checkInput(&validator, textField, (CFRange){range.location,range.length},_cfstr(string));
}

- (BOOL) textFieldShouldReturn: (UITextField *)textField {
    return [textField resignFirstResponder];
}
@end
