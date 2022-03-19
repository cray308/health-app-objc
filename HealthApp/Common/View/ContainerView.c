#include "ContainerView.h"
#include "Views.h"

Class ContainerViewClass;

id containerView_init(VCacheRef tbl, CCacheRef clr, CFStringRef title, ContainerView **ref, int spacing) {
    id self = Sels.new(ContainerViewClass, Sels.nw);
    ContainerView *data = (ContainerView *)((char *)self + ViewSize);
    ConstraintCache const *cc = &tbl->cc;
    data->divider = Sels.new(View, Sels.nw);
    data->headerLabel = createLabel(tbl, clr, title, UIFontTextStyleTitle3, true);
    data->stack = createStackView(tbl, NULL, 0, 1, 0, spacing, (Padding){.top = 5});

    id containerHeight = cc->init(cc->cls, cc->cr, data->divider, 8, 0, nil, 0, 1, 21);
    cc->lowerPri(containerHeight, cc->lp, 999);
    cc->activateC(containerHeight, cc->ac, true);
    id divLine = Sels.new(View, Sels.nw);
    tbl->view.setTrans(divLine, tbl->view.trans, false);
    tbl->view.setBG(divLine, tbl->view.sbg, clr->getColor(clr->cls, clr->sc, ColorSeparator));
    id lineHeight = cc->init(cc->cls, cc->cr, divLine, 8, 0, nil, 0, 1, 1);
    cc->lowerPri(lineHeight, cc->lp, 999);
    cc->activateC(lineHeight, cc->ac, true);
    tbl->view.addSub(data->divider, tbl->view.asv, divLine);
    const void *constraints[] = {
        cc->init(cc->cls, cc->cr, divLine, 3, 0, data->divider, 3, 1, 0),
        cc->init(cc->cls, cc->cr, divLine, 5, 0, data->divider, 5, 1, 0),
        cc->init(cc->cls, cc->cr, divLine, 6, 0, data->divider, 6, 1, 0)
    };
    CFArrayRef divArray = CFArrayCreate(NULL, constraints, 3, NULL);
    cc->activateArr(cc->cls, cc->aar, divArray);
    CFRelease(divArray);
    Sels.viewRel(divLine, Sels.rel);

    id vStack = createStackView(tbl, (id []){data->divider, data->headerLabel, data->stack},
                                3, 1, 0, 0, (Padding){0});
    tbl->view.setTrans(vStack, tbl->view.trans, false);
    tbl->view.addSub(self, tbl->view.asv, vStack);
    pin(cc, vStack, self);
    Sels.viewRel(vStack, Sels.rel);
    *ref = data;
    return self;
}

void containerView_deinit(id self, SEL _cmd) {
    ContainerView *ptr = (ContainerView *)((char *)self + ViewSize);
    Sels.viewRel(ptr->divider, Sels.rel);
    Sels.viewRel(ptr->headerLabel, Sels.rel);
    Sels.viewRel(ptr->stack, Sels.rel);
    msgSup0(void, (&(struct objc_super){self, View}), _cmd);
}

void containerView_updateColors(ContainerView *data, VCacheRef tbl, CCacheRef clr) {
    tbl->label.setColor(data->headerLabel, tbl->label.stc, clr->getColor(clr->cls, clr->sc, ColorLabel));
    id divLine = (id)CFArrayGetValueAtIndex(msg0(CFArrayRef, data->divider, sel_getUid("subviews")), 0);
    tbl->view.setBG(divLine, tbl->view.sbg, clr->getColor(clr->cls, clr->sc, ColorSeparator));
}
