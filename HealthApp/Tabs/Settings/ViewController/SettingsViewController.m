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

id settingsVC_init(SettingsTabCoordinator *delegate) {
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
    textValidator_setup(&validator, 0);

    CFStringRef titles[4]; fillStringArray(titles, CFSTR("maxWeight%d"), 4);
    UILabel *planLabel = createLabel(localize(CFSTR("planPickerTitle")), TextFootnote, 4, 20);
    picker = createSegmentedControl(CFSTR("settingsSegment%d"),3,userData->currentPlan + 1,0,0,40);
    UIView *planContainer = createStackView((id []){planLabel, picker}, 2, 1, 2, (Padding){0});
    UIStackView *cStack = createStackView((id []){planContainer}, 1, 1, 0, (Padding){0,8,0,8});
    [cStack setCustomSpacing:20 afterView:planContainer];
    UIStackView *vStack = createStackView((id[]){cStack}, 1, 1, 20, (Padding){20, 0, 20, 0});

    UIToolbar *toolbar = createToolbar(self, @selector(dismissKeyboard));

    for (int i = 0; i < 4; ++i)
        [cStack addArrangedSubview:validator_add(&validator, self, titles[i], 0, 999, toolbar)];

    validator.button = createButton(localize(CFSTR("settingsSave")), UIColor.systemBlueColor,
                                    BtnBackground, 0, self, @selector(buttonTapped:), 40);
    [vStack addArrangedSubview:validator.button];

    [vStack addArrangedSubview:createButton(localize(CFSTR("settingsDelete")), UIColor.systemRedColor,
                                            BtnBackground, 1, self, @selector(buttonTapped:), 40)];

    UIScrollView *scrollView = createScrollView();
    [self.view addSubview:scrollView];
    [scrollView addSubview:vStack];

    pin(scrollView, self.view.safeAreaLayoutGuide, (Padding){0}, 0);
    pin(vStack, scrollView, (Padding){0}, 0);
    setEqualWidths(vStack, scrollView);

    [cStack release];
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
