#ifndef ContainerView_h
#define ContainerView_h

#include <CoreFoundation/CoreFoundation.h>
#include <objc/objc.h>

extern Class ContainerViewClass;

typedef struct {
    id divider;
    id header;
    id stack;
} ContainerView;

typedef struct {
    id view;
    ContainerView *data;
} CVPair;

id containerView_init(ContainerView **ref, CFStringRef header CF_CONSUMED);
void containerView_deinit(id self, SEL _cmd);

#endif /* ContainerView_h */
