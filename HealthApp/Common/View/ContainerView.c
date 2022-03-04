#include "ContainerView.h"
#include "CocoaHelpers.h"
#include "Views.h"

Class ContainerViewClass;

id containerView_init(CFStringRef title, ContainerView **ref, int spacing, bool margins) {
    id self = createNew(ContainerViewClass);
    ContainerView *data = (ContainerView *) ((char *)self + ViewSize);
    data->divider = createNew(ViewClass);
    data->headerLabel = createLabel(title, UIFontTextStyleTitle3, true);
    data->stack = createStackView(NULL, 0, 1, spacing, (Padding){.top = 5});

    setHeight(data->divider, 21, true);
    id divLine = createNew(ViewClass);
    setUsesAutolayout(divLine);
    setBackground(divLine, createColor(ColorSeparator));
    setHeight(divLine, 1, true);
    addSubview(data->divider, divLine);
    id constraints[] = {
        createConstraint(divLine, 3, 0, data->divider, 3, 0),
        createConstraint(divLine, 5, 0, data->divider, 5, 0),
        createConstraint(divLine, 6, 0, data->divider, 6, 0)
    };
    CFArrayRef divArray = CFArrayCreate(NULL, (const void **)constraints, 3, NULL);
    activateConstraintsArray(divArray);
    CFRelease(divArray);

    Padding padding = {0};
    if (margins)
        padding.left = padding.right = 8;
    id vStack = createStackView((id []){data->divider, data->headerLabel, data->stack},
                                3, 1, 0, padding);
    setUsesAutolayout(vStack);
    addSubview(self, vStack);
    pin(vStack, self);

    releaseObj(vStack);
    releaseObj(divLine);
    *ref = data;
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

void containerView_updateColors(ContainerView *data, id labelColor, id divColor) {
    setTextColor(data->headerLabel, labelColor);
    id divLine = (id) CFArrayGetValueAtIndex(getSubviews(data->divider), 0);
    setBackground(divLine, divColor);
}
