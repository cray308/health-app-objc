#ifndef ContainerView_h
#define ContainerView_h

#include <CoreFoundation/CFBase.h>
#include <objc/runtime.h>

extern Class ContainerViewClass;

typedef struct {
    id divider;
    id headerLabel;
    id stack;
} ContainerView;

id containerView_init(CFStringRef title, int spacing, bool margins);
void containerView_deinit(id self, SEL _cmd);
void containerView_updateColors(id self, id labelColor, id divColor);

#endif /* ContainerView_h */
