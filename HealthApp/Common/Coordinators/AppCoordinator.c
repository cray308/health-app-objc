//
//  AppCoordinator.c
//  HealthApp
//
//  Created by Christopher Ray on 3/20/21.
//

#import "AppCoordinator.h"
#import "HomeTabCoordinator.h"
#import "HistoryTabCoordinator.h"
#import "SettingsTabCoordinator.h"

typedef enum {
    TabHome, TabHistory, TabSettings
} AppTab;

AppCoordinator *appCoordinatorShared = NULL;

void appCoordinator_start(AppCoordinator *this, id tabVC) {
    id controllers[3];
    id items[3];
    CFStringRef titles[] = {CFSTR("Home"), CFSTR("History"), CFSTR("Settings")};
    CFStringRef imgNames[] = {CFSTR("house"), CFSTR("chart.bar"), CFSTR("gear")};

    for (int i = 0; i < 3; ++i) {
        id image = ((id(*)(Class,SEL,CFStringRef))objc_msgSend)
        (objc_getClass("UIImage"), sel_getUid("systemImageNamed:"), imgNames[i]);

        items[i] = ((id(*)(id,SEL,CFStringRef,id,int))objc_msgSend)
        (allocClass("UITabBarItem"), sel_getUid("initWithTitle:image:tag:"), titles[i], image, i);

        controllers[i] = ((id(*)(id,SEL,CFStringRef,id))objc_msgSend)
        (allocNavVC(), sel_getUid("initWithNibName:bundle:"), NULL, nil);

        id navBar = ((id(*)(id,SEL))objc_msgSend)(controllers[i], sel_getUid("navigationBar"));

        ((void(*)(id,SEL,id))objc_msgSend)(navBar, sel_getUid("setBarTintColor:"),
                                           createColor("tertiarySystemGroupedBackgroundColor"));
        ((void(*)(id,SEL,id))objc_msgSend)(controllers[i], sel_getUid("setTabBarItem:"), items[i]);
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

    ((void(*)(id,SEL,id,bool))objc_msgSend)(tabVC, sel_getUid("setViewControllers:animated:"),
                                            createArray(controllers, 3), false);

    for (int i = 0; i < 3; ++i) {
        releaseObj(controllers[i]);
        releaseObj(items[i]);
    }
}

void appCoordinator_updatedUserInfo(AppCoordinator *this) {
    homeCoordinator_resetUI(this->children[TabHome]);
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
