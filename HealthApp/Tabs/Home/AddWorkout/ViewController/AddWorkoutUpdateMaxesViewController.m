//
//  AddWorkoutUpdateMaxesViewController.m
//  HealthApp
//
//  Created by Christopher Ray on 6/13/21.
//

#import "AddWorkoutUpdateMaxesViewController.h"
#import "ViewControllerHelpers.h"
#include "InputValidator.h"

@interface AddWorkoutUpdateMaxesViewController() {
    USet_char *validChars;
    AddWorkoutCoordinator *delegate;
    UITextField *textFields[4];
    bool validInput[4];
    short results[4];
    UIButton *finishButton;
}
@end

@implementation AddWorkoutUpdateMaxesViewController
- (id) initWithDelegate: (AddWorkoutCoordinator *)_delegate {
    if (!(self = [super initWithNibName:nil bundle:nil])) return nil;
    delegate = _delegate;
    validChars = inputValidator_createNumberCharacterSet();
    return self;
}

- (void) dealloc {
    uset_free(char, validChars);
    for (int i = 0; i < 4; ++i) [textFields[i] release];
    [super dealloc];
}

- (void) viewDidLoad {
    [super viewDidLoad];
    self.view.backgroundColor = UIColor.secondarySystemBackgroundColor;

    NSString *titles[] = {@"Squat", @"Pull-up", @"Bench", @"Deadlift"};
    UIStackView *stacks[4];

    for (int i = 0; i < 4; ++i) {
        UILabel *label = [[UILabel alloc] initWithFrame:CGRectZero];
        label.text = titles[i];
        label.font = [UIFont preferredFontForTextStyle:UIFontTextStyleBody];

        textFields[i] = [[UITextField alloc] initWithFrame:CGRectZero];
        textFields[i].delegate = self;
        textFields[i].backgroundColor = UIColor.tertiarySystemBackgroundColor;
        textFields[i].placeholder = @"Weight";
        textFields[i].textAlignment = NSTextAlignmentLeft;
        textFields[i].borderStyle = UITextBorderStyleRoundedRect;
        textFields[i].keyboardType = UIKeyboardTypeNumberPad;

        stacks[i] = [[UIStackView alloc] initWithArrangedSubviews:@[label, textFields[i]]];
        stacks[i].translatesAutoresizingMaskIntoConstraints = false;
        stacks[i].backgroundColor = UIColor.secondarySystemBackgroundColor;
        stacks[i].spacing = 5;
        stacks[i].distribution = UIStackViewDistributionFillEqually;
        [stacks[i] setLayoutMarginsRelativeArrangement:true];
        stacks[i].layoutMargins = (UIEdgeInsets){.top = 4, .left = 8, .bottom = 4, .right = 8};
        [self.view addSubview:stacks[i]];

        [label release];
    }

    finishButton = [UIButton buttonWithType:UIButtonTypeSystem];
    finishButton.translatesAutoresizingMaskIntoConstraints = false;
    [finishButton setTitle:@"Finish" forState:UIControlStateNormal];
    [finishButton setTitleColor:UIColor.systemBlueColor forState:UIControlStateNormal];
    [finishButton setTitleColor:UIColor.systemGrayColor forState:UIControlStateDisabled];
    finishButton.frame = (CGRect){.size = {.width = self.view.frame.size.width / 3, .height = 30}};
    [finishButton addTarget:self action:@selector(didPressFinish) forControlEvents:UIControlEventTouchUpInside];
    UIBarButtonItem *rightItem = [[UIBarButtonItem alloc] initWithCustomView:finishButton];
    [finishButton setEnabled:false];
    self.navigationItem.rightBarButtonItem = rightItem;

    [NSLayoutConstraint activateConstraints:@[
        [stacks[0].topAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.topAnchor constant:30],
        [stacks[0].leadingAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.leadingAnchor],
        [stacks[0].trailingAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.trailingAnchor],
        [stacks[0].heightAnchor constraintEqualToConstant:40]
    ]];
    for (int i = 1; i < 4; ++i) {
        [NSLayoutConstraint activateConstraints:@[
            [stacks[i].topAnchor constraintEqualToAnchor:stacks[i - 1].bottomAnchor constant:20],
            [stacks[i].leadingAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.leadingAnchor],
            [stacks[i].trailingAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.trailingAnchor],
            [stacks[i].heightAnchor constraintEqualToConstant:40]
        ]];
    }

    for (int i = 0; i < 4; ++i) [stacks[i] release];
    [rightItem release];

    createToolbar(self, @selector(dismissKeyboard),
                  (UITextField *[]){textFields[0], textFields[1], textFields[2], textFields[3], nil});
}

- (void) didPressFinish {
    addWorkoutCoordinator_finishedAddingNewWeights(delegate, results);
}

- (void) dismissKeyboard {
    [self.view endEditing:true];
}

- (BOOL) textField: (UITextField *)textField shouldChangeCharactersInRange: (NSRange)range
 replacementString: (NSString *)string {
    if (!inputValidator_validateNumericInput(validChars, (__bridge CFStringRef) string)) return false;

    int i = 0;
    for (; i < 4; ++i) {
        if (textField == textFields[i]) break;
    }

    NSString *initialText = textField.text ? textField.text : @"";
    CFStringRef newText = CFBridgingRetain([initialText stringByReplacingCharactersInRange:range withString:string]);
    if (!CFStringGetLength(newText)) {
        CFRelease(newText);
        [finishButton setEnabled:false];
        validInput[i] = false;
        return true;
    }

    int newWeight = CFStringGetIntValue(newText);
    CFRelease(newText);

    if (newWeight < 0 || newWeight > 999) {
        [finishButton setEnabled:false];
        validInput[i] = false;
        return true;
    }

    validInput[i] = true;
    results[i] = (short) newWeight;

    for (i = 0; i < 4; ++i) {
        if (!validInput[i]) {
            [finishButton setEnabled:false];
            return true;
        }
    }

    [finishButton setEnabled:true];
    return true;
}

- (BOOL) textFieldShouldReturn: (UITextField *)textField {
    [textField resignFirstResponder];
    return true;
}
@end
