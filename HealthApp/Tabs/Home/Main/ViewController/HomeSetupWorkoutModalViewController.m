//
//  HomeSetupWorkoutModalViewController.m
//  HealthApp
//
//  Created by Christopher Ray on 6/12/21.
//

#import "HomeSetupWorkoutModalViewController.h"
#include "ViewControllerHelpers.h"
#include "Exercise.h"

@interface HomeSetupWorkoutModalViewController() {
    USet_char *validChars;
    CFStringInlineBuffer buf;
    HomeTabCoordinator *delegate;
    CFStringRef *names;
    int count;
    int index;
    unsigned char type;
    UIButton *submitButton;
    UITextField *workoutTextField;
    UITextField *fields[3];
    bool validInput[3];
    int inputs[3];
    int maxes[3];
}
@end

@implementation HomeSetupWorkoutModalViewController
- (id) initWithDelegate: (HomeTabCoordinator *)_delegate
                   type: (unsigned char)_type names: (CFStringRef *)_names count: (int)_count {
    if (!(self = [super initWithNibName:nil bundle:nil])) return nil;
    delegate = _delegate;
    names = _names;
    count = _count;
    type = _type;
    return self;
}

- (void) dealloc {
    if (validChars) uset_free(char, validChars);
    for (int i = 0; i < count; ++i) CFRelease(names[i]);
    free(names);
    [workoutTextField release];
    for (int i = 0; i < 3; ++i) {
        if (fields[i]) [fields[i] release];
    }
    [super dealloc];
}

- (void) viewDidLoad {
    [super viewDidLoad];
    self.view.backgroundColor = UIColor.systemGroupedBackgroundColor;

    workoutTextField = [[UITextField alloc] initWithFrame:CGRectZero];
    workoutTextField.translatesAutoresizingMaskIntoConstraints = false;
    workoutTextField.delegate = self;
    workoutTextField.backgroundColor = UIColor.tertiarySystemBackgroundColor;
    workoutTextField.text = (__bridge NSString*) names[0];
    workoutTextField.textAlignment = NSTextAlignmentCenter;
    workoutTextField.borderStyle = UITextBorderStyleRoundedRect;

    UILabel *workoutLabel = [[UILabel alloc] initWithFrame:CGRectZero];
    workoutLabel.translatesAutoresizingMaskIntoConstraints = false;
    workoutLabel.text = @"Choose workout";
    workoutLabel.font = [UIFont preferredFontForTextStyle:UIFontTextStyleFootnote];

    UIPickerView *workoutPicker = [[UIPickerView alloc] init];
    workoutPicker.delegate = self;
    workoutTextField.inputView = workoutPicker;

    UIView *workoutContainer = [[UIView alloc] initWithFrame:CGRectZero];
    workoutContainer.translatesAutoresizingMaskIntoConstraints = false;
    [workoutContainer addSubview:workoutLabel];
    [workoutContainer addSubview:workoutTextField];
    [self.view addSubview:workoutContainer];

    submitButton = [UIButton buttonWithType:UIButtonTypeSystem];
    submitButton.translatesAutoresizingMaskIntoConstraints = false;
    submitButton.backgroundColor = UIColor.secondarySystemBackgroundColor;
    [submitButton setTitle:@"Go" forState:UIControlStateNormal];
    [submitButton setTitleColor:UIColor.systemBlueColor forState:UIControlStateNormal];
    [submitButton setTitleColor:UIColor.systemGrayColor forState:UIControlStateDisabled];
    [submitButton addTarget:self action:@selector(didPressFinish)
           forControlEvents:UIControlEventTouchUpInside];
    [submitButton setEnabled:false];
    [self.view addSubview:submitButton];

    NSString *titles[3] = {0};

    switch (type) {
        case WorkoutTypeStrength:
            titles[0] = @"Sets";
            titles[1] = @"Reps";
            titles[2] = @"Max Weight Percentage";
            maxes[0] = 5;
            maxes[1] = 5;
            maxes[2] = 100;
            break;

        case WorkoutTypeSE:
            titles[0] = @"Sets";
            titles[1] = @"Reps";
            validInput[2] = true;
            inputs[2] = 1;
            maxes[0] = 3;
            maxes[1] = 50;
            break;

        case WorkoutTypeEndurance:
            titles[1] = @"Duration (mins)";
            validInput[0] = validInput[2] = true;
            inputs[0] = inputs[2] = 1;
            maxes[1] = 180;
            break;

        case WorkoutTypeHIC:
            memset(validInput, true, 3 * sizeof(bool));
            memset(inputs, 1, 3 * sizeof(int));
            [submitButton setEnabled:true];
            break;
    }

    UIStackView *textFieldStack = [[UIStackView alloc] initWithFrame:CGRectZero];
    textFieldStack.translatesAutoresizingMaskIntoConstraints = false;
    textFieldStack.axis = UILayoutConstraintAxisVertical;
    textFieldStack.spacing = 20;
    [self.view addSubview:textFieldStack];

    bool createCharSet = false;

    for (int i = 0; i < 3; ++i) {
        if (!titles[i]) continue;
        createCharSet = true;
        UILabel *label = [[UILabel alloc] initWithFrame:CGRectZero];
        label.text = titles[i];
        label.font = [UIFont preferredFontForTextStyle:UIFontTextStyleBody];
        label.adjustsFontSizeToFitWidth = true;

        fields[i] = [[UITextField alloc] initWithFrame:CGRectZero];
        fields[i].delegate = self;
        fields[i].backgroundColor = UIColor.tertiarySystemBackgroundColor;
        fields[i].textAlignment = NSTextAlignmentLeft;
        fields[i].borderStyle = UITextBorderStyleRoundedRect;
        fields[i].keyboardType = UIKeyboardTypeNumberPad;

        UITextField *toolbarFields[] = {fields[i], nil};
        createToolbar(self, @selector(dismissKeyboard), toolbarFields);

        UIStackView *hStack = [[UIStackView alloc] initWithArrangedSubviews:@[label, fields[i]]];
        hStack.backgroundColor = UIColor.secondarySystemBackgroundColor;
        hStack.spacing = 5;
        hStack.distribution = UIStackViewDistributionFillEqually;
        [hStack setLayoutMarginsRelativeArrangement:true];
        hStack.layoutMargins = (UIEdgeInsets){.top = 4, .left = 8, .bottom = 4, .right = 8};
        [textFieldStack addArrangedSubview:hStack];
        [label release];
        [hStack release];
    }

    if (createCharSet) validChars = createNumberCharacterSet();

    UIButton *cancelButton = [UIButton buttonWithType:UIButtonTypeSystem];
    cancelButton.translatesAutoresizingMaskIntoConstraints = false;
    [cancelButton setTitle:@"Cancel" forState:UIControlStateNormal];
    [cancelButton setTitleColor:UIColor.systemBlueColor forState:UIControlStateNormal];
    [cancelButton setTitleColor:UIColor.systemGrayColor forState:UIControlStateDisabled];
    cancelButton.frame = (CGRect){.size = {.width = self.view.frame.size.width / 3, .height = 30}};
    [cancelButton addTarget:self action:@selector(pressedCancel)
           forControlEvents:UIControlEventTouchUpInside];
    UIBarButtonItem *leftItem = [[UIBarButtonItem alloc] initWithCustomView:cancelButton];
    self.navigationItem.leftBarButtonItem = leftItem;

    UILayoutGuide *guide = self.view.safeAreaLayoutGuide;
    [NSLayoutConstraint activateConstraints:@[
        [workoutContainer.topAnchor constraintEqualToAnchor:guide.topAnchor constant:30],
        [workoutContainer.leadingAnchor constraintEqualToAnchor:guide.leadingAnchor],
        [workoutContainer.trailingAnchor constraintEqualToAnchor:guide.trailingAnchor],

        [workoutLabel.topAnchor constraintEqualToAnchor:workoutContainer.topAnchor],
        [workoutLabel.leadingAnchor constraintEqualToAnchor:workoutContainer.leadingAnchor],
        [workoutLabel.trailingAnchor constraintEqualToAnchor:workoutContainer.trailingAnchor],
        [workoutLabel.heightAnchor constraintEqualToConstant:20],

        [workoutTextField.topAnchor constraintEqualToAnchor:workoutLabel.bottomAnchor constant:2],
        [workoutTextField.leadingAnchor constraintEqualToAnchor:workoutContainer.leadingAnchor],
        [workoutTextField.trailingAnchor constraintEqualToAnchor:workoutContainer.trailingAnchor],
        [workoutTextField.bottomAnchor constraintEqualToAnchor:workoutContainer.bottomAnchor],
        [workoutTextField.heightAnchor constraintEqualToConstant:40],

        [textFieldStack.leadingAnchor constraintEqualToAnchor:guide.leadingAnchor],
        [textFieldStack.trailingAnchor constraintEqualToAnchor:guide.trailingAnchor],
        [textFieldStack.topAnchor constraintEqualToAnchor:workoutContainer.bottomAnchor
                                                 constant:20],

        [submitButton.leadingAnchor constraintEqualToAnchor:guide.leadingAnchor],
        [submitButton.trailingAnchor constraintEqualToAnchor:guide.trailingAnchor],
        [submitButton.topAnchor constraintEqualToAnchor:textFieldStack.bottomAnchor constant:20],
        [submitButton.heightAnchor constraintEqualToConstant:40]
    ]];

    UITextField *toolbarFields[] = {workoutTextField, nil};
    createToolbar(self, @selector(dismissKeyboard), toolbarFields);

    [workoutLabel release];
    [workoutContainer release];
    [textFieldStack release];
    [workoutPicker release];
    [leftItem release];
}

- (void) didPressFinish {
    homeCoordinator_finishedSettingUpCustomWorkout(delegate, type, index,
                                                   inputs[0], inputs[1], inputs[2]);
}

- (void) pressedCancel {
    [delegate->navigationController.viewControllers[0] dismissViewControllerAnimated:true
                                                                          completion:nil];
}

- (void) dismissKeyboard {
    [self.view endEditing:true];
}

- (BOOL) textField: (UITextField *)textField
shouldChangeCharactersInRange: (NSRange)range replacementString: (NSString *)string {
    if (!validateNumericInput(validChars, (__bridge CFStringRef) string, &buf)) return false;

    int i = 0;
    for (; i < 3; ++i) {
        if (fields[i] && textField == fields[i]) break;
    }
    if (i == 3) return true;

    NSString *initialText = textField.text ? textField.text : @"";
    CFStringRef newText = CFBridgingRetain([initialText stringByReplacingCharactersInRange:range
                                                                                withString:string]);
    if (!CFStringGetLength(newText)) {
        CFRelease(newText);
        [submitButton setEnabled:false];
        validInput[i] = false;
        return true;
    }

    int value = CFStringGetIntValue(newText);
    CFRelease(newText);

    if (value < 0 || value > maxes[i]) {
        [submitButton setEnabled:false];
        validInput[i] = false;
        return true;
    }

    validInput[i] = true;
    inputs[i] = (int) value;

    for (i = 0; i < 3; ++i) {
        if (!validInput[i]) {
            [submitButton setEnabled:false];
            return true;
        }
    }

    [submitButton setEnabled:true];
    return true;
}

- (BOOL) textFieldShouldReturn: (UITextField *)textField {
    [textField resignFirstResponder];
    return true;
}

- (NSInteger) numberOfComponentsInPickerView: (UIPickerView *)pickerView {
    return 1;
}

- (NSInteger) pickerView: (UIPickerView *)pickerView numberOfRowsInComponent: (NSInteger)component {
    return count;
}

- (NSString *) pickerView: (UIPickerView *)pickerView
              titleForRow: (NSInteger)row forComponent: (NSInteger)component {
    return (__bridge NSString*) names[row];
}

- (void) pickerView: (UIPickerView *)pickerView
       didSelectRow: (NSInteger)row inComponent: (NSInteger)component {
    index = (int) row;
    workoutTextField.text = (__bridge NSString*) names[index];
}
@end
