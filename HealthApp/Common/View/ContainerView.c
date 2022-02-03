#include "ContainerView.h"
#include <stdlib.h>
#include "CocoaHelpers.h"
#include "Views.h"

Class ContainerViewClass;
Ivar ContainerViewDataRef;

id containerView_init(CFStringRef title, int spacing, bool margins) {
    id self = createNew(ContainerViewClass);
    disableAutoresizing(self);
#ifndef __clang_analyzer__
    ContainerViewData *data = malloc(sizeof(ContainerViewData));
    data->divider = createView(-1);
    data->headerLabel = createLabel(title, TextTitle3, 4, true);
    data->stack = createStackView(NULL, 0, 1, spacing, (Padding){.top = 5});

    setHeight(data->divider, 21, true);
    id divLine = createBackgroundView(ColorSeparator, 1, true);
    addSubview(data->divider, divLine);
    pin(divLine, data->divider, (Padding){0}, EdgeBottom);

    Padding padding = {0};
    if (margins)
        padding.left = padding.right = 8;
    id vStack = createStackView((id []){data->divider, data->headerLabel, data->stack},
                                3, 1, 0, padding);
    addSubview(self, vStack);
    pin(vStack, self, (Padding){0}, 0);

    releaseObj(vStack);
    releaseObj(divLine);
    object_setIvar(self, ContainerViewDataRef, (id) data);
#endif
    return self;
}

void containerView_deinit(id self, SEL _cmd) {
    ContainerViewData *ptr = (ContainerViewData *) object_getIvar(self, ContainerViewDataRef);
    struct objc_super super = {self, objc_getClass("UIView")};
    releaseObj(ptr->divider);
    releaseObj(ptr->headerLabel);
    releaseObj(ptr->stack);
    free(ptr);
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);
}
