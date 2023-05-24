#include "AppDelegate.h"
#include "DatabaseManager.h"
#include "HistoryVC.h"
#include "HomeVC.h"
#include "InputVC.h"
#include "SettingsVC.h"
#include "SetupWorkoutVC.h"
#include "StatusView.h"
#include "SwiftBridging.h"
#include "WorkoutVC.h"

extern CGFloat UIFontWeightSemibold;

static int orientations;
static int barStyle;
static Class DMNavVC;
static Class UIApplication;
static SEL sharedApplication;
static SEL delegate;
static id (*sharedAppImp)(Class,SEL);
static id (*delegateImp)(id,SEL);

static AppDelegate *getAppDel(void) {
    id app = sharedAppImp(UIApplication, sharedApplication);
    return (AppDelegate *)delegateImp(app, delegate);
}

UserData const *getUserData(void) { return &getAppDel()->userData; }

#pragma mark - VC Helpers

static int getStatusBarStyle(id self _U_, SEL _cmd _U_) { return barStyle; }

static void setupNavBarColor(id bar, CCacheRef clr) {
    const void *keys[] = {NSForegroundColorAttributeName};
    CFDictionaryRef dict = CFDictionaryCreate(NULL, keys, (const void *[]){
        clr->getColor(clr->cls, clr->sc, ColorLabel)
    }, 1, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    msg1(void, CFDictionaryRef, bar, sel_getUid("setTitleTextAttributes:"), dict);
    CFRelease(dict);
}

static void setupBarGeneric(id bar, Class appearanceClass, id color, SEL sbgc) {
    if (appearanceClass) {
        SEL scrollEdge = sel_getUid("setScrollEdgeAppearance:");
        id appearance = Sels.new(appearanceClass, Sels.nw);
        msg0(void, appearance, sel_getUid("configureWithOpaqueBackground"));
        msg1(void, id, appearance, sbgc, color);
        msg1(void, id, bar, sel_getUid("setStandardAppearance:"), appearance);
        if (msg1(bool, SEL, bar, sel_getUid("respondsToSelector:"), scrollEdge))
            msg1(void, id, bar, scrollEdge, appearance);
        Sels.objRel(appearance, Sels.rel);
    } else {
        msg1(void, bool, bar, sel_getUid("setTranslucent:"), false);
        msg1(void, id, bar, sel_getUid("setBarTintColor:"), color);
    }
}

static void setupTabVC(AppDelegate *self, id vc, Class TabAppear) {
    Class NavAppear = objc_getClass("UINavigationBarAppearance");
    id tabBar = msg0(id, vc, sel_getUid("tabBar"));
    id barColor = clsF1(id, int, self->clr.cls, sel_getUid("getBarColorWithType:"), 0);
    SEL nb = sel_getUid("navigationBar");
    setupBarGeneric(tabBar, TabAppear, barColor, self->tbl.view.sbg);
    if (!TabAppear)
        msg1(void, id, tabBar, sel_getUid("setUnselectedItemTintColor:"),
             self->clr.getColor(self->clr.cls, self->clr.sc, ColorGray));
    CFArrayRef ctrls = msg0(CFArrayRef, vc, sel_getUid("viewControllers"));
    for (int i = 0; i < 3; ++i) {
        id navVC = (id)CFArrayGetValueAtIndex(ctrls, i);
        id navBar = msg0(id, navVC, nb);
        setupBarGeneric(navBar, NavAppear, barColor, self->tbl.view.sbg);
        if (!TabAppear) {
            setupNavBarColor(navBar, &self->clr);
            msg0(void, navVC, sel_getUid("setNeedsStatusBarAppearanceUpdate"));
        }
    }
}

static void present(id window, id child) {
    msg1(void, id, window, sel_getUid("setTintColor:"), nil);
    msg3(void, id, bool, id, msg0(id, window, sel_getUid("rootViewController")),
         sel_getUid("presentViewController:animated:completion:"), child, true, nil);
}

void presentVC(id child) { present(getAppDel()->window, child); }

void presentModalVC(id modal) {
    AppDelegate *self = getAppDel();
    id nav = msg1(id, id, Sels.alloc(DMNavVC, Sels.alo),
                  sel_getUid("initWithRootViewController:"), modal);
    id bar = msg0(id, nav, sel_getUid("navigationBar"));
    Class appear = objc_getClass("UINavigationBarAppearance");
    setupBarGeneric(bar, appear, clsF1(id, int, self->clr.cls, sel_getUid("getBarColorWithType:"), 1),
                    self->tbl.view.sbg);
    if (!appear) setupNavBarColor(bar, &self->clr);
    present(self->window, nav);
    Sels.vcRel(nav, Sels.rel);
    Sels.vcRel(modal, Sels.rel);
}

void dismissPresentedVC(Callback handler) {
    AppDelegate *self = getAppDel();
    id window = self->window;
    id root = msg0(id, window, sel_getUid("rootViewController"));
    msg2(void, bool, Callback, root, sel_getUid("dismissViewControllerAnimated:completion:"), true, ^{
        if (handler) handler();
        msg1(void, id, window, sel_getUid("setTintColor:"),
             self->clr.getColor(self->clr.cls, self->clr.sc, ColorRed));
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
    AppDelegate *del = getAppDel();
    id vc = alertCtrlCreate(self, nil, title, message);
    id fg = del->clr.getColor(del->clr.cls, del->clr.sc, ColorLabel);
    const void *keys[] = {NSForegroundColorAttributeName, NSFontAttributeName};
    Class Font = objc_getClass("UIFont");
    SEL gf = sel_getUid("systemFontOfSize:weight:");
    CFDictionaryRef titleDict = CFDictionaryCreate(NULL, keys, (const void *[]){
        fg, clsF2(id, CGFloat, CGFloat, Font, gf, 17, UIFontWeightSemibold)
    }, 2, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFDictionaryRef msgDict = CFDictionaryCreate(NULL, keys, (const void *[]){
        fg, clsF2(id, CGFloat, CGFloat, Font, gf, 13, UIFontWeightRegular)
    }, 2, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFAttributedStringRef titleString = CFAttributedStringCreate(NULL, title, titleDict);
    CFAttributedStringRef msgString = CFAttributedStringCreate(NULL, message, msgDict);
    SEL sKV = sel_getUid("setValue:forKey:"), gsv = sel_getUid("subviews");
    msg2(void, id, CFStringRef, vc, sKV, (id)titleString, CFSTR("attributedTitle"));
    msg2(void, id, CFStringRef, vc, sKV, (id)msgString, CFSTR("attributedMessage"));

    id view = msg0(id, vc, sel_getUid("view"));
    for (int i = 0; i < 3; ++i) {
        view = (id)CFArrayGetValueAtIndex(msg0(CFArrayRef, view, gsv), 0);
    }
    del->tbl.view.setBG(view, del->tbl.view.sbg,
                        del->clr.getColor(del->clr.cls, del->clr.sc, ColorTertiaryBG));

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
        msg1(void, id, self->window, sel_getUid("setTintColor:"),
             self->clr.getColor(self->clr.cls, self->clr.sc, ColorRed));
    });
    msg1(void, id, ctrl, sel_getUid("addAction:"), action);
    CFRelease(title);
}

#pragma mark - Application Delegate

bool appDelegate_didFinishLaunching(AppDelegate *self, SEL _cmd _U_, id app _U_, id opt _U_) {
    DMNavVC = objc_allocateClassPair(objc_getClass("UINavigationController"), "DMNavVC", 0);
    objc_registerClassPair(DMNavVC);
    Class TabAppear = objc_getClass("UITabBarAppearance");
    initNSData(TabAppear, &self->clr, (Class*[]){&View, &VC}, (size_t*[]){&ViewSize, &VCSize});
    Class clsRefs[2]; initViewData(&self->tbl, clsRefs);
    UIApplication = objc_getClass("UIApplication");
    sharedApplication = sel_getUid("sharedApplication");
    delegate = sel_getUid("delegate");
    sharedAppImp = (id(*)(Class,SEL))getImpC(UIApplication, sharedApplication);
    delegateImp = (id(*)(id,SEL))getImpO(UIApplication, delegate);
    orientations = msg0(long, clsF0(id, objc_getClass("UIDevice"), sel_getUid("currentDevice")),
                        sel_getUid("userInterfaceIdiom")) == 1 ? 26 : 2;

    int tzDiff = 0, week = 0;
    id unCenter = clsF0(id, objc_getClass("UNUserNotificationCenter"),
                        sel_getUid("currentNotificationCenter"));
    CFDictionaryRef prefs = CFPreferencesCopyAppValue(PrefsKey, kCFPreferencesCurrentApplication);
    if (prefs != NULL) {
        tzDiff = userData_init(&self->userData, prefs, &week, TabAppear);
    } else {
        userData_create(&self->userData, TabAppear);
        msg2(void, unsigned long, void(^)(bool,id), unCenter,
             sel_getUid("requestAuthorizationWithOptions:completionHandler:"), 6, ^(bool g _U_, id e _U_){});
        createDB();
    }

    initExerciseData(week);
    initWorkoutStrings();
    initStatVData(clsRefs[0]);
    initValidatorStrings(clsRefs[1]);

    IMP imps[] = {(IMP)setupWorkoutVC_numberOfComponents, (IMP)setupWorkoutVC_didSelectRow,
        (IMP)setupWorkoutVC_getTitle, (IMP)alertCtrlCreate
    };
    SEL pickerSel = sel_getUid("pickerView:titleForRow:forComponent:");
    if (!TabAppear) {
        barStyle = self->userData.darkMode ? 2 : 0;
        class_addMethod(DMNavVC, sel_getUid("preferredStatusBarStyle"), (IMP)getStatusBarStyle, "i@:");
        imps[0] = (IMP)setupWorkoutVC_numberOfComponentsLegacy;
        imps[1] = (IMP)setupWorkoutVC_didSelectRowLegacy;
        imps[2] = (IMP)setupWorkoutVC_getAttrTitle;
        imps[3] = (IMP)alertCtrlCreateLegacy;
        pickerSel = sel_getUid("pickerView:attributedTitleForRow:forComponent:");
        setupAppColors(self->clr.cls, self->userData.darkMode, false);
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
    self->children[0] = homeVC_init(&self->tbl, &self->clr);
    self->children[1] = historyVC_init(&historyModel, &self->tbl, &self->clr);
    self->children[2] = settingsVC_init(&self->tbl, &self->clr);

    Class Item = objc_getClass("UITabBarItem"), Image = objc_getClass("UIImage");
    SEL itemInit = sel_getUid("initWithTitle:image:tag:"), setItem = sel_getUid("setTabBarItem:");
    SEL imgInit = sel_getUid("imageNamed:"), navInit = sel_getUid("initWithRootViewController:");
    id controllers[3];
    CFStringRef imgNames[] = {CFSTR("ico_house"), CFSTR("ico_chart"), CFSTR("ico_gear")};
    CFStringRef titles[3];
    fillStringArray(titles, CFSTR("tabs%d"), 3);

    for (int i = 0; i < 3; ++i) {
        id img = clsF1(id, CFStringRef, Image, imgInit, imgNames[i]);
        id item = msg3(id, CFStringRef, id, long, Sels.alloc(Item, Sels.alo),
                       itemInit, titles[i], img, i);
        controllers[i] = msg1(id, id, Sels.alloc(DMNavVC, Sels.alo), navInit, self->children[i]);
        msg1(void, id, controllers[i], setItem, item);
        Sels.objRel(item, Sels.rel);
        CFRelease(titles[i]);
    }

    self->window = Sels.new(objc_getClass("UIWindow"), Sels.nw);
    msg1(void, id, self->window, sel_getUid("setTintColor:"),
         self->clr.getColor(self->clr.cls, self->clr.sc, ColorRed));
    id tabVC = Sels.new(objc_getClass("UITabBarController"), Sels.nw);
    msg1(void, id, self->window, sel_getUid("setRootViewController:"), tabVC);
    CFArrayRef array = CFArrayCreate(NULL, (const void **)controllers, 3, NULL);
    msg2(void, CFArrayRef, bool, tabVC, sel_getUid("setViewControllers:animated:"), array, false);
    setupTabVC(self, tabVC, TabAppear);
    for (int i = 0; i < 3; ++i) {
        Sels.vcRel(controllers[i], Sels.rel);
    }
    CFRelease(array);
    Sels.vcRel(tabVC, Sels.rel);
    msg0(void, self->window, sel_getUid("makeKeyAndVisible"));
    runStartupJob(historyModel, self->userData.weekStart, tzDiff);
    return true;
}

int appDelegate_supportedOrientations(AppDelegate *self _U_, SEL _cmd _U_, id app _U_, id win _U_) {
    return orientations;
}

#pragma mark - Child VC Callbacks

void updateUserInfo(uint8_t plan, uint8_t darkMode, int const *weights) {
    AppDelegate *self = getAppDel();
    uint8_t updates = userData_update(&self->userData, plan, darkMode, weights);
    if (updates & MaskCurrentPlan)
        homeVC_createWorkoutsList(self->children[0], &self->userData);

    if (updates & MaskDarkMode) {
        barStyle = darkMode ? 2 : 0;
        setupAppColors(self->clr.cls, darkMode, true);
        msg1(void, id, self->window, sel_getUid("setTintColor:"),
             self->clr.getColor(self->clr.cls, self->clr.sc, ColorRed));
        setupTabVC(self, msg0(id, self->window, sel_getUid("rootViewController")),
                   objc_getClass("UITabBarAppearance"));
        setupCharts(darkMode);
        homeVC_updateColors(self->children[0]);
        if (msg0(bool, self->children[1], sel_getUid("isViewLoaded")))
            historyVC_updateColors(self->children[1], darkMode);
        settingsVC_updateColors(self->children[2], darkMode);
    }
}

void deleteAppData(void) {
    AppDelegate *self = getAppDel();
    if (userData_clear(&self->userData))
        homeVC_updateWorkoutsList((HomeVC *)((char *)self->children[0] + VCSize), 0);
    historyVC_clearData(self->children[1]);
    deleteStoredData();
}

void addWorkoutData(Workout const *workout, uint8_t day, int *weights, bool pop) {
    AppDelegate *self = getAppDel();
    if (workout->duration < MinWorkoutDuration) goto cleanup;

    bool updatedWeights = false;
    uint8_t completed = userData_addWorkoutData(&self->userData, day, weights, &updatedWeights);
    if (completed) homeVC_handleFinishedWorkout(self->children[0], completed);
    if (updatedWeights) {
        id vc = self->children[2];
        if (msg0(bool, vc, sel_getUid("isViewLoaded")))
            inputVC_updateFields((InputVC *)((char *)vc + VCSize), self->userData.lifts);
    }

    saveWorkoutData(workout->duration, workout->type, weights);
cleanup:
    if (pop) {
        id navVC = msg0(id, self->children[0], sel_getUid("navigationController"));
        msg1(id, bool, navVC, sel_getUid("popViewControllerAnimated:"), true);
    }
}
