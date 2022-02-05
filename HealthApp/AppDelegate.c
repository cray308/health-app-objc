#include "AppDelegate.h"
#include "AppCoordinator.h"
#include "AppUserData.h"
#include "InputVC.h"
#include "PersistenceService.h"
#include "SetupWorkoutVC.h"
#include "ViewControllerHelpers.h"
#include "WorkoutVC.h"

#if DEBUG
extern void persistenceService_create(void);
#endif

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
        (((void(*)(id,SEL,int,void(^)(BOOL,id)))objc_msgSend)
         (center, sel_getUid("requestAuthorizationWithOptions:completionHandler:"),
          6, ^(BOOL granted _U_, id error _U_) {}));
    } else {
        tzOffset = userInfo_initFromStorage();
    }

    char const *voidSig = "v@:", *titleForRow = "@@:@qq";
    if (legacy) {
        SEL tint = sel_getUid("tintColorDidChange");
        class_addMethod(DMBackgroundViewClass, tint, (IMP) dmBackgroundView_updateColors, voidSig);
        class_addMethod(DMButtonClass, tint, (IMP) dmButton_updateColors, voidSig);
        class_addMethod(DMLabelClass, tint, (IMP) dmLabel_updateColors, voidSig);
        class_addMethod(DMTextFieldClass, tint, (IMP) dmField_updateColors, voidSig);
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
    objc_registerClassPair(DMBackgroundViewClass);
    objc_registerClassPair(DMButtonClass);
    objc_registerClassPair(DMLabelClass);
    objc_registerClassPair(DMTextFieldClass);
    objc_registerClassPair(SetupWorkoutVCClass);
    SetupWorkoutVCDataRef = class_getInstanceVariable(SetupWorkoutVCClass, "data");

    setTintColor(self->window, createColor(ColorRed));
    id tabVC = createNew(objc_getClass("UITabBarController"));
    void (*fetchHandler)(void*);
    void *arg = appCoordinator_start(tabVC, &fetchHandler);
    setObject(self->window, sel_getUid("setRootViewController:"), tabVC);
    voidFunc(self->window, sel_getUid("makeKeyAndVisible"));
    releaseObj(tabVC);

    persistenceService_start(tzOffset, fetchHandler, arg);
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
