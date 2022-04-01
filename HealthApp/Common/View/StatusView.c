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
    data->button = createButton(tbl, clr, text, ColorLabel, UIFontTextStyleHeadline, target, action);
    tbl->view.setTag(data->button, tbl->view.stg, tag);
    setCorner(tbl->view.layer(data->button, tbl->view.glyr), scrad, 5);
    tbl->view.setBG(data->button, tbl->view.sbg, clr->getColor(clr->cls, clr->sc, ColorSecondaryBGGrouped));
    setHeight(&tbl->cc, data->button, 50, true, true);
    data->headerLabel = createLabel(tbl, clr, NULL, UIFontTextStyleSubheadline, ColorLabel);
    tbl->label.setLines(data->headerLabel, tbl->label.snl, 0);
    tbl->view.setIsAcc(data->headerLabel, tbl->view.sace, false);
    data->box = Sels.new(View, Sels.nw);
    setCorner(tbl->view.layer(data->box, tbl->view.glyr), scrad, 5);
    id boxWidth = tbl->cc.init(tbl->cc.cls, tbl->cc.cr, data->box, 7, 0, nil, 0, 1, 20);
    tbl->cc.lowerPri(boxWidth, tbl->cc.lp, 999);
    tbl->cc.activateC(boxWidth, tbl->cc.ac, true);
    setHeight(&tbl->cc, data->box, 20, false, true);
    id hStack = createHStack(tbl, (id []){data->button, data->box});
    id vStack = createVStack((id []){data->headerLabel, hStack}, 2);
    tbl->stack.setMargins(vStack, tbl->stack.smr, (HAInsets){4, 0, 4, 0});
    tbl->stack.setSpace(vStack, tbl->stack.ssp, 4);
    msg1(void, bool, vStack, tbl->view.trans, false);
    tbl->view.addSub(self, tbl->view.asv, vStack);
    pin(&tbl->cc, vStack, self);
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
