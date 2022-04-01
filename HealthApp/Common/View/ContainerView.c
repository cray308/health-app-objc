#include "ContainerView.h"
#include "Views.h"

Class ContainerViewClass;
extern uint64_t UIAccessibilityTraitHeader;

id containerView_init(VCacheRef tbl, CCacheRef clr, CFStringRef title, ContainerView **ref) {
    id self = Sels.new(ContainerViewClass, Sels.nw);
    ContainerView *data = (ContainerView *)((char *)self + ViewSize);
    data->divider = Sels.new(View, Sels.nw);
    tbl->view.setBG(data->divider, tbl->view.sbg, clr->getColor(clr->cls, clr->sc, ColorSeparator));
    setHeight(&tbl->cc, data->divider, 1, false, true);
    data->headerLabel = createLabel(tbl, clr, title, UIFontTextStyleTitle3, ColorLabel);
    tbl->label.setLines(data->headerLabel, tbl->label.snl, 0);
    tbl->view.setTraits(data->headerLabel, tbl->view.satrs, UIAccessibilityTraitHeader);
    data->stack = createVStack(NULL, 0);
    tbl->stack.setMargins(data->stack, tbl->stack.smr, (HAInsets){.top = 4});

    id vStack = createVStack((id []){data->divider, data->headerLabel, data->stack}, 3);
    msg1(void, bool, vStack, tbl->view.trans, false);
    tbl->stack.setSpaceAfter(vStack, tbl->stack.scsp, 20, data->divider);
    tbl->view.addSub(self, tbl->view.asv, vStack);
    pin(&tbl->cc, vStack, self);
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
    tbl->view.setBG(data->divider, tbl->view.sbg, clr->getColor(clr->cls, clr->sc, ColorSeparator));
}
