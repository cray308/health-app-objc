#include "SettingsVC.h"
#include "AppDelegate.h"
#include "InputVC.h"
#include "Views.h"

extern uint64_t UIAccessibilityTraitButton;

Class SwitchViewClass;
Class SettingsVCClass;

#pragma mark - Dark Mode Switch

static void handleSwitchToggle(id self, int value) {
    CFLocaleRef locale = CFLocaleCopyCurrent();
    CFStringRef valStr = formatStr(locale, CFSTR("%d"), value);
    msg1(void, CFStringRef, self, sel_getUid("setAccessibilityValue:"), valStr);
    CFRelease(valStr);
    CFRelease(locale);
}

static id switchView_init(SwitchView **ref, bool darkMode) {
    id self = new(SwitchViewClass);
    SwitchView *v = (SwitchView *)getIVV(self);
    *ref = v;

    CFStringRef dmText = localize(CFSTR("darkMode"));
    setIsAccessibilityElement(self, true);
    setAccessibilityTraits(self, UIAccessibilityTraitButton);
    setAccessibilityLabel(self, dmText);
    setBackgroundColor(self, getColor(ColorSecondaryBGGrouped));
    setCornerRadius(self);
    setHeight(self, ViewHeightDefault, true, false);

    v->label = createLabel(dmText, UIFontTextStyleBody, ColorLabel);
    v->sv = new(objc_getClass("UISwitch"));
    msg1(void, bool, v->sv, sel_getUid("setOn:"), darkMode);
    addTarget(v->sv, self, sel_getUid("valueDidChange"), UIControlEventValueChanged);
    msg2(void, float, long, v->sv, sel_getUid("setContentHuggingPriority:forAxis:"),
         LayoutPriorityRequired, UILayoutConstraintAxisHorizontal);
    msg2(void, float, long, v->sv, sel_getUid("setContentCompressionResistancePriority:forAxis:"),
         LayoutPriorityRequired, UILayoutConstraintAxisHorizontal);

    id stack = createHStack((id []){v->label, v->sv});
    setTrans(stack);
    setLayoutMargins(stack, ((HAInsets){0, 8, 0, 8}));
    addSubview(self, stack);
    pin(stack, self);
    handleSwitchToggle(self, darkMode);
    releaseV(stack);
    return self;
}

void switchView_didChange(id self, SEL _cmd _U_) {
    handleSwitchToggle(self, msg0(bool, ((SwitchView *)getIVV(self))->sv, sel_getUid("isOn")));
}

bool switchView_activate(id self, SEL _cmd _U_) {
    id sv = ((SwitchView *)getIVV(self))->sv;
    bool newVal = !msg0(bool, sv, sel_getUid("isOn"));
    msg1(void, bool, sv, sel_getUid("setOn:"), newVal);
    handleSwitchToggle(self, newVal);
    return true;
}

#pragma mark - VC

void settingsVC_updateColors(id self, bool darkMode) {
    InputVC *p = (InputVC *)getIVVC(self);
    SettingsVC *d = (SettingsVC *)getIVIVCS(p);
    setBackgroundColor(msg0(id, self, sel_getUid("view")), getColor(ColorPrimaryBGGrouped));
    id red = getColor(ColorRed), label = getColor(ColorLabel);
    id fieldBg = getColor(ColorTertiaryBG);
    setTextColor(d->planLabel, label);
    updateSegmentedControl(d->planPicker, darkMode);
    setBackgroundColor(d->dmSwitch.view, getColor(ColorSecondaryBGGrouped));
    setTextColor(d->dmSwitch.d->label, label);
    updateButtonColors(d->deleteButton, ColorRed);
    updateButtonColors(p->button, ColorBlue);
    id barColor = clsF1(id, int, UIColor, sel_getUid("getBarColorWithType:"), BarColorModal);
    msg1(void, id, p->toolbar, sel_getUid("setBarTintColor:"), barColor);
    msg1(void, id, p->toolbar, sel_getUid("setTintColor:"), red);
    for (int i = 0; i < 4; ++i) {
        InputView *v = p->children[i].data;
        setTextColor(v->errorLabel, red);
        setTextColor(v->hintLabel, label);
        setTextColor(v->field, label);
        setBackgroundColor(v->field, fieldBg);
        msg1(void, long, v->field, sel_getUid("setKeyboardAppearance:"), darkMode);
    }
}

void settingsVC_viewDidLoad(id self, SEL _cmd) {
    msgSup0(void, (&(struct objc_super){self, InputVCClass}), _cmd);

    InputVC *p = (InputVC *)getIVVC(self);
    SettingsVC *d = (SettingsVC *)getIVIVCS(p);
    setBackgroundColor(msg0(id, self, sel_getUid("view")), getColor(ColorPrimaryBGGrouped));
    setupNavItem(self, CFSTR("settingsTitle"), NULL);

    UserInfo const *info = getUserInfo();
    const unsigned char darkMode = info->darkMode;
    unsigned char segment = info->currentPlan + 1;

    d->planLabel = createLabel(localize(CFSTR("planPickerTitle")),
                               UIFontTextStyleSubheadline, ColorLabel);
    addArrangedSubview(p->vStack, d->planLabel);
    setCustomSpacing(p->vStack, ViewSpacing, d->planLabel);

    d->planPicker = createSegmentedControl(CFSTR("settingsSegment%d"), segment);
    setHeight(d->planPicker, ViewHeightDefault, true, false);
    addArrangedSubview(p->vStack, d->planPicker);
    setCustomSpacing(p->vStack, GroupSpacing, d->planPicker);

    if (isCharValueValid(darkMode)) {
        updateSegmentedControl(d->planPicker, darkMode);
        d->dmSwitch.view = switchView_init(&d->dmSwitch.d, darkMode);
        addArrangedSubview(p->vStack, d->dmSwitch.view);
        setCustomSpacing(p->vStack, GroupSpacing, d->dmSwitch.view);
    }

    CFStringRef liftNames[4];
    fillStringArray(liftNames, CFSTR("exNames%02d"), 4);
    CFLocaleRef locale = CFLocaleCopyCurrent();
    CFStringRef fieldKey = localize(CFSTR("maxWeight"));
    int kb = getKBForLocale(locale);
    for (int i = 0; i < 4; ++i) {
        CFMutableStringRef adjName = CFStringCreateMutableCopy(NULL, 128, liftNames[i]);
        CFRelease(liftNames[i]);
        CFStringLowercase(adjName, locale);
        inputVC_addChild(self, formatStr(NULL, fieldKey, adjName), kb, 0, FieldMaxDefault);
        CFRelease(adjName);
    }
    CFRelease(fieldKey);
    CFRelease(locale);
    setCustomSpacing(p->vStack, GroupSpacing, p->children[3].view);

    id viewBG = getColor(ColorSecondaryBGGrouped);
    SEL btnTap = sel_getUid("buttonTapped:");
    p->button = createButton(localize(CFSTR("settingsSave")),
                             ColorBlue, UIFontTextStyleBody, self, btnTap);
    setBackgroundColor(p->button, viewBG);
    setCornerRadius(p->button);
    setHeight(p->button, ViewHeightDefault, true, false);
    addArrangedSubview(p->vStack, p->button);
    setCustomSpacing(p->vStack, GroupSpacing, p->button);

    d->deleteButton = createButton(localize(CFSTR("settingsDelete")),
                                   ColorRed, UIFontTextStyleBody, self, btnTap);
    setTag(d->deleteButton, 1);
    setBackgroundColor(d->deleteButton, viewBG);
    setCornerRadius(d->deleteButton);
    setHeight(d->deleteButton, ViewHeightDefault, true, false);
    addArrangedSubview(p->vStack, d->deleteButton);

    inputVC_updateFields(p, info->liftMaxes);
}

void settingsVC_buttonTapped(id self, SEL _cmd _U_, id btn) {
    int tag = (int)getTag(btn);
    CFStringRef message = tag ? CFSTR("alertMsgDelete") : CFSTR("alertMsgSave");
    id ctrl = createAlertController(CFSTR("settingsAlertTitle"), message);
    addAlertAction(ctrl, CFSTR("cancel"), UIAlertActionStyleCancel, NULL);
    if (tag) {
        addAlertAction(ctrl, CFSTR("delete"), UIAlertActionStyleDestructive, ^{ deleteAppData(); });
        showAlert(ctrl);
        return;
    }

    InputVC *p = (InputVC *)getIVVC(self);
    SettingsVC *d = (SettingsVC *)getIVIVCS(p);
    unsigned char dark = UCHAR_MAX;
    if (d->dmSwitch.view) dark = msg0(bool, d->dmSwitch.d->sv, sel_getUid("isOn"));
    long plan = msg0(long, d->planPicker, sel_getUid("selectedSegmentIndex")) - 1;
    for (int i = 0; i < 4; ++i) {
        d->results[i] = (short)lrintf(p->children[i].data->result * toSavedMass);
    }
    addAlertAction(ctrl, CFSTR("save"), UIAlertActionStyleDefault, ^{
        updateUserInfo((unsigned char)plan, dark, d->results);
    });
    showAlert(ctrl);
}
