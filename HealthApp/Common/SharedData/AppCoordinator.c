#include "AppCoordinator.h"
#include "AppDelegate.h"
#include "AppUserData.h"
#include "HomeVC.h"
#include "HistoryVC.h"
#include "PersistenceService.h"
#include "SettingsVC.h"
#include "ViewControllerHelpers.h"

extern void toggleDarkModeForCharts(bool);

enum {
    TabHome, TabHistory, TabSettings
};

AppCoordinator *appCoordinator = NULL;

void *appCoordinator_start(id tabVC, void (**fetchHandler)(void*)) {
    toggleDarkModeForCharts(userData->darkMode);
    appCoordinator = calloc(1, sizeof(AppCoordinator));
    SEL itemInit = sel_getUid("initWithTitle:image:tag:");
    SEL vcInit = sel_getUid("initWithNibName:bundle:"), setter = sel_getUid("setTabBarItem:");
    id controllers[3];
    id items[3];
    CFStringRef imgNames[] = {CFSTR("ico_house"), CFSTR("ico_chart"), CFSTR("ico_gear")};
    CFStringRef titles[3]; fillStringArray(titles, CFSTR("tabs%d"), 3);

    for (int i = 0; i < 3; ++i) {
        id image = createImage(imgNames[i]);
        id _item = allocClass(objc_getClass("UITabBarItem"));
        items[i] = ((id(*)(id,SEL,CFStringRef,id,int))objc_msgSend)
        (_item, itemInit, titles[i], image, i);

        id _obj = allocNavVC();
        controllers[i] = ((id(*)(id,SEL,CFStringRef,id))objc_msgSend)
        (_obj, vcInit, NULL, nil);
        setObject(controllers[i], setter, items[i]);
    }

    id navBar = getNavBar(controllers[0]);
    setBool(navBar, sel_getUid("setPrefersLargeTitles:"), true);
    setBool(controllers[2], sel_getUid("setNavigationBarHidden:"), true);

    appCoordinator->children[0] = homeVC_init();
    setupNavVC(controllers[0], appCoordinator->children[0]);

    void *result;
    appCoordinator->children[1] = historyVC_init(&result, fetchHandler);
    setupNavVC(controllers[1], appCoordinator->children[1]);

    appCoordinator->children[2] = settingsVC_init();
    setupNavVC(controllers[2], appCoordinator->children[2]);

    CFArrayRef array = CFArrayCreate(NULL, (const void **)controllers, 3, &(CFArrayCallBacks){0});
    ((void(*)(id,SEL,CFArrayRef,bool))objc_msgSend)
    (tabVC, sel_getUid("setViewControllers:animated:"), array, false);

    for (int i = 0; i < 3; ++i) {
        releaseObj(controllers[i]);
        releaseObj(items[i]);
    }
    CFRelease(array);
    return result;
}

void appCoordinator_updateUserInfo(signed char plan, signed char darkMode, short *weights) {
    bool updateHome = plan != userData->currentPlan;
    if (appUserData_updateUserSettings(plan, darkMode, weights)) {
        id window = appDel_setWindowTint(createColor(ColorRed));
        id tabVC = getObject(window, sel_getUid("rootViewController"));
        voidFunc(tabVC, sel_getUid("viewDidLayoutSubviews"));
        toggleDarkModeForCharts(userData->darkMode);
        CFArrayRef ctrls = getViewControllers(tabVC);
        homeVC_updateColors(getFirstVC((id) CFArrayGetValueAtIndex(ctrls, TabHome)));
        settingsVC_updateColors(getFirstVC((id) CFArrayGetValueAtIndex(ctrls, TabSettings)));
        if (appCoordinator->loadedViewControllers & LoadedVC_History)
            historyVC_updateColors(getFirstVC((id) CFArrayGetValueAtIndex(ctrls, TabHistory)));
    }
    free(weights);
    if (updateHome)
        homeVC_createWorkoutsList(appCoordinator->children[TabHome]);
}

void appCoordinator_deleteAppData(void) {
    bool updateHome = userData->completedWorkouts;
    appUserData_deleteSavedData();
    persistenceService_deleteUserData();
    if (updateHome)
        homeVC_updateWorkoutsList(appCoordinator->children[TabHome]);
    bool reloadHist = appCoordinator->loadedViewControllers & LoadedVC_History;
    historyVC_clearData(appCoordinator->children[TabHistory], reloadHist);
}

void appCoordinator_updateMaxWeights(short *weights) {
    appUserData_updateWeightMaxes(weights);
    if (appCoordinator->loadedViewControllers & LoadedVC_Settings)
        settingsVC_updateWeightFields(appCoordinator->children[TabSettings]);
}
