#include "StatusView.h"
#include "Views.h"

extern CFStringRef UIFontTextStyleHeadline;

Class StatusViewClass;

static CFStringRef sepFmt;

void initStatusViewData(void) { sepFmt = localize(CFSTR("sep")); }

id statusView_init(StatusView **ref,
                   CFStringRef header, CFStringRef title, int tag, id target, SEL action) {
    id self = new(StatusViewClass);
    StatusView *v = (StatusView *)getIVV(self);
    *ref = v;

    v->button = createButton(title, ColorLabel, UIFontTextStyleHeadline, target, action);
    setTag(v->button, tag);
    setCornerRadius(v->button);
    setBackgroundColor(v->button, getColor(ColorSecondaryBGGrouped));
    setHeight(v->button, 50, true, true);

    if (header) CFRetain(header);
    v->header = createLabel(header, UIFontTextStyleSubheadline, ColorLabel);
    setIsAccessibilityElement(v->header, false);

    v->box = new(View);
    setCornerRadius(v->box);
    id boxWidth = makeConstraint(v->box, NSLayoutAttributeWidth, 0, nil, 0, 20);
    lowerPriority(boxWidth);
    setActive(boxWidth);
    setHeight(v->box, 20, false, true);

    id hStack = createHStack((id []){v->button, v->box});
    id vStack = createVStack((id []){v->header, hStack}, 2);
    setTrans(vStack);
    setLayoutMargins(vStack, ViewMargins);
    setSpacing(vStack, ViewSpacing);
    addSubview(self, vStack);
    pin(vStack, self);
    releaseV(hStack);
    releaseV(vStack);
    return self;
}

void statusView_deinit(id self, SEL _cmd) {
    StatusView *v = (StatusView *)getIVV(self);
    releaseV(v->header);
    releaseV(v->box);
    msgSup0(void, (&(struct objc_super){self, View}), _cmd);
}

void statusView_updateAccessibility(StatusView *v) {
    CFStringRef header = getText(v->header);
    CFStringRef title = getBtnTitle(v->button);
    if (!header) {
        setAccessibilityLabel(v->button, title);
        return;
    }
    CFStringRef label = formatStr(NULL, sepFmt, header, title);
    setAccessibilityLabel(v->button, label);
    CFRelease(label);
}
