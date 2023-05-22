#include "ContainerView.h"
#include "Views.h"

extern uint64_t UIAccessibilityTraitHeader;

Class ContainerViewClass;

id containerView_init(VCacheRef tbl, CCacheRef clr, ContainerView **ref, CFStringRef header) {
    id self = Sels.new(ContainerViewClass, Sels.nw);
    ContainerView *v = (ContainerView *)((char *)self + ViewSize);
    *ref = v;
    v->divider = Sels.new(View, Sels.nw);
    tbl->view.setBG(v->divider, tbl->view.sbg, clr->getColor(clr->cls, clr->sc, ColorDiv));
    setHeight(&tbl->cc, v->divider, 1, false, true);
    v->header = createLabel(tbl, clr, header, UIFontTextStyleTitle3, ColorLabel);
    tbl->label.setLines(v->header, tbl->label.snl, 0);
    tbl->view.setTraits(v->header, tbl->view.satrs, UIAccessibilityTraitHeader);
    v->stack = createVStack(NULL, 0);
    tbl->stack.setMargins(v->stack, tbl->stack.smr, (HAInsets){.top = 4});

    id vStack = createVStack((id []){v->divider, v->header, v->stack}, 3);
    msg1(void, bool, vStack, tbl->view.trans, false);
    tbl->stack.setSpaceAfter(vStack, tbl->stack.scsp, 20, v->divider);
    tbl->view.addSub(self, tbl->view.asv, vStack);
    pin(&tbl->cc, vStack, self);
    Sels.viewRel(vStack, Sels.rel);
    return self;
}

void containerView_deinit(id self, SEL _cmd) {
    ContainerView *v = (ContainerView *)((char *)self + ViewSize);
    Sels.viewRel(v->divider, Sels.rel);
    Sels.viewRel(v->header, Sels.rel);
    Sels.viewRel(v->stack, Sels.rel);
    msgSup0(void, (&(struct objc_super){self, View}), _cmd);
}

void containerView_updateColors(ContainerView *v, VCacheRef tbl, CCacheRef clr) {
    tbl->label.setColor(v->header, tbl->label.stc, clr->getColor(clr->cls, clr->sc, ColorLabel));
    tbl->view.setBG(v->divider, tbl->view.sbg, clr->getColor(clr->cls, clr->sc, ColorDiv));
}
