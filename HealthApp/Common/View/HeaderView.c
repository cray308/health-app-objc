#include "HeaderView.h"
#include "Views.h"

Class ReusableView;
Class HeaderViewClass;
size_t ReusableViewSize;

id headerView_initWithFrame(id self, SEL _cmd, CGRect frame) {
    self = msgSupV(supSig(id, CGRect), self, ReusableView, _cmd, frame);
    HeaderView *v = getIVR(self);

    id divContainer = new(View);
    setHeight(divContainer, GroupSpacing, false, false);

    v->divider = new(View);
    useConstraints(v->divider);
    setBackgroundColor(v->divider, getColor(ColorDiv));
    setHeight(v->divider, 1, false, false);
    addSubview(divContainer, v->divider);

    CFArrayRef constraints = CFArrayCreate(NULL, (const void *[]){
        makeConstraint(v->divider, LayoutAttributeTop, 0, divContainer, LayoutAttributeTop, 0),
        makeConstraint(v->divider, LayoutAttributeLeading, 0,
                       divContainer, LayoutAttributeLeading, 0),
        makeConstraint(v->divider, LayoutAttributeTrailing, 0,
                       divContainer, LayoutAttributeTrailing, 0)
    }, 3, NULL);
    activateConstraints(constraints);
    CFRelease(constraints);

    v->label = createLabel(NULL, UIFontTextStyleTitle3, ColorLabel);
    setAccessibilityTraits(v->label, UIAccessibilityTraitStaticText | UIAccessibilityTraitHeader);
    setContentHuggingPriority(v->label, 249, ConstraintAxisVertical);
    setContentCompressionResistancePriority(v->label, 745, ConstraintAxisVertical);

    id stack = createVStack((id []){divContainer, v->label}, 2);
    useStackConstraints(stack);
    setLayoutMargins(stack, (HAInsets){0, 8, 0, 8});

    id contentView = new(View);
    useConstraints(contentView);
    addSubview(self, contentView);
    pin(contentView, self);
    addSubview(contentView, stack);
    pin(stack, contentView);
    releaseView(divContainer);
    releaseView(stack);
    releaseView(contentView);
    return self;
}

void headerView_deinit(id self, SEL _cmd) {
    HeaderView *v = getIVR(self);
    releaseView(v->divider);
    releaseView(v->label);
    msgSupV(supSig(void), self, ReusableView, _cmd);
}
