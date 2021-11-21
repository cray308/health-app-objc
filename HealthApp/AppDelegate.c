#include "AppDelegate.h"
#include "AppCoordinator.h"
#include "AppUserData.h"
#include "PersistenceService.h"
#include "ViewControllerHelpers.h"

extern void initWorkoutStrings(void);
extern void initValidatorStrings(void);
extern void handleIOSVersion(void);
extern void historyCoordinator_fetchData(void*);

#if DEBUG
extern void persistenceService_create(void);
#endif

Class AppDelegateClass;

bool appDelegate_didFinishLaunching(AppDelegate *self, SEL _cmd _U_,
                                    id application _U_, id options _U_) {
    initValidatorStrings();
    initWorkoutStrings();
    handleIOSVersion();
    CGRect bounds;
    getScreenBounds(&bounds);
    self->window = createObjectWithFrame(objc_getClass("UIWindow"), bounds);

    CFStringRef hasLaunchedKey = CFSTR("hasLaunched");
    id defaults = getUserDefaults();
    bool hasLaunched = ((bool(*)(id,SEL,CFStringRef))objc_msgSend)
    (defaults, sel_getUid("boolForKey:"), hasLaunchedKey);

    persistenceService_init();
    int tzOffset = 0;

    if (!hasLaunched) {
        ((void(*)(id,SEL,bool,CFStringRef))objc_msgSend)
        (defaults, sel_getUid("setBool:forKey:"), true, hasLaunchedKey);
        userInfo_create();
#if DEBUG
        persistenceService_create();
#endif
        id center = getNotificationCenter();
        ((void(*)(id,SEL,int,void(^)(BOOL,id)))objc_msgSend)
        (center, sel_getUid("requestAuthorizationWithOptions:completionHandler:"),
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
        char const *colorField = "colorCode", *voidSig = "v@:";
        class_addIvar(DMBackgroundViewClass, colorField, sizeof(int), 0, "i");
        class_addIvar(DMButtonClass, colorField, sizeof(int), 0, "i");
        class_addIvar(DMButtonClass, "background", sizeof(bool), 0, "c");
        class_addIvar(DMLabelClass, colorField, sizeof(int), 0, "i");

        SEL tint = sel_getUid("tintColorDidChange");
        class_addMethod(DMBackgroundViewClass, tint, (IMP) dmBackgroundView_updateColors, voidSig);
        class_addMethod(DMButtonClass, tint, (IMP) dmButton_updateColors, voidSig);
        class_addMethod(DMLabelClass, tint, (IMP) dmLabel_updateColors, voidSig);
        class_addMethod(DMTextFieldClass, tint, (IMP) dmField_updateColors, voidSig);
        class_addMethod(DMTabVC, sel_getUid("viewDidLayoutSubviews"),
                        (IMP) dmTabVC_updateColors, voidSig);
    }

    objc_registerClassPair(DMTabVC);
    objc_registerClassPair(DMBackgroundViewClass);
    objc_registerClassPair(DMButtonClass);
    objc_registerClassPair(DMLabelClass);
    objc_registerClassPair(DMTextFieldClass);

    if (osVersion < Version14)
        setTintColor(self->window, createColor(ColorRed));
    id tabVC = getObject(allocClass(DMTabVC), sel_getUid("init"));
    appCoordinator_start(tabVC);
    setObject(self->window, sel_getUid("setRootViewController:"), tabVC);
    voidFunc(self->window, sel_getUid("makeKeyAndVisible"));
    releaseObj(tabVC);

    persistenceService_start(tzOffset, historyCoordinator_fetchData, appCoordinator->children[1]);
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
