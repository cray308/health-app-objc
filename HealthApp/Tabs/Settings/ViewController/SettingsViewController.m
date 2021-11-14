#import "TextFieldViewController.h"
#include "SettingsTabCoordinator.h"
#include "AppUserData.h"
#include "AppCoordinator.h"

@interface SettingsViewController: TextFieldViewController @end
@interface SettingsViewController() {
    @public SettingsTabCoordinator *delegate;
    UISegmentedControl *picker;
    SwitchContainer *switchContainer;
    UILabel *planLabel;
    UIButton *deleteButton;
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
        CFStringRef str = createNumberString(userData->liftMaxes[i]);
        setLabelText(vc->validator.children[i].field, str);
        inputView_reset(&vc->validator.children[i], userData->liftMaxes[i]);
        CFRelease(str);
    }
    enableButton(vc->validator.button, true);
}

void settingsVC_updateColors(SettingsViewController *vc, SEL _cmd _U_) {
    setBackground(vc.view, getBackground(PrimaryBG, true));
    updateSegmentedControl(vc->picker);
    CFArrayRef items = getArray(vc->validator.toolbar, sel_getUid("items"));
    setTintColor((id) CFArrayGetValueAtIndex(items, 1), createColor(ColorRed));
}

@implementation SettingsViewController
- (void) viewDidLoad {
    [super viewDidLoad];
    setBackground(self.view, getBackground(PrimaryBG, true));
    self.navigationItem.title = _nsstr(localize(CFSTR("titles2")));
    validator_setup(&validator, 8, true, self);

    CFStringRef titles[4]; fillStringArray(titles, CFSTR("maxWeight%d"), 4);
    planLabel = createLabel(localize(CFSTR("planPickerTitle")), TextFootnote, 4, true);
    picker = createSegmentedControl(CFSTR("settingsSegment%d"),3,userData->currentPlan + 1,0,0,44);
    UIView *planContainer = createStackView((id []){planLabel, picker}, 2, 1, 2, (Padding){0,8,0,8});
    UIStackView *cStack = createStackView((id []){planContainer}, 1, 1, 0, (Padding){0});
    [cStack setCustomSpacing:20 afterView:planContainer];
    validator.vStack = createStackView((id[]){cStack}, 1, 1, 20, (Padding){20, 0, 20, 0});

    if (osVersion == Version12) {
        switchContainer = createSwitch(localize(CFSTR("darkMode")), userData->darkMode);
        [cStack addArrangedSubview:switchContainer->view];
        [cStack setCustomSpacing:20 afterView:switchContainer->view];
    }

    for (int i = 0; i < 4; ++i)
        [cStack addArrangedSubview:validator_add(&validator, self, titles[i], 0, 999)];

    validator.button = createButton(localize(CFSTR("settingsSave")), ColorBlue, BtnBackground, 0,
                                    self, @selector(buttonTapped:), 44);
    [validator.vStack addArrangedSubview:validator.button];

    deleteButton = createButton(localize(CFSTR("settingsDelete")), ColorRed, BtnBackground, 1,
                                self, @selector(buttonTapped:), 44);
    [validator.vStack addArrangedSubview:deleteButton];
    addVStackToScrollView(validator.vStack, validator.scrollView);

    [cStack release];
    [planContainer release];

    settingsVC_updateWeightFields(self);
    appCoordinator->loadedViewControllers |= LoadedViewController_Settings;
}

- (void) buttonTapped: (UIButton *)sender {
    if (!sender.tag) {
        bool dark = switchContainer ? ((UISwitch *) switchContainer->switchView).on : false;
        signed char plan = ((signed char) picker.selectedSegmentIndex - 1);
        for (int i = 0; i < 4; ++i)
            results[i] = validator.children[i].result;
        settingsCoordinator_handleSaveTap(delegate, results, plan, dark);
    } else {
        settingsCoordinator_handleDeleteTap(delegate);
    }
}
@end
