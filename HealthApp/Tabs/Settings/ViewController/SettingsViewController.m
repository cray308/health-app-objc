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

@interface SettingsViewController() {
    USet_char *validChars;
    SettingsTabCoordinator *delegate;
    UISegmentedControl *planPicker;
    UITextField *textFields[5];
    bool validInput[4];
    short results[4];
    short maxes[4];
    UIButton *saveButton;
}
@end

@implementation SettingsViewController
- (id) initWithDelegate: (SettingsTabCoordinator *)delegate {
    if (!(self = [super initWithNibName:nil bundle:nil])) return nil;
    self->delegate = delegate;
    memcpy(maxes, (short []){999, 999, 999, 999}, 4 * sizeof(short));
    return self;
}

- (void) viewDidLoad {
    [super viewDidLoad];
    setBackground(self.view, UIColor.systemGroupedBackgroundColor);
    self.navigationItem.title = @"App Settings";
    validChars = createNumberCharacterSet();

    UILabel *planLabel = createLabel(CFSTR("Change workout plan"), UIFontTextStyleFootnote,
                                     NSTextAlignmentNatural);

    int plan = appUserDataShared->currentPlan;
    CFStringRef segments[] = {CFSTR("None"), CFSTR("Base-Building"), CFSTR("Continuation")};
    planPicker = createSegmentedControl(segments, 3, plan >= 0 ? plan + 1 : 0, nil, nil);

    UIView *planContainer = createView(nil, false);
    [planContainer addSubview:planLabel];
    [planContainer addSubview:planPicker];

    UIStackView *stacks[4];
    CFStringRef titles[] = {
        CFSTR("Max Squat"), CFSTR("Max Pull-up"), CFSTR("Max Bench"), CFSTR("Max Deadlift")
    };

    for (int i = 0; i < 4; ++i) {
        UILabel *label = createLabel(titles[i], UIFontTextStyleBody, NSTextAlignmentNatural);
        textFields[i] = createTextfield(self, NULL, CFSTR("Weight"), NSTextAlignmentLeft, 4);
        stacks[i] = createStackView((id []){label, textFields[i]}, 2, 0, 5, 1,
                                    (HAEdgeInsets){4, 5, 4, 8});
        [label release];
    }

    saveButton = createButton(CFSTR("Save Settings"), UIColor.systemBlueColor,
                              UIColor.systemGrayColor, UIFontTextStyleBody,
                              UIColor.secondarySystemGroupedBackgroundColor, false, true, 0,
                              self, @selector(saveButtonPressed));
    [self updateWeightFields];

    UIButton *deleteDataButton = createButton(CFSTR("Delete Data"), UIColor.systemRedColor, nil,
                                              UIFontTextStyleBody,
                                              UIColor.secondarySystemGroupedBackgroundColor,
                                              false, true, 0, self, @selector(deleteButtonPressed));

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
        setLabelText(textFields[i], text);
        CFRelease(text);
    }
    memset(validInput, true, 4 * sizeof(bool));
    enableButton(saveButton, true);
}

#pragma mark - Selectors

- (void) dismissKeyboard {
    [self.view endEditing:true];
}

- (void) saveButtonPressed {
    const int segment = (int) planPicker.selectedSegmentIndex;
    settingsCoordinator_handleSaveTap(delegate, results, !segment ? -1 : segment - 1);
}

- (void) deleteButtonPressed {
    settingsCoordinator_handleDeleteTap(delegate);
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
