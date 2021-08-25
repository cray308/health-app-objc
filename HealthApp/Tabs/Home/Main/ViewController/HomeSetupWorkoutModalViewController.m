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
    HomeTabCoordinator *delegate;
    Array_str *names;
    int index;
    unsigned char type;
    UIButton *submitButton;
    UITextField *workoutTextField;
    UITextField *fields[3];
    bool validInput[3];
    short inputs[3];
    short maxes[3];
}
@end

@implementation HomeSetupWorkoutModalViewController
- (id) initWithDelegate: (HomeTabCoordinator *)delegate
                   type: (unsigned char)type names: (Array_str *)names {
    if (!(self = [super initWithNibName:nil bundle:nil])) return nil;
    self->delegate = delegate;
    self->names = names;
    self->type = type;
    return self;
}

- (void) dealloc {
    if (validChars) uset_free(char, validChars);
    array_free(str, names);
    [workoutTextField release];
    for (int i = 0; i < 3; ++i) {
        if (fields[i]) [fields[i] release];
    }
    [super dealloc];
}

- (void) viewDidLoad {
    [super viewDidLoad];
    setBackground(self.view, UIColor.systemGroupedBackgroundColor);

    workoutTextField = createTextfield(self, names->arr[0], NULL, NSTextAlignmentCenter, 0);
    UILabel *workoutLabel = createLabel(CFSTR("Choose workout"), UIFontTextStyleFootnote,
                                        NSTextAlignmentNatural);

    UIPickerView *workoutPicker = [[UIPickerView alloc] init];
    workoutPicker.delegate = self;
    workoutTextField.inputView = workoutPicker;

    UIView *workoutContainer = createView(nil, false);
    [workoutContainer addSubview:workoutLabel];
    [workoutContainer addSubview:workoutTextField];
    [self.view addSubview:workoutContainer];

    submitButton = createButton(CFSTR("Go"), UIColor.systemBlueColor, UIColor.systemGrayColor,
                                nil, UIColor.secondarySystemBackgroundColor, false, false, 0,
                                self, @selector(didPressFinish));
    [self.view addSubview:submitButton];

    CFStringRef titles[3] = {0};

    switch (type) {
        case WorkoutTypeStrength:
            titles[0] = CFSTR("Sets");
            titles[1] = CFSTR("Reps");
            titles[2] = CFSTR("Max Weight Percentage");
            maxes[0] = 5;
            maxes[1] = 5;
            maxes[2] = 100;
            break;

        case WorkoutTypeSE:
            titles[0] = CFSTR("Sets");
            titles[1] = CFSTR("Reps");
            validInput[2] = true;
            inputs[2] = 1;
            maxes[0] = 3;
            maxes[1] = 50;
            break;

        case WorkoutTypeEndurance:
            titles[1] = CFSTR("Duration (mins)");
            validInput[0] = validInput[2] = true;
            inputs[0] = inputs[2] = 1;
            maxes[1] = 180;
            break;

        case WorkoutTypeHIC:
            memset(validInput, true, 3 * sizeof(bool));
            memset(inputs, 1, 3 * sizeof(short));
            enableButton(submitButton, true);
            break;
    }

    UIStackView *textFieldStack = createStackView(NULL, 0, 1, 20, 0, (HAEdgeInsets){0});
    [self.view addSubview:textFieldStack];

    bool createCharSet = false;

    for (int i = 0; i < 3; ++i) {
        if (!titles[i]) continue;
        createCharSet = true;
        UILabel *label = createLabel(titles[i], UIFontTextStyleBody, NSTextAlignmentNatural);
        fields[i] = createTextfield(self, NULL, NULL, NSTextAlignmentLeft, 4);
        createToolbar(self, @selector(dismissKeyboard), (id []){fields[i], nil});
        UIStackView *hStack = createStackView((id []){label, fields[i]}, 2, 0, 5, 1,
                                              (HAEdgeInsets){4, 8, 4, 8});
        [textFieldStack addArrangedSubview:hStack];
        [label release];
        [hStack release];
    }

    if (createCharSet) validChars = createNumberCharacterSet();

    UIButton *cancelButton = createButton(CFSTR("Cancel"), UIColor.systemBlueColor,
                                          UIColor.systemGrayColor, nil, nil, false, true, 0,
                                          self, @selector(pressedCancel));
    setNavButton(self.navigationItem, true, cancelButton, self.view.frame.size.width);

    UILayoutGuide *guide = self.view.safeAreaLayoutGuide;
    activateConstraints((id []){
        [workoutContainer.topAnchor constraintEqualToAnchor:guide.topAnchor constant:30],
        [workoutContainer.leadingAnchor constraintEqualToAnchor:guide.leadingAnchor constant:8],
        [workoutContainer.trailingAnchor constraintEqualToAnchor:guide.trailingAnchor constant:-8],

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
    }, 19);

    createToolbar(self, @selector(dismissKeyboard), (id []){workoutTextField, nil});

    [workoutLabel release];
    [workoutContainer release];
    [textFieldStack release];
    [workoutPicker release];
}

- (void) didPressFinish {
    homeCoordinator_finishedSettingUpCustomWorkout(delegate, type, index, inputs);
}

- (void) pressedCancel {
    UINavigationController *navVC = delegate->navVC;
    [navVC.viewControllers[0] dismissViewControllerAnimated:true completion:nil];
}

- (void) dismissKeyboard {
    [self.view endEditing:true];
}

- (BOOL) textField: (UITextField *)textField
shouldChangeCharactersInRange: (NSRange)range replacementString: (NSString *)string {
    return checkTextfield(textField, (CFRange){range.location, range.length},
                          (__bridge CFStringRef) string, validChars, submitButton, fields, 3,
                          maxes, inputs, validInput);
}

- (BOOL) textFieldShouldReturn: (UITextField *)textField {
    [textField resignFirstResponder];
    return true;
}

- (NSInteger) numberOfComponentsInPickerView: (UIPickerView *)pickerView {
    return 1;
}

- (NSInteger) pickerView: (UIPickerView *)pickerView numberOfRowsInComponent: (NSInteger)component {
    return names->size;
}

- (NSString *) pickerView: (UIPickerView *)pickerView
              titleForRow: (NSInteger)row forComponent: (NSInteger)component {
    return (__bridge NSString*) names->arr[row];
}

- (void) pickerView: (UIPickerView *)pickerView
       didSelectRow: (NSInteger)row inComponent: (NSInteger)component {
    index = (int) row;
    setLabelText(workoutTextField, names->arr[index]);
}
@end
