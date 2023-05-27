#include "ContainerView.h"
#include "Views.h"

extern uint64_t UIAccessibilityTraitHeader;

Class ContainerViewClass;

id containerView_init(ContainerView **ref, CFStringRef header) {
    id self = new(ContainerViewClass);
    ContainerView *v = (ContainerView *)((char *)self + ViewSize);
    *ref = v;

    v->divider = new(View);
    setBackgroundColor(v->divider, getColor(ColorDiv));
    setHeight(v->divider, 1, false, true);

    v->header = createLabel(header, UIFontTextStyleTitle3, ColorLabel);
    setAccessibilityTraits(v->header, UIAccessibilityTraitHeader);

    v->stack = createVStack(NULL, 0);
    setLayoutMargins(v->stack, (HAInsets){.top = 4});

    id vStack = createVStack((id []){v->divider, v->header, v->stack}, 3);
    useStackConstraints(vStack);
    setCustomSpacing(vStack, GroupSpacing, v->divider);
    addSubview(self, vStack);
    pin(vStack, self);
    releaseView(vStack);
    return self;
}

void containerView_deinit(id self, SEL _cmd) {
    ContainerView *v = (ContainerView *)((char *)self + ViewSize);
    releaseView(v->divider);
    releaseView(v->header);
    releaseView(v->stack);
    msgSupV(supSig(), self, View, _cmd);
}

void containerView_updateColors(ContainerView *v) {
    setTextColor(v->header, getColor(ColorLabel));
    setBackgroundColor(v->divider, getColor(ColorDiv));
}
