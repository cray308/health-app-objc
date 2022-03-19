#ifndef HomeVC_h
#define HomeVC_h

#include "ContainerView.h"

typedef struct {
    CCacheRef clr;
    VCacheRef tbl;
    ContainerView *planContainer;
    ContainerView *customContainer;
    int numWorkouts;
} HomeVC;

#endif /* HomeVC_h */
