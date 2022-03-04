#include "SettingsVC.h"
#include "AppDelegate.h"
#include "AppUserData.h"
#include "InputVC.h"
#include "ViewControllerHelpers.h"

Class SettingsVCClass;

id settingsVC_init(void) {
    return createNew(SettingsVCClass);
}

void settingsVC_updateColors(id self, unsigned char darkMode) {
    InputVC *parent = (InputVC *) ((char *)self + VCSize);
    SettingsVC *data = (SettingsVC *) ((char *)self + InputVCSize);
    setBackground(getView(self), createColor(ColorPrimaryBGGrouped));
    id red = createColor(ColorRed), label = createColor(ColorLabel);
    id btnBg = createColor(ColorSecondaryBGGrouped), fieldBg = createColor(ColorTertiaryBG);
    CFArrayRef views = getArrangedSubviews(data->planContainer);
    setTextColor((id) CFArrayGetValueAtIndex(views, 0), label);
    updateSegmentedControl((id) CFArrayGetValueAtIndex(views, 1), label, darkMode);
    setBackground(data->switchContainer, btnBg);
    id view = (id) CFArrayGetValueAtIndex(getSubviews(data->switchContainer), 0);
    setTextColor((id)CFArrayGetValueAtIndex(getArrangedSubviews(view), 0), label);
    setButtonColor(data->deleteButton, red, 0);
    setBackground(data->deleteButton, btnBg);
    setButtonColor(parent->button, createColor(ColorBlue), 0);
    setButtonColor(parent->button, createColor(ColorSecondaryLabel), 2);
    setBackground(parent->button, btnBg);
    setBarTint(parent->toolbar, getBarColor(ColorBarModal));
    view = (id) CFArrayGetValueAtIndex(msg0(CFArrayRef, parent->toolbar, sel_getUid("items")), 1);
    setTintColor(view, red);
    for (int i = 0; i < 4; ++i) {
        InputView *ptr = parent->children[i];
        setTextColor(ptr->errorLabel, red);
        setTextColor(ptr->hintLabel, label);
        setTextColor(ptr->field, label);
        setBackground(ptr->field, fieldBg);
        setKBColor(ptr->field, darkMode);
    }
}

void settingsVC_viewDidLoad(id self, SEL _cmd) {
    struct objc_super super = {self, InputVCClass};
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);

    CFBundleRef bundle = CFBundleGetMainBundle();
    InputVC *parent = (InputVC *) ((char *)self + VCSize);
    SettingsVC *data = (SettingsVC *) ((char *)self + InputVCSize);
    id viewBG = createColor(ColorSecondaryBGGrouped);
    setBackground(getView(self), createColor(ColorPrimaryBGGrouped));
    setVCTitle(self, CFBundleCopyLocalizedString(bundle, CFSTR("settingsTitle"), NULL, NULL));

    const unsigned char darkMode = userData->darkMode;
    unsigned char segment = userData->currentPlan + 1;
    id planLabel = createLabel(CFBundleCopyLocalizedString(bundle, CFSTR("planPickerTitle"),
                                                           NULL, NULL),
                               UIFontTextStyleFootnote, true);
    id picker = createSegmentedControl(bundle, CFSTR("settingsSegment%d"), segment);
    setHeight(picker, 44, false);
    data->planContainer = createStackView((id []){planLabel,picker}, 2, 1, 2,(Padding){0, 8, 0, 8});
    id aboveTF = createStackView((id[]){data->planContainer}, 1, 1, 20, (Padding){0, 0, 20, 0});

    setMargins(parent->vStack, ((HAInsets){20, 0, 20, 0}));
    addArrangedSubview(parent->vStack, aboveTF);

    if (!(darkMode & 128)) {
        updateSegmentedControl(picker, createColor(ColorLabel), darkMode);
        data->switchContainer = createNew(ViewClass);
        setBackground(data->switchContainer, viewBG);
        setHeight(data->switchContainer, 44, false);
        id switchView = createNew(objc_getClass("UISwitch"));
        msg1(void, bool, switchView, sel_getUid("setOn:"), darkMode);
        id label = createLabel(CFBundleCopyLocalizedString(bundle, CFSTR("darkMode"), NULL, NULL),
                               UIFontTextStyleBody, true);
        id sv = createStackView((id[]){label, switchView}, 2, 0, 5, (Padding){0, 8, 0, 8});
        setUsesAutolayout(sv);
        centerHStack(sv);
        addSubview(data->switchContainer, sv);
        pin(sv, data->switchContainer);
        addArrangedSubview(aboveTF, data->switchContainer);
        releaseObj(sv);
        releaseObj(label);
        releaseObj(switchView);
    }

    CFStringRef titles[4];
    fillStringArray(bundle, titles, CFSTR("maxWeight%d"), 4);
    for (int i = 0; i < 4; ++i) {
        inputVC_addChild(self, titles[i], 0, 999);
    }

    SEL btnTap = sel_getUid("buttonTapped:");
    parent->button = createButton(CFBundleCopyLocalizedString(bundle, CFSTR("settingsSave"),
                                                              NULL, NULL),
                                  ColorBlue, 0, self, btnTap);
    setBackground(parent->button, viewBG);
    setHeight(parent->button, 44, false);

    data->deleteButton = createButton(CFBundleCopyLocalizedString(bundle, CFSTR("settingsDelete"),
                                                                  NULL, NULL),
                                      ColorRed, 1, self, btnTap);
    setBackground(data->deleteButton, viewBG);
    setHeight(data->deleteButton, 44, false);

    id belowTF = createStackView((id[]){parent->button, data->deleteButton}, 2, 1, 20,
                                 (Padding){20, 0, 0, 0});
    addArrangedSubview(parent->vStack, belowTF);

    releaseObj(aboveTF);
    releaseObj(planLabel);
    releaseObj(belowTF);
    releaseObj(picker);

    inputVC_updateFields(parent, userData->liftMaxes);
}

void settingsVC_buttonTapped(id self, SEL _cmd _U_, id btn) {
    CFBundleRef bundle = CFBundleGetMainBundle();
    int tag = (int) getTag(btn);
    CFStringRef msgKey = tag ? CFSTR("alertMsgDelete") : CFSTR("alertMsgSave");
    id ctrl = createAlertController(CFBundleCopyLocalizedString(bundle, CFSTR("settingsAlertTitle"),
                                                                NULL, NULL),
                                    CFBundleCopyLocalizedString(bundle, msgKey, NULL, NULL));
    addAlertAction(ctrl, CFBundleCopyLocalizedString(bundle, CFSTR("cancel"), NULL, NULL), 1, NULL);
    if (tag) {
        addAlertAction(ctrl, CFBundleCopyLocalizedString(bundle, CFSTR("delete"), NULL, NULL), 2, ^{
            appDel_deleteAppData();
        });
        presentVC(ctrl);
        return;
    }

    SettingsVC *data = (SettingsVC *) ((char *)self + InputVCSize);
    unsigned char dark = 0xff;
    if (data->switchContainer) {
        id sv = (id) CFArrayGetValueAtIndex(getSubviews(data->switchContainer), 0);
        id switchView = (id) CFArrayGetValueAtIndex(getArrangedSubviews(sv), 1);
        dark = msg0(bool, switchView, sel_getUid("isOn")) ? 1 : 0;
    }
    id picker = (id) CFArrayGetValueAtIndex(getArrangedSubviews(data->planContainer), 1);
    unsigned char plan = (unsigned char) (getSelectedSegment(picker) - 1);

    short *arr = data->results;
    InputView **fields = ((InputVC *) ((char *)self + VCSize))->children;
    for (int i = 0; i < 4; ++i) {
        arr[i] = fields[i]->result;
    }
    addAlertAction(ctrl, CFBundleCopyLocalizedString(bundle, CFSTR("save"), NULL, NULL), 0, ^{
        appDel_updateUserInfo(plan, dark, arr);
    });
    presentVC(ctrl);
}
