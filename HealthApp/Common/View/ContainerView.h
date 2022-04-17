#ifndef ContainerView_h
#define ContainerView_h

#include "ColorCache.h"
#include "ViewCache.h"

typedef struct {
    id divider;
    id header;
    id stack;
} ContainerView;

typedef struct {
    id view;
    ContainerView *data;
} CVPair;

id containerView_init(VCacheRef tbl, CCacheRef clr,
                      ContainerView **v, CFStringRef header CF_CONSUMED);
void containerView_updateColors(ContainerView *v, VCacheRef tbl, CCacheRef clr);

#endif /* ContainerView_h */
