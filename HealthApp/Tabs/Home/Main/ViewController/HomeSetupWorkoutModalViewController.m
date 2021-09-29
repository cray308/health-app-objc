//
//  HomeSetupWorkoutModalViewController.m
//  HealthApp
//
//  Created by Christopher Ray on 6/12/21.
//

#import "HomeSetupWorkoutModalViewController.h"
#include "HomeTabCoordinator.h"
#include "ViewControllerHelpers.h"
#include "Exercise.h"

@interface SetupWorkoutSheet() {
    @public HomeTabCoordinator *delegate;
    @public Array_str *names;
    UITextField *workoutTextField;
    @public Validator validator;
    @public WorkoutParams output;
}
@end

id setupWorkoutVC_init(void *delegate, uchar type, Array_str *names) {
    SetupWorkoutSheet *this = [[SetupWorkoutSheet alloc] initWithNibName:nil bundle:nil];
    this->delegate = delegate;
    this->names = names;
    workoutParams_init(&this->output, -1);
    this->output.type = type;
    return this;
}

@implementation SetupWorkoutSheet
- (void) dealloc {
    textValidator_free(&validator);
    array_free(str, names);
    [workoutTextField release];
    [super dealloc];
}

- (void) viewDidLoad {
    [super viewDidLoad];
    setBackground(self.view, UIColor.systemGroupedBackgroundColor);

    workoutTextField = createTextfield(nil, names->arr[0], NSTextAlignmentCenter, 0, 0);
    UILabel *workoutLabel = createLabel(localize(CFSTR("setupWorkoutPickerTitle")),TextFootnote, 4);

    UIPickerView *workoutPicker = [[UIPickerView alloc] init];
    workoutPicker.delegate = self;
    workoutTextField.inputView = workoutPicker;

    UIView *workoutContainer = createView(nil, false);
    [workoutContainer addSubview:workoutLabel];
    [workoutContainer addSubview:workoutTextField];
    [self.view addSubview:workoutContainer];

    UIButton *cancelButton = createButton(localize(CFSTR("cancel")), UIColor.systemBlueColor, 0,
                                          0, self, @selector(pressedCancel));
    UIButton *submitButton = createButton(localize(CFSTR("go")), UIColor.systemBlueColor, 0,
                                          0, self, @selector(pressedFinish));
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
    UIStackView *textFieldStack = createStackView(NULL, 0, 1, 0, (Padding){0});
    [self.view addSubview:textFieldStack];

    for (int i = 0; i < 3; ++i) {
        if (!titles[i]) continue;
        UIView *v = validator_addChild(&validator, self, localize(titles[i]), 1, maxes[i], toolbar);
        [textFieldStack addArrangedSubview:v];
    }

    if (validator.count)
        textValidator_setup(&validator);

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

- (void) pressedFinish {
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
    return checkInput(&validator, textField, (CFRange){range.location,range.length},_cfstr(string));
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
    return _nsstr(names->arr[row]);
}

- (void) pickerView: (UIPickerView *)pickerView
       didSelectRow: (NSInteger)row inComponent: (NSInteger)component {
    output.index = (int) row;
    setLabelText(workoutTextField, names->arr[output.index]);
}
@end
