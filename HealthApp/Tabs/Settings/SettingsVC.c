#include "SettingsVC.h"
#include "AppDelegate.h"
#include "AppUserData.h"
#include "InputVC.h"
#include "ViewControllerHelpers.h"

Class SettingsVCClass;

id settingsVC_init(void) {
    return createNew(SettingsVCClass);
}

void settingsVC_updateWeightFields(id self) {
    InputVC *parent = (InputVC *) ((char *)self + VCSize);
    for (int i = 0; i < 4; ++i) {
        short value = userData->liftMaxes[i];
        InputView *child = (InputView *) ((char *)parent->children[i] + ViewSize);
        CFStringRef str = CFStringCreateWithFormat(NULL, NULL, CFSTR("%d"), value);
        setLabelText(child->field, str);
        inputView_reset(child, value);
        CFRelease(str);
    }
    enableButton(parent->button, true);
}

void settingsVC_updateColors(id self) {
    InputVC *par = (InputVC *) ((char *)self + VCSize);
    SettingsVC *data = (SettingsVC *) ((char *)self + InputVCSize);
    setBackground(getView(self), createColor(ColorPrimaryBGGrouped));
    CFArrayRef views = getArrangedSubviews(data->planContainer);
    setTextColor((id) CFArrayGetValueAtIndex(views, 0), createColor(ColorLabel));
    updateSegmentedControl((id) CFArrayGetValueAtIndex(views, 1));
    setBackground(data->switchContainer, createColor(ColorSecondaryBGGrouped));
    id view = (id) CFArrayGetValueAtIndex(getSubviews(data->switchContainer), 0);
    setTextColor((id)CFArrayGetValueAtIndex(getArrangedSubviews(view), 0), createColor(ColorLabel));
    updateButtonColors(data->deleteButton, ColorRed);
    updateButtonColors(par->button, ColorBlue);
    setBarTint(par->toolbar, getBarColor(ColorBarModal));
    view = (id) CFArrayGetValueAtIndex(getArray(par->toolbar, sel_getUid("items")), 1);
    setTintColor(view, createColor(ColorRed));
    for (int i = 0; i < 4; ++i) {
        InputView *ptr = (InputView *) ((char *)par->children[i] + ViewSize);
        setTextColor(ptr->errorLabel, createColor(ColorRed));
        setTextColor(ptr->hintLabel, createColor(ColorLabel));
        setTextColor(ptr->field, createColor(ColorLabel));
        setBackground(ptr->field, createColor(ColorTertiaryBG));
        setKBColor(ptr->field, userData->darkMode ? 1 : 0);
    }
}

void settingsVC_viewDidLoad(id self, SEL _cmd) {
    struct objc_super super = {self, InputVCClass};
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);

    InputVC *parent = (InputVC *) ((char *)self + VCSize);
    SettingsVC *data = (SettingsVC *) ((char *)self + InputVCSize);
    setBackground(getView(self), createColor(ColorPrimaryBGGrouped));
    setVCTitle(self, localize(CFSTR("settingsTitle")));

    unsigned char segment = userData->currentPlan + 1;
    id planLabel = createLabel(localize(CFSTR("planPickerTitle")), TextFootnote, 4, true);
    id picker = createSegmentedControl(CFSTR("settingsSegment%d"), 3, segment, nil, nil, 44);
    data->planContainer = createStackView((id []){planLabel,picker}, 2, 1, 2,(Padding){0, 8, 0, 8});
    id aboveTF = createStackView((id[]){data->planContainer}, 1, 1, 20, (Padding){0, 0, 20, 0});

    setMargins(parent->vStack, ((HAInsets){20, 0, 20, 0}));
    addArrangedSubview(parent->vStack, aboveTF);

    const unsigned char darkMode = userData->darkMode;
    if (darkMode != 0xff) {
        data->switchContainer = createBackgroundView(ColorSecondaryBGGrouped, 44, false);
        id switchView = createNew(objc_getClass("UISwitch"));
        setBool(switchView, sel_getUid("setOn:"), darkMode);
        id label = createLabel(localize(CFSTR("darkMode")), TextBody, 4, true);
        id sv = createStackView((id[]){label, switchView}, 2, 0, 5, (Padding){0, 8, 0, 8});
        centerHStack(sv);
        addSubview(data->switchContainer, sv);
        pin(sv, data->switchContainer, (Padding){0}, 0);
        addArrangedSubview(aboveTF, data->switchContainer);
        releaseObj(sv);
        releaseObj(label);
        releaseObj(switchView);
    }

    CFStringRef titles[4];
    fillStringArray(titles, CFSTR("maxWeight%d"), 4);
    for (int i = 0; i < 4; ++i) {
        inputVC_addChild(self, titles[i], 0, 999);
    }

    SEL btnTap = sel_getUid("buttonTapped:");
    parent->button = createButton(localize(CFSTR("settingsSave")), ColorBlue, BtnBackground, 0,
                                  self, btnTap, 44);

    data->deleteButton = createButton(localize(CFSTR("settingsDelete")), ColorRed, BtnBackground, 1,
                                      self, btnTap, 44);

    id belowTF = createStackView((id[]){parent->button, data->deleteButton}, 2, 1, 20,
                                 (Padding){20, 0, 0, 0});
    addArrangedSubview(parent->vStack, belowTF);

    releaseObj(aboveTF);
    releaseObj(planLabel);
    releaseObj(belowTF);
    releaseObj(picker);

    settingsVC_updateWeightFields(self);
}

void settingsVC_buttonTapped(id self, SEL _cmd _U_, id btn) {
    int tag = (int) getTag(btn);
    CFStringRef message = localize(tag ? CFSTR("alertMsgDelete") : CFSTR("alertMsgSave"));
    id ctrl = createAlertController(localize(CFSTR("settingsAlertTitle")), message);
    addAlertAction(ctrl, localize(CFSTR("cancel")), 1, NULL);
    if (tag) {
        addAlertAction(ctrl, localize(CFSTR("delete")), 2, ^{ appDel_deleteAppData(); });
        presentVC(self, ctrl);
        return;
    }

    SettingsVC *data = (SettingsVC *) ((char *)self + InputVCSize);
    unsigned char dark = 0xff;
    if (data->switchContainer) {
        id sv = (id) CFArrayGetValueAtIndex(getSubviews(data->switchContainer), 0);
        id switchView = (id) CFArrayGetValueAtIndex(getArrangedSubviews(sv), 1);
        dark = getBool(switchView, sel_getUid("isOn")) ? 1 : 0;
    }
    id picker = (id) CFArrayGetValueAtIndex(getArrangedSubviews(data->planContainer), 1);
    unsigned char plan = (unsigned char) (getSelectedSegment(picker) - 1);

    short *arr = data->results;
    id *fields = ((InputVC *) ((char *)self + VCSize))->children;
    for (int i = 0; i < 4; ++i) {
        arr[i] = ((InputView *) ((char *)fields[i] + ViewSize))->result;
    }
    addAlertAction(ctrl, localize(CFSTR("save")), 0, ^{ appDel_updateUserInfo(plan, dark, arr); });
    presentVC(self, ctrl);
}
