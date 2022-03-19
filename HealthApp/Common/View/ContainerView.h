#ifndef ContainerView_h
#define ContainerView_h

#include "ColorCache.h"
#include "ViewCache.h"

typedef struct {
    id divider;
    id headerLabel;
    id stack;
} ContainerView;

id containerView_init(VCacheRef tbl, CCacheRef clr,
                      CFStringRef title CF_CONSUMED, ContainerView **ref, int spacing);
void containerView_updateColors(ContainerView *data, VCacheRef tbl, CCacheRef clr);

#endif /* ContainerView_h */
