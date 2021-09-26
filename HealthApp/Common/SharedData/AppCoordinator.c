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

AppCoordinator *appCoordinatorShared = NULL;

void appCoordinator_start(AppCoordinator *this, id tabVC) {
    id controllers[3];
    id items[3];
    CFStringRef imgNames[] = {CFSTR("house"), CFSTR("chart.bar"), CFSTR("gear")};

    for (int i = 0; i < 3; ++i) {
        id image = createImage(imgNames[i]);
        CFStringRef title = CFStringCreateWithFormat(NULL, NULL, CFSTR("tabs%d"), i);
        items[i] = ((id(*)(id,SEL,CFStringRef,id,int))objc_msgSend)
        (allocClass("UITabBarItem"),
         sel_getUid("initWithTitle:image:tag:"), localize(title), image, i);

        controllers[i] = ((id(*)(id,SEL,CFStringRef,id))objc_msgSend)
        (allocNavVC(), sel_getUid("initWithNibName:bundle:"), NULL, nil);

        id navBar = ((id(*)(id,SEL))objc_msgSend)(controllers[i], sel_getUid("navigationBar"));

        ((void(*)(id,SEL,id))objc_msgSend)(navBar, sel_getUid("setBarTintColor:"),
                                           createColor("tertiarySystemGroupedBackgroundColor"));
        ((void(*)(id,SEL,id))objc_msgSend)(controllers[i], sel_getUid("setTabBarItem:"), items[i]);
        CFRelease(title);
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

    memcpy(this->children, (void *[]){homeCoord, histCoord, settingsCoord}, 3 * sizeof(void *));

    CFArrayRef array = CFArrayCreate(NULL, (const void **)controllers, 3, &kCocoaArrCallbacks);
    ((void(*)(id,SEL,CFArrayRef,bool))objc_msgSend)
    (tabVC, sel_getUid("setViewControllers:animated:"), array, false);

    for (int i = 0; i < 3; ++i) {
        releaseObj(controllers[i]);
        releaseObj(items[i]);
    }
    CFRelease(array);
}

void appCoordinator_updatedUserInfo(AppCoordinator *this) {
    homeCoordinator_resetUI(this->children[TabHome]);
}

void appCoordinator_fetchHistory(AppCoordinator *this) {
    historyCoordinator_fetchData(this->children[TabHistory]);
}

void appCoordinator_deletedAppData(AppCoordinator *this) {
    homeCoordinator_updateUI(this->children[TabHome]);
    if (this->loadedViewControllers & LoadedViewController_History) {
        historyCoordinator_updateUI(this->children[TabHistory]);
    }
}

void appCoordinator_updateMaxWeights(AppCoordinator *this) {
    if (this->loadedViewControllers & LoadedViewController_Settings) {
        settingsCoordinator_updateWeightText(this->children[TabSettings]);
    }
}
