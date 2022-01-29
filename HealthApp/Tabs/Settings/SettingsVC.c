#include "SettingsVC.h"
#include <stdlib.h>
#include "AppCoordinator.h"
#include "AppUserData.h"
#include "InputVC.h"
#include "ViewControllerHelpers.h"

Class SettingsVCClass;
Ivar SettingsVCDataRef;

static inline void addCancelAction(id ctrl, Callback handler) {
    addAlertAction(ctrl, localize(CFSTR("cancel")), 1, handler);
}

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
    SettingsVCData *data = (SettingsVCData *) object_getIvar(self, SettingsVCDataRef);
    id view = getView(self);
    setBackground(view, createColor(ColorPrimaryBGGrouped));
    updateSegmentedControl(data->picker);
    id toolbar = ((InputVCData *) object_getIvar(self, InputVCDataRef))->toolbar;
    CFArrayRef items = getArray(toolbar, sel_getUid("items"));
    id doneButton = (id) CFArrayGetValueAtIndex(items, 1);
    setTintColor(doneButton, createColor(ColorRed));
    setBarTint(toolbar);
}

void settingsVC_viewDidLoad(id self, SEL _cmd) {
    struct objc_super super = {self, InputVCClass};
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);

    SettingsVCData *data = (SettingsVCData *) object_getIvar(self, SettingsVCDataRef);
    InputVCData *parent = (InputVCData *) object_getIvar(self, InputVCDataRef);
    id view = getView(self);
    setBackground(view, createColor(ColorPrimaryBGGrouped));
    setVCTitle(self, localize(CFSTR("settingsTitle")));

    id planLabel = createLabel(localize(CFSTR("planPickerTitle")), TextFootnote, 4, true);
    data->picker = createSegmentedControl(CFSTR("settingsSegment%d"), 3,
                                          userData->currentPlan + 1, nil, nil, 44);
    id planContainer = createStackView((id []){planLabel, data->picker}, 2, 1, 2,
                                       (Padding){0, 8, 0, 8});
    id aboveTF = createStackView((id[]){planContainer}, 1, 1, 20, (Padding){0, 0, 20, 0});

    setMargins(parent->vStack, ((HAInsets){20, 0, 20, 0}));
    addArrangedSubview(parent->vStack, aboveTF);

    if (userData->darkMode >= 0) {
        id switchContainer = createBackgroundView(ColorSecondaryBGGrouped, 44, false);
        data->switchView = createNew(objc_getClass("UISwitch"));
        setBool(data->switchView, sel_getUid("setOn:"), userData->darkMode ? true : false);
        id label = createLabel(localize(CFSTR("darkMode")), TextBody, 4, true);
        id sv = createStackView((id[]){label,data->switchView}, 2, 0, 5, (Padding){0, 8, 0, 8});
        centerHStack(sv);
        addSubview(switchContainer, sv);
        pin(sv, switchContainer, (Padding){0}, 0);

        addArrangedSubview(aboveTF, switchContainer);

        releaseObj(switchContainer);
        releaseObj(sv);
        releaseObj(label);
    }

    CFStringRef titles[4];
    fillStringArray(titles, CFSTR("maxWeight%d"), 4);
    for (int i = 0; i < 4; ++i) {
        inputVC_addChild(self, titles[i], 0, 999);
    }

    SEL btnTap = sel_getUid("buttonTapped:");
    parent->button = createButton(localize(CFSTR("settingsSave")), ColorBlue, BtnBackground, 0,
                                  self, btnTap, 44);

    id deleteButton = createButton(localize(CFSTR("settingsDelete")), ColorRed, BtnBackground, 1,
                                   self, btnTap, 44);

    id belowTF = createStackView((id[]){parent->button, deleteButton}, 2, 1, 20,
                                 (Padding){20, 0, 0, 0});
    addArrangedSubview(parent->vStack, belowTF);

    releaseObj(aboveTF);
    releaseObj(planContainer);
    releaseObj(planLabel);
    releaseObj(belowTF);

    settingsVC_updateWeightFields(self);
}

void settingsVC_buttonTapped(id self, SEL _cmd _U_, id btn) {
    int tag = getTag(btn);
    CFStringRef message = localize(tag ? CFSTR("alertMsgDelete") : CFSTR("alertMsgSave"));
    id ctrl = createAlertController(localize(CFSTR("settingsAlertTitle")), message);
    if (tag) {
        addAlertAction(ctrl, localize(CFSTR("delete")), 2, ^{ appCoordinator_deleteAppData(); });
        addCancelAction(ctrl, NULL);
        presentVC(self, ctrl);
        return;
    }

    SettingsVCData *data = (SettingsVCData *) object_getIvar(self, SettingsVCDataRef);
    signed char dark = -1;
    if (data->switchView)
        dark = getBool(data->switchView, sel_getUid("isOn")) ? 1 : 0;
    signed char plan = ((signed char) getSelectedSegment(data->picker)) - 1;

    short *results = malloc(sizeof(short) << 2);
    id *fields = ((InputVCData *) object_getIvar(self, InputVCDataRef))->children;
    for (int i = 0; i < 4; ++i) {
        results[i] = ((InputViewData *) object_getIvar(fields[i], InputViewDataRef))->result;
    }
    addAlertAction(ctrl, localize(CFSTR("save")), 0, ^{
        appCoordinator_updateUserInfo(plan, dark, results);
    });
    addCancelAction(ctrl, ^{ free(results); });
    presentVC(self, ctrl);
}
