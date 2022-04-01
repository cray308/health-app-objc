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
    SettingsVC *data = (SettingsVC *)((char *)sup + sizeof(InputVC));
    VCacheRef tbl = sup->tbl;
    CCacheRef clr = sup->clr;
    tbl->view.setBG(msg0(id, self, sel_getUid("view")), tbl->view.sbg,
                    clr->getColor(clr->cls, clr->sc, ColorPrimaryBGGrouped));
    id red = clr->getColor(clr->cls, clr->sc, ColorRed), label = clr->getColor(clr->cls, clr->sc, ColorLabel);
    id btnBg = clr->getColor(clr->cls, clr->sc, ColorSecondaryBGGrouped);
    id fieldBg = clr->getColor(clr->cls, clr->sc, ColorTertiaryBG);
    tbl->label.setColor(data->planLabel, tbl->label.stc, label);
    updateSegmentedControl(clr, data->planPicker, darkMode);
    tbl->view.setBG(data->switchContainer, tbl->view.sbg, btnBg);
    id view = (id)CFArrayGetValueAtIndex(msg0(CFArrayRef, data->switchContainer, sel_getUid("subviews")), 0);
    tbl->label.setColor((id)CFArrayGetValueAtIndex(tbl->stack.getSub(view, tbl->stack.gsv), 0),
                        tbl->label.stc, label);
    tbl->button.setColor(data->deleteButton, tbl->button.sbc, red, 0);
    tbl->view.setBG(data->deleteButton, tbl->view.sbg, btnBg);
    tbl->button.setColor(sup->button, tbl->button.sbc, clr->getColor(clr->cls, clr->sc, ColorBlue), 0);
    tbl->button.setColor(sup->button, tbl->button.sbc,
                         clr->getColor(clr->cls, clr->sc, ColorSecondaryLabel), 2);
    tbl->view.setBG(sup->button, tbl->view.sbg, btnBg);
    msg1(void, id, sup->toolbar, sel_getUid("setBarTintColor:"),
         clsF1(id, int, clr->cls, sel_getUid("getBarColorWithType:"), 1));
    msg1(void, id, sup->toolbar, sel_getUid("setTintColor:"), red);
    for (int i = 0; i < 4; ++i) {
        InputView *ptr = sup->children[i].data;
        tbl->label.setColor(ptr->errorLabel, tbl->label.stc, red);
        tbl->label.setColor(ptr->hintLabel, tbl->label.stc, label);
        tbl->field.setColor(ptr->field, tbl->label.stc, label);
        tbl->view.setBG(ptr->field, tbl->view.sbg, fieldBg);
        msg1(void, long, ptr->field, sel_getUid("setKeyboardAppearance:"), darkMode);
    }
}

void settingsVC_viewDidLoad(id self, SEL _cmd) {
    msgSup0(void, (&(struct objc_super){self, InputVCClass}), _cmd);

    CFBundleRef bundle = CFBundleGetMainBundle();
    InputVC *sup = (InputVC *)((char *)self + VCSize);
    VCacheRef tbl = sup->tbl;
    SettingsVC *data = (SettingsVC *)((char *)sup + sizeof(InputVC));
    id viewBG = sup->clr->getColor(sup->clr->cls, sup->clr->sc, ColorSecondaryBGGrouped);
    tbl->view.setBG(msg0(id, self, sel_getUid("view")), tbl->view.sbg,
                    sup->clr->getColor(sup->clr->cls, sup->clr->sc, ColorPrimaryBGGrouped));
    setVCTitle(msg0(id, self, sel_getUid("navigationItem")), localize(bundle, CFSTR("settingsTitle")));

    UserInfo const *info = getUserInfo();
    const unsigned char darkMode = info->darkMode;
    unsigned char segment = info->currentPlan + 1;
    data->planLabel = createLabel(tbl, sup->clr, localize(bundle, CFSTR("planPickerTitle")),
                                  UIFontTextStyleFootnote, ColorLabel);
    tbl->label.setLines(data->planLabel, tbl->label.snl, 0);
    tbl->stack.addSub(sup->vStack, tbl->stack.asv, data->planLabel);
    tbl->stack.setSpaceAfter(sup->vStack, tbl->stack.scsp, 4, data->planLabel);
    data->planPicker = createSegmentedControl(bundle, CFSTR("settingsSegment%d"), segment);
    setHeight(&tbl->cc, data->planPicker, 44, true, false);
    tbl->stack.addSub(sup->vStack, tbl->stack.asv, data->planPicker);
    tbl->stack.setSpaceAfter(sup->vStack, tbl->stack.scsp, 20, data->planPicker);

    if (darkMode < 2) {
        updateSegmentedControl(sup->clr, data->planPicker, darkMode);
        data->switchContainer = Sels.new(View, Sels.nw);
        tbl->view.setBG(data->switchContainer, tbl->view.sbg, viewBG);
        msg1(void, CGFloat, tbl->view.layer(data->switchContainer, tbl->view.glyr),
             sel_getUid("setCornerRadius:"), 5);
        setHeight(&tbl->cc, data->switchContainer, 44, true, false);
        id switchView = Sels.new(objc_getClass("UISwitch"), Sels.nw);
        msg1(void, bool, switchView, sel_getUid("setOn:"), darkMode);
        id label = createLabel(tbl, sup->clr, localize(bundle, CFSTR("darkMode")),
                               UIFontTextStyleBody, ColorLabel);
        id sv = createHStack(tbl, (id []){label, switchView});
        msg1(void, bool, sv, tbl->view.trans, false);
        tbl->stack.setMargins(sv, tbl->stack.smr, (HAInsets){0, 8, 0, 8});
        tbl->view.addSub(data->switchContainer, tbl->view.asv, sv);
        pin(&tbl->cc, sv, data->switchContainer);
        tbl->stack.addSub(sup->vStack, tbl->stack.asv, data->switchContainer);
        tbl->stack.setSpaceAfter(sup->vStack, tbl->stack.scsp, 20, data->switchContainer);
        Sels.viewRel(sv, Sels.rel);
        Sels.viewRel(label, Sels.rel);
        Sels.viewRel(switchView, Sels.rel);
    }

    CFStringRef liftNames[4];
    fillStringArray(bundle, liftNames, CFSTR("liftTypes%d"), 4);
    CFStringRef fieldKey = localize(bundle, CFSTR("maxWeight"));
    for (int i = 0; i < 4; ++i) {
        inputVC_addChild(self, formatStr(fieldKey, liftNames[i]), 0, 999);
        CFRelease(liftNames[i]);
    }
    CFRelease(fieldKey);
    tbl->stack.setSpaceAfter(sup->vStack, tbl->stack.scsp, 20, sup->children[3].view);

    SEL btnTap = sel_getUid("buttonTapped:");
    sup->button = createButton(tbl, sup->clr, localize(bundle, CFSTR("settingsSave")),
                               ColorBlue, UIFontTextStyleBody, self, btnTap);
    tbl->view.setBG(sup->button, tbl->view.sbg, viewBG);
    setHeight(&tbl->cc, sup->button, 44, true, false);
    tbl->stack.addSub(sup->vStack, tbl->stack.asv, sup->button);
    tbl->stack.setSpaceAfter(sup->vStack, tbl->stack.scsp, 20, sup->button);

    data->deleteButton = createButton(tbl, sup->clr, localize(bundle, CFSTR("settingsDelete")),
                                      ColorRed, UIFontTextStyleBody, self, btnTap);
    tbl->view.setTag(data->deleteButton, tbl->view.stg, 1);
    tbl->view.setBG(data->deleteButton, tbl->view.sbg, viewBG);
    setHeight(&tbl->cc, data->deleteButton, 44, true, false);
    tbl->stack.addSub(sup->vStack, tbl->stack.asv, data->deleteButton);

    inputVC_updateFields(sup, info->liftMaxes);
}

void settingsVC_buttonTapped(id self, SEL _cmd _U_, id btn) {
    InputVC *sup = (InputVC *)((char *)self + VCSize);
    SettingsVC *data = (SettingsVC *)((char *)sup + sizeof(InputVC));
    VCacheRef tbl = sup->tbl;
    CFBundleRef bundle = CFBundleGetMainBundle();
    int tag = (int)tbl->view.getTag(btn, tbl->view.gtg);
    CFStringRef msgKey = tag ? CFSTR("alertMsgDelete") : CFSTR("alertMsgSave");
    id ctrl = createAlertController(localize(bundle, CFSTR("settingsAlertTitle")), localize(bundle, msgKey));
    addAlertAction(ctrl, localize(bundle, CFSTR("cancel")), 1, NULL);
    if (tag) {
        addAlertAction(ctrl, localize(bundle, CFSTR("delete")), 2, ^{ deleteAppData(); });
        presentVC(ctrl);
        return;
    }

    unsigned char dark = 0xff;
    if (data->switchContainer) {
        id sv = (id)CFArrayGetValueAtIndex(
          msg0(CFArrayRef, data->switchContainer, sel_getUid("subviews")), 0);
        id switchView = (id)CFArrayGetValueAtIndex(tbl->stack.getSub(sv, tbl->stack.gsv), 1);
        dark = msg0(bool, switchView, sel_getUid("isOn")) ? 1 : 0;
    }
    unsigned char plan =
      (unsigned char)(msg0(long, data->planPicker, sel_getUid("selectedSegmentIndex")) - 1);

    short *arr = data->results;
    for (int i = 0; i < 4; ++i) {
        arr[i] = sup->children[i].data->result;
    }
    addAlertAction(ctrl, localize(bundle, CFSTR("save")), 0, ^{ updateUserInfo(plan, dark, arr); });
    presentVC(ctrl);
}
