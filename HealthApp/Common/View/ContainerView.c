#include "ContainerView.h"
#include "Views.h"

extern CFStringRef UIFontTextStyleTitle3;
extern uint64_t UIAccessibilityTraitHeader;
extern uint64_t UIAccessibilityTraitStaticText;

Class ContainerViewClass;

id containerView_init(ContainerView **ref, CFStringRef header) {
    id self = new(ContainerViewClass);
    ContainerView *v = getIVV(ContainerView, self);
    *ref = v;

    v->divider = new(View);
    setBackgroundColor(v->divider, getColor(ColorDiv));
    setHeight(v->divider, 1, false, true);

    v->header = createLabel(header, UIFontTextStyleTitle3, ColorLabel);
    setAccessibilityTraits(v->header, UIAccessibilityTraitStaticText | UIAccessibilityTraitHeader);
    setID(v->header, CFSTR("containerHeader"))

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
    ContainerView *v = getIVV(ContainerView, self);
    releaseView(v->divider);
    releaseView(v->header);
    releaseView(v->stack);
    msgSupV(supSig(), self, View, _cmd);
}
