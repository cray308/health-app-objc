#include "SettingsVC.h"
#include "AppDelegate.h"
#include "InputVC.h"
#include "Views.h"

#define setOn(s, o) msg1(void, bool, (s), sel_getUid("setOn:"), (o))

Class SwitchViewClass;
Class SettingsVCClass;

static SEL sio;
static bool (*isOn)(id, SEL);

#pragma mark - Dark Mode Switch

static id switchView_init(SwitchView **ref, bool darkMode) {
    Class Switch = objc_getClass("UISwitch");
    sio = sel_getUid("isOn");
    isOn = (bool(*)(id, SEL))class_getMethodImplementation(Switch, sio);

    id self = new(SwitchViewClass);
    SwitchView *v = (SwitchView *)((char *)self + ViewSize);
    *ref = v;

    CFStringRef labelText = localize(CFSTR("darkMode"));
    setBackgroundColor(self, getColor(ColorSecondaryBGGrouped));
    addCornerRadius(self);
    setHeight(self, ViewHeightDefault, true, false);

    v->label = createLabel(labelText, UIFontTextStyleBody, ColorLabel);
    v->button = new(Switch);
    setOn(v->button, darkMode);

    id stack = createHStack((id []){v->label, v->button});
    useStackConstraints(stack);
    setLayoutMargins(stack, (HAInsets){0, 8, 0, 8});
    addSubview(self, stack);
    pin(stack, self);
    releaseView(stack);
    return self;
}

#pragma mark - VC

void settingsVC_updateColors(id self, bool darkMode) {
    InputVC *p = (InputVC *)((char *)self + VCSize);
    SettingsVC *d = (SettingsVC *)((char *)p + sizeof(InputVC));
    setBackgroundColor(getView(self), getColor(ColorPrimaryBGGrouped));
    id red = getColor(ColorRed), labelColor = getColor(ColorLabel);
    id fieldBackground = getColor(ColorTertiaryBG);
    setTextColor(d->planLabel, labelColor);
    updateSegmentedControl(d->planControl, darkMode);
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

    InputVC *p = (InputVC *)((char *)self + VCSize);
    SettingsVC *d = (SettingsVC *)((char *)p + sizeof(InputVC));
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
        updateSegmentedControl(planControl, data->darkMode);
        d->darkModeSwitch.view = switchView_init(&d->darkModeSwitch.data, data->darkMode);
        addArrangedSubview(p->vStack, d->darkModeSwitch.view);
        setCustomSpacing(p->vStack, GroupSpacing, d->darkModeSwitch.view);
    }

    CFLocaleRef locale = CFLocaleCopyCurrent();
    int kbType = getKeyboardForLocale(locale);
    CFStringRef fieldKey = localize(CFSTR("maxWeight")), liftNames[4];
    fillStringArray(liftNames, CFSTR("exNames%02d"), 4);
    for (int i = 0; i < 4; ++i) {
        CFMutableStringRef adjustedName = CFStringCreateMutableCopy(NULL, 128, liftNames[i]);
        CFStringLowercase(adjustedName, locale);
        inputVC_addChild(self, formatStr(NULL, fieldKey, adjustedName), kbType, 0, FieldMaxDefault);
        CFRelease(liftNames[i]);
        CFRelease(adjustedName);
    }
    CFRelease(locale);
    CFRelease(fieldKey);
    setCustomSpacing(p->vStack, GroupSpacing, p->children[3].view);

    id buttonBackground = getColor(ColorSecondaryBGGrouped);
    SEL tapSel = getTapSel();
    p->button = createButton(localize(CFSTR("settingsSave")), ColorBlue, self, tapSel);
    setBackgroundColor(p->button, buttonBackground);
    addCornerRadius(p->button);
    setHeight(p->button, ViewHeightDefault, true, false);
    addArrangedSubview(p->vStack, p->button);
    setCustomSpacing(p->vStack, GroupSpacing, p->button);

    id deleteButton = createButton(localize(CFSTR("settingsDelete")), ColorRed, self, tapSel);
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

    InputVC *sup = (InputVC *)((char *)self + VCSize);
    SettingsVC *d = (SettingsVC *)((char *)sup + sizeof(InputVC));
    uint8_t darkMode = UCHAR_MAX;
    if (d->darkModeSwitch.view) darkMode = isOn(d->darkModeSwitch.data->button, sio);
    CFLocaleRef locale = CFLocaleCopyCurrent();
    float toSavedMass = getSavedMassFactor(locale);
    CFRelease(locale);
    for (int i = 0; i < 4; ++i) {
        d->results[i] = (int)lrintf(sup->children[i].data->result * toSavedMass);
    }
    addAlertAction(alert, CFSTR("save"), ActionStyleDefault, ^{
        updateUserInfo((uint8_t)(getSelectedSegmentIndex(d->planControl) - 1), darkMode, d->results);
    });
end:
    disableWindowTint();
    presentVC(self, alert);
}
