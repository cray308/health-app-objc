#include "SettingsVC.h"
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
    SettingsVCData *data = calloc(1, sizeof(SettingsVCData));
    object_setIvar(self, SettingsVCDataRef, (id) data);
    return self;
}

void settingsVC_updateWeightFields(id self) {
    InputVCData *parent = (InputVCData *) object_getIvar(self, InputVCDataRef);
    for (int i = 0; i < 4; ++i) {
        short value = userData->liftMaxes[i];
        InputViewData *child =
        (InputViewData *) object_getIvar(parent->children[i], InputViewDataRef);
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
}

void settingsVC_viewDidLoad(id self, SEL _cmd) {
    struct objc_super super = {self, InputVCClass};
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);

    SettingsVCData *data = (SettingsVCData *) object_getIvar(self, SettingsVCDataRef);
    InputVCData *parent = (InputVCData *) object_getIvar(self, InputVCDataRef);
    id view = getView(self);
    setBackground(view, createColor(ColorPrimaryBGGrouped));
    setVCTitle(self, localize(CFSTR("titles2")));

    id planLabel = createLabel(localize(CFSTR("planPickerTitle")), TextFootnote, 4, true);
    data->picker = createSegmentedControl(CFSTR("settingsSegment%d"), 3,
                                          userData->currentPlan + 1, nil, nil, 44);
    id planContainer = createStackView((id []){planLabel, data->picker}, 2, 1, 2,
                                       (Padding){0, 8, 0, 8});

    setMargins(parent->vStack, ((HAInsets){20, 0, 20, 0}));
    addArrangedSubview(parent->vStack, planContainer);
    setSpacingAfter(parent->vStack, planContainer, 20);

    if (osVersion == Version12) {
        id switchContainer = createBackgroundView(ColorSecondaryBGGrouped, 44);
        data->switchView = createObjectWithFrame(objc_getClass("UISwitch"), CGRectZero);
        setBool(data->switchView, sel_getUid("setOn:"), userData->darkMode);
        id label = createLabel(localize(CFSTR("darkMode")), TextBody, 4, true);
        id sv = createStackView((id[]){label,data->switchView}, 2, 0, 5, (Padding){0, 8, 0, 8});
        centerHStack(sv);
        addSubview(switchContainer, sv);
        pin(sv, switchContainer, (Padding){0}, 0);

        addArrangedSubview(parent->vStack, switchContainer);
        setSpacingAfter(parent->vStack, switchContainer, 20);

        releaseObj(switchContainer);
        releaseObj(sv);
        releaseObj(label);
    }

    CFStringRef titles[4];
    fillStringArray(titles, CFSTR("maxWeight%d"), 4);
    for (int i = 0; i < 4; ++i)
        inputVC_addChild(self, titles[i], 0, 999);
    setSpacingAfter(parent->vStack, parent->children[3], 20);

    SEL btnTap = sel_getUid("buttonTapped:");
    parent->button = createButton(localize(CFSTR("settingsSave")), ColorBlue, BtnBackground, 0,
                                  self, btnTap, 44);
    addArrangedSubview(parent->vStack, parent->button);
    setSpacingAfter(parent->vStack, parent->button, 20);

    id deleteButton = createButton(localize(CFSTR("settingsDelete")), ColorRed, BtnBackground, 1,
                                   self, btnTap, 44);
    addArrangedSubview(parent->vStack, deleteButton);

    releaseObj(planContainer);
    releaseObj(planLabel);

    settingsVC_updateWeightFields(self);
    appCoordinator->loadedViewControllers |= LoadedVC_Settings;
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
    bool dark = false;
    if (data->switchView) {
        dark = getBool(data->switchView, sel_getUid("isOn"));
    }
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
