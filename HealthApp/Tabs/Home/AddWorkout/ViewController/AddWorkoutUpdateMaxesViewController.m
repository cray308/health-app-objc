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

id updateMaxesVC_init(void *delegate) {
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
        activateConstraints((id []){
            [views[i].leadingAnchor constraintEqualToAnchor:guide.leadingAnchor],
            [views[i].trailingAnchor constraintEqualToAnchor:guide.trailingAnchor],
        }, 2);
        CFRelease(key);
    }

    UIButton *finishButton = createButton(localize(CFSTR("finish")), UIColor.systemBlueColor, 0,
                                          0, self, @selector(didPressFinish));
    setNavButton(self.navigationItem, false, finishButton, self.view.frame.size.width);
    enableButton(finishButton, false);
    validator.button = finishButton;

    activateConstraints((id []){
        [views[0].topAnchor constraintEqualToAnchor:guide.topAnchor constant:20]
    }, 1);
    for (int i = 1; i < 4; ++i) {
        activateConstraints((id []){
            [views[i].topAnchor constraintEqualToAnchor:views[i - 1].bottomAnchor]
        }, 1);
    }

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
