#include "AppDelegate.h"
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CFPreferences.h>
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
void persistenceService_create(void);
#endif
void setupAppColors(unsigned char darkMode, bool deleteOld);
void toggleDarkModeForCharts(bool);
id colorCreateLegacy(id self, SEL _cmd, int type);
id colorCreate(id self, SEL _cmd, int type);
id barColorCreateLegacy(id self, SEL _cmd, int type);
id barColorCreate(id self, SEL _cmd, int type);
int dmNavVC_getStatusBarStyle(id self, SEL _cmd);
int dmNavVC_getStatusBarStyleDark(id self, SEL _cmd);
id alertCtrlCreate(id self, SEL _cmd, CFStringRef title, CFStringRef message);
id alertCtrlCreateLegacy(id self, SEL _cmd, CFStringRef title, CFStringRef message);

Class AppDelegateClass;

bool appDelegate_didFinishLaunching(AppDelegate *self, SEL _cmd _U_,
                                    id application _U_, id options _U_) {
    bool legacy = !objc_getClass("UITabBarAppearance");
    UserInfo const *info;
    CGRect bounds;
    getScreenBounds(&bounds);
    self->window = createObjectWithFrame(objc_getClass("UIWindow"), bounds);

    CFStringRef hasLaunchedKey = CFSTR("hasLaunched");
    CFBooleanRef value = CFPreferencesCopyAppValue(hasLaunchedKey, kCFPreferencesCurrentApplication);

    persistenceService_init();
    int tzOffset = 0, week = 0;

    if (value == NULL) {
        CFNumberRef newValue = CFNumberCreate(NULL, kCFNumberCharType, &(bool){true});
        CFPreferencesSetAppValue(hasLaunchedKey, newValue, kCFPreferencesCurrentApplication);
        CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
        CFRelease(newValue);
        userInfo_create(legacy, &info);
#if DEBUG
        persistenceService_create();
#endif
        msg2(void, unsigned long, void(^)(bool,id), getUserNotificationCenter(),
             sel_getUid("requestAuthorizationWithOptions:completionHandler:"),
             6, ^(bool granted _U_, id error _U_) {});
    } else {
        tzOffset = userInfo_initFromStorage(legacy, &week, &info);
        CFRelease(value);
    }

    CFBundleRef bundle = CFBundleGetMainBundle();
    initValidatorStrings(bundle);
    initExerciseData(week, bundle);
    initWorkoutStrings(bundle);
    char const *colorCreateSig = "@@:i";
    Class colorMeta = objc_getMetaClass("UIColor");
    Class alertMeta = objc_getMetaClass("UIAlertController");
    if (legacy) {
        const unsigned char darkMode = info->darkMode;
        IMP imp = (IMP) (darkMode ? dmNavVC_getStatusBarStyleDark : dmNavVC_getStatusBarStyle);
        class_addMethod(DMNavVC, sel_getUid("preferredStatusBarStyle"), imp, "i@:");
        class_addMethod(SetupWorkoutVCClass, sel_getUid("numberOfComponentsInPickerView:"),
                        (IMP) setupWorkoutVC_numberOfComponentsLegacy, "q@:@");
        class_addMethod(SetupWorkoutVCClass, sel_getUid("pickerView:didSelectRow:inComponent:"),
                        (IMP) setupWorkoutVC_didSelectRowLegacy, "v@:@qq");
        class_addMethod(SetupWorkoutVCClass,
                        sel_getUid("pickerView:attributedTitleForRow:forComponent:"),
                        (IMP) setupWorkoutVC_attrTitleForRow, "@@:@qq");
        class_addMethod(colorMeta, sel_getUid("getColorWithType:"),
                        (IMP) colorCreateLegacy, colorCreateSig);
        class_addMethod(colorMeta, sel_getUid("getBarColorWithType:"),
                        (IMP) barColorCreateLegacy, colorCreateSig);
        class_addMethod(alertMeta, sel_getUid("getCtrlWithTitle:message:"),
                        (IMP) alertCtrlCreateLegacy, "@@:@@");
        setupAppColors(darkMode, false);
        toggleDarkModeForCharts(darkMode);
    } else {
        class_addMethod(SetupWorkoutVCClass, sel_getUid("numberOfComponentsInPickerView:"),
                        (IMP) setupWorkoutVC_numberOfComponents, "q@:@");
        class_addMethod(SetupWorkoutVCClass, sel_getUid("pickerView:didSelectRow:inComponent:"),
                        (IMP) setupWorkoutVC_didSelectRow, "v@:@qq");
        class_addMethod(SetupWorkoutVCClass, sel_getUid("pickerView:titleForRow:forComponent:"),
                        (IMP) setupWorkoutVC_titleForRow, "@@:@qq");
        class_addMethod(colorMeta, sel_getUid("getColorWithType:"),
                        (IMP) colorCreate, colorCreateSig);
        class_addMethod(colorMeta, sel_getUid("getBarColorWithType:"),
                        (IMP) barColorCreate, colorCreateSig);
        class_addMethod(alertMeta, sel_getUid("getCtrlWithTitle:message:"),
                        (IMP) alertCtrlCreate, "@@:@@");
    }

    objc_registerClassPair(DMNavVC);
    objc_registerClassPair(SetupWorkoutVCClass);

    setTintColor(self->window, createColor(ColorRed));
    id tabVC = createNew(objc_getClass("UITabBarController"));
    msg1(void, id, self->window, sel_getUid("setRootViewController:"), tabVC);
    void (*fetchHandler)(void*);
    void *fetchArg;
    Class itemClass = objc_getClass("UITabBarItem");
    SEL itemInit = sel_getUid("initWithTitle:image:tag:"), setItem = sel_getUid("setTabBarItem:");
    id controllers[3];
    CFStringRef imgNames[] = {CFSTR("ico_house"), CFSTR("ico_chart"), CFSTR("ico_gear")};
    CFStringRef titles[3];
    fillStringArray(bundle, titles, CFSTR("tabs%d"), 3);
    self->children[0] = homeVC_init(bundle);
    self->children[1] = historyVC_init(bundle, &fetchArg, &fetchHandler);
    self->children[2] = settingsVC_init();

    for (int i = 0; i < 3; ++i) {
        id img = createImage(imgNames[i]);
        id item = msg3(id, CFStringRef,id,long, allocClass(itemClass), itemInit, titles[i], img, i);
        controllers[i] = createNavVC(self->children[i]);
        msg1(void, id, controllers[i], setItem, item);
        releaseObj(item);
        CFRelease(titles[i]);
    }

    CFArrayRef array = CFArrayCreate(NULL, (const void **)controllers, 3, NULL);
    msg2(void, CFArrayRef, bool, tabVC, sel_getUid("setViewControllers:animated:"), array, false);
    setupTabVC(tabVC);

    for (int i = 0; i < 3; ++i) {
        releaseObj(controllers[i]);
    }
    CFRelease(array);
    releaseObj(tabVC);

    msg0(void, self->window, sel_getUid("makeKeyAndVisible"));
    persistenceService_start(tzOffset, info->weekStart, fetchHandler, fetchArg);
    return true;
}

int appDelegate_supportedOrientations(AppDelegate *self _U_, SEL _cmd _U_,
                                      id application _U_, id window _U_) {
    id device = clsF0(id, objc_getClass("UIDevice"), sel_getUid("currentDevice"));
    return msg0(long, device, sel_getUid("userInterfaceIdiom")) == 1 ? 26 : 2;
}

static AppDelegate *getAppDel(void) {
    id app = clsF0(id, objc_getClass("UIApplication"), sel_getUid("sharedApplication"));
    return (AppDelegate *) msg0(id, app, sel_getUid("delegate"));
}

id appDel_getWindow(void) { return getAppDel()->window; }

void appDel_updateUserInfo(unsigned char plan, unsigned char darkMode, short *weights) {
    AppDelegate *self = getAppDel();
    unsigned char changes = appUserData_updateUserSettings(plan, darkMode, weights);
    if (changes & 2) {
        Method method = class_getInstanceMethod(DMNavVC, sel_getUid("preferredStatusBarStyle"));
        IMP newImp = (IMP) (darkMode ? dmNavVC_getStatusBarStyleDark : dmNavVC_getStatusBarStyle);
        method_setImplementation(method, newImp);
        setupAppColors(darkMode, true);
        setTintColor(self->window, createColor(ColorRed));
        setupTabVC(getRootVC(self->window));
        toggleDarkModeForCharts(darkMode);
        homeVC_updateColors(self->children[0]);
        if (isViewLoaded(self->children[1]))
            historyVC_updateColors(self->children[1], darkMode);
        settingsVC_updateColors(self->children[2], darkMode);
    }
    if (changes & 1)
        homeVC_createWorkoutsList(self->children[0], plan);
}

void appDel_deleteAppData(void) {
    id *arr = getAppDel()->children;
    bool updateHome = appUserData_deleteSavedData();
    persistenceService_deleteUserData();
    if (updateHome)
        homeVC_updateWorkoutsList((HomeVC *) ((char *)arr[0] + VCSize), 0);
    historyVC_clearData(arr[1]);
}

void appDel_updateMaxWeights(short *weights) {
    id vc = getAppDel()->children[2];
    short results[4];
    if (appUserData_updateWeightMaxes(weights, results) && isViewLoaded(vc))
        inputVC_updateFields((InputVC *) ((char *)vc + VCSize), results);
}
