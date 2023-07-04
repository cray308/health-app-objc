#include "AppDelegate.h"
#include "DatabaseManager.h"
#include "HistoryVC.h"
#include "HomeVC.h"
#include "InputVC.h"
#include "SettingsVC.h"
#include "Views_VCExt.h"
#include "WorkoutVC.h"

enum {
    OrientationMaskPortrait = 2
};

enum {
    AuthorizationOptionSound = 2,
    AuthorizationOptionAlert = 4
};

enum {
    TabHome,
    TabHistory,
    TabSettings
};

static AppDelegate *getAppDelegate(void) {
    id shared = msgV(clsSig(id), objc_getClass("UIApplication"), sel_getUid("sharedApplication"));
    return (AppDelegate *)msgV(objSig(id), shared, sel_getUid("delegate"));
}

UserData const *getUserData(void) { return &getAppDelegate()->userData; }

#pragma mark - Application Delegate

bool appDelegate_didFinishLaunching(AppDelegate *self, SEL _cmd _U_,
                                    id app _U_, CFDictionaryRef options _U_) {
    int tzDiff = 0, week = 0;
    id unCenter = getNotificationCenter();
    CFDictionaryRef prefs = CFPreferencesCopyAppValue(PrefsKey, kCFPreferencesCurrentApplication);
    if (prefs != NULL) {
        tzDiff = userData_init(&self->userData, prefs, &week);
    } else {
        userData_create(&self->userData);
        msgV(objSig(void, u_long, void(^)(bool, id)), unCenter,
             sel_getUid("requestAuthorizationWithOptions:completionHandler:"),
             AuthorizationOptionSound | AuthorizationOptionAlert, ^(bool granted _U_, id err _U_){});
        createDB();
    }

    initWorkoutData(week);

    HistoryModel *historyModel;
    self->tabs[TabHome] = new(HomeVCClass);
    self->tabs[TabHistory] = historyVC_init(&historyModel);
    self->tabs[TabSettings] = new(SettingsVCClass);

    Class TabBarItem = objc_getClass("UITabBarItem");
    SEL iItem = sel_getUid("initWithTitle:image:tag:");
    id (*itemInit)(id, SEL, CFStringRef, id, long) =
      (id(*)(id, SEL, CFStringRef, id, long))class_getMethodImplementation(TabBarItem, iItem);
    SEL nstbi = sel_getUid("setTabBarItem:");
    void (*setItem)(id, SEL, id) = (void(*)(id, SEL, id))class_getMethodImplementation(NavVC, nstbi);
    id controllers[3];
    CFStringRef imageNames[] = {CFSTR("house"), CFSTR("chart.bar"), CFSTR("gear")}, titles[3];
    fillStringArray(titles, CFSTR("tabs%d"), 3);

    for (int i = 0; i < 3; ++i) {
        id item = itemInit(alloc(TabBarItem), iItem, titles[i], getImage(imageNames[i]), i);
        controllers[i] = createNavVC(self->tabs[i]);
        setItem(controllers[i], nstbi, item);
        releaseObject(item);
        CFRelease(titles[i]);
    }

    self->window = new(objc_getClass("UIWindow"));
    id tabVC = new(objc_getClass("UITabBarController"));
    msgV(objSig(void, id), self->window, sel_getUid("setRootViewController:"), tabVC);
    CFArrayRef vcArr = CFArrayCreate(NULL, (const void **)controllers, 3, NULL);
    msgV(objSig(void, CFArrayRef), tabVC, sel_getUid("setViewControllers:"), vcArr);

    id barColor = getBarColor(CFSTR("navBarColor"));
    id tabBar = msgV(objSig(id), tabVC, sel_getUid("tabBar"));
    setupBar(tabBar, objc_getClass("UITabBarAppearance"), barColor);
    Class NavBarAppearance = getNavBarAppearanceClass();
    for (int i = 0; i < 3; ++i) {
        setupBar(getNavBar(controllers[i]), NavBarAppearance, barColor);
        releaseVC(controllers[i]);
    }
    CFRelease(vcArr);
    releaseVC(tabVC);
    msgV(objSig(void), self->window, sel_getUid("makeKeyAndVisible"));
    setDelegate(unCenter, (id)self);
    runStartupJob(historyModel, self->userData.weekStart, tzDiff);
    return true;
}

u_long appDelegate_supportedInterfaceOrientations(id self _U_, SEL _cmd _U_,
                                                  id app _U_, id window _U_) {
    return OrientationMaskPortrait;
}

void appDelegate_willPresentNotification(AppDelegate *self, SEL _cmd _U_,
                                         id center _U_, id notification, void (^callback)(u_long)) {
    id req = msgV(objSig(id), notification, sel_getUid("request"));
    int identifier = CFStringGetIntValue(msgV(objSig(CFStringRef), req, sel_getUid("identifier")));
    id navVC = getNavVC(self->tabs[TabHome]);
    CFArrayRef controllers = msgV(objSig(CFArrayRef), navVC, sel_getUid("viewControllers"));
    if (CFArrayGetCount(controllers) == 2)
        workoutVC_receivedNotification((id)CFArrayGetValueAtIndex(controllers, 1), identifier);
    callback(0);
}

#pragma mark - Child VC Callbacks

void updateUserInfo(uint8_t plan, int const *weights) {
    AppDelegate *self = getAppDelegate();
    if (userData_update(&self->userData, plan, weights))
        homeVC_createWorkoutsList(self->tabs[TabHome], &self->userData);
}

void deleteAppData(void) {
    AppDelegate *self = getAppDelegate();
    if (userData_clear(&self->userData))
        homeVC_updateWorkoutsList(getIVVC(HomeVC, self->tabs[TabHome]), 0);
    historyVC_clearData(self->tabs[TabHistory]);
    deleteStoredData();
}

void addWorkout(Workout const *workout, uint8_t day, int *weights, bool pop) {
    AppDelegate *self = getAppDelegate();
    if (workout->duration < MinWorkoutDuration) goto cleanup;

    bool updatedWeights = false;
    uint8_t completed = userData_addWorkoutData(&self->userData, day, weights, &updatedWeights);
    if (completed) homeVC_handleFinishedWorkout(self->tabs[TabHome], completed);
    if (updatedWeights && isViewLoaded(self->tabs[TabSettings]))
        inputVC_updateFields(getIVVC(InputVC, self->tabs[TabSettings]), self->userData.lifts);

    saveWorkoutData(workout->duration, workout->type, weights);
cleanup:
    if (pop) {
        id navVC = getNavVC(self->tabs[TabHome]);
        msgV(objSig(id, bool), navVC, sel_getUid("popViewControllerAnimated:"), true);
    }
}
