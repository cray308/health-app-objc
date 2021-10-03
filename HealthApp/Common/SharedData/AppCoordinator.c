//
//  AppCoordinator.c
//  HealthApp
//
//  Created by Christopher Ray on 3/20/21.
//

#include "AppCoordinator.h"
#include "HomeTabCoordinator.h"
#include "HistoryTabCoordinator.h"
#include "SettingsTabCoordinator.h"
#include "ViewControllerHelpers.h"

typedef enum {
    TabHome, TabHistory, TabSettings
} AppTab;

AppCoordinator *appCoordinator = NULL;

void appCoordinator_start(id tabVC) {
    appCoordinator = calloc(1, sizeof(AppCoordinator));
    id controllers[3];
    id items[3];
    CFStringRef imgNames[] = {CFSTR("house"), CFSTR("chart.bar"), CFSTR("gear")};
    CFStringRef titles[3]; fillStringArray(titles, CFSTR("tabs%d"), 3);

    for (int i = 0; i < 3; ++i) {
        id image = createImage(imgNames[i], true);
        items[i] = ((id(*)(id,SEL,CFStringRef,id,int))objc_msgSend)
        (allocClass("UITabBarItem"), sel_getUid("initWithTitle:image:tag:"), titles[i], image, i);

        controllers[i] = ((id(*)(id,SEL,CFStringRef,id))objc_msgSend)
        (allocNavVC(), sel_getUid("initWithNibName:bundle:"), NULL, nil);

        id navBar = getObject(controllers[i], sel_getUid("navigationBar"));

        setObject(navBar, sel_getUid("setBarTintColor:"),
                  createColor("tertiarySystemGroupedBackgroundColor"));
        setObject(controllers[i], sel_getUid("setTabBarItem:"), items[i]);
    }

    HomeTabCoordinator *homeCoord = calloc(1, sizeof(HomeTabCoordinator));
    homeCoord->navVC = controllers[0];
    homeCoordinator_start(homeCoord);

    HistoryTabCoordinator *histCoord = calloc(1, sizeof(HistoryTabCoordinator));
    histCoord->navVC = controllers[1];
    historyCoordinator_start(histCoord);

    SettingsTabCoordinator *settingsCoord = malloc(sizeof(SettingsTabCoordinator));
    settingsCoord->navVC = controllers[2];
    settingsCoordinator_start(settingsCoord);

    memcpy(appCoordinator->children,
           (void *[]){homeCoord, histCoord, settingsCoord}, 3 * sizeof(void *));

    CFArrayRef array = CFArrayCreate(NULL, (const void **)controllers, 3, &kCocoaArrCallbacks);
    ((void(*)(id,SEL,CFArrayRef,bool))objc_msgSend)
    (tabVC, sel_getUid("setViewControllers:animated:"), array, false);

    for (int i = 0; i < 3; ++i) {
        releaseObj(controllers[i]);
        releaseObj(items[i]);
    }
    CFRelease(array);
}

void appCoordinator_updatedUserInfo(void) {
    homeCoordinator_resetUI(appCoordinator->children[TabHome]);
}

void appCoordinator_fetchHistory(void) {
    historyCoordinator_fetchData(appCoordinator->children[TabHistory]);
}

void appCoordinator_deletedAppData(void) {
    homeCoordinator_updateUI(appCoordinator->children[TabHome]);
    bool updateVC = appCoordinator->loadedViewControllers & LoadedViewController_History;
    historyCoordinator_updateUI(appCoordinator->children[TabHistory], updateVC);
}

void appCoordinator_updateMaxWeights(void) {
    if (appCoordinator->loadedViewControllers & LoadedViewController_Settings) {
        settingsCoordinator_updateWeightText(appCoordinator->children[TabSettings]);
    }
}
