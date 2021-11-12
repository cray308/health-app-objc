//
//  ContainerView.c
//  HealthApp
//
//  Created by Christopher Ray on 10/3/21.
//

#include "ContainerView.h"
#include <objc/message.h>
#include "CocoaHelpers.h"
#include "Views.h"

void container_refresh(Container *c) {
    setTextColor(c->headerLabel, createColor(ColorLabel));
    setBackground(c->divider, createColor(ColorSeparator));
}

id createContainer(Container *c, CFStringRef title, int hidden, int spacing, bool margins) {
    c->views = array_new(object);
    c->view = createView(nil, false, -1, -1);
    c->divider = createView(createColor(ColorSeparator), false, -1, 1);
    c->headerLabel = createLabel(title, TextTitle3, 4, true);
    c->stack = createStackView(NULL, 0, 1, spacing, (Padding){.top = 5});
    Padding padding = {0};
    if (margins)
        padding.left = padding.right = 8;
    id vStack = createStackView((id []){c->divider, c->headerLabel, c->stack}, 3, 1, 0, padding);
    addSubview(c->view, vStack);
    ((void(*)(id,SEL,CGFloat,id))objc_msgSend)(vStack, sel_getUid("setCustomSpacing:afterView:"),
                                               20, c->divider);
    pin(vStack, c->view, (Padding){0}, 0);

    if (hidden & HideDivider)
        hideView(c->divider, true);
    if (hidden & HideLabel)
        hideView(c->headerLabel, true);

    releaseObj(vStack);
    return c->view;
}

void containers_free(Container *c, int size) {
    for (int i = 0; i < size; ++i) {
        if (c[i].view) {
            releaseObj(c[i].view);
            releaseObj(c[i].divider);
            releaseObj(c[i].headerLabel);
            releaseObj(c[i].stack);
            array_free(object, c[i].views);
        }
    }
}

void container_add(Container *c, id v) {
    array_push_back(object, c->views, v);
    setObject(c->stack, sel_getUid("addArrangedSubview:"), v);
}

void container_clear(Container *c) {
    array_clear(object, c->views);
    CFArrayRef views = getArray(c->stack, sel_getUid("arrangedSubviews"));
    int count = (int) CFArrayGetCount(views);
    for (int i = 0; i < count; ++i) {
        id v = (id) CFArrayGetValueAtIndex(views, i);
        removeView(v);
    }
}
