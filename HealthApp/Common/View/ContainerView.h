#ifndef ContainerView_h
#define ContainerView_h

#include <CoreFoundation/CFBase.h>
#include <objc/objc.h>

typedef struct {
    id divider;
    id headerLabel;
    id stack;
} ContainerView;

id containerView_init(CFStringRef title CF_CONSUMED, ContainerView **ref, int spacing);
void containerView_updateColors(ContainerView *data, id labelColor, id divColor);

#endif /* ContainerView_h */
