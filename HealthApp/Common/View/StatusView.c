#include "StatusView.h"
#include "CocoaHelpers.h"
#include "Views.h"

#define addCornerRadius(v) msg1(void, CGFloat, getLayer(v), sel_getUid("setCornerRadius:"), 5)

extern id UIFontTextStyleHeadline;

Class StatusViewClass;

id statusView_init(CFStringRef text, StatusView **ref, int tag, id target, SEL action) {
    id self = createNew(StatusViewClass);
    StatusView *data = (StatusView *) ((char *)self + ViewSize);
    setTag(self, tag);
    data->button = createButton(text, ColorLabel, tag, target, action);
    addCornerRadius(data->button);
    setFont(getTitleLabel(data->button), getPreferredFont(UIFontTextStyleHeadline));
    setBackground(data->button, createColor(ColorSecondaryBGGrouped));
    setHeight(data->button, 50, true);
    data->headerLabel = createLabel(NULL, UIFontTextStyleSubheadline, false);
    data->box = createNew(ViewClass);
    addCornerRadius(data->box);
    id width = createConstraint(data->box, 7, 0, nil, 0, 20);
    lowerPriority(width);
    activateConstraint(width);
    setHeight(data->box, 20, true);
    id hStack = createStackView((id []){data->button, data->box}, 2, 0, 5, (Padding){0});
    centerHStack(hStack);
    id vStack = createStackView((id []){data->headerLabel, hStack}, 2, 1, 4, (Padding){4, 0, 4, 0});
    setUsesAutolayout(vStack);
    addSubview(self, vStack);
    pin(vStack, self);
    releaseObj(hStack);
    releaseObj(vStack);
    *ref = data;
    return self;
}

void statusView_deinit(id self, SEL _cmd) {
    StatusView *ptr = (StatusView *) ((char *)self + ViewSize);
    struct objc_super super = {self, ViewClass};
    releaseObj(ptr->headerLabel);
    releaseObj(ptr->box);
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);
}

void statusView_updateAccessibility(StatusView *ptr, CFStringRef stateText) {
    CFStringRef header = getText(ptr->headerLabel);
    CFMutableStringRef label = CFStringCreateMutableCopy(NULL, 128, CFSTR(""));
    if (header)
        CFStringAppendFormat(label, NULL, CFSTR("%@. "), header);
    CFStringAppend(label, msg0(CFStringRef, ptr->button, sel_getUid("currentTitle")));
    if (stateText)
        CFStringAppendFormat(label, NULL, CFSTR(". %@"), stateText);
    setAccessibilityLabel(ptr->button, label);
    CFRelease(label);
}
