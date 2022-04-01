#ifndef HomeVC_h
#define HomeVC_h

#include "ContainerView.h"

typedef struct {
    CCacheRef clr;
    VCacheRef tbl;
    CVPair planContainer;
    ContainerView *customContainer;
    int numWorkouts;
} HomeVC;

#endif /* HomeVC_h */
