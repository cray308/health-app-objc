//
//  AppCoordinator.h
//  HealthApp
//
//  Created by Christopher Ray on 3/20/21.
//

#ifndef AppCoordinator_h
#define AppCoordinator_h

#include "AppTypes.h"

enum {
    LoadedViewController_Home = 0x1,
    LoadedViewController_History = 0x2,
    LoadedViewController_Settings = 0x4
};

typedef struct {
    byte loadedViewControllers;
    void *children[3];
} AppCoordinator;

extern AppCoordinator *appCoordinator;

#endif /* AppCoordinator_h */
