#include "StatusView.h"
#include "CocoaHelpers.h"
#include "Views.h"

Class StatusViewClass;

id statusView_init(CFStringRef text, int tag, id target, SEL action) {
    id self = createNew(StatusViewClass);
    StatusView *data = (StatusView *) ((char *)self + ViewSize);
    setTag(self, tag);
    int params = BtnLargeFont | BtnBackground | BtnRounded;
    data->button = createButton(text, ColorLabel, params, tag, target, action);
    setHeight(data->button, 50, true);
    data->headerLabel = createLabel(NULL, TextSubhead, false);
    data->box = createView();
    setCGFloat(getLayer(data->box), sel_getUid("setCornerRadius:"), 5);
    setWidth(data->box, 20);
    setHeight(data->box, 20, true);
    id hStack = createStackView((id []){data->button, data->box}, 2, 0, 5, (Padding){0});
    centerHStack(hStack);
    id vStack = createStackView((id []){data->headerLabel, hStack}, 2, 1, 4, (Padding){4, 0, 4, 0});
    addSubview(self, vStack);
    pin(vStack, self, (Padding){0}, 0);
    releaseObj(hStack);
    releaseObj(vStack);
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
    id titleLabel = getTitleLabel(ptr->button);
    CFStringRef btnTitle = getText(titleLabel);
    CFStringAppend(label, btnTitle);
    if (stateText)
        CFStringAppendFormat(label, NULL, CFSTR(". %@"), stateText);
    setAccessibilityLabel(ptr->button, label);
    CFRelease(label);
}
