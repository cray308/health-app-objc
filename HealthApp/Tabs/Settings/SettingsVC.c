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
    CFArrayRef views = tbl->stack.getSub(data->planContainer, tbl->stack.gsv);
    tbl->label.setColor((id)CFArrayGetValueAtIndex(views, 0), tbl->label.stc, label);
    updateSegmentedControl(clr, (id)CFArrayGetValueAtIndex(views, 1), darkMode);
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
         clsF1(id, int, clr->cls, sel_getUid("getBarColorWithType:"), ColorBarModal));
    msg1(void, id, sup->toolbar, sel_getUid("setTintColor:"), red);
    for (int i = 0; i < 4; ++i) {
        InputView *ptr = sup->children[i];
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
    ConstraintCache const *cc = &tbl->cc;
    SettingsVC *data = (SettingsVC *)((char *)sup + sizeof(InputVC));
    id viewBG = sup->clr->getColor(sup->clr->cls, sup->clr->sc, ColorSecondaryBGGrouped);
    tbl->view.setBG(msg0(id, self, sel_getUid("view")), tbl->view.sbg,
                    sup->clr->getColor(sup->clr->cls, sup->clr->sc, ColorPrimaryBGGrouped));
    setVCTitle(self, localize(bundle, CFSTR("settingsTitle")));

    UserInfo const *info = getUserInfo();
    const unsigned char darkMode = info->darkMode;
    unsigned char segment = info->currentPlan + 1;
    id planLabel = createLabel(tbl, sup->clr, localize(bundle, CFSTR("planPickerTitle")),
                               UIFontTextStyleFootnote, true);
    id picker = createSegmentedControl(bundle, CFSTR("settingsSegment%d"), segment);
    cc->activateC(cc->init(cc->cls, cc->cr, picker, 8, 0, nil, 0, 1, 44), cc->ac, true);
    data->planContainer = createStackView(tbl, (id []){planLabel, picker}, 2, 1, 0, 2, (Padding){0, 8, 0, 8});
    id aboveTF = createStackView(tbl, (id []){data->planContainer}, 1, 1, 0, 20, (Padding){0, 0, 20, 0});

    tbl->stack.setMargins(sup->vStack, tbl->stack.smr, (HAInsets){20, 0, 20, 0});
    tbl->stack.addSub(sup->vStack, tbl->stack.asv, aboveTF);

    if (darkMode < 2) {
        updateSegmentedControl(sup->clr, picker, darkMode);
        data->switchContainer = Sels.new(View, Sels.nw);
        tbl->view.setBG(data->switchContainer, tbl->view.sbg, viewBG);
        cc->activateC(cc->init(cc->cls, cc->cr, data->switchContainer, 8, 0, nil, 0, 1, 44), cc->ac, true);
        id switchView = Sels.new(objc_getClass("UISwitch"), Sels.nw);
        msg1(void, bool, switchView, sel_getUid("setOn:"), darkMode);
        id label = createLabel(tbl, sup->clr, localize(bundle, CFSTR("darkMode")),
                               UIFontTextStyleBody, true);
        id sv = createStackView(tbl, (id []){label, switchView}, 2, 0, 3, 5, (Padding){0, 8, 0, 8});
        tbl->view.setTrans(sv, tbl->view.trans, false);
        tbl->view.addSub(data->switchContainer, tbl->view.asv, sv);
        pin(cc, sv, data->switchContainer);
        tbl->stack.addSub(aboveTF, tbl->stack.asv, data->switchContainer);
        Sels.viewRel(sv, Sels.rel);
        Sels.viewRel(label, Sels.rel);
        Sels.viewRel(switchView, Sels.rel);
    }

    CFStringRef titles[4];
    fillStringArray(bundle, titles, CFSTR("maxWeight%d"), 4);
    for (int i = 0; i < 4; ++i) {
        inputVC_addChild(self, titles[i], 0, 999);
    }

    SEL btnTap = sel_getUid("buttonTapped:");
    sup->button = createButton(tbl, sup->clr, localize(bundle, CFSTR("settingsSave")),
                               ColorBlue, UIFontTextStyleBody, 0, self, btnTap);
    tbl->view.setBG(sup->button, tbl->view.sbg, viewBG);
    cc->activateC(cc->init(cc->cls, cc->cr, sup->button, 8, 0, nil, 0, 1, 44), cc->ac, true);

    data->deleteButton = createButton(tbl, sup->clr, localize(bundle, CFSTR("settingsDelete")),
                                      ColorRed, UIFontTextStyleBody, 1, self, btnTap);
    tbl->view.setBG(data->deleteButton, tbl->view.sbg, viewBG);
    cc->activateC(cc->init(cc->cls, cc->cr, data->deleteButton, 8, 0, nil, 0, 1, 44), cc->ac, true);

    id belowTF = createStackView(tbl, (id []){sup->button, data->deleteButton}, 2, 1, 0, 20,
                                 (Padding){.top = 20});
    tbl->stack.addSub(sup->vStack, tbl->stack.asv, belowTF);

    Sels.viewRel(planLabel, Sels.rel);
    Sels.viewRel(aboveTF, Sels.rel);
    Sels.viewRel(belowTF, Sels.rel);
    Sels.viewRel(picker, Sels.rel);

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
    id picker = (id)CFArrayGetValueAtIndex(tbl->stack.getSub(data->planContainer, tbl->stack.gsv), 1);
    unsigned char plan = (unsigned char)(msg0(long, picker, sel_getUid("selectedSegmentIndex")) - 1);

    short *arr = data->results;
    for (int i = 0; i < 4; ++i) {
        arr[i] = sup->children[i]->result;
    }
    addAlertAction(ctrl, localize(bundle, CFSTR("save")), 0, ^{ updateUserInfo(plan, dark, arr); });
    presentVC(ctrl);
}
