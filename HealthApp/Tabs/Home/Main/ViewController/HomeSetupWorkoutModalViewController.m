//
//  HomeSetupWorkoutModalViewController.m
//  HealthApp
//
//  Created by Christopher Ray on 6/12/21.
//

#import "HomeSetupWorkoutModalViewController.h"
#include "HomeTabCoordinator.h"
#include "ViewControllerHelpers.h"
#include "InputView.h"

@interface HomeSetupWorkoutModalViewController() {
    HomeTabCoordinator *delegate;
    Array_str *names;
    UITextField *workoutTextField;
    TextValidator validator;
    WorkoutParams output;
}
@end

@implementation HomeSetupWorkoutModalViewController
- (id) initWithDelegate: (void *)delegate type: (uchar)type names: (Array_str *)names {
    if (!(self = [super initWithNibName:nil bundle:nil])) return nil;
    self->delegate = delegate;
    self->names = names;
    memcpy(&validator, &(TextValidator){
        .children = {[0 ... 3] = {.minVal = 1}}
    }, sizeof(TextValidator));
    workoutParams_init(&output, -1);
    output.type = type;
    return self;
}

- (void) dealloc {
    if (validator.set)
        uset_free(char, validator.set);
    array_free(str, names);
    [workoutTextField release];
    [super dealloc];
}

- (void) viewDidLoad {
    [super viewDidLoad];
    setBackground(self.view, UIColor.systemGroupedBackgroundColor);

    workoutTextField = createTextfield(nil, names->arr[0], NSTextAlignmentCenter, 0);
    UILabel *workoutLabel = createLabel(localize(CFSTR("setupWorkoutPickerTitle")),
                                        UIFontTextStyleFootnote, 4);

    UIPickerView *workoutPicker = [[UIPickerView alloc] init];
    workoutPicker.delegate = self;
    workoutTextField.inputView = workoutPicker;

    UIView *workoutContainer = createView(nil, false);
    [workoutContainer addSubview:workoutLabel];
    [workoutContainer addSubview:workoutTextField];
    [self.view addSubview:workoutContainer];

    UIButton *cancelButton = createButton(localize(CFSTR("cancel")), UIColor.systemBlueColor,
                                          nil, nil, false, true, 0, self, @selector(pressedCancel));
    UIButton *submitButton = createButton(localize(CFSTR("go")), UIColor.systemBlueColor, nil, nil,
                                          false, false, 0, self, @selector(didPressFinish));
    setNavButton(self.navigationItem, true, cancelButton, self.view.frame.size.width);
    setNavButton(self.navigationItem, false, submitButton, self.view.frame.size.width);
    enableButton(submitButton, false);
    validator.button = submitButton;

    short maxes[] = {5, 5, 100};
    CFStringRef titles[] = {CFSTR("setupWorkoutSets"), CFSTR("setupWorkoutReps"), NULL};

    switch (output.type) {
        case WorkoutTypeStrength:
            titles[2] = CFSTR("setupWorkoutMaxWeight");
            break;
        case WorkoutTypeSE:
            maxes[0] = 3;
            maxes[1] = 50;
            break;
        case WorkoutTypeEndurance:
            titles[0] = NULL;
            titles[1] = CFSTR("setupWorkoutDuration");
            maxes[1] = 180;
            break;
        default:
            titles[0] = titles[1] = NULL;
            enableButton(submitButton, true);
    }

    UIToolbar *toolbar = createToolbar(self, @selector(dismissKeyboard));
    UIStackView *textFieldStack = createStackView(NULL, 0, 1, 0, 0, (HAEdgeInsets){0});
    [self.view addSubview:textFieldStack];

    for (int i = 0; i < 3; ++i) {
        if (!titles[i]) continue;
        InputView *v = [[InputView alloc] initWithDelegate:self fieldHint:localize(titles[i])
                                                       tag:validator.count min:1 max:maxes[i]];
        validator.children[validator.count].inputView = v;
        validator.children[validator.count++].maxVal = maxes[i];
        v->field.inputAccessoryView = toolbar;
        [textFieldStack addArrangedSubview:v];
        [v release];
    }

    if (validator.count)
        validator.set = createNumberCharacterSet();

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
                                                 constant:20]
    }, 15);

    workoutTextField.inputAccessoryView = toolbar;

    [workoutLabel release];
    [workoutContainer release];
    [textFieldStack release];
    [workoutPicker release];
    [toolbar release];
}

- (void) didPressFinish {
    switch (output.type) {
        case WorkoutTypeStrength:
            output.weight = validator.children[2].result;
        case WorkoutTypeSE:
            output.sets = validator.children[0].result;
            output.reps = validator.children[1].result;
            break;

        case WorkoutTypeEndurance:
            output.reps = validator.children[0].result;
        default: ;
    }
    homeCoordinator_finishedSettingUpCustomWorkout(delegate, &output);
}

- (void) pressedCancel {
    UINavigationController *navVC = delegate->navVC;
    [navVC.viewControllers[0] dismissViewControllerAnimated:true completion:nil];
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

- (NSInteger) numberOfComponentsInPickerView: (UIPickerView *)pickerView { return 1; }

- (NSInteger) pickerView: (UIPickerView *)pickerView numberOfRowsInComponent: (NSInteger)component {
    return names->size;
}

- (NSString *) pickerView: (UIPickerView *)pickerView
              titleForRow: (NSInteger)row forComponent: (NSInteger)component {
    return (__bridge NSString*) names->arr[row];
}

- (void) pickerView: (UIPickerView *)pickerView
       didSelectRow: (NSInteger)row inComponent: (NSInteger)component {
    output.index = (int) row;
    setLabelText(workoutTextField, names->arr[output.index]);
}
@end
