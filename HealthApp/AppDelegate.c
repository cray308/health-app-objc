#include "AppDelegate.h"
#include "AppUserData.h"
#include "HomeVC.h"
#include "HistoryVC.h"
#include "InputVC.h"
#include "PersistenceService.h"
#include "SettingsVC.h"
#include "SetupWorkoutVC.h"
#include "ViewControllerHelpers.h"
#include "WorkoutVC.h"

#if DEBUG
extern void persistenceService_create(void);
#endif
extern void toggleDarkModeForCharts(bool);

Class AppDelegateClass;

bool appDelegate_didFinishLaunching(AppDelegate *self, SEL _cmd _U_,
                                    id application _U_, id options _U_) {
    initValidatorStrings();
    initExerciseStrings();
    initWorkoutStrings();
    bool legacy = handleIOSVersion();
    CGRect bounds;
    getScreenBounds(&bounds);
    self->window = createObjectWithFrame(objc_getClass("UIWindow"), bounds);

    CFStringRef hasLaunchedKey = CFSTR("hasLaunched");
    id defaults = getUserDefaults();
    bool hasLaunched = (((bool(*)(id,SEL,CFStringRef))objc_msgSend)
                        (defaults, sel_getUid("boolForKey:"), hasLaunchedKey));

    persistenceService_init();
    int tzOffset = 0;

    if (!hasLaunched) {
        (((void(*)(id,SEL,bool,CFStringRef))objc_msgSend)
         (defaults, sel_getUid("setBool:forKey:"), true, hasLaunchedKey));
        userInfo_create(legacy);
#if DEBUG
        persistenceService_create();
#endif
        id center = getNotificationCenter();
        (((void(*)(id,SEL,unsigned long,void(^)(BOOL,id)))objc_msgSend)
         (center, sel_getUid("requestAuthorizationWithOptions:completionHandler:"),
          6, ^(BOOL granted _U_, id error _U_) {}));
    } else {
        tzOffset = userInfo_initFromStorage();
    }

    char const *titleForRow = "@@:@qq";
    if (legacy) {
        class_addMethod(DMNavVC, sel_getUid("preferredStatusBarStyle"),
                        (IMP) dmNavVC_getStatusBarStyle, "i@:");
        class_addMethod(SetupWorkoutVCClass,
                        sel_getUid("pickerView:attributedTitleForRow:forComponent:"),
                        (IMP) setupWorkoutVC_attrTitleForRow, titleForRow);
    } else {
        class_addMethod(SetupWorkoutVCClass, sel_getUid("pickerView:titleForRow:forComponent:"),
                        (IMP) setupWorkoutVC_titleForRow, titleForRow);
    }

    objc_registerClassPair(DMNavVC);
    objc_registerClassPair(SetupWorkoutVCClass);

    setTintColor(self->window, createColor(ColorRed));
    id tabVC = createNew(objc_getClass("UITabBarController"));
    setObject(self->window, sel_getUid("setRootViewController:"), tabVC);
    void (*fetchHandler)(void*);
    void *fetchArg;
    SEL itemInit = sel_getUid("initWithTitle:image:tag:"), setItem = sel_getUid("setTabBarItem:");
    id controllers[3], items[3];
    CFStringRef imgNames[] = {CFSTR("ico_house"), CFSTR("ico_chart"), CFSTR("ico_gear")};
    CFStringRef titles[3];
    fillStringArray(titles, CFSTR("tabs%d"), 3);
    self->children[0] = homeVC_init();
    self->children[1] = historyVC_init(&fetchArg, &fetchHandler);
    self->children[2] = settingsVC_init();

    for (int i = 0; i < 3; ++i) {
        id image = createImage(imgNames[i]);
        id _item = allocClass(objc_getClass("UITabBarItem"));
        items[i] = (((id(*)(id,SEL,CFStringRef,id,long))objc_msgSend)
                    (_item, itemInit, titles[i], image, i));
        controllers[i] = createNavVC(self->children[i]);
        setObject(controllers[i], setItem, items[i]);
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
    releaseObj(tabVC);

    voidFunc(self->window, sel_getUid("makeKeyAndVisible"));
    toggleDarkModeForCharts(userData->darkMode);
    persistenceService_start(tzOffset, fetchHandler, fetchArg);
    return true;
}

int appDelegate_supportedOrientations(AppDelegate *self _U_, SEL _cmd _U_,
                                      id application _U_, id window _U_) {
    id device = staticMethod(objc_getClass("UIDevice"), sel_getUid("currentDevice"));
    return getInt(device, sel_getUid("userInterfaceIdiom")) == 1 ? 26 : 2;
}

static AppDelegate *getAppDel(void) {
    id app = staticMethod(objc_getClass("UIApplication"), sel_getUid("sharedApplication"));
    return (AppDelegate *) getObject(app, sel_getUid("delegate"));
}

void appDel_setWindowTint(id color) {
    setTintColor(getAppDel()->window, color);
}

void appDel_updateUserInfo(signed char plan, signed char darkMode, short *weights) {
    AppDelegate *self = getAppDel();
    bool updateHome = plan != userData->currentPlan;
    if (appUserData_updateUserSettings(plan, darkMode, weights)) {
        setTintColor(self->window, createColor(ColorRed));
        id tabVC = getObject(self->window, sel_getUid("rootViewController"));
        setupTabVC(tabVC);
        toggleDarkModeForCharts(userData->darkMode);
        homeVC_updateColors(self->children[0]);
        historyVC_updateColors(self->children[1]);
        settingsVC_updateColors(self->children[2]);
    }
    if (updateHome)
        homeVC_createWorkoutsList(self->children[0]);
}

void appDel_deleteAppData(void) {
    AppDelegate *self = getAppDel();
    bool updateHome = userData->completedWorkouts;
    appUserData_deleteSavedData();
    persistenceService_deleteUserData();
    if (updateHome)
        homeVC_updateWorkoutsList(self->children[0]);
    historyVC_clearData(self->children[1]);
}

void appDel_updateMaxWeights(short *weights) {
    AppDelegate *self = getAppDel();
    appUserData_updateWeightMaxes(weights);
    if (isViewLoaded(self->children[2]))
        settingsVC_updateWeightFields(self->children[2]);
}
