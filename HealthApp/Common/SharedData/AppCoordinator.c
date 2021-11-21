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

typedef struct {
    id navVC;
} SettingsTabCoordinator;

AppCoordinator *appCoordinator = NULL;

void appCoordinator_start(id tabVC) {
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

    HomeTabCoordinator *homeCoord = calloc(1, sizeof(HomeTabCoordinator));
    homeCoord->navVC = controllers[0];
    homeCoordinator_start(homeCoord);

    HistoryTabCoordinator *histCoord = calloc(1, sizeof(HistoryTabCoordinator));
    histCoord->navVC = controllers[1];
    historyCoordinator_start(histCoord);

    SettingsTabCoordinator *settingsCoord = malloc(sizeof(SettingsTabCoordinator));
    settingsCoord->navVC = controllers[2];
    setupNavVC(settingsCoord->navVC, settingsVC_init());

    memcpy(appCoordinator->children,
           (void *[]){homeCoord, histCoord, settingsCoord}, 3 * sizeof(void *));

    CFArrayRef array = CFArrayCreate(NULL, (const void **)controllers, 3, &(CFArrayCallBacks){0});
    ((void(*)(id,SEL,CFArrayRef,bool))objc_msgSend)
    (tabVC, sel_getUid("setViewControllers:animated:"), array, false);

    for (int i = 0; i < 3; ++i) {
        releaseObj(controllers[i]);
        releaseObj(items[i]);
    }
    CFRelease(array);
}

void appCoordinator_updateUserInfo(signed char plan, bool darkMode, short *weights) {
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
    HomeTabCoordinator *home = (HomeTabCoordinator *) appCoordinator->children[TabHome];
    homeViewModel_fetchData(&home->model);
    homeVC_createWorkoutsList(getFirstVC(home->navVC));
    free(weights);
}

void appCoordinator_deleteAppData(void) {
    appUserData_deleteSavedData();
    persistenceService_deleteUserData();
    HomeTabCoordinator *home = (HomeTabCoordinator *) appCoordinator->children[TabHome];
    HistoryTabCoordinator *hist = (HistoryTabCoordinator *) appCoordinator->children[TabHistory];
    homeVC_updateWorkoutsList(getFirstVC(home->navVC));
    array_clear(weekData, hist->model.data);
    if (appCoordinator->loadedViewControllers & LoadedVC_History) {
        historyVC_refresh(getFirstVC(hist->navVC));
    }
}

void appCoordinator_updateMaxWeights(short *weights) {
    appUserData_updateWeightMaxes(weights);
    if (appCoordinator->loadedViewControllers & LoadedVC_Settings) {
        id navVC = ((SettingsTabCoordinator *) appCoordinator->children[TabSettings])->navVC;
        settingsVC_updateWeightFields(getFirstVC(navVC));
    }
    free(weights);
}
