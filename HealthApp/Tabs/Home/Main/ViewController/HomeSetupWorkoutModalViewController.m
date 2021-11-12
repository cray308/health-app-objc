#import "TextFieldViewController.h"
#include "HomeTabCoordinator.h"
#include "ExerciseManager.h"

@interface SetupWorkoutSheet: TextFieldViewController<UIPickerViewDelegate, UIPickerViewDataSource> @end
@interface SetupWorkoutSheet() {
    @public HomeTabCoordinator *delegate;
    @public Array_str *names;
    UITextField *workoutTextField;
    @public WorkoutParams output;
}
@end
@implementation SetupWorkoutSheet
- (void) dealloc {
    array_free(str, names);
    [workoutTextField release];
    [super dealloc];
}

- (void) viewDidLoad {
    [super viewDidLoad];
    setBackground(self.view, createColor(ColorSecondarySystemBackground));
    validator_setup(&validator, 8, output.type != WorkoutHIC, self);

    CFStringRef pickerTitle = localize(CFSTR("setupWorkoutTitle"));
    UILabel *workoutLabel = createLabel(pickerTitle, TextFootnote, 4, false);
    workoutTextField = createTextfield(nil, names->arr[0], pickerTitle, NSTextAlignmentCenter, 0, 0);
    UIStackView *workoutContainer = createStackView((id []){workoutLabel, workoutTextField},
                                                    2, 1, 2, (Padding){30,8,0,8});

    UIPickerView *workoutPicker = [[UIPickerView alloc] init];
    workoutPicker.delegate = self;
    workoutTextField.inputView = workoutPicker;
    workoutTextField.inputAccessoryView = validator.toolbar;

    validator.vStack = createStackView((id []){workoutContainer}, 1, 1, 0, (Padding){0});
    [validator.vStack setCustomSpacing:20 afterView:workoutContainer];
    addVStackToScrollView(validator.vStack, validator.scrollView);

    UIButton *cancelButton = createButton(localize(CFSTR("cancel")), createColor(ColorBlue), 0,
                                          0, self, @selector(pressedCancel), -1);
    UIButton *submitButton = createButton(localize(CFSTR("go")), createColor(ColorBlue), 0,
                                          0, self, @selector(pressedFinish), -1);
    setNavButton(self.navigationItem, true, cancelButton, self.view.frame.size.width);
    setNavButton(self.navigationItem, false, submitButton, self.view.frame.size.width);
    enableButton(submitButton, false);
    validator.button = submitButton;

    short maxes[] = {5, 5, 100};
    CFStringRef rows[] = {CFSTR("setupWorkoutSets"), CFSTR("setupWorkoutReps"), NULL};

    switch (output.type) {
        case WorkoutStrength:
            rows[2] = CFSTR("setupWorkoutMaxWeight");
            break;
        case WorkoutSE:
            maxes[0] = 3;
            maxes[1] = 50;
            break;
        case WorkoutEndurance:
            rows[0] = NULL;
            rows[1] = CFSTR("setupWorkoutDuration");
            maxes[1] = 180;
            break;
        default:
            rows[0] = rows[1] = NULL;
            enableButton(submitButton, true);
    }

    for (int i = 0; i < 3; ++i) {
        if (!rows[i]) continue;
        [validator.vStack addArrangedSubview:validator_add(&validator, self, localize(rows[i]), 1, maxes[i])];
    }

    [workoutLabel release];
    [workoutContainer release];
    [workoutPicker release];
}

- (void) pressedFinish {
    switch (output.type) {
        case WorkoutStrength:
            output.weight = validator.children[2].result;
        case WorkoutSE:
            output.sets = validator.children[0].result;
            output.reps = validator.children[1].result;
            break;

        case WorkoutEndurance:
            output.reps = validator.children[0].result;
        default: ;
    }
    homeCoordinator_finishedSettingUpCustomWorkout(delegate, &output);
}

- (void) pressedCancel { dismissPresentedVC(self.presentingViewController, nil); }

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

id setupWorkoutVC_init(HomeTabCoordinator *delegate, byte type, Array_str *names) {
    SetupWorkoutSheet *this = [[SetupWorkoutSheet alloc] initWithNibName:nil bundle:nil];
    this->delegate = delegate;
    this->names = names;
    workoutParams_init(&this->output, -1);
    this->output.type = type;
    return this;
}
