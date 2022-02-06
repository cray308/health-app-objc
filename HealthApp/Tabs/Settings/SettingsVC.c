#include "SettingsVC.h"
#include <stdlib.h>
#include "AppDelegate.h"
#include "AppUserData.h"
#include "InputVC.h"
#include "ViewControllerHelpers.h"

Class SettingsVCClass;
Ivar SettingsVCDataRef;

id settingsVC_init(void) {
    id self = createVC(SettingsVCClass);
#ifndef __clang_analyzer__
    SettingsVCData *data = calloc(1, sizeof(SettingsVCData));
    object_setIvar(self, SettingsVCDataRef, (id) data);
#endif
    return self;
}

void settingsVC_updateWeightFields(id self) {
    InputVCData *parent = (InputVCData *) object_getIvar(self, InputVCDataRef);
    for (int i = 0; i < 4; ++i) {
        short value = userData->liftMaxes[i];
        InputViewData *child = ((InputViewData *)
                                object_getIvar(parent->children[i], InputViewDataRef));
        CFStringRef str = CFStringCreateWithFormat(NULL, NULL, CFSTR("%d"), value);
        setLabelText(child->field, str);
        inputView_reset(child, value);
        CFRelease(str);
    }
    enableButton(parent->button, true);
}

void settingsVC_updateColors(id self) {
    InputVCData *par = (InputVCData *) object_getIvar(self, InputVCDataRef);
    SettingsVCData *data = (SettingsVCData *) object_getIvar(self, SettingsVCDataRef);
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
        InputViewData *ptr = ((InputViewData *) object_getIvar(par->children[i], InputViewDataRef));
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

    SettingsVCData *data = (SettingsVCData *) object_getIvar(self, SettingsVCDataRef);
    InputVCData *parent = (InputVCData *) object_getIvar(self, InputVCDataRef);
    setBackground(getView(self), createColor(ColorPrimaryBGGrouped));
    setVCTitle(self, localize(CFSTR("settingsTitle")));

    id planLabel = createLabel(localize(CFSTR("planPickerTitle")), TextFootnote, 4, true);
    id picker = createSegmentedControl(CFSTR("settingsSegment%d"), 3,
                                       userData->currentPlan + 1, nil, nil, 44);
    data->planContainer = createStackView((id []){planLabel,picker}, 2, 1, 2,(Padding){0, 8, 0, 8});
    id aboveTF = createStackView((id[]){data->planContainer}, 1, 1, 20, (Padding){0, 0, 20, 0});

    setMargins(parent->vStack, ((HAInsets){20, 0, 20, 0}));
    addArrangedSubview(parent->vStack, aboveTF);

    if (userData->darkMode >= 0) {
        data->switchContainer = createBackgroundView(ColorSecondaryBGGrouped, 44, false);
        id switchView = createNew(objc_getClass("UISwitch"));
        setBool(switchView, sel_getUid("setOn:"), userData->darkMode ? true : false);
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
    int tag = getTag(btn);
    CFStringRef message = localize(tag ? CFSTR("alertMsgDelete") : CFSTR("alertMsgSave"));
    id ctrl = createAlertController(localize(CFSTR("settingsAlertTitle")), message);
    addAlertAction(ctrl, localize(CFSTR("cancel")), 1, NULL);
    if (tag) {
        addAlertAction(ctrl, localize(CFSTR("delete")), 2, ^{ appDel_deleteAppData(); });
        presentVC(self, ctrl);
        return;
    }

    SettingsVCData *data = (SettingsVCData *) object_getIvar(self, SettingsVCDataRef);
    signed char dark = -1;
    if (data->switchContainer) {
        id sv = (id) CFArrayGetValueAtIndex(getSubviews(data->switchContainer), 0);
        id switchView = (id) CFArrayGetValueAtIndex(getArrangedSubviews(sv), 1);
        dark = getBool(switchView, sel_getUid("isOn")) ? 1 : 0;
    }
    id picker = (id) CFArrayGetValueAtIndex(getArrangedSubviews(data->planContainer), 1);
    signed char plan = ((signed char) getSelectedSegment(picker)) - 1;

    short *arr = data->results;
    id *fields = ((InputVCData *) object_getIvar(self, InputVCDataRef))->children;
    for (int i = 0; i < 4; ++i) {
        arr[i] = ((InputViewData *) object_getIvar(fields[i], InputViewDataRef))->result;
    }
    addAlertAction(ctrl, localize(CFSTR("save")), 0, ^{ appDel_updateUserInfo(plan, dark, arr); });
    presentVC(self, ctrl);
}
