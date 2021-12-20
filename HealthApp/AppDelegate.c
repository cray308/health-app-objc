#include "AppDelegate.h"
#include "AppCoordinator.h"
#include "AppTypes.h"
#include "AppUserData.h"
#include "InputVC.h"
#include "PersistenceService.h"
#include "SetupWorkoutVC.h"
#include "ViewControllerHelpers.h"
#include "WorkoutVC.h"

#define TITLE_FOR_ROW "@@:@" LHASymbol LHASymbol
#define NUM_COMPONENTS LHASymbol "@:@"
#define NUM_ROWS LHASymbol "@:@" LHASymbol
#define DID_SELECT_ROW "v@:@" LHASymbol LHASymbol

#if DEBUG
extern void persistenceService_create(void);
#endif

Class AppDelegateClass;

bool appDelegate_didFinishLaunching(AppDelegate *self, SEL _cmd _U_,
                                    id application _U_, id options _U_) {
    initValidatorStrings();
    initExerciseStrings();
    initWorkoutStrings();
    bool setWindowTint;
    bool legacy = handleIOSVersion(&setWindowTint);
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
        userInfo_create(legacy);
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

    char const *voidSig = "v@:", *tapSig = "v@:@", *dataKey = "data";
    SetupWorkoutVCClass = objc_allocateClassPair(InputVCClass, "SetupWorkoutVC", 0);
    class_addProtocol(SetupWorkoutVCClass, objc_getProtocol("UIPickerViewDelegate"));
    class_addProtocol(SetupWorkoutVCClass, objc_getProtocol("UIPickerViewDataSource"));
    class_addIvar(SetupWorkoutVCClass, dataKey, sizeof(SetupWorkoutVCData*), 0, "^{?=@@@Ci}");
    class_addMethod(SetupWorkoutVCClass, sel_getUid("dealloc"),
                    (IMP) setupWorkoutVC_deinit, voidSig);
    class_addMethod(SetupWorkoutVCClass, sel_getUid("viewDidLoad"),
                    (IMP) setupWorkoutVC_viewDidLoad, voidSig);
    class_addMethod(SetupWorkoutVCClass, sel_getUid("buttonTapped:"),
                    (IMP) setupWorkoutVC_tappedButton, tapSig);
    class_addMethod(SetupWorkoutVCClass, sel_getUid("numberOfComponentsInPickerView:"),
                    (IMP) setupWorkoutVC_numberOfComponents, NUM_COMPONENTS);
    class_addMethod(SetupWorkoutVCClass, sel_getUid("pickerView:numberOfRowsInComponent:"),
                    (IMP) setupWorkoutVC_numberOfRows, NUM_ROWS);
    class_addMethod(SetupWorkoutVCClass, sel_getUid("pickerView:didSelectRow:inComponent:"),
                    (IMP) setupWorkoutVC_didSelectRow, DID_SELECT_ROW);

    DMNavVC = objc_allocateClassPair(objc_getClass("UINavigationController"), "DMNavVC", 0);
    DMTabVC = objc_allocateClassPair(objc_getClass("UITabBarController"), "DMTabVC", 0);
    DMButtonClass = objc_allocateClassPair(objc_getClass("UIButton"), "DMButton", 0);
    DMLabelClass = objc_allocateClassPair(objc_getClass("UILabel"), "DMLabel", 0);
    DMTextFieldClass = objc_allocateClassPair(objc_getClass("UITextField"), "DMTextField", 0);
    DMBackgroundViewClass = objc_allocateClassPair(objc_getClass("UIView"), "DMBackgroundView", 0);

    if (legacy) {
        char const *colorField = "colorCode";
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
        class_addMethod(DMNavVC, sel_getUid("preferredStatusBarStyle"),
                        (IMP) dmNavVC_getStatusBarStyle, "i@:");
        class_addMethod(SetupWorkoutVCClass,
                        sel_getUid("pickerView:attributedTitleForRow:forComponent:"),
                        (IMP) setupWorkoutVC_attrTitleForRow, TITLE_FOR_ROW);
    } else {
        class_addMethod(SetupWorkoutVCClass, sel_getUid("pickerView:titleForRow:forComponent:"),
                        (IMP) setupWorkoutVC_titleForRow, TITLE_FOR_ROW);
    }

    objc_registerClassPair(DMNavVC);
    objc_registerClassPair(DMTabVC);
    objc_registerClassPair(DMBackgroundViewClass);
    objc_registerClassPair(DMButtonClass);
    objc_registerClassPair(DMLabelClass);
    objc_registerClassPair(DMTextFieldClass);
    objc_registerClassPair(SetupWorkoutVCClass);
    SetupWorkoutVCDataRef = class_getInstanceVariable(SetupWorkoutVCClass, dataKey);

    if (setWindowTint)
        setTintColor(self->window, createColor(ColorRed));
    id tabVC = getObject(allocClass(DMTabVC), sel_getUid("init"));
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
