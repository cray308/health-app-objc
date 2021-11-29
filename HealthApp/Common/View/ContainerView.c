#include "ContainerView.h"
#include "CocoaHelpers.h"
#include "Views.h"

gen_array_source(object, id, DSDefault_shallowCopy, releaseObj)

Class ContainerViewClass;
Ivar ContainerViewDataRef;

id containerView_init(CFStringRef title, int hidden, int spacing, bool margins) {
    id self = createObjectWithFrame(ContainerViewClass, CGRectZero);
    disableAutoresizing(self);

    ContainerViewData *data = malloc(sizeof(ContainerViewData));
    data->views = array_new(object);
    data->divider = createView(false, -1);
    data->headerLabel = createLabel(title, TextTitle3, 4, true);
    data->stack = createStackView(NULL, 0, 1, spacing, (Padding){.top = 5});

    setHeight(data->divider, 21, false);
    id divLine = createBackgroundView(ColorSeparator, 1, false);
    addSubview(data->divider, divLine);
    pin(divLine, data->divider, (Padding){0}, EdgeBottom);

    Padding padding = {0};
    if (margins)
        padding.left = padding.right = 8;
    id vStack = createStackView((id []){data->divider, data->headerLabel, data->stack},
                                3, 1, 0, padding);
    addSubview(self, vStack);
    pin(vStack, self, (Padding){0}, 0);

    if (hidden & HideDivider)
        hideView(data->divider, true);
    if (hidden & HideLabel)
        hideView(data->headerLabel, true);

    releaseObj(vStack);
    releaseObj(divLine);
    object_setIvar(self, ContainerViewDataRef, (id) data);
    return self;
}

void containerView_add(id self, id v) {
    ContainerViewData *data = (ContainerViewData *) object_getIvar(self, ContainerViewDataRef);
    array_push_back(object, data->views, v);
    addArrangedSubview(data->stack, v);
}

void containerView_deinit(id self, SEL _cmd) {
    ContainerViewData *ptr = (ContainerViewData *) object_getIvar(self, ContainerViewDataRef);
    struct objc_super super = {self, objc_getClass("UIView")};
    releaseObj(ptr->divider);
    releaseObj(ptr->headerLabel);
    releaseObj(ptr->stack);
    array_free(object, ptr->views);
    free(ptr);
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);
}
