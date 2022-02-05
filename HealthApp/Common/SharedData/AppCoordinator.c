#include "AppCoordinator.h"
#include <stdlib.h>
#include "AppDelegate.h"
#include "AppUserData.h"
#include "HomeVC.h"
#include "HistoryVC.h"
#include "PersistenceService.h"
#include "SettingsVC.h"
#include "ViewControllerHelpers.h"

extern void toggleDarkModeForCharts(bool);

AppCoordinator *appCoordinator = NULL;

void *appCoordinator_start(id tabVC, void (**fetchHandler)(void*)) {
    toggleDarkModeForCharts(userData->darkMode);
    appCoordinator = calloc(1, sizeof(AppCoordinator));
    SEL itemInit = sel_getUid("initWithTitle:image:tag:"), setter = sel_getUid("setTabBarItem:");
    id controllers[3];
    id items[3];
    CFStringRef imgNames[] = {CFSTR("ico_house"), CFSTR("ico_chart"), CFSTR("ico_gear")};
    CFStringRef titles[3]; fillStringArray(titles, CFSTR("tabs%d"), 3);

    appCoordinator->children[0] = homeVC_init();
    void *result;
    appCoordinator->children[1] = historyVC_init(&result, fetchHandler);
    appCoordinator->children[2] = settingsVC_init();

    for (int i = 0; i < 3; ++i) {
        id image = createImage(imgNames[i]);
        id _item = allocClass(objc_getClass("UITabBarItem"));
        items[i] = (((id(*)(id,SEL,CFStringRef,id,int))objc_msgSend)
                    (_item, itemInit, titles[i], image, i));
        controllers[i] = createNavVC(appCoordinator->children[i]);
        setObject(controllers[i], setter, items[i]);
        releaseObj(items[i]);
    }

    CFArrayRef array = CFArrayCreate(NULL, (const void **)controllers, 3, NULL);
    (((void(*)(id,SEL,CFArrayRef,bool))objc_msgSend)
     (tabVC, sel_getUid("setViewControllers:animated:"), array, false));
    setupTabVC(tabVC);

    for (int i = 0; i < 3; ++i) {
        releaseObj(controllers[i]);
    }
    CFRelease(array);
    return result;
}

void appCoordinator_updateUserInfo(signed char plan, signed char darkMode, short *weights) {
    bool updateHome = plan != userData->currentPlan;
    if (appUserData_updateUserSettings(plan, darkMode, weights)) {
        id window = appDel_setWindowTint(createColor(ColorRed));
        id tabVC = getObject(window, sel_getUid("rootViewController"));
        setupTabVC(tabVC);
        toggleDarkModeForCharts(userData->darkMode);
        homeVC_updateColors(appCoordinator->children[0]);
        historyVC_updateColors(appCoordinator->children[1]);
        settingsVC_updateColors(appCoordinator->children[2]);
    }
    if (updateHome)
        homeVC_createWorkoutsList(appCoordinator->children[0]);
}

void appCoordinator_deleteAppData(void) {
    bool updateHome = userData->completedWorkouts;
    appUserData_deleteSavedData();
    persistenceService_deleteUserData();
    if (updateHome)
        homeVC_updateWorkoutsList(appCoordinator->children[0]);
    historyVC_clearData(appCoordinator->children[1]);
}

void appCoordinator_updateMaxWeights(short *weights) {
    appUserData_updateWeightMaxes(weights);
    if (isViewLoaded(appCoordinator->children[2]))
        settingsVC_updateWeightFields(appCoordinator->children[2]);
}
