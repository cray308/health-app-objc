#include "StatusView.h"
#include "Views.h"

Class StatusViewClass;

static CFStringRef separatorFormat;

void initStatusViewData(void) { separatorFormat = localize(CFSTR("separator")); }

id statusView_init(StatusView **ref,
                   CFStringRef header, CFStringRef title, int tag, id target, SEL action) {
    id self = new(StatusViewClass);
    StatusView *v = getIVV(StatusView, self);
    *ref = v;

    v->button = createButton(title, ColorLabel, target, action);
    setTag(v->button, tag);
    addCornerRadius(v->button);
    setBackgroundColor(v->button, getColor(ColorSecondaryBGGrouped));
    setHeight(v->button, 50, true, true);

    v->header = createLabel(header, UIFontTextStyleSubheadline, ColorLabel);
    setIsAccessibilityElement(v->header, false);

    v->box = new(View);
    addCornerRadius(v->box);
    id boxWidthConstraint = makeConstraint(v->box, LayoutAttributeWidth, 0, nil, 0, 20);
    lowerPriority(boxWidthConstraint);
    setActive(boxWidthConstraint);
    setHeight(v->box, 20, false, true);

    id hStack = createHStack((id []){v->button, v->box});
    id vStack = createVStack((id []){v->header, hStack}, 2);
    useStackConstraints(vStack);
    setLayoutMargins(vStack, ViewMargins);
    setSpacing(vStack, ViewSpacing);
    addSubview(self, vStack);
    pin(vStack, self);
    releaseView(hStack);
    releaseView(vStack);
    return self;
}

void statusView_deinit(id self, SEL _cmd) {
    StatusView *v = getIVV(StatusView, self);
    releaseView(v->header);
    releaseView(v->box);
    msgSupV(supSig(), self, View, _cmd);
}

void statusView_updateAccessibility(StatusView *v) {
    CFStringRef header = getText(v->header);
    CFStringRef title = getCurrentTitle(v->button);
    if (!header) {
        setAccessibilityLabel(v->button, title);
        return;
    }
    CFStringRef label = formatStr(NULL, separatorFormat, header, title);
    setAccessibilityLabel(v->button, label);
    CFRelease(label);
}
