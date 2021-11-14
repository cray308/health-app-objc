//
//  AppDelegate.c
//  HealthApp
//
//  Created by Christopher Ray on 3/20/21.
//

#include "AppDelegate.h"
#include <CoreFoundation/CFString.h>
#include <objc/message.h>
#include "ViewControllerHelpers.h"

extern void initExerciseStrings(void);
extern void initCircuitStrings(void);
extern void initTimerStrings(void);
extern void initExerciseMgrStrings(void);
extern void initValidatorStrings(void);
extern void userInfo_create(void);
extern int userInfo_initFromStorage(void);
extern void persistenceService_init(void);
extern void persistenceService_start(int tzOffset);
extern void appCoordinator_start(id tabVC);
extern void appCoordinator_fetchHistory(void);
extern void handleIOSVersion(void);

#if DEBUG
extern void persistenceService_create(void);
#endif

Class AppDelegateClass;

bool appDelegate_didFinishLaunching(AppDelegate *self, SEL _cmd _U_,
                                    id application _U_, id options _U_) {
    initValidatorStrings();
    initExerciseStrings();
    initCircuitStrings();
    initTimerStrings();
    initExerciseMgrStrings();
    handleIOSVersion();
    CGRect bounds;
    getScreenBounds(&bounds);
    self->window = createObjectWithFrame("UIWindow", bounds);

    CFStringRef hasLaunchedKey = CFSTR("hasLaunched");
    bool hasLaunched = ((bool(*)(id,SEL,CFStringRef))objc_msgSend)
    (getUserDefaults(), sel_getUid("boolForKey:"), hasLaunchedKey);

    persistenceService_init();
    int tzOffset = 0;

    if (!hasLaunched) {
        ((void(*)(id,SEL,bool,CFStringRef))objc_msgSend)
        (getUserDefaults(), sel_getUid("setBool:forKey:"), true, hasLaunchedKey);
        userInfo_create();
#if DEBUG
        persistenceService_create();
#endif
        ((void(*)(id,SEL,int,void(^)(BOOL,id)))objc_msgSend)
        (getNotificationCenter(), sel_getUid("requestAuthorizationWithOptions:completionHandler:"),
         6, ^(BOOL granted _U_, id error _U_) {});
    } else {
        tzOffset = userInfo_initFromStorage();
    }

    DMTabVC = objc_allocateClassPair(objc_getClass("UITabBarController"), "DMTabVC", 0);
    DMButtonClass = objc_allocateClassPair(objc_getClass("UIButton"), "DMButton", 0);
    DMLabelClass = objc_allocateClassPair(objc_getClass("UILabel"), "DMLabel", 0);
    DMTextFieldClass = objc_allocateClassPair(objc_getClass("UITextField"), "DMTextField", 0);
    DMBackgroundViewClass = objc_allocateClassPair(objc_getClass("UIView"), "DMBackgroundView", 0);

    if (osVersion == Version12) {
        class_addIvar(DMBackgroundViewClass, "colorType", sizeof(bool), 0, "c");
        class_addIvar(DMButtonClass, "colorCode", sizeof(int), 0, "i");
        class_addIvar(DMButtonClass, "background", sizeof(bool), 0, "c");
        class_addIvar(DMLabelClass, "colorCode", sizeof(int), 0, "i");

        SEL tint = sel_getUid("tintColorDidChange");
        class_addMethod(DMBackgroundViewClass, tint, (IMP) dmBackgroundView_updateColors, "v@:");
        class_addMethod(DMButtonClass, tint, (IMP) dmButton_updateColors, "v@:");
        class_addMethod(DMLabelClass, tint, (IMP) dmLabel_updateColors, "v@:");
        class_addMethod(DMTextFieldClass, tint, (IMP) dmField_updateColors, "v@:");
        class_addMethod(DMTabVC, sel_getUid("viewDidLayoutSubviews"),
                        (IMP) dmTabVC_updateColors, "v@:");
    }

    objc_registerClassPair(DMTabVC);
    objc_registerClassPair(DMBackgroundViewClass);
    objc_registerClassPair(DMButtonClass);
    objc_registerClassPair(DMLabelClass);
    objc_registerClassPair(DMTextFieldClass);

    if (osVersion < Version14)
        setTintColor(self->window, createColor(ColorRed));
    id tabVC = getObject(allocClass("DMTabVC"), sel_getUid("init"));
    appCoordinator_start(tabVC);
    setObject(self->window, sel_getUid("setRootViewController:"), tabVC);
    voidFunc(self->window, sel_getUid("makeKeyAndVisible"));
    releaseObj(tabVC);

    persistenceService_start(tzOffset);
    appCoordinator_fetchHistory();
    return true;
}

int appDelegate_supportedOrientations(AppDelegate *self _U_, SEL _cmd _U_,
                                      id application _U_, id window _U_) {
    id device = staticMethod(objc_getClass("UIDevice"), sel_getUid("currentDevice"));
    int idiom = getInt(device, sel_getUid("userInterfaceIdiom"));
    return idiom == 1 ? 26 : 2;
}

id appDel_setWindowTint(id color) {
    id app = staticMethod(objc_getClass("UIApplication"), sel_getUid("sharedApplication"));
    AppDelegate *delegate = (AppDelegate *) getObject(app, sel_getUid("delegate"));
    setTintColor(delegate->window, color);
    return delegate->window;
}
