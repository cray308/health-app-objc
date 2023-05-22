#include "SettingsVC.h"
#include "AppDelegate.h"
#include "InputVC.h"
#include "Views.h"

void updateUserInfo(unsigned char plan, unsigned char darkMode, short *weights);
void deleteAppData(void);

Class SettingsVCClass;

id settingsVC_init(VCacheRef tbl, CCacheRef clr) {
    return msg2(id, VCacheRef, CCacheRef, Sels.alloc(SettingsVCClass, Sels.alo),
                sel_getUid("initWithVCache:cCache:"), tbl, clr);
}

void settingsVC_updateColors(id self, unsigned char darkMode) {
    InputVC *sup = (InputVC *)((char *)self + VCSize);
    SettingsVC *d = (SettingsVC *)((char *)sup + sizeof(InputVC));
    VCacheRef tbl = sup->tbl;
    CCacheRef clr = sup->clr;
    tbl->view.setBG(msg0(id, self, sel_getUid("view")), tbl->view.sbg,
                    clr->getColor(clr->cls, clr->sc, ColorPrimaryBGGrouped));
    id red = clr->getColor(clr->cls, clr->sc, ColorRed);
    id label = clr->getColor(clr->cls, clr->sc, ColorLabel);
    id btnBg = clr->getColor(clr->cls, clr->sc, ColorSecondaryBGGrouped);
    id fieldBg = clr->getColor(clr->cls, clr->sc, ColorTertiaryBG);
    tbl->label.setColor(d->planLabel, tbl->label.stc, label);
    updateSegmentedControl(clr, d->planPicker, darkMode);
    tbl->view.setBG(d->switchContainer, tbl->view.sbg, btnBg);
    id view = (id)CFArrayGetValueAtIndex(
      msg0(CFArrayRef, d->switchContainer, sel_getUid("subviews")), 0);
    tbl->label.setColor((id)CFArrayGetValueAtIndex(tbl->stack.getSub(view, tbl->stack.gsv), 0),
                        tbl->label.stc, label);
    tbl->button.setColor(d->deleteButton, tbl->button.sbc, red, 0);
    tbl->view.setBG(d->deleteButton, tbl->view.sbg, btnBg);
    tbl->button.setColor(sup->button, tbl->button.sbc,
                         clr->getColor(clr->cls, clr->sc, ColorBlue), 0);
    tbl->button.setColor(sup->button, tbl->button.sbc,
                         clr->getColor(clr->cls, clr->sc, ColorDisabled), 2);
    tbl->view.setBG(sup->button, tbl->view.sbg, btnBg);
    msg1(void, id, sup->toolbar, sel_getUid("setBarTintColor:"),
         clsF1(id, int, clr->cls, sel_getUid("getBarColorWithType:"), 1));
    msg1(void, id, sup->toolbar, sel_getUid("setTintColor:"), red);
    for (int i = 0; i < 4; ++i) {
        InputView *v = sup->children[i].data;
        tbl->label.setColor(v->errorLabel, tbl->label.stc, red);
        tbl->label.setColor(v->hintLabel, tbl->label.stc, label);
        msg1(void, id, v->field, tbl->label.stc, label);
        msg1(void, id, v->field, tbl->view.sbg, fieldBg);
        msg1(void, long, v->field, sel_getUid("setKeyboardAppearance:"), darkMode);
    }
}

void settingsVC_viewDidLoad(id self, SEL _cmd) {
    msgSup0(void, (&(struct objc_super){self, InputVCClass}), _cmd);

    InputVC *sup = (InputVC *)((char *)self + VCSize);
    VCacheRef tbl = sup->tbl;
    SettingsVC *d = (SettingsVC *)((char *)sup + sizeof(InputVC));
    id viewBG = sup->clr->getColor(sup->clr->cls, sup->clr->sc, ColorSecondaryBGGrouped);
    tbl->view.setBG(msg0(id, self, sel_getUid("view")), tbl->view.sbg,
                    sup->clr->getColor(sup->clr->cls, sup->clr->sc, ColorPrimaryBGGrouped));
    setVCTitle(msg0(id, self, sel_getUid("navigationItem")), localize(CFSTR("settingsTitle")));

    UserInfo const *info = getUserInfo();
    const unsigned char darkMode = info->darkMode;
    unsigned char segment = info->currentPlan + 1;
    d->planLabel = createLabel(tbl, sup->clr, localize(CFSTR("planPickerTitle")),
                               UIFontTextStyleSubheadline, ColorLabel);
    tbl->label.setLines(d->planLabel, tbl->label.snl, 0);
    tbl->stack.addSub(sup->vStack, tbl->stack.asv, d->planLabel);
    tbl->stack.setSpaceAfter(sup->vStack, tbl->stack.scsp, 4, d->planLabel);
    d->planPicker = createSegmentedControl(CFSTR("settingsSegment%d"), segment);
    setHeight(&tbl->cc, d->planPicker, 44, true, false);
    tbl->stack.addSub(sup->vStack, tbl->stack.asv, d->planPicker);
    tbl->stack.setSpaceAfter(sup->vStack, tbl->stack.scsp, 20, d->planPicker);

    SEL setCorner = sel_getUid("setCornerRadius:");
    if (darkMode < 2) {
        updateSegmentedControl(sup->clr, d->planPicker, darkMode);
        d->switchContainer = Sels.new(View, Sels.nw);
        tbl->view.setBG(d->switchContainer, tbl->view.sbg, viewBG);
        msg1(void, CGFloat, tbl->view.layer(d->switchContainer, tbl->view.glyr), setCorner, 5);
        setHeight(&tbl->cc, d->switchContainer, 44, true, false);
        id switchView = Sels.new(objc_getClass("UISwitch"), Sels.nw);
        msg1(void, bool, switchView, sel_getUid("setOn:"), darkMode);
        id label = createLabel(tbl, sup->clr, localize(CFSTR("darkMode")),
                               UIFontTextStyleBody, ColorLabel);
        id sv = createHStack(tbl, (id []){label, switchView});
        msg1(void, bool, sv, tbl->view.trans, false);
        tbl->stack.setMargins(sv, tbl->stack.smr, (HAInsets){0, 8, 0, 8});
        tbl->view.addSub(d->switchContainer, tbl->view.asv, sv);
        pin(&tbl->cc, sv, d->switchContainer);
        tbl->stack.addSub(sup->vStack, tbl->stack.asv, d->switchContainer);
        tbl->stack.setSpaceAfter(sup->vStack, tbl->stack.scsp, 20, d->switchContainer);
        Sels.viewRel(sv, Sels.rel);
        Sels.viewRel(label, Sels.rel);
        Sels.viewRel(switchView, Sels.rel);
    }

    CFStringRef liftNames[4];
    fillStringArray(liftNames, CFSTR("exNames%02d"), 4);
    CFLocaleRef l = CFLocaleCopyCurrent();
    CFStringRef fieldKey = localize(CFSTR("maxWeight"));
    int kb = massType ? 8 : 4;
    for (int i = 0; i < 4; ++i) {
        CFMutableStringRef adjName = CFStringCreateMutableCopy(NULL, 128, liftNames[i]);
        CFRelease(liftNames[i]);
        CFStringLowercase(adjName, l);
        inputVC_addChild(self, formatStr(NULL, fieldKey, adjName), kb, 0, 999);
        CFRelease(adjName);
    }
    CFRelease(fieldKey);
    CFRelease(l);
    tbl->stack.setSpaceAfter(sup->vStack, tbl->stack.scsp, 20, sup->children[3].view);

    SEL btnTap = sel_getUid("buttonTapped:");
    sup->button = createButton(tbl, sup->clr, localize(CFSTR("settingsSave")),
                               ColorBlue, UIFontTextStyleBody, self, btnTap);
    tbl->view.setBG(sup->button, tbl->view.sbg, viewBG);
    msg1(void, CGFloat, tbl->view.layer(sup->button, tbl->view.glyr), setCorner, 5);
    setHeight(&tbl->cc, sup->button, 44, true, false);
    tbl->stack.addSub(sup->vStack, tbl->stack.asv, sup->button);
    tbl->stack.setSpaceAfter(sup->vStack, tbl->stack.scsp, 20, sup->button);

    d->deleteButton = createButton(tbl, sup->clr, localize(CFSTR("settingsDelete")),
                                   ColorRed, UIFontTextStyleBody, self, btnTap);
    tbl->view.setTag(d->deleteButton, tbl->view.stg, 1);
    tbl->view.setBG(d->deleteButton, tbl->view.sbg, viewBG);
    msg1(void, CGFloat, tbl->view.layer(d->deleteButton, tbl->view.glyr), setCorner, 5);
    setHeight(&tbl->cc, d->deleteButton, 44, true, false);
    tbl->stack.addSub(sup->vStack, tbl->stack.asv, d->deleteButton);

    inputVC_updateFields(sup, info->liftMaxes);
}

void settingsVC_buttonTapped(id self, SEL _cmd _U_, id btn) {
    InputVC *sup = (InputVC *)((char *)self + VCSize);
    SettingsVC *d = (SettingsVC *)((char *)sup + sizeof(InputVC));
    VCacheRef tbl = sup->tbl;
    int tag = (int)tbl->view.getTag(btn, tbl->view.gtg);
    id ctrl = createAlertController(
      CFSTR("settingsAlertTitle"), tag ? CFSTR("alertMsgDelete") : CFSTR("alertMsgSave"));
    addAlertAction(ctrl, CFSTR("cancel"), 1, NULL);
    if (tag) {
        addAlertAction(ctrl, CFSTR("delete"), 2, ^{ deleteAppData(); });
        presentVC(ctrl);
        return;
    }

    unsigned char dark = 0xff;
    if (d->switchContainer) {
        id sv = (id)CFArrayGetValueAtIndex(
          msg0(CFArrayRef, d->switchContainer, sel_getUid("subviews")), 0);
        id switchView = (id)CFArrayGetValueAtIndex(tbl->stack.getSub(sv, tbl->stack.gsv), 1);
        dark = msg0(bool, switchView, sel_getUid("isOn")) ? 1 : 0;
    }
    unsigned char plan =
      (unsigned char)(msg0(long, d->planPicker, sel_getUid("selectedSegmentIndex")) - 1);

    short *arr = d->results;
    for (int i = 0; i < 4; ++i) {
        arr[i] = (short)lrintf(sup->children[i].data->result * toSavedMass);
    }
    addAlertAction(ctrl, CFSTR("save"), 0, ^{ updateUserInfo(plan, dark, arr); });
    presentVC(ctrl);
}
