#ifndef ContainerView_h
#define ContainerView_h

#include <CoreFoundation/CFBase.h>
#include <objc/runtime.h>

extern Class ContainerViewClass;
extern Ivar ContainerViewDataRef;

typedef struct {
    id divider;
    id headerLabel;
    id stack;
} ContainerViewData;

id containerView_init(CFStringRef title, int spacing, bool margins);
void containerView_add(id self, id v);
void containerView_deinit(id self, SEL _cmd);

#endif /* ContainerView_h */
