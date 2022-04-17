#include "StatusView.h"
#include "Views.h"

extern CFStringRef UIFontTextStyleHeadline;

Class StatusViewClass;
static SEL scrad, gct;
static void (*setCorner)(id,SEL,CGFloat);
static CFStringRef (*getTitle)(id,SEL);
static CFStringRef sepFmt;

void initStatVData(Class Button) {
    scrad = sel_getUid("setCornerRadius:");
    gct = sel_getUid("currentTitle");
    setCorner = (void(*)(id,SEL,CGFloat))getImpO(clsF0(Class, View, sel_getUid("layerClass")), scrad);
    getTitle = (CFStringRef(*)(id,SEL))getImpO(Button, gct);
    sepFmt = localize(CFSTR("sep"));
}

id statusView_init(VCacheRef tbl, CCacheRef clr, StatusView **ref, int tag, id target, SEL action) {
    id self = Sels.new(StatusViewClass, Sels.nw);
    StatusView *v = (StatusView *)((char *)self + ViewSize);
    *ref = v;
    v->button = createButton(tbl, clr, NULL, ColorLabel, UIFontTextStyleHeadline, target, action);
    tbl->view.setTag(v->button, tbl->view.stg, tag);
    setCorner(tbl->view.layer(v->button, tbl->view.glyr), scrad, 5);
    tbl->view.setBG(v->button, tbl->view.sbg,
                    clr->getColor(clr->cls, clr->sc, ColorSecondaryBGGrouped));
    setHeight(&tbl->cc, v->button, 50, true, true);
    v->headerLabel = createLabel(tbl, clr, NULL, UIFontTextStyleSubheadline, ColorLabel);
    tbl->label.setLines(v->headerLabel, tbl->label.snl, 0);
    tbl->view.setIsAcc(v->headerLabel, tbl->view.sace, false);
    v->box = Sels.new(View, Sels.nw);
    setCorner(tbl->view.layer(v->box, tbl->view.glyr), scrad, 5);
    id boxWidth = tbl->cc.init(tbl->cc.cls, tbl->cc.cr, v->box, 7, 0, nil, 0, 1, 20);
    tbl->cc.lowerPri(boxWidth, tbl->cc.lp, 999);
    tbl->cc.activateC(boxWidth, tbl->cc.ac, true);
    setHeight(&tbl->cc, v->box, 20, false, true);
    id hStack = createHStack(tbl, (id []){v->button, v->box});
    id vStack = createVStack((id []){v->headerLabel, hStack}, 2);
    tbl->stack.setMargins(vStack, tbl->stack.smr, (HAInsets){4, 0, 4, 0});
    tbl->stack.setSpace(vStack, tbl->stack.ssp, 4);
    msg1(void, bool, vStack, tbl->view.trans, false);
    tbl->view.addSub(self, tbl->view.asv, vStack);
    pin(&tbl->cc, vStack, self);
    Sels.viewRel(hStack, Sels.rel);
    Sels.viewRel(vStack, Sels.rel);
    return self;
}

void statusView_deinit(id self, SEL _cmd) {
    StatusView *v = (StatusView *)((char *)self + ViewSize);
    Sels.viewRel(v->headerLabel, Sels.rel);
    Sels.viewRel(v->box, Sels.rel);
    msgSup0(void, (&(struct objc_super){self, View}), _cmd);
}

void statusView_updateAccessibility(StatusView *v, VCacheRef tbl) {
    CFStringRef header = tbl->label.getText(v->headerLabel, tbl->label.gtxt);
    CFStringRef title = getTitle(v->button, gct);
    if (!header) {
        tbl->view.setAcc(v->button, tbl->view.sacl, title);
        return;
    }
    CFStringRef label = formatStr(NULL, sepFmt, header, title);
    tbl->view.setAcc(v->button, tbl->view.sacl, label);
    CFRelease(label);
}
