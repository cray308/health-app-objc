#include "ContainerView.h"
#include "CocoaHelpers.h"
#include "Views.h"

Class ContainerViewClass;

id containerView_init(CFStringRef title, int spacing, bool margins) {
    id _self = createNew(ContainerViewClass);
    id self = getObject(_self, sel_getUid("init"));
    disableAutoresizing(self);
    ContainerView *data = (ContainerView *) ((char *)self + ViewSize);
    data->divider = createView();
    data->headerLabel = createLabel(title, TextTitle3, true);
    data->stack = createStackView(NULL, 0, 1, spacing, (Padding){.top = 5});

    setHeight(data->divider, 21, true);
    id divLine = createView();
    setBackground(divLine, createColor(ColorSeparator));
    setHeight(divLine, 1, true);
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
    return self;
}

void containerView_deinit(id self, SEL _cmd) {
    ContainerView *ptr = (ContainerView *) ((char *)self + ViewSize);
    struct objc_super super = {self, ViewClass};
    releaseObj(ptr->divider);
    releaseObj(ptr->headerLabel);
    releaseObj(ptr->stack);
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);
}

void containerView_updateColors(id self, id labelColor, id divColor) {
    ContainerView *data = (ContainerView *) ((char *)self + ViewSize);
    setTextColor(data->headerLabel, labelColor);
    id divLine = (id) CFArrayGetValueAtIndex(getSubviews(data->divider), 0);
    setBackground(divLine, divColor);
}
