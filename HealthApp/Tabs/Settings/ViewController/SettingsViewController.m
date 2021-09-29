//
//  SettingsViewController.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "SettingsViewController.h"
#include "SettingsTabCoordinator.h"
#include "ViewControllerHelpers.h"
#include "AppUserData.h"
#include "AppCoordinator.h"

@interface SettingsViewController() {
    @public SettingsTabCoordinator *delegate;
    UISegmentedControl *picker;
    @public Validator validator;
    short results[4];
}
@end

id settingsVC_init(void *delegate) {
    SettingsViewController *this = [[SettingsViewController alloc] initWithNibName:nil bundle:nil];
    this->delegate = delegate;
    return this;
}

void settingsVC_updateWeightFields(SettingsViewController *vc) {
    for (int i = 0; i < 4; ++i) {
        CFStringRef str = CFStringCreateWithFormat(NULL, NULL, CFSTR("%d"), userData->liftMaxes[i]);
        setLabelText(vc->validator.children[i].field, str);
        inputView_reset(&vc->validator.children[i], userData->liftMaxes[i]);
        CFRelease(str);
    }
    enableButton(vc->validator.button, true);
}

@implementation SettingsViewController
- (void) viewDidLoad {
    [super viewDidLoad];
    setBackground(self.view, UIColor.systemGroupedBackgroundColor);
    self.navigationItem.title = _nsstr(localize(CFSTR("titles2")));
    textValidator_setup(&validator);

    UILabel *planLabel = createLabel(localize(CFSTR("pickerTitle")), TextFootnote, 4);
    
    CFStringRef segments[3];
    for (int i = 0; i < 3; ++i) {
        CFStringRef key = CFStringCreateWithFormat(NULL, NULL, CFSTR("settingsSegment%d"), i);
        segments[i] = localize(key);
        CFRelease(key);
    }
    picker = createSegmentedControl(segments, 3, userData->currentPlan + 1, nil, nil);

    UIView *planContainer = createView(nil, false);
    [planContainer addSubview:planLabel];
    [planContainer addSubview:picker];

    UIView *views[4];
    UIToolbar *toolbar = createToolbar(self, @selector(dismissKeyboard));

    for (int i = 0; i < 4; ++i) {
        CFStringRef key = CFStringCreateWithFormat(NULL, NULL, CFSTR("maxWeight%d"), i);
        views[i] = validator_addChild(&validator, self, localize(key), 0, 999, toolbar);
        CFRelease(key);
    }

    UIButton *saveButton = createButton(localize(CFSTR("settingsSave")), UIColor.systemBlueColor,
                                        BtnBackground, 0, self, @selector(buttonTapped:));
    validator.button = saveButton;

    UIButton *deleteButton = createButton(localize(CFSTR("settingsDelete")), UIColor.systemRedColor,
                                          BtnBackground, 1, self, @selector(buttonTapped:));

    id subviews[] = {
        planContainer, views[0], views[1], views[2], views[3], saveButton, deleteButton
    };
    UIStackView *vStack = createStackView(subviews, 7, 1, 0, (Padding){20, 0, 20, 0});

    UIScrollView *scrollView = createScrollView();
    [self.view addSubview:scrollView];
    [scrollView addSubview:vStack];
    [vStack setCustomSpacing:20 afterView:planContainer];
    [vStack setCustomSpacing:20 afterView:views[3]];
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

        [picker.topAnchor constraintEqualToAnchor:planLabel.bottomAnchor constant:2],
        [picker.leadingAnchor constraintEqualToAnchor:planContainer.leadingAnchor constant:8],
        [picker.trailingAnchor constraintEqualToAnchor:planContainer.trailingAnchor constant:-8],
        [picker.bottomAnchor constraintEqualToAnchor:planContainer.bottomAnchor],
        [picker.heightAnchor constraintEqualToConstant:40],

        [saveButton.heightAnchor constraintEqualToConstant:40],
        [deleteButton.heightAnchor constraintEqualToConstant:40]
    }, 20);

    [vStack release];
    [scrollView release];
    [planContainer release];
    [planLabel release];
    [toolbar release];

    settingsVC_updateWeightFields(self);
    appCoordinator->loadedViewControllers |= LoadedViewController_Settings;
}

- (void) dismissKeyboard { [self.view endEditing:true]; }

- (void) buttonTapped: (UIButton *)sender {
    if (!sender.tag) {
        schar plan = ((schar) picker.selectedSegmentIndex - 1);
        for (int i = 0; i < 4; ++i)
            results[i] = validator.children[i].result;
        settingsCoordinator_handleSaveTap(delegate, results, plan);
    } else {
        settingsCoordinator_handleDeleteTap(delegate);
    }
}

- (BOOL) textField: (UITextField *)textField
shouldChangeCharactersInRange: (NSRange)range replacementString: (NSString *)string {
    return checkInput(&validator, textField, (CFRange){range.location,range.length},_cfstr(string));
}

- (BOOL) textFieldShouldReturn: (UITextField *)textField {
    return [textField resignFirstResponder];
}
@end
