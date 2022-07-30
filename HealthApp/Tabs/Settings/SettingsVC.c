#include "SettingsVC.h"
#include "AppDelegate.h"
#include "InputVC.h"
#include "Views.h"

Class SettingsVCClass;

void settingsVC_viewDidLoad(id self, SEL _cmd) {
    msgSup0(void, (&(struct objc_super){self, InputVCClass}), _cmd);

    InputVC *p = (InputVC *)getIVVC(self);
    setBackgroundColor(msg0(id, self, sel_getUid("view")), getColor(ColorPrimaryBGGrouped));
    setupNavItem(self, CFSTR("settingsTitle"), NULL);

    UserInfo const *info = getUserInfo();
    unsigned char segment = info->currentPlan + 1;

    id planLabel = createLabel(localize(CFSTR("planPickerTitle")),
                               UIFontTextStyleSubheadline, ColorLabel);
    addArrangedSubview(p->vStack, planLabel);
    setCustomSpacing(p->vStack, ViewSpacing, planLabel);
    releaseV(planLabel);

    id planPicker = createSegmentedControl(CFSTR("settingsSegment%d"), segment);
    setHeight(planPicker, ViewHeightDefault, true, false);
    addArrangedSubview(p->vStack, planPicker);
    setCustomSpacing(p->vStack, GroupSpacing, planPicker);
    ((SettingsVC *)getIVIVCS(p))->planPicker = planPicker;

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

    id deleteButton = createButton(localize(CFSTR("settingsDelete")),
                                   ColorRed, UIFontTextStyleBody, self, btnTap);
    setTag(deleteButton, 1);
    setBackgroundColor(deleteButton, viewBG);
    setCornerRadius(deleteButton);
    setHeight(deleteButton, ViewHeightDefault, true, false);
    addArrangedSubview(p->vStack, deleteButton);

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
    long plan = msg0(long, d->planPicker, sel_getUid("selectedSegmentIndex")) - 1;
    for (int i = 0; i < 4; ++i) {
        d->results[i] = (short)lrintf(p->children[i].data->result * toSavedMass);
    }
    addAlertAction(ctrl, CFSTR("save"), UIAlertActionStyleDefault, ^{
        updateUserInfo((unsigned char)plan, d->results);
    });
    showAlert(ctrl);
}
