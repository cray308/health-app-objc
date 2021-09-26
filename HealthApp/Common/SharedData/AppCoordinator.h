//
//  AppCoordinator.h
//  HealthApp
//
//  Created by Christopher Ray on 3/20/21.
//

#ifndef AppCoordinator_h
#define AppCoordinator_h

#include "CocoaHelpers.h"

typedef enum {
    LoadedViewController_Home = 0x1,
    LoadedViewController_History = 0x2,
    LoadedViewController_Settings = 0x4
} LoadedViewController;

typedef struct {
    unsigned char loadedViewControllers;
    void *children[3];
} AppCoordinator;

extern AppCoordinator *appCoordinatorShared;

void appCoordinator_start(AppCoordinator *coordinator, id tabVC);
void appCoordinator_updatedUserInfo(AppCoordinator *coordinator);
void appCoordinator_fetchHistory(AppCoordinator *this);
void appCoordinator_deletedAppData(AppCoordinator *coordinator);
void appCoordinator_updateMaxWeights(AppCoordinator *coordinator);

#endif /* AppCoordinator_h */
