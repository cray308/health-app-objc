//
//  SettingsViewController.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "SettingsViewController.h"
#import "InputView.h"
#include "ViewControllerHelpers.h"
#include "AppUserData.h"
#include "AppCoordinator.h"

@interface SettingsViewController() {
    SettingsTabCoordinator *delegate;
    UISegmentedControl *planPicker;
    TextValidator validator;
    short results[4];
}
@end

@implementation SettingsViewController
- (id) initWithDelegate: (SettingsTabCoordinator *)delegate {
    if (!(self = [super initWithNibName:nil bundle:nil])) return nil;
    self->delegate = delegate;
    memcpy(&validator, &(TextValidator){
        .count = 4, .children = {[0 ... 3] = {.maxVal = 999}}
    }, sizeof(TextValidator));
    return self;
}

- (void) viewDidLoad {
    [super viewDidLoad];
    setBackground(self.view, UIColor.systemGroupedBackgroundColor);
    self.navigationItem.title = (__bridge NSString*) localize(CFSTR("titles2"));
    validator.set = createNumberCharacterSet();

    UILabel *planLabel = createLabel(localize(CFSTR("planPickerTitle")),
                                     UIFontTextStyleFootnote, 4);
    
    CFStringRef segments[3];
    for (int i = 0; i < 3; ++i) {
        CFStringRef key = CFStringCreateWithFormat(NULL, NULL, CFSTR("settingsSegment%d"), i);
        segments[i] = localize(key);
        CFRelease(key);
    }
    int startIndex = appUserDataShared->currentPlan >= 0 ? appUserDataShared->currentPlan + 1 : 0;
    planPicker = createSegmentedControl(segments, 3, startIndex, nil, nil);

    UIView *planContainer = createView(nil, false);
    [planContainer addSubview:planLabel];
    [planContainer addSubview:planPicker];

    InputView *views[4];
    UIToolbar *toolbar = createToolbar(self, @selector(dismissKeyboard));

    for (int i = 0; i < 4; ++i) {
        CFStringRef key = CFStringCreateWithFormat(NULL, NULL, CFSTR("maxWeight%d"), i);
        views[i] = [[InputView alloc] initWithDelegate:self fieldHint:localize(key)
                                                          tag:i min:0 max:999];
        validator.children[i].inputView = views[i];
        views[i]->field.inputAccessoryView = toolbar;
        CFRelease(key);
    }

    UIButton *saveButton = createButton(localize(CFSTR("settingsSaveButtonTitle")),
                                        UIColor.systemBlueColor, UIFontTextStyleBody,
                                        UIColor.secondarySystemGroupedBackgroundColor, false, true,
                                        0, self, @selector(buttonTapped:));
    validator.button = saveButton;

    UIButton *deleteButton = createButton(localize(CFSTR("settingsDeleteButtonTitle")),
                                          UIColor.systemRedColor, UIFontTextStyleBody,
                                          UIColor.secondarySystemGroupedBackgroundColor,
                                          false, true, 1, self, @selector(buttonTapped:));

    id subviews[] = {
        planContainer, views[0], views[1], views[2], views[3], saveButton, deleteButton
    };
    UIStackView *vStack = createStackView(subviews, 7, 1, 0, 0, (HAEdgeInsets){20, 0, 20, 0});

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

        [planPicker.topAnchor constraintEqualToAnchor:planLabel.bottomAnchor constant:2],
        [planPicker.leadingAnchor constraintEqualToAnchor:planContainer.leadingAnchor constant:8],
        [planPicker.trailingAnchor constraintEqualToAnchor:planContainer.trailingAnchor
                                                  constant:-8],
        [planPicker.bottomAnchor constraintEqualToAnchor:planContainer.bottomAnchor],
        [planPicker.heightAnchor constraintEqualToConstant:40],

        [saveButton.heightAnchor constraintEqualToConstant:40],
        [deleteButton.heightAnchor constraintEqualToConstant:40]
    }, 20);

    [vStack release];
    [scrollView release];
    [planContainer release];
    [planLabel release];
    for (int i = 0; i < 4; ++i)
        [views[i] release];
    [toolbar release];

    [self updateWeightFields];
    appCoordinatorShared->loadedViewControllers |= LoadedViewController_Settings;
}

- (void) updateWeightFields {
    for (int i = 0; i < 4; ++i) {
        short value = appUserDataShared->liftMaxes[i];
        CFStringRef text = CFStringCreateWithFormat(NULL, NULL, CFSTR("%d"), value);
        setLabelText(((InputView *) validator.children[i].inputView)->field, text);
        resetInputChild(&validator.children[i], value);
        CFRelease(text);
    }
    enableButton(validator.button, true);
}

- (void) dismissKeyboard { [self.view endEditing:true]; }

- (void) buttonTapped: (UIButton *)sender {
    if (!sender.tag) {
        const int segment = (int) planPicker.selectedSegmentIndex;
        for (int i = 0; i < 4; ++i)
            results[i] = validator.children[i].result;
        settingsCoordinator_handleSaveTap(delegate, results, !segment ? -1 : segment - 1);
    } else {
        settingsCoordinator_handleDeleteTap(delegate);
    }
}

- (BOOL) textField: (UITextField *)textField
shouldChangeCharactersInRange: (NSRange)range replacementString: (NSString *)string {
    return checkInput(textField, (CFRange){range.location, range.length},
                      (__bridge CFStringRef) string, &validator);
}

- (BOOL) textFieldShouldReturn: (UITextField *)textField {
    return [textField resignFirstResponder];
}
@end
