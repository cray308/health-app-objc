#include "AppDelegate.h"
#include "DatabaseManager.h"
#include "HistoryVC.h"
#include "HomeVC.h"
#include "InputVC.h"
#include "SettingsVC.h"
#include "SetupWorkoutVC.h"
#include "SwiftBridging.h"
#include "WorkoutVC.h"

static int barStyle;
static Class DMNavVC;

static AppDelegate *getAppDel(void) {
    id shared = clsF0(id, objc_getClass("UIApplication"), sel_getUid("sharedApplication"));
    return (AppDelegate *)msg0(id, shared, sel_getUid("delegate"));
}

UserData const *getUserData(void) { return &getAppDel()->userData; }

#pragma mark - VC Helpers

static int getStatusBarStyle(id self _U_, SEL _cmd _U_) { return barStyle; }

static void setupNavBarColor(id bar) {
    const void *keys[] = {NSForegroundColorAttributeName};
    CFDictionaryRef dict = CFDictionaryCreate(NULL, keys, (const void *[]){
        getColor(ColorLabel)
    }, 1, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    msg1(void, CFDictionaryRef, bar, sel_getUid("setTitleTextAttributes:"), dict);
    CFRelease(dict);
}

static void setupBarGeneric(id bar, Class appearanceClass, id color) {
    if (appearanceClass) {
        SEL scrollEdge = sel_getUid("setScrollEdgeAppearance:");
        id appearance = new(appearanceClass);
        msg0(void, appearance, sel_getUid("configureWithOpaqueBackground"));
        msg1(void, id, appearance, SetBackgroundSel, color);
        msg1(void, id, bar, sel_getUid("setStandardAppearance:"), appearance);
        if (respondsToSelector(bar, scrollEdge))
            msg1(void, id, bar, scrollEdge, appearance);
        releaseObject(appearance);
    } else {
        msg1(void, bool, bar, sel_getUid("setTranslucent:"), false);
        setBarTintColor(bar, color);
    }
}

static void setupTabVC(id vc, Class TabAppear) {
    Class NavAppear = objc_getClass("UINavigationBarAppearance");
    id tabBar = msg0(id, vc, sel_getUid("tabBar"));
    id barColor = getBarColor(BarColorNav);
    SEL nb = sel_getUid("navigationBar");
    setupBarGeneric(tabBar, TabAppear, barColor);
    if (!TabAppear)
        msg1(void, id, tabBar, sel_getUid("setUnselectedItemTintColor:"), getColor(ColorGray));
    CFArrayRef ctrls = msg0(CFArrayRef, vc, sel_getUid("viewControllers"));
    for (int i = 0; i < 3; ++i) {
        id navVC = (id)CFArrayGetValueAtIndex(ctrls, i);
        id navBar = msg0(id, navVC, nb);
        setupBarGeneric(navBar, NavAppear, barColor);
        if (!TabAppear) {
            setupNavBarColor(navBar);
            msg0(void, navVC, sel_getUid("setNeedsStatusBarAppearanceUpdate"));
        }
    }
}

static void present(id window, id child) {
    setTintColor(window, nil);
    msg3(void, id, bool, id, msg0(id, window, sel_getUid("rootViewController")),
         sel_getUid("presentViewController:animated:completion:"), child, true, nil);
}

void presentVC(id child) { present(getAppDel()->window, child); }

void presentModalVC(id modal) {
    AppDelegate *self = getAppDel();
    id nav = msg1(id, id, alloc(DMNavVC), sel_getUid("initWithRootViewController:"), modal);
    id bar = msg0(id, nav, sel_getUid("navigationBar"));
    Class appear = objc_getClass("UINavigationBarAppearance");
    setupBarGeneric(bar, appear, getBarColor(BarColorModal));
    if (!appear) setupNavBarColor(bar);
    present(self->window, nav);
    releaseVC(nav);
    releaseVC(modal);
}

void dismissPresentedVC(Callback handler) {
    AppDelegate *self = getAppDel();
    id window = self->window;
    id root = msg0(id, window, sel_getUid("rootViewController"));
    msg2(void, bool, Callback, root, sel_getUid("dismissViewControllerAnimated:completion:"), true, ^{
        if (handler) handler();
        setTintColor(window, getColor(ColorRed));
    });
}

id createAlertController(CFStringRef titleKey, CFStringRef msgKey) {
    CFStringRef title = localize(titleKey), message = localize(msgKey);
    id ctrl = clsF2(id, CFStringRef, CFStringRef, objc_getClass("UIAlertController"),
                    sel_getUid("getCtrlWithTitle:message:"), title, message);
    CFRelease(title);
    CFRelease(message);
    return ctrl;
}

static id alertCtrlCreate(id self, SEL _cmd _U_, CFStringRef title, CFStringRef message) {
    return clsF3(id, CFStringRef, CFStringRef, long, (Class)self,
                 sel_getUid("alertControllerWithTitle:message:preferredStyle:"), title, message, 1);
}

static id alertCtrlCreateLegacy(id self, SEL _cmd _U_, CFStringRef title, CFStringRef message) {
    id vc = alertCtrlCreate(self, nil, title, message);
    id fg = getColor(ColorLabel);
    const void *keys[] = {NSForegroundColorAttributeName, NSFontAttributeName};
    CFDictionaryRef titleDict = CFDictionaryCreate(NULL, keys, (const void *[]){
        fg, getSystemFont(17, UIFontWeightSemibold)
    }, 2, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFDictionaryRef msgDict = CFDictionaryCreate(NULL, keys, (const void *[]){
        fg, getSystemFont(FontSizeSmall, UIFontWeightRegular)
    }, 2, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFAttributedStringRef titleString = CFAttributedStringCreate(NULL, title, titleDict);
    CFAttributedStringRef msgString = CFAttributedStringCreate(NULL, message, msgDict);
    SEL sKV = sel_getUid("setValue:forKey:");
    msg2(void, id, CFStringRef, vc, sKV, (id)titleString, CFSTR("attributedTitle"));
    msg2(void, id, CFStringRef, vc, sKV, (id)msgString, CFSTR("attributedMessage"));

    id view = getView(vc);
    for (int i = 0; i < 3; ++i) {
        view = (id)CFArrayGetValueAtIndex(getSubviews(view), 0);
    }
    setBackgroundColor(view, getColor(ColorTertiaryBG));

    CFRelease(titleDict);
    CFRelease(msgDict);
    CFRelease(titleString);
    CFRelease(msgString);
    return vc;
}

void addAlertAction(id ctrl, CFStringRef titleKey, int style, Callback handler) {
    CFStringRef title = localize(titleKey);
    id action = clsF3(id, CFStringRef, long, void(^)(id), objc_getClass("UIAlertAction"),
                      sel_getUid("actionWithTitle:style:handler:"), title, style, ^(id hdlr _U_) {
        if (handler) handler();
        AppDelegate *self = getAppDel();
        setTintColor(self->window, getColor(ColorRed));
    });
    msg1(void, id, ctrl, sel_getUid("addAction:"), action);
    CFRelease(title);
}

#pragma mark - Application Delegate

bool appDelegate_didFinishLaunching(AppDelegate *self, SEL _cmd _U_, id app _U_, id opt _U_) {
    DMNavVC = objc_allocateClassPair(objc_getClass("UINavigationController"), "DMNavVC", 0);
    objc_registerClassPair(DMNavVC);
    Class TabBarAppearance = getTabBarAppearanceClass();
    int tzDiff = 0, week = 0;
    id unCenter = getNotificationCenter();
    CFDictionaryRef prefs = CFPreferencesCopyAppValue(PrefsKey, kCFPreferencesCurrentApplication);
    if (prefs != NULL) {
        tzDiff = userData_init(&self->userData, prefs, &week, TabBarAppearance);
    } else {
        userData_create(&self->userData, TabBarAppearance);
        msg2(void, u_long, void(^)(bool, id), unCenter,
             sel_getUid("requestAuthorizationWithOptions:completionHandler:"),
             6, ^(bool granted _U_, id err _U_){});
        createDB();
    }

    initWorkoutData(week);

    IMP imps[] = {(IMP)setupWorkoutVC_numberOfComponents, (IMP)setupWorkoutVC_didSelectRow,
        (IMP)setupWorkoutVC_getTitle, (IMP)alertCtrlCreate
    };
    SEL pickerSel = sel_getUid("pickerView:titleForRow:forComponent:");
    if (!TabBarAppearance) {
        barStyle = self->userData.darkMode ? 2 : 0;
        class_addMethod(DMNavVC, sel_getUid("preferredStatusBarStyle"), (IMP)getStatusBarStyle, "i@:");
        imps[0] = (IMP)setupWorkoutVC_numberOfComponentsLegacy;
        imps[1] = (IMP)setupWorkoutVC_didSelectRowLegacy;
        imps[2] = (IMP)setupWorkoutVC_getAttrTitle;
        imps[3] = (IMP)alertCtrlCreateLegacy;
        pickerSel = sel_getUid("pickerView:attributedTitleForRow:forComponent:");
        setupAppColors(self->userData.darkMode);
    }
    class_addMethod(SetupWorkoutVCClass, sel_getUid("numberOfComponentsInPickerView:"),
                    imps[0], "q@:@");
    class_addMethod(SetupWorkoutVCClass, sel_getUid("pickerView:didSelectRow:inComponent:"),
                    imps[1], "v@:@qq");
    class_addMethod(SetupWorkoutVCClass, pickerSel, imps[2], "@@:@qq");
    class_addMethod(objc_getMetaClass("UIAlertController"),
                    sel_registerName("getCtrlWithTitle:message:"), imps[3], "@@:@@");
    setupCharts(self->userData.darkMode);

    HistoryModel *historyModel;
    self->tabs[0] = new(HomeVCClass);
    self->tabs[1] = historyVC_init(&historyModel);
    self->tabs[2] = new(SettingsVCClass);

    Class TabBarItem = objc_getClass("UITabBarItem");
    SEL iItem = sel_getUid("initWithTitle:image:tag:");
    id (*itemInit)(id, SEL, CFStringRef, id, long) =
      (id(*)(id, SEL, CFStringRef, id, long))class_getMethodImplementation(TabBarItem, iItem);
    SEL navInit = sel_getUid("initWithRootViewController:");
    SEL nstbi = sel_getUid("setTabBarItem:");
    void (*setItem)(id, SEL, id) = (void(*)(id, SEL, id))class_getMethodImplementation(DMNavVC, nstbi);
    id controllers[3];
    CFStringRef imageNames[] = {CFSTR("ico_house"), CFSTR("ico_chart"), CFSTR("ico_gear")}, titles[3];
    fillStringArray(titles, CFSTR("tabs%d"), 3);

    for (int i = 0; i < 3; ++i) {
        id item = itemInit(alloc(TabBarItem), iItem, titles[i], getImage(imageNames[i]), i);
        controllers[i] = msg1(id, id, alloc(DMNavVC), navInit, self->tabs[i]);
        setItem(controllers[i], nstbi, item);
        releaseObject(item);
        CFRelease(titles[i]);
    }

    self->window = new(objc_getClass("UIWindow"));
    setTintColor(self->window, getColor(ColorRed));
    id tabVC = new(objc_getClass("UITabBarController"));
    msg1(void, id, self->window, sel_getUid("setRootViewController:"), tabVC);
    CFArrayRef vcArr = CFArrayCreate(NULL, (const void **)controllers, 3, NULL);
    msg1(void, CFArrayRef, tabVC, sel_getUid("setViewControllers:"), vcArr);
    setupTabVC(tabVC, TabBarAppearance);
    for (int i = 0; i < 3; ++i) {
        releaseVC(controllers[i]);
    }
    CFRelease(vcArr);
    releaseVC(tabVC);
    msg0(void, self->window, sel_getUid("makeKeyAndVisible"));
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
        barStyle = darkMode ? 2 : 0;
        setupAppColors(darkMode);
        setTintColor(self->window, getColor(ColorRed));
        setupTabVC(msg0(id, self->window, sel_getUid("rootViewController")),
                   getTabBarAppearanceClass());
        setupCharts(darkMode);
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
        msg1(id, bool, navVC, sel_getUid("popViewControllerAnimated:"), true);
    }
}
