#include "SettingsVC.h"
#include "AppDelegate.h"
#include "InputVC.h"
#include "Views.h"

#define setOn(s, o) msgV(objSig(void, bool), (s), sel_getUid("setOn:"), (o))

Class SwitchViewClass;
Class SettingsVCClass;

static SEL sio;
static bool (*isOn)(id, SEL);

#pragma mark - Dark Mode Switch

static void updateSwitchAccessibility(id self, int value) {
    CFLocaleRef locale = copyLocale();
    CFStringRef newValue = formatStr(locale, CFSTR("%d"), value);
    setAccessibilityValue(self, newValue);
    CFRelease(locale);
    CFRelease(newValue);
}

static id switchView_init(SwitchView **ref, bool darkMode) {
    Class Switch = objc_getClass("UISwitch");
    sio = sel_getUid("isOn");
    isOn = (bool(*)(id, SEL))class_getMethodImplementation(Switch, sio);

    id self = new(SwitchViewClass);
    SwitchView *v = getIVV(SwitchView, self);
    *ref = v;

    CFStringRef labelText = localize(CFSTR("darkMode"));
    makeCustomViewAccessible(self)
    setAccessibilityTraits(self, UIAccessibilityTraitButton);
    setAccessibilityLabel(self, labelText);
    setBackgroundColor(self, getColor(ColorSecondaryBGGrouped));
    addCornerRadius(self);
    setHeight(self, ViewHeightDefault, true, false);

    v->label = createLabel(labelText, UIFontTextStyleBody, ColorLabel);
    v->button = new(Switch);
    setOn(v->button, darkMode);
    addTarget(v->button, self, getValueChangedSel(), ControlEventValueChanged);
    setContentHuggingPriority(v->button, LayoutPriorityRequired, ConstraintAxisHorizontal);
    setContentCompressionResistancePriority(v->button,
                                            LayoutPriorityRequired, ConstraintAxisHorizontal);

    id stack = createHStack((id []){v->label, v->button});
    useStackConstraints(stack);
    setLayoutMargins(stack, (HAInsets){0, 8, 0, 8});
    addSubview(self, stack);
    pin(stack, self);
    updateSwitchAccessibility(self, darkMode);
    releaseView(stack);
    return self;
}

void switchView_changedValue(id self, SEL _cmd _U_) {
    updateSwitchAccessibility(self, isOn(getIVV(SwitchView, self)->button, sio));
}

bool switchView_accessibilityActivate(id self, SEL _cmd _U_) {
    id switchView = getIVV(SwitchView, self)->button;
    bool newValue = !isOn(switchView, sio);
    setOn(switchView, newValue);
    updateSwitchAccessibility(self, newValue);
    return true;
}

#pragma mark - VC

void settingsVC_updateColors(id self, bool darkMode) {
    InputVC *p = getIVVC(InputVC, self);
    SettingsVC *d = getIVVCS(SettingsVC, p);
    setBackgroundColor(getView(self), getColor(ColorPrimaryBGGrouped));
    id red = getColor(ColorRed), labelColor = getColor(ColorLabel);
    id fieldBackground = getColor(ColorTertiaryBG);
    setTextColor(d->planLabel, labelColor);
    updateSegmentedControlColors(d->planControl, darkMode);
    setBackgroundColor(d->darkModeSwitch.view, getColor(ColorSecondaryBGGrouped));
    setTextColor(d->darkModeSwitch.data->label, labelColor);
    updateButtonColors(d->deleteButton, ColorRed);
    updateButtonColors(p->button, ColorBlue);
    setBarTintColor(p->toolbar, getBarColor(BarColorModal));
    setTintColor(p->toolbar, red);
    for (int i = 0; i < 4; ++i) {
        InputView *v = p->children[i].data;
        setTextColor(v->errorLabel, red);
        setTextColor(v->hintLabel, labelColor);
        setFieldTextColor(v->field, labelColor);
        setFieldBackgroundColor(v->field, fieldBackground);
        setKeyboardAppearance(v->field, darkMode);
    }
}

void settingsVC_viewDidLoad(id self, SEL _cmd) {
    inputVC_viewDidLoad(self, _cmd);

    InputVC *p = getIVVC(InputVC, self);
    SettingsVC *d = getIVVCS(SettingsVC, p);
    setBackgroundColor(getView(self), getColor(ColorPrimaryBGGrouped));
    setupNavItem(self, CFSTR("settingsTitle"), NULL);

    id planLabel = createLabel(localize(CFSTR("planPicker")), UIFontTextStyleSubheadline, ColorLabel);
    addArrangedSubview(p->vStack, planLabel);
    setCustomSpacing(p->vStack, ViewSpacing, planLabel);
    d->planLabel = planLabel;

    UserData const *data = getUserData();
    id planControl = createSegmentedControl(CFSTR("settingsSegment%d"), (uint8_t)(data->plan + 1));
    setHeight(planControl, ViewHeightDefault, true, false);
    addArrangedSubview(p->vStack, planControl);
    setCustomSpacing(p->vStack, GroupSpacing, planControl);
    d->planControl = planControl;

    if (isCharValid(data->darkMode)) {
        updateSegmentedControlColors(planControl, data->darkMode);
        d->darkModeSwitch.view = switchView_init(&d->darkModeSwitch.data, data->darkMode);
        addArrangedSubview(p->vStack, d->darkModeSwitch.view);
        setCustomSpacing(p->vStack, GroupSpacing, d->darkModeSwitch.view);
    }

    CFLocaleRef locale = copyLocale();
    int kbType = getKeyboardForLocale(locale);
    CFStringRef fieldKey = localize(CFSTR("maxWeight")), liftNames[4];
    fillStringArray(liftNames, CFSTR("exNames%02d"), 4);
    for (int i = 0; i < 4; ++i) {
        CFMutableStringRef adjustedName = CFStringCreateMutableCopy(NULL, 128, liftNames[i]);
        CFStringLowercase(adjustedName, locale);
        inputVC_addField(self, formatStr(NULL, fieldKey, adjustedName), kbType, 0, FieldMaxDefault);
        CFRelease(liftNames[i]);
        CFRelease(adjustedName);
    }
    CFRelease(locale);
    CFRelease(fieldKey);
    setCustomSpacing(p->vStack, GroupSpacing, p->children[3].view);

    id buttonBackground = getColor(ColorSecondaryBGGrouped);
    SEL tapSel = getTapSel();
    p->button = createButton(localize(CFSTR("settingsSave")), ColorBlue, false, self, tapSel);
    setBackgroundColor(p->button, buttonBackground);
    addCornerRadius(p->button);
    setHeight(p->button, ViewHeightDefault, true, false);
    addArrangedSubview(p->vStack, p->button);
    setCustomSpacing(p->vStack, GroupSpacing, p->button);

    id deleteButton = createButton(localize(CFSTR("settingsDelete")), ColorRed, false, self, tapSel);
    setTag(deleteButton, 1);
    setBackgroundColor(deleteButton, buttonBackground);
    addCornerRadius(deleteButton);
    setHeight(deleteButton, ViewHeightDefault, true, false);
    addArrangedSubview(p->vStack, deleteButton);
    d->deleteButton = deleteButton;

    inputVC_updateFields(p, data->lifts);
}

void settingsVC_buttonTapped(id self, SEL _cmd _U_, id button) {
    long tag = getTag(button);
    id alert = createAlert(CFSTR("settingsAlert"), tag ? CFSTR("msgDelete") : CFSTR("msgSave"));
    addAlertAction(alert, CFSTR("cancel"), ActionStyleCancel, NULL);
    if (tag) {
        addAlertAction(alert, CFSTR("delete"), ActionStyleDestructive, ^{ deleteAppData(); });
        goto end;
    }

    InputVC *p = getIVVC(InputVC, self);
    SettingsVC *d = getIVVCS(SettingsVC, p);
    uint8_t darkMode = UCHAR_MAX;
    if (d->darkModeSwitch.view) darkMode = isOn(d->darkModeSwitch.data->button, sio);
    CFLocaleRef locale = copyLocale();
    float toSavedMass = getSavedMassFactor(locale);
    CFRelease(locale);
    for (int i = 0; i < 4; ++i) {
        d->results[i] = (int)lrintf(p->children[i].data->result * toSavedMass);
    }
    addAlertAction(alert, CFSTR("save"), ActionStyleDefault, ^{
        updateUserInfo((uint8_t)(getSelectedSegmentIndex(d->planControl) - 1), darkMode, d->results);
    });
end:
    disableWindowTint();
    presentVC(self, alert);
}
