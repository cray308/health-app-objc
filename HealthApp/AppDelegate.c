#include "AppDelegate.h"
#include "HistoryVC.h"
#include "HomeVC.h"
#include "InputVC.h"
#include "PersistenceManager.h"
#include "SettingsVC.h"
#include "StatusView.h"
#include "Views_VCExt.h"
#include "WorkoutVC.h"

enum {
    UIUserInterfaceIdiomPad = 1
};

enum {
    UIInterfaceOrientationMaskPortrait = 2,
    UIInterfaceOrientationMaskAllButUpsideDown = 26
};

enum {
    UNAuthorizationOptionSound = 2,
    UNAuthorizationOptionAlert = 4
};

enum {
    IndexHome,
    IndexHistory,
    IndexSettings
};

static u_long orientations = UIInterfaceOrientationMaskPortrait;
static Class UIApplication;
static SEL sharedApplication;
static SEL delegate;
static id (*sharedAppImp)(Class,SEL);
static id (*delegateImp)(id,SEL);

static AppDelegate *getAppDel(void) {
    return (AppDelegate *)delegateImp(sharedAppImp(UIApplication, sharedApplication), delegate);
}

id getAppWindow(void) { return getAppDel()->window; }

UserInfo const *getUserInfo(void) { return &getAppDel()->userData; }

#pragma mark - Application Delegate

bool appDelegate_didFinishLaunching(AppDelegate *self, SEL _cmd _U_, id app _U_, id opt _U_) {
    Class TabAppear = objc_getClass("UITabBarAppearance");
    initNSData(TabAppear, (Class*[]){&View, &VC}, (size_t*[]){&ViewSize, &VCSize});
    initViewData((void(*[])(void)){initStatusViewData, initValidatorData});
    UIApplication = objc_getClass("UIApplication");
    sharedApplication = sel_getUid("sharedApplication");
    delegate = sel_getUid("delegate");
    sharedAppImp = (id(*)(Class,SEL))getImpC(UIApplication, sharedApplication);
    delegateImp = (id(*)(id,SEL))getImpO(UIApplication, delegate);
    id dev = clsF0(id, objc_getClass("UIDevice"), sel_getUid("currentDevice"));
    if (msg0(long, dev, sel_getUid("userInterfaceIdiom")) == UIUserInterfaceIdiomPad)
        orientations = UIInterfaceOrientationMaskAllButUpsideDown;

    int tzDiff = 0, week = 0;
    time_t weekStart;
    id unc = clsF0(id, objc_getClass("UNUserNotificationCenter"), sel_getUid("currentNotificationCenter"));
    CFBooleanRef saved = CFPreferencesCopyAppValue(HasLaunchedKey, kCFPreferencesCurrentApplication);
    if (saved != NULL) {
        CFRelease(saved);
        tzDiff = userInfo_init(&self->userData, &weekStart, &week, TabAppear);
        self->context = context_init();
    } else {
        userInfo_create(&self->userData, &weekStart, TabAppear);
        u_long options = UNAuthorizationOptionSound | UNAuthorizationOptionAlert;
        msg2(void, u_long, void(^)(bool,id), unc,
             sel_getUid("requestAuthorizationWithOptions:completionHandler:"), options, ^(bool g _U_, id e _U_){});
        self->context = context_init_first_launch();
    }

    bool ltr = initVCData(TabAppear, self->userData.darkMode, initSetupWorkoutVCData);
    initWorkoutData(week, &toSavedMass);

    HistoryModel *model;
    self->children[IndexHome] = homeVC_init(weekStart + (DaySeconds >> 1));
    self->children[IndexHistory] = historyVC_init(&model);
    self->children[IndexSettings] = new(SettingsVCClass);

    Class Item = objc_getClass("UITabBarItem");
    SEL itemInit = sel_getUid("initWithTitle:image:tag:"), setItem = sel_getUid("setTabBarItem:");
    SEL navInit = sel_getUid("initWithRootViewController:");
    id controllers[3];
    CFStringRef imgs[] = {CFSTR("ico_house"), CFSTR("ico_chart"), CFSTR("ico_gear")}, titles[3];
    fillStringArray(titles, CFSTR("tabs%d"), 3);

    for (int i = 0; i < 3; ++i) {
        id _i = alloc(Item);
        id item = msg3(id, CFStringRef, id, long, _i, itemInit, titles[i], getImg(imgs[i]), i);
        controllers[i] = msg1(id, id, alloc(DMNavVC), navInit, self->children[i]);
        msg1(void, id, controllers[i], setItem, item);
        releaseO(item);
        CFRelease(titles[i]);
    }

    self->window = new(objc_getClass("UIWindow"));
    msg1(void, id, self->window, sel_getUid("setTintColor:"), getColor(ColorRed));
    id tabVC = new(objc_getClass("UITabBarController"));
    msg1(void, id, self->window, sel_getUid("setRootViewController:"), tabVC);
    CFArrayRef array = CFArrayCreate(NULL, (const void **)controllers, 3, NULL);
    msg2(void, CFArrayRef, bool, tabVC, sel_getUid("setViewControllers:animated:"), array, false);
    setupTabVC(tabVC, TabAppear);
    for (int i = 0; i < 3; ++i) {
        releaseVC(controllers[i]);
    }
    CFRelease(array);
    releaseVC(tabVC);
    msg0(void, self->window, sel_getUid("makeKeyAndVisible"));
    setDelegate(unc, (id)self);
    runStartupJob(self->context, model, weekStart, tzDiff, ltr);
    return true;
}

u_long appDelegate_supportedOrientations(id self _U_, SEL _cmd _U_, id app _U_, id win _U_) {
    return orientations;
}

void appDelegate_receivedNotif(AppDelegate *self, SEL _cmd _U_,
                               id unc _U_, id notif, void (^callback)(u_long)) {
    id req = msg0(id, notif, sel_getUid("request"));
    int identifier = CFStringGetIntValue(msg0(CFStringRef, req, sel_getUid("identifier")));
    id nav = msg0(id, self->children[IndexHome], sel_getUid("navigationController"));
    CFArrayRef ctrls = msg0(CFArrayRef, nav, sel_getUid("viewControllers"));
    if (CFArrayGetCount(ctrls) == 2)
        workoutVC_receivedNotif((id)CFArrayGetValueAtIndex(ctrls, 1), identifier);
    callback(0);
}

#pragma mark - Child VC Callbacks

void updateUserInfo(unsigned char plan, unsigned char darkMode, short *weights) {
    AppDelegate *self = getAppDel();
    unsigned char res = userInfo_update(&self->userData, plan, darkMode, weights);
    if (res & MaskCurrentPlan) homeVC_createWorkoutsList(self->children[IndexHome], &self->userData);

    if (res & MaskDarkMode) {
        handleTintChange(self->window, darkMode);
        homeVC_updateColors(self->children[IndexHome]);
        if (msg0(bool, self->children[IndexHistory], sel_getUid("isViewLoaded")))
            historyVC_updateColors(self->children[IndexHistory], darkMode);
        settingsVC_updateColors(self->children[IndexSettings], darkMode);
    }
}

void deleteAppData(void) {
    AppDelegate *self = getAppDel();
    if (userInfo_deleteData(&self->userData))
        homeVC_updateWorkoutsList((HomeVC *)getIVVC(self->children[IndexHome]), 0);
    historyVC_clearData(self->children[IndexHistory]);

    deleteStoredData(self->context);
}

void addWorkoutData(id caller, Workout *workout, unsigned char day, short *weights, bool pop) {
    id navVC = msg0(id, caller, sel_getUid("navigationController"));
    if (workout->duration < MinWorkoutDuration) goto cleanup;

    AppDelegate *self = getAppDel();
    bool updatedWeights = false;
    unsigned char completed = userInfo_addWorkoutData(&self->userData, day, weights, &updatedWeights);
    if (completed) {
        CFArrayRef ctrls = msg0(CFArrayRef, navVC, sel_getUid("viewControllers"));
        homeVC_handleFinishedWorkout((id)CFArrayGetValueAtIndex(ctrls, 0), completed);
    }
    if (updatedWeights) {
        id vc = self->children[IndexSettings];
        if (msg0(bool, vc, sel_getUid("isViewLoaded")))
            inputVC_updateFields((InputVC *)getIVVC(vc), self->userData.liftMaxes);
    }

    saveWorkoutData(self->context, workout->duration, workout->type, weights);
cleanup:
    if (pop) msg1(id, bool, navVC, sel_getUid("popViewControllerAnimated:"), true);
}
