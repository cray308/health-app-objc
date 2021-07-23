//
//  AddWorkoutUpdateMaxesViewController.m
//  HealthApp
//
//  Created by Christopher Ray on 6/13/21.
//

#import "AddWorkoutUpdateMaxesViewController.h"
#import "AddWorkoutViewModel.h"
#import "ViewControllerHelpers.h"

@interface AddWorkoutUpdateMaxesViewController() {
    AddWorkoutViewModel *viewModel;
    UITextField *textFields[4];
    bool validInput[4];
    unsigned short results[4];
    UIButton *finishButton;
}

@end

@implementation AddWorkoutUpdateMaxesViewController

- (id) initWithViewModel: (AddWorkoutViewModel *)model {
    if (!(self = [super initWithNibName:nil bundle:nil])) return nil;
    viewModel = model;
    return self;
}

- (void) dealloc {
    for (int i = 0; i < 4; ++i) { [textFields[i] release]; }
    [super dealloc];
}

- (void) viewDidLoad {
    [super viewDidLoad];
    self.view.backgroundColor = UIColor.secondarySystemBackgroundColor;
    [self setupSubviews];
    UITextField *fields[] = {textFields[0], textFields[1], textFields[2], textFields[3], nil};
    createToolbar(self, @selector(dismissKeyboard), fields);

}

- (void) setupSubviews {
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
        stacks[i].layoutMargins = UIEdgeInsetsMake(4, 8, 4, 8);
        [self.view addSubview:stacks[i]];

        [label release];
    }

    finishButton = [UIButton buttonWithType:UIButtonTypeSystem];
    finishButton.translatesAutoresizingMaskIntoConstraints = false;
    [finishButton setTitle:@"Finish" forState:UIControlStateNormal];
    [finishButton setTitleColor:UIColor.systemBlueColor forState:UIControlStateNormal];
    [finishButton setTitleColor:UIColor.systemGrayColor forState:UIControlStateDisabled];
    finishButton.frame = CGRectMake(0, 0, self.view.frame.size.width / 3, 30);
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

    for (int i = 0; i < 4; ++i) { [stacks[i] release]; }
    [rightItem release];
}

- (void) didPressFinish {
    addWorkoutViewModel_finishedAddingNewWeights(viewModel, self, results);
}

- (void) dismissKeyboard {
    [self.view endEditing:true];
}

- (BOOL) textField: (UITextField *)textField shouldChangeCharactersInRange: (NSRange)range replacementString: (NSString *)string {
    if (!viewController_validateNumericInput((__bridge CFStringRef) string)) return false;

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
    results[i] = (unsigned short) newWeight;

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
