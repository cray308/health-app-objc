#ifndef ContainerView_h
#define ContainerView_h

#include <CoreFoundation/CFBase.h>
#include <objc/runtime.h>
#include "ObjectArray.h"

extern Class ContainerViewClass;
extern Ivar ContainerViewDataRef;

enum {
    HideDivider = 0x1,
    HideLabel = 0x2
};

typedef struct __containerVData {
    id divider;
    id headerLabel;
    id stack;
    Array_object *views;
} ContainerViewData;

id containerView_init(CFStringRef title, int hidden, int spacing, bool margins);
void containerView_add(id self, id v);
void containerView_deinit(id self, SEL _cmd);

#endif /* ContainerView_h */
