#include "AppDelegate.h"
#include "DatabaseManager.h"
#include "HistoryVC.h"
#include "HomeVC.h"
#include "InputVC.h"
#include "SettingsVC.h"
#include "Views_VCExt.h"
#include "WorkoutVC.h"

static AppDelegate *getAppDel(void) {
    id shared = msgV(clsSig(id), objc_getClass("UIApplication"), sel_getUid("sharedApplication"));
    return (AppDelegate *)msgV(objSig(id), shared, sel_getUid("delegate"));
}

id getAppWindow(void) { return getAppDel()->window; }

UserData const *getUserData(void) { return &getAppDel()->userData; }

#pragma mark - Application Delegate

bool appDelegate_didFinishLaunching(AppDelegate *self, SEL _cmd _U_, id app _U_, id opt _U_) {
    Class TabBarAppearance = getTabBarAppearanceClass();
    int tzDiff = 0, week = 0;
    id unCenter = getNotificationCenter();
    CFDictionaryRef prefs = CFPreferencesCopyAppValue(PrefsKey, kCFPreferencesCurrentApplication);
    if (prefs != NULL) {
        tzDiff = userData_init(&self->userData, prefs, &week, TabBarAppearance);
    } else {
        userData_create(&self->userData, TabBarAppearance);
        msgV(objSig(void, u_long, void(^)(bool, id)), unCenter,
             sel_getUid("requestAuthorizationWithOptions:completionHandler:"),
             6, ^(bool granted _U_, id err _U_){});
        createDB();
    }

    initVCData(self->userData.darkMode);
    initWorkoutData(week);

    HistoryModel *historyModel;
    self->tabs[0] = new(HomeVCClass);
    self->tabs[1] = historyVC_init(&historyModel);
    self->tabs[2] = new(SettingsVCClass);

    Class TabBarItem = objc_getClass("UITabBarItem");
    SEL iItem = sel_getUid("initWithTitle:image:tag:");
    id (*itemInit)(id, SEL, CFStringRef, id, long) =
      (id(*)(id, SEL, CFStringRef, id, long))class_getMethodImplementation(TabBarItem, iItem);
    SEL nstbi = sel_getUid("setTabBarItem:");
    void (*setItem)(id, SEL, id) = (void(*)(id, SEL, id))class_getMethodImplementation(NavVC, nstbi);
    id controllers[3];
    CFStringRef imageNames[] = {CFSTR("ico_house"), CFSTR("ico_chart"), CFSTR("ico_gear")}, titles[3];
    fillStringArray(titles, CFSTR("tabs%d"), 3);

    for (int i = 0; i < 3; ++i) {
        id item = itemInit(alloc(TabBarItem), iItem, titles[i], getImage(imageNames[i]), i);
        controllers[i] = createNavVC(self->tabs[i]);
        setItem(controllers[i], nstbi, item);
        releaseObject(item);
        CFRelease(titles[i]);
    }

    self->window = new(objc_getClass("UIWindow"));
    setTintColor(self->window, getColor(ColorRed));
    id tabVC = new(objc_getClass("UITabBarController"));
    msgV(objSig(void, id), self->window, sel_getUid("setRootViewController:"), tabVC);
    CFArrayRef vcArr = CFArrayCreate(NULL, (const void **)controllers, 3, NULL);
    msgV(objSig(void, CFArrayRef), tabVC, sel_getUid("setViewControllers:"), vcArr);
    setupTabVC(tabVC, TabBarAppearance);
    for (int i = 0; i < 3; ++i) {
        releaseVC(controllers[i]);
    }
    CFRelease(vcArr);
    releaseVC(tabVC);
    msgV(objSig(void), self->window, sel_getUid("makeKeyAndVisible"));
    runStartupJob(historyModel, self->userData.weekStart, tzDiff);
    return true;
}

int appDelegate_supportedOrientations(AppDelegate *self _U_, SEL _cmd _U_, id app _U_, id win _U_) {
    return 2;
}

#pragma mark - Child VC Callbacks

void updateUserInfo(uint8_t plan, uint8_t darkMode, int const *weights) {
    AppDelegate *self = getAppDel();
    uint8_t updates = userData_update(&self->userData, plan, darkMode, weights);
    if (updates & MaskCurrentPlan)
        homeVC_createWorkoutsList(self->tabs[0], &self->userData);

    if (updates & MaskDarkMode) {
        handleTintChange(self->window, darkMode);
        homeVC_updateColors(self->tabs[0]);
        if (isViewLoaded(self->tabs[1]))
            historyVC_updateColors(self->tabs[1], darkMode);
        settingsVC_updateColors(self->tabs[2], darkMode);
    }
}

void deleteAppData(void) {
    AppDelegate *self = getAppDel();
    if (userData_clear(&self->userData))
        homeVC_updateWorkoutsList((HomeVC *)((char *)self->tabs[0] + VCSize), 0);
    historyVC_clearData(self->tabs[1]);
    deleteStoredData();
}

void addWorkoutData(Workout const *workout, uint8_t day, int *weights, bool pop) {
    AppDelegate *self = getAppDel();
    if (workout->duration < MinWorkoutDuration) goto cleanup;

    bool updatedWeights = false;
    uint8_t completed = userData_addWorkoutData(&self->userData, day, weights, &updatedWeights);
    if (completed) homeVC_handleFinishedWorkout(self->tabs[0], completed);
    if (updatedWeights && isViewLoaded(self->tabs[2]))
        inputVC_updateFields((InputVC *)((char *)self->tabs[2] + VCSize), self->userData.lifts);

    saveWorkoutData(workout->duration, workout->type, weights);
cleanup:
    if (pop) {
        id navVC = getNavVC(self->tabs[0]);
        msgV(objSig(id, bool), navVC, sel_getUid("popViewControllerAnimated:"), true);
    }
}
