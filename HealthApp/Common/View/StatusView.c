#include "StatusView.h"
#include "CocoaHelpers.h"
#include "Views.h"

Class StatusViewClass;
Ivar StatusViewDataRef;

id statusView_init(CFStringRef text, int tag, id target, SEL action) {
    id self = createNew(StatusViewClass);
#ifndef __clang_analyzer__
    StatusViewData *data = malloc(sizeof(StatusViewData));
    setTag(self, tag);
    int params = BtnLargeFont | BtnBackground | BtnRounded;
    data->button = createButton(text, ColorLabel, params, tag, target, action, 50);
    data->headerLabel = createLabel(NULL, TextSubhead, 4, false);
    data->box = createView(20);
    id hStack = createStackView((id []){data->button, data->box}, 2, 0, 5, (Padding){0});
    centerHStack(hStack);
    id vStack = createStackView((id []){data->headerLabel, hStack}, 2, 1, 4, (Padding){4, 0, 4, 0});
    addSubview(self, vStack);
    pin(vStack, self, (Padding){0}, 0);
    releaseObj(hStack);
    releaseObj(vStack);
    object_setIvar(self, StatusViewDataRef, (id) data);
#endif
    return self;
}

void statusView_deinit(id self, SEL _cmd) {
    StatusViewData *ptr = (StatusViewData *) object_getIvar(self, StatusViewDataRef);
    struct objc_super super = {self, objc_getClass("UIView")};
    releaseObj(ptr->headerLabel);
    releaseObj(ptr->box);
    free(ptr);
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);
}

void statusView_updateAccessibility(id self, CFStringRef stateText) {
    StatusViewData *ptr = (StatusViewData *) object_getIvar(self, StatusViewDataRef);
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
