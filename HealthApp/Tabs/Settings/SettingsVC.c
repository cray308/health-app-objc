#include "SettingsVC.h"
#include "AppDelegate.h"
#include "InputVC.h"
#include "Views.h"

Class SettingsVCClass;

void settingsVC_viewDidLoad(id self, SEL _cmd) {
    inputVC_viewDidLoad(self, _cmd);

    InputVC *p = getIVVC(InputVC, self);
    SettingsVC *d = getIVVCS(SettingsVC, p);
    setBackgroundColor(getView(self), getColor(ColorPrimaryBGGrouped));
    setupNavItem(self, CFSTR("settingsTitle"), NULL);

    id planLabel = createLabel(localize(CFSTR("planPicker")), UIFontTextStyleSubheadline, ColorLabel);
    addArrangedSubview(p->vStack, planLabel);
    setCustomSpacing(p->vStack, ViewSpacing, planLabel);
    releaseView(planLabel);

    UserData const *data = getUserData();
    id planControl = createSegmentedControl(CFSTR("settingsSegment%d"), (uint8_t)(data->plan + 1));
    setHeight(planControl, ViewHeightDefault, true, false);
    addArrangedSubview(p->vStack, planControl);
    setCustomSpacing(p->vStack, GroupSpacing, planControl);
    d->planControl = planControl;

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
    CFLocaleRef locale = copyLocale();
    float toSavedMass = getSavedMassFactor(locale);
    CFRelease(locale);
    for (int i = 0; i < 4; ++i) {
        d->results[i] = (int)lrintf(p->children[i].data->result * toSavedMass);
    }
    addAlertAction(alert, CFSTR("save"), ActionStyleDefault, ^{
        updateUserInfo((uint8_t)(getSelectedSegmentIndex(d->planControl) - 1), d->results);
    });
end:
    disableWindowTint();
    presentVC(self, alert);
}
