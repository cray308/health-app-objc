#include "ContainerView.h"
#include "Views.h"

extern CFStringRef UIFontTextStyleTitle3;
extern uint64_t UIAccessibilityTraitHeader;

Class ContainerViewClass;

id containerView_init(ContainerView **ref, CFStringRef header) {
    id self = new(ContainerViewClass);
    ContainerView *v = (ContainerView *)getIVV(self);
    *ref = v;

    v->divider = new(View);
    setBackgroundColor(v->divider, getColor(ColorDiv));
    setHeight(v->divider, 1, false, true);

    v->header = createLabel(header, UIFontTextStyleTitle3, ColorLabel);
    setAccessibilityTraits(v->header, UIAccessibilityTraitHeader);

    v->stack = createVStack(NULL, 0);
    setLayoutMargins(v->stack, ((HAInsets){.top = 4}));

    id vStack = createVStack((id []){v->divider, v->header, v->stack}, 3);
    setTrans(vStack);
    setCustomSpacing(vStack, GroupSpacing, v->divider);
    addSubview(self, vStack);
    pin(vStack, self);
    releaseV(vStack);
    return self;
}

void containerView_deinit(id self, SEL _cmd) {
    ContainerView *v = (ContainerView *)getIVV(self);
    releaseV(v->divider);
    releaseV(v->header);
    releaseV(v->stack);
    msgSup0(void, (&(struct objc_super){self, View}), _cmd);
}
