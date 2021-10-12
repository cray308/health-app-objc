//
//  Constraints.c
//  HealthApp
//
//  Created by Christopher Ray on 10/3/21.
//

#include "Constraints.h"
#include <objc/message.h>
#include "CocoaHelpers.h"

static struct AnchorNames {
    char const *const top;
    char const *const bottom;
    char const *const left;
    char const *const right;
    char const *const width;
    char const *const height;
    char const *const centerY;
} const anchors = {
    "topAnchor",
    "bottomAnchor",
    "leadingAnchor",
    "trailingAnchor",
    "widthAnchor",
    "heightAnchor",
    "centerYAnchor"
};

static inline void activateConstraints(id *constraints, int count) {
    CFArrayRef array = CFArrayCreate(NULL, (const void **)constraints, count, &kCocoaArrCallbacks);
    ((void(*)(Class,SEL,CFArrayRef))objc_msgSend)(objc_getClass("NSLayoutConstraint"),
                                                  sel_getUid("activateConstraints:"), array);
    CFRelease(array);
}

static inline id getAnchor(id view, const char *name) {
    return getObject(view, sel_getUid(name));
}

static inline id createConstraint(id a1, id a2, int constant) {
    id result;
    if (a2) {
        result = ((id(*)(id,SEL,id,CGFloat))objc_msgSend)
        (a1, sel_getUid("constraintEqualToAnchor:constant:"), a2, constant);
    } else {
        result = getObjectWithFloat(a1, sel_getUid("constraintEqualToConstant:"), constant);
    }
    return result;
}

void setWidth(id v, int width) {
    activateConstraints((id []){createConstraint(getAnchor(v, anchors.width), nil, width)}, 1);
}

void setMinHeight(id v, int height) {
    id c = getObjectWithFloat(getAnchor(v, anchors.height),
                              sel_getUid("constraintGreaterThanOrEqualToConstant:"), height);
    activateConstraints((id []){c}, 1);
}

void setHeight(id v, int height) {
    activateConstraints((id []){createConstraint(getAnchor(v, anchors.height), nil, height)}, 1);
}

void setEqualWidths(id v, id v2) {
    activateConstraints((id []){
        createConstraint(getAnchor(v, anchors.width), getAnchor(v2, anchors.width), 0)
    }, 1);
}

void pin(id v, id container, Padding margins, unsigned excluded) {
    static const unsigned edges[] = {EdgeTop, EdgeLeft, EdgeBottom, EdgeRight};
    id constraints[4] = {0};
    int len = 0;
    for (int i = 0; i < 4; ++i) {
        if (edges[i] & excluded) continue;
        id c;
        switch (edges[i]) {
            case EdgeTop:
                c = createConstraint(getAnchor(v, anchors.top),
                                     getAnchor(container, anchors.top), margins.top);
                break;
            case EdgeLeft:
                c = createConstraint(getAnchor(v, anchors.left),
                                     getAnchor(container, anchors.left), margins.left);
                break;
            case EdgeBottom:
                c = createConstraint(getAnchor(container, anchors.bottom),
                                     getAnchor(v, anchors.bottom), margins.bottom);
                break;
            default:
                c = createConstraint(getAnchor(container, anchors.right),
                                     getAnchor(v, anchors.right), margins.right);
        }
        constraints[len++] = c;
    }
    activateConstraints(constraints, len);
}
