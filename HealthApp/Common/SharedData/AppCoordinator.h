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

extern AppCoordinator *appCoordinator;

void appCoordinator_start(id tabVC);
void appCoordinator_updatedUserInfo(void);
void appCoordinator_fetchHistory(void);
void appCoordinator_deletedAppData(void);
void appCoordinator_updateMaxWeights(void);

#endif /* AppCoordinator_h */
