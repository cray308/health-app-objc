//
//  AddWorkoutUpdateMaxesViewController.m
//  HealthApp
//
//  Created by Christopher Ray on 6/13/21.
//

#import "AddWorkoutUpdateMaxesViewController.h"
#import "ViewControllerHelpers.h"

@interface AddWorkoutUpdateMaxesViewController() {
    USet_char *validChars;
    AddWorkoutCoordinator *delegate;
    UITextField *textFields[5];
    bool validInput[4];
    short results[4];
    short maxes[4];
    UIButton *finishButton;
}
@end

@implementation AddWorkoutUpdateMaxesViewController
- (id) initWithDelegate: (AddWorkoutCoordinator *)_delegate {
    if (!(self = [super initWithNibName:nil bundle:nil])) return nil;
    self->delegate = _delegate;
    self->validChars = createNumberCharacterSet();
    memcpy(maxes, (short []){999, 999, 999, 999}, 4 * sizeof(short));
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

    CFStringRef titles[] = {CFSTR("Squat"), CFSTR("Pull-up"), CFSTR("Bench"), CFSTR("Deadlift")};
    UIStackView *stacks[4];

    for (int i = 0; i < 4; ++i) {
        UILabel *label = createLabel(titles[i], UIFontTextStyleBody, false, NSTextAlignmentNatural);
        textFields[i] = createTextfield(self, CFSTR("Weight"), NSTextAlignmentLeft, 4);
        stacks[i] = createStackView((id []){label, textFields[i]}, 2, 0, 5, 1,
                                    (HAEdgeInsets){4, 8, 4, 8});
        [self.view addSubview:stacks[i]];
        [label release];
    }

    finishButton = createButton(CFSTR("Finish"), UIColor.systemBlueColor, UIColor.systemGrayColor,
                                NULL, nil, false, false, false, 0);
    [finishButton addTarget:self action:@selector(didPressFinish)
           forControlEvents:UIControlEventTouchUpInside];
    setNavButton(self.navigationItem, false, finishButton, self.view.frame.size.width);

    UILayoutGuide *guide = self.view.safeAreaLayoutGuide;
    activateConstraints((id []){
        [stacks[0].topAnchor constraintEqualToAnchor:guide.topAnchor constant:30],
        [stacks[0].leadingAnchor constraintEqualToAnchor:guide.leadingAnchor],
        [stacks[0].trailingAnchor constraintEqualToAnchor:guide.trailingAnchor],
        [stacks[0].heightAnchor constraintEqualToConstant:40]
    }, 4);
    for (int i = 1; i < 4; ++i) {
        activateConstraints((id []){
            [stacks[i].topAnchor constraintEqualToAnchor:stacks[i - 1].bottomAnchor constant:20],
            [stacks[i].leadingAnchor constraintEqualToAnchor:guide.leadingAnchor],
            [stacks[i].trailingAnchor constraintEqualToAnchor:guide.trailingAnchor],
            [stacks[i].heightAnchor constraintEqualToConstant:40]
        }, 4);
    }

    for (int i = 0; i < 4; ++i) [stacks[i] release];
    createToolbar(self, @selector(dismissKeyboard), textFields);
}

- (void) didPressFinish {
    addWorkoutCoordinator_finishedAddingNewWeights(delegate, results);
}

- (void) dismissKeyboard {
    [self.view endEditing:true];
}

- (BOOL) textField: (UITextField *)textField
shouldChangeCharactersInRange: (NSRange)range replacementString: (NSString *)string {
    return checkTextfield(textField, (CFRange){range.location, range.length},
                          (__bridge CFStringRef) string, validChars, finishButton, textFields, 4,
                          maxes, results, validInput);
}

- (BOOL) textFieldShouldReturn: (UITextField *)textField {
    [textField resignFirstResponder];
    return true;
}
@end
