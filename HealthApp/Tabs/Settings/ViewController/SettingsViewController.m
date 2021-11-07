#import "TextFieldViewController.h"
#include "SettingsTabCoordinator.h"
#include "AppUserData.h"
#include "AppCoordinator.h"

@interface SettingsViewController: TextFieldViewController @end
@interface SettingsViewController() {
    @public SettingsTabCoordinator *delegate;
    UISegmentedControl *picker;
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

@implementation SettingsViewController
- (void) viewDidLoad {
    [super viewDidLoad];
    setBackground(self.view, UIColor.systemGroupedBackgroundColor);
    self.navigationItem.title = _nsstr(localize(CFSTR("titles2")));
    validator_setup(&validator, 0, true, self);

    CFStringRef titles[4]; fillStringArray(titles, CFSTR("maxWeight%d"), 4);
    UILabel *planLabel = createLabel(localize(CFSTR("planPickerTitle")), TextFootnote, 4, true);
    picker = createSegmentedControl(CFSTR("settingsSegment%d"),3,userData->currentPlan + 1,0,0,44);
    UIView *planContainer = createStackView((id []){planLabel, picker}, 2, 1, 2, (Padding){0});
    UIStackView *cStack = createStackView((id []){planContainer}, 1, 1, 0, (Padding){0,8,0,8});
    [cStack setCustomSpacing:20 afterView:planContainer];
    validator.vStack = createStackView((id[]){cStack}, 1, 1, 20, (Padding){20, 0, 20, 0});

    for (int i = 0; i < 4; ++i)
        [cStack addArrangedSubview:validator_add(&validator, self, titles[i], 0, 999)];

    validator.button = createButton(localize(CFSTR("settingsSave")), UIColor.systemBlueColor,
                                    BtnBackground, 0, self, @selector(buttonTapped:), 44);
    [validator.vStack addArrangedSubview:validator.button];

    [validator.vStack addArrangedSubview:createButton(localize(CFSTR("settingsDelete")), UIColor.systemRedColor,
                                            BtnBackground, 1, self, @selector(buttonTapped:), 44)];
    addVStackToScrollView(validator.vStack, validator.scrollView);

    [cStack release];
    [planContainer release];
    [planLabel release];

    settingsVC_updateWeightFields(self);
    appCoordinator->loadedViewControllers |= LoadedViewController_Settings;
}

- (void) buttonTapped: (UIButton *)sender {
    if (!sender.tag) {
        signed char plan = ((signed char) picker.selectedSegmentIndex - 1);
        for (int i = 0; i < 4; ++i)
            results[i] = validator.children[i].result;
        settingsCoordinator_handleSaveTap(delegate, results, plan);
    } else {
        settingsCoordinator_handleDeleteTap(delegate);
    }
}
@end
