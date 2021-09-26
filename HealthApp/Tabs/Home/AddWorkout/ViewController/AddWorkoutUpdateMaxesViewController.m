//
//  AddWorkoutUpdateMaxesViewController.m
//  HealthApp
//
//  Created by Christopher Ray on 6/13/21.
//

#import "AddWorkoutUpdateMaxesViewController.h"
#import "InputView.h"
#include "AddWorkoutCoordinator.h"
#include "ViewControllerHelpers.h"

@interface AddWorkoutUpdateMaxesViewController() {
    AddWorkoutCoordinator *delegate;
    TextValidator validator;
}
@end

@implementation AddWorkoutUpdateMaxesViewController
- (id) initWithDelegate: (void *)delegate {
    if (!(self = [super initWithNibName:nil bundle:nil])) return nil;
    self->delegate = delegate;
    memcpy(&validator, &(TextValidator){
        .count = 4, .children = {[0 ... 3] = {.minVal = 1, .maxVal = 999}}
    }, sizeof(TextValidator));
    validator.set = createNumberCharacterSet();
    return self;
}

- (void) dealloc {
    uset_free(char, validator.set);
    [super dealloc];
}

- (void) viewDidLoad {
    [super viewDidLoad];
    setBackground(self.view, UIColor.secondarySystemBackgroundColor);

    UIToolbar *toolbar = createToolbar(self, @selector(dismissKeyboard));
    InputView *views[4];
    UILayoutGuide *guide = self.view.safeAreaLayoutGuide;

    for (int i = 0; i < 4; ++i) {
        CFStringRef key = CFStringCreateWithFormat(NULL, NULL, CFSTR("maxWeight%d"), i);
        views[i] = [[InputView alloc] initWithDelegate:self fieldHint:localize(key)
                                                   tag:i min:1 max:999];
        validator.children[i].inputView = views[i];
        views[i]->field.inputAccessoryView = toolbar;
        [self.view addSubview:views[i]];
        activateConstraints((id []){
            [views[i].leadingAnchor constraintEqualToAnchor:guide.leadingAnchor],
            [views[i].trailingAnchor constraintEqualToAnchor:guide.trailingAnchor],
        }, 2);
        CFRelease(key);
    }

    UIButton *finishButton = createButton(localize(CFSTR("updatesMaxesFinishText")),
                                          UIColor.systemBlueColor, nil, nil, false, false, 0,
                                          self, @selector(didPressFinish));
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

    for (int i = 0; i < 4; ++i)
        [views[i] release];
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
    return checkInput(textField, (CFRange){range.location, range.length},
                      (__bridge CFStringRef) string, &validator);
}

- (BOOL) textFieldShouldReturn: (UITextField *)textField {
    return [textField resignFirstResponder];
}
@end
