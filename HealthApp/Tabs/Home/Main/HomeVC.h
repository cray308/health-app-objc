#ifndef HomeVC_h
#define HomeVC_h

#include "AppDelegate.h"
#include "ContainerView.h"

typedef struct {
    ContainerView *planContainer;
    ContainerView *customContainer;
    int numWorkouts;
} HomeVC;

#endif /* HomeVC_h */
