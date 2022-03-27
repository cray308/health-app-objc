#include "StatusView.h"
#include "Views.h"

extern CFStringRef UIFontTextStyleHeadline;

Class StatusViewClass;
static SEL scrad, gct;
static void (*setCorner)(id,SEL,CGFloat);
static CFStringRef (*getTitle)(id,SEL);

void initStatVData(Class Button) {
    scrad = sel_getUid("setCornerRadius:");
    gct = sel_getUid("currentTitle");
    setCorner = (void(*)(id,SEL,CGFloat))getImpO(clsF0(Class, View, sel_getUid("layerClass")), scrad);
    getTitle = (CFStringRef(*)(id,SEL))getImpO(Button, gct);
}

id statusView_init(VCacheRef tbl, CCacheRef clr, CFStringRef text,
                   StatusView **ref, int tag, id target, SEL action) {
    id self = Sels.new(StatusViewClass, Sels.nw);
    StatusView *data = (StatusView *)((char *)self + ViewSize);
    ConstraintCache const *cc = &tbl->cc;
    tbl->view.setTag(self, tbl->view.stg, tag);
    data->button = createButton(tbl, clr, text, ColorLabel, UIFontTextStyleHeadline, tag, target, action);
    setCorner(tbl->view.layer(data->button, tbl->view.glyr), scrad, 5);
    tbl->view.setBG(data->button, tbl->view.sbg, clr->getColor(clr->cls, clr->sc, ColorSecondaryBGGrouped));
    id btnHeight = cc->init(cc->cls, cc->cr, data->button, 8, 1, nil, 0, 1, 50);
    cc->lowerPri(btnHeight, cc->lp, 999);
    cc->activateC(btnHeight, cc->ac, true);
    data->headerLabel = createLabel(tbl, clr, NULL, UIFontTextStyleSubheadline, 0);
    tbl->view.setIsAcc(data->headerLabel, tbl->view.sace, false);
    data->box = Sels.new(View, Sels.nw);
    setCorner(tbl->view.layer(data->box, tbl->view.glyr), scrad, 5);
    id boxWidth = cc->init(cc->cls, cc->cr, data->box, 7, 0, nil, 0, 1, 20);
    cc->lowerPri(boxWidth, cc->lp, 999);
    cc->activateC(boxWidth, cc->ac, true);
    id boxHeight = cc->init(cc->cls, cc->cr, data->box, 8, 0, nil, 0, 1, 20);
    cc->lowerPri(boxHeight, cc->lp, 999);
    cc->activateC(boxHeight, cc->ac, true);
    id hStack = createStackView(tbl, (id []){data->button, data->box}, 2, 0, 5, (Padding){0});
    id vStack = createStackView(tbl, (id []){data->headerLabel, hStack}, 2, 1, 4, (Padding){4, 0, 4, 0});
    msg1(void, bool, vStack, tbl->view.trans, false);
    tbl->view.addSub(self, tbl->view.asv, vStack);
    pin(cc, vStack, self);
    Sels.viewRel(hStack, Sels.rel);
    Sels.viewRel(vStack, Sels.rel);
    *ref = data;
    return self;
}

void statusView_deinit(id self, SEL _cmd) {
    StatusView *ptr = (StatusView *)((char *)self + ViewSize);
    Sels.viewRel(ptr->headerLabel, Sels.rel);
    Sels.viewRel(ptr->box, Sels.rel);
    msgSup0(void, (&(struct objc_super){self, View}), _cmd);
}

void statusView_updateAccessibility(StatusView *ptr, VCacheRef tbl) {
    CFStringRef header = tbl->label.getText(ptr->headerLabel, tbl->label.gtxt);
    CFMutableStringRef label = CFStringCreateMutable(NULL, 128);
    if (header)
        CFStringAppendFormat(label, NULL, CFSTR("%@. "), header);
    CFStringAppend(label, getTitle(ptr->button, gct));
    tbl->view.setAcc(ptr->button, tbl->view.sacl, label);
    CFRelease(label);
}
