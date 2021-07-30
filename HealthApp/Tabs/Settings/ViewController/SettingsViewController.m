//
//  SettingsViewController.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "SettingsViewController.h"
#import "ViewControllerHelpers.h"
#include "AppUserData.h"
#import "AppDelegate.h"
#import "PersistenceService.h"
#include "InputValidator.h"

#define _U_ __attribute__((__unused__))

@interface SettingsViewController() {
    USet_char *validChars;
    UISegmentedControl *planPicker;
    UITextField *textFields[4];
    bool validInput[4];
    short results[4];
    UIButton *saveButton;
}
@end

@implementation SettingsViewController
- (id) init {
    if (!(self = [super initWithNibName:nil bundle:nil])) return nil;
    return self;
}

- (void) dealloc {
    if (validChars) uset_free(char, validChars);
    for (int i = 0; i < 4; ++i) [textFields[i] release];
    [planPicker release];
    [super dealloc];
}

- (void) viewDidLoad {
    [super viewDidLoad];
    self.view.backgroundColor = UIColor.systemGroupedBackgroundColor;
    self.navigationItem.title = @"App Settings";
    validChars = inputValidator_createNumberCharacterSet();

    UILabel *planLabel = [[UILabel alloc] initWithFrame:CGRectZero];
    planLabel.translatesAutoresizingMaskIntoConstraints = false;
    planLabel.text = @"Change workout plan";
    planLabel.font = [UIFont preferredFontForTextStyle:UIFontTextStyleFootnote];

    planPicker = [[UISegmentedControl alloc] initWithItems:@[@"None", @"Base-Building", @"Continuation"]];
    planPicker.translatesAutoresizingMaskIntoConstraints = false;
    planPicker.selectedSegmentIndex = appUserDataShared->currentPlan >= 0 ? appUserDataShared->currentPlan + 1 : 0;
    planPicker.layer.cornerRadius = 5;
    planPicker.tintColor = UIColor.systemGray2Color;

    UIView *planContainer = [[UIView alloc] initWithFrame:CGRectZero];
    [planContainer addSubview:planLabel];
    [planContainer addSubview:planPicker];

    UIStackView *stacks[4];
    NSString *titles[] = {@"Max Squat", @"Max Pull-up", @"Max Bench", @"Max Deadlift"};

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
        stacks[i].backgroundColor = UIColor.secondarySystemGroupedBackgroundColor;
        stacks[i].spacing = 5;
        stacks[i].distribution = UIStackViewDistributionFillEqually;
        [stacks[i] setLayoutMarginsRelativeArrangement:true];
        stacks[i].layoutMargins = (UIEdgeInsets){.top = 4, .left = 5, .bottom = 4, .right = 8};

        [label release];
    }

    saveButton = [UIButton buttonWithType:UIButtonTypeSystem];
    saveButton.backgroundColor = UIColor.secondarySystemGroupedBackgroundColor;
    [saveButton setTitle:@"Save Settings" forState:UIControlStateNormal];
    saveButton.titleLabel.font = [UIFont preferredFontForTextStyle:UIFontTextStyleBody];
    saveButton.titleLabel.adjustsFontSizeToFitWidth = true;
    [saveButton setTitleColor:UIColor.systemBlueColor forState: UIControlStateNormal];
    [saveButton setTitleColor:UIColor.systemGrayColor forState: UIControlStateDisabled];
    [saveButton addTarget:self action:@selector(saveButtonPressed) forControlEvents:UIControlEventTouchUpInside];
    [self updateWeightFields];

    UIButton *deleteDataButton = [UIButton buttonWithType:UIButtonTypeSystem];
    deleteDataButton.backgroundColor = UIColor.secondarySystemGroupedBackgroundColor;
    [deleteDataButton setTitle:@"Delete Data" forState: UIControlStateNormal];
    deleteDataButton.titleLabel.font = [UIFont preferredFontForTextStyle:UIFontTextStyleBody];
    deleteDataButton.titleLabel.adjustsFontSizeToFitWidth = true;
    [deleteDataButton setTitleColor:UIColor.systemRedColor forState:UIControlStateNormal];
    [deleteDataButton addTarget:self action:@selector(deleteButtonPressed) forControlEvents:UIControlEventTouchUpInside];

    UIStackView *vStack = [[UIStackView alloc] initWithArrangedSubviews:@[
        planContainer, stacks[0], stacks[1], stacks[2], stacks[3], saveButton, deleteDataButton
    ]];
    vStack.translatesAutoresizingMaskIntoConstraints = false;
    vStack.axis = UILayoutConstraintAxisVertical;
    vStack.spacing = 20;
    [vStack setLayoutMarginsRelativeArrangement:true];
    vStack.layoutMargins = (UIEdgeInsets){.top = 20};

    UIScrollView *scrollView = [[UIScrollView alloc] initWithFrame:CGRectZero];
    scrollView.translatesAutoresizingMaskIntoConstraints = false;
    scrollView.autoresizingMask = UIViewAutoresizingFlexibleHeight;
    scrollView.bounces = true;
    scrollView.showsVerticalScrollIndicator = true;
    [self.view addSubview:scrollView];
    [scrollView addSubview:vStack];
    [vStack setCustomSpacing:40 afterView:stacks[3]];
    [vStack setCustomSpacing:40 afterView:saveButton];

    [NSLayoutConstraint activateConstraints:@[
        [scrollView.leadingAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.leadingAnchor],
        [scrollView.trailingAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.trailingAnchor],
        [scrollView.topAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.topAnchor],
        [scrollView.bottomAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.bottomAnchor],

        [vStack.leadingAnchor constraintEqualToAnchor:scrollView.leadingAnchor],
        [vStack.trailingAnchor constraintEqualToAnchor:scrollView.trailingAnchor],
        [vStack.topAnchor constraintEqualToAnchor:scrollView.topAnchor],
        [vStack.bottomAnchor constraintEqualToAnchor:scrollView.bottomAnchor],
        [vStack.widthAnchor constraintEqualToAnchor:scrollView.widthAnchor],

        [planLabel.topAnchor constraintEqualToAnchor:planContainer.topAnchor],
        [planLabel.leadingAnchor constraintEqualToAnchor:planContainer.leadingAnchor],
        [planLabel.trailingAnchor constraintEqualToAnchor:planContainer.trailingAnchor],
        [planLabel.heightAnchor constraintEqualToConstant:20],

        [planPicker.topAnchor constraintEqualToAnchor:planLabel.bottomAnchor constant:2],
        [planPicker.leadingAnchor constraintEqualToAnchor:planContainer.leadingAnchor],
        [planPicker.trailingAnchor constraintEqualToAnchor:planContainer.trailingAnchor],
        [planPicker.bottomAnchor constraintEqualToAnchor:planContainer.bottomAnchor],
        [planPicker.heightAnchor constraintEqualToConstant:40],

        [stacks[0].heightAnchor constraintEqualToConstant:40],
        [stacks[1].heightAnchor constraintEqualToConstant:40],
        [stacks[2].heightAnchor constraintEqualToConstant:40],
        [stacks[3].heightAnchor constraintEqualToConstant:40],
        [saveButton.heightAnchor constraintEqualToConstant:40],
        [deleteDataButton.heightAnchor constraintEqualToConstant:40]
    ]];

    [vStack release];
    [scrollView release];
    [planContainer release];
    [planLabel release];
    for (int i = 0; i < 4; ++i) [stacks[i] release];

    createToolbar(self, @selector(dismissKeyboard),
                  (UITextField *[]){textFields[0], textFields[1], textFields[2], textFields[3], nil});

    AppDelegate *app = (AppDelegate *) UIApplication.sharedApplication.delegate;
    if (app) {
        app->coordinator.loadedViewControllers |= LoadedViewController_Settings;
    }
}

- (void) updateWeightFields {
    for (int i = 0; i < 4; ++i) {
        results[i] = appUserDataShared->liftMaxes[i];
        CFStringRef text = CFStringCreateWithFormat(NULL, NULL, CFSTR("%d"), results[i]);
        textFields[i].text = (__bridge NSString*) text;
        CFRelease(text);
    }
    memset(validInput, true, 4 * sizeof(bool));
    [saveButton setEnabled:true];
}

#pragma mark - Selectors

- (void) dismissKeyboard {
    [self.view endEditing:true];
}

- (void) saveButtonPressed {
    const int segment = (int) planPicker.selectedSegmentIndex;
    signed char plan = segment == 0 ? -1 : segment - 1;

    AlertDetails details = {CFSTR("Are you sure?"), CFSTR("This will save the currently entered data.")};
    UIAlertAction *okAction = [UIAlertAction actionWithTitle:@"Save" style:UIAlertActionStyleDefault
                                                     handler:^(UIAlertAction * _Nonnull action _U_) {
        appUserData_updateWeightMaxes(results);
        appUserData_setWorkoutPlan(plan);
        AppDelegate *app = (AppDelegate *) UIApplication.sharedApplication.delegate;
        if (app) appCoordinator_updatedUserInfo(&app->coordinator);
    }];
    UIAlertAction *cancelAction = [UIAlertAction actionWithTitle:@"Cancel" style:UIAlertActionStyleCancel handler:nil];
    viewController_showAlert(self, &details, okAction, cancelAction);
}

- (void) deleteButtonPressed {
    AlertDetails details = {CFSTR("Are you sure?"), CFSTR("This will delete all workout history.")};
    UIAlertAction *okAction = [UIAlertAction actionWithTitle:@"Delete" style:UIAlertActionStyleDestructive
                                                     handler:^(UIAlertAction * _Nonnull action _U_) {
        persistenceService_deleteUserData();
        appUserData_deleteSavedData();
        AppDelegate *app = (AppDelegate *) UIApplication.sharedApplication.delegate;
        if (app) appCoordinator_deletedAppData(&app->coordinator);
    }];
    UIAlertAction *cancelAction = [UIAlertAction actionWithTitle:@"Cancel" style:UIAlertActionStyleCancel handler:nil];
    viewController_showAlert(self, &details, okAction, cancelAction);
}

#pragma mark - TextField Delegate

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
        [saveButton setEnabled:false];
        validInput[i] = false;
        return true;
    }

    int newWeight = CFStringGetIntValue(newText);
    CFRelease(newText);

    if (newWeight < 0 || newWeight > 999) {
        [saveButton setEnabled:false];
        validInput[i] = false;
        return true;
    }

    validInput[i] = true;
    results[i] = (short) newWeight;

    for (i = 0; i < 4; ++i) {
        if (!validInput[i]) {
            [saveButton setEnabled:false];
            return true;
        }
    }

    [saveButton setEnabled:true];
    return true;
}

-(BOOL) textFieldShouldReturn: (UITextField *)textField {
    [textField resignFirstResponder];
    return true;
}
@end
