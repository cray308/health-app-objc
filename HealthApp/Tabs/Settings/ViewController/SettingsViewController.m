//
//  SettingsViewController.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "SettingsViewController.h"
#include "ViewControllerHelpers.h"
#include "AppUserData.h"
#include "AppCoordinator.h"
#import "PersistenceService.h"

@interface SettingsViewController() {
    USet_char *validChars;
    UISegmentedControl *planPicker;
    UITextField *textFields[5];
    bool validInput[4];
    short results[4];
    short maxes[4];
    UIButton *saveButton;
}
@end

@implementation SettingsViewController
- (id) init {
    if (!(self = [super initWithNibName:nil bundle:nil])) return nil;
    memcpy(maxes, (short []){999, 999, 999, 999}, 4 * sizeof(short));
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
    validChars = createNumberCharacterSet();

    UILabel *planLabel = createLabel(CFSTR("Change workout plan"), UIFontTextStyleFootnote, false, NSTextAlignmentNatural);

    planPicker = [[UISegmentedControl alloc] initWithItems:@[
        @"None", @"Base-Building", @"Continuation"
    ]];
    planPicker.translatesAutoresizingMaskIntoConstraints = false;
    int plan = appUserDataShared->currentPlan;
    planPicker.selectedSegmentIndex = plan >= 0 ? plan + 1 : 0;
    planPicker.layer.cornerRadius = 5;
    planPicker.tintColor = UIColor.systemGray2Color;

    UIView *planContainer = createView(nil, false);
    [planContainer addSubview:planLabel];
    [planContainer addSubview:planPicker];

    UIStackView *stacks[4];
    CFStringRef titles[] = {
        CFSTR("Max Squat"), CFSTR("Max Pull-up"), CFSTR("Max Bench"), CFSTR("Max Deadlift")
    };

    for (int i = 0; i < 4; ++i) {
        UILabel *label = createLabel(titles[i], UIFontTextStyleBody, false, NSTextAlignmentNatural);
        textFields[i] = createTextfield(self, CFSTR("Weight"), NSTextAlignmentLeft, 4);
        stacks[i] = createStackView((id []){label, textFields[i]}, 2, 0, 5, 1,
                                    (HAEdgeInsets){4, 5, 4, 8});
        [label release];
    }

    saveButton = createButton(CFSTR("Save Settings"), UIColor.systemBlueColor,
                              UIColor.systemGrayColor, UIFontTextStyleBody,
                              UIColor.secondarySystemGroupedBackgroundColor, false, true, true, 0);
    [saveButton addTarget:self action:@selector(saveButtonPressed)
         forControlEvents:UIControlEventTouchUpInside];
    [self updateWeightFields];

    UIButton *deleteDataButton = createButton(CFSTR("Delete Data"), UIColor.systemRedColor, nil,
                                              UIFontTextStyleBody,
                                              UIColor.secondarySystemGroupedBackgroundColor,
                                              false, true, true, 0);
    [deleteDataButton addTarget:self action:@selector(deleteButtonPressed)
               forControlEvents:UIControlEventTouchUpInside];

    id subviews[] = {
        planContainer, stacks[0], stacks[1], stacks[2], stacks[3], saveButton, deleteDataButton
    };
    UIStackView *vStack = createStackView(subviews, 7, 1, 20, 0, (HAEdgeInsets){20, 0, 0, 0});

    UIScrollView *scrollView = createScrollView();
    [self.view addSubview:scrollView];
    [scrollView addSubview:vStack];
    [vStack setCustomSpacing:40 afterView:stacks[3]];
    [vStack setCustomSpacing:40 afterView:saveButton];

    UILayoutGuide *guide = self.view.safeAreaLayoutGuide;
    activateConstraints((id []){
        [scrollView.leadingAnchor constraintEqualToAnchor:guide.leadingAnchor],
        [scrollView.trailingAnchor constraintEqualToAnchor:guide.trailingAnchor],
        [scrollView.topAnchor constraintEqualToAnchor:guide.topAnchor],
        [scrollView.bottomAnchor constraintEqualToAnchor:guide.bottomAnchor],

        [vStack.leadingAnchor constraintEqualToAnchor:scrollView.leadingAnchor],
        [vStack.trailingAnchor constraintEqualToAnchor:scrollView.trailingAnchor],
        [vStack.topAnchor constraintEqualToAnchor:scrollView.topAnchor],
        [vStack.bottomAnchor constraintEqualToAnchor:scrollView.bottomAnchor],
        [vStack.widthAnchor constraintEqualToAnchor:scrollView.widthAnchor],

        [planLabel.topAnchor constraintEqualToAnchor:planContainer.topAnchor],
        [planLabel.leadingAnchor constraintEqualToAnchor:planContainer.leadingAnchor constant:8],
        [planLabel.trailingAnchor constraintEqualToAnchor:planContainer.trailingAnchor constant:-8],
        [planLabel.heightAnchor constraintEqualToConstant:20],

        [planPicker.topAnchor constraintEqualToAnchor:planLabel.bottomAnchor constant:2],
        [planPicker.leadingAnchor constraintEqualToAnchor:planContainer.leadingAnchor constant:8],
        [planPicker.trailingAnchor constraintEqualToAnchor:planContainer.trailingAnchor
                                                  constant:-8],
        [planPicker.bottomAnchor constraintEqualToAnchor:planContainer.bottomAnchor],
        [planPicker.heightAnchor constraintEqualToConstant:40],

        [stacks[0].heightAnchor constraintEqualToConstant:40],
        [stacks[1].heightAnchor constraintEqualToConstant:40],
        [stacks[2].heightAnchor constraintEqualToConstant:40],
        [stacks[3].heightAnchor constraintEqualToConstant:40],
        [saveButton.heightAnchor constraintEqualToConstant:40],
        [deleteDataButton.heightAnchor constraintEqualToConstant:40]
    }, 24);

    [vStack release];
    [scrollView release];
    [planContainer release];
    [planLabel release];
    for (int i = 0; i < 4; ++i) [stacks[i] release];

    createToolbar(self, @selector(dismissKeyboard), textFields);

    appCoordinatorShared->loadedViewControllers |= LoadedViewController_Settings;
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

    UIAlertController *ctrl = [UIAlertController
                               alertControllerWithTitle:@"Are you sure?"
                               message:@"This will save the currently entered data."
                               preferredStyle:UIAlertControllerStyleAlert];

    [ctrl addAction:[UIAlertAction actionWithTitle:@"Save" style:UIAlertActionStyleDefault
                                           handler:^(UIAlertAction * _Nonnull action _U_) {
        appUserData_updateWeightMaxes(results);
        appUserData_setWorkoutPlan(plan);
        appCoordinator_updatedUserInfo(appCoordinatorShared);
    }]];
    [ctrl addAction:[UIAlertAction actionWithTitle:@"Cancel"
                                             style:UIAlertActionStyleCancel handler:nil]];
    [self presentViewController:ctrl animated:true completion:nil];
}

- (void) deleteButtonPressed {
    UIAlertController *ctrl = [UIAlertController
                               alertControllerWithTitle:@"Are you sure?"
                               message:@"This will delete all workout history."
                               preferredStyle:UIAlertControllerStyleAlert];

    [ctrl addAction:[UIAlertAction actionWithTitle:@"Delete" style:UIAlertActionStyleDestructive
                                           handler:^(UIAlertAction * _Nonnull action _U_) {
        persistenceService_deleteUserData();
        appUserData_deleteSavedData();
        appCoordinator_deletedAppData(appCoordinatorShared);
    }]];
    [ctrl addAction:[UIAlertAction actionWithTitle:@"Cancel"
                                             style:UIAlertActionStyleCancel handler:nil]];
    [self presentViewController:ctrl animated:true completion:nil];
}

#pragma mark - TextField Delegate

- (BOOL) textField: (UITextField *)textField
shouldChangeCharactersInRange: (NSRange)range replacementString: (NSString *)string {
    return checkTextfield(textField, (CFRange){range.location, range.length},
                          (__bridge CFStringRef) string, validChars, saveButton, textFields, 4,
                          maxes, results, validInput);
}

-(BOOL) textFieldShouldReturn: (UITextField *)textField {
    return [textField resignFirstResponder];
}
@end
