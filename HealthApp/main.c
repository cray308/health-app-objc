#include <objc/runtime.h>
#include <CoreFoundation/CFString.h>
#include "AppDelegate.h"
#include "SetupWorkoutVC.h"
#include "StatusView.h"
#include "InputVC.h"
#include "HomeVC.h"
#include "HistoryVC.h"
#include "SettingsVC.h"
#include "WorkoutVC.h"
#include "UpdateMaxesVC.h"

extern Class StatusViewClass;
extern Class InputViewClass;
extern Class ContainerViewClass;
extern Class HomeVCClass;
extern Class HistoryVCClass;
extern Class SettingsVCClass;
extern Class WorkoutVCClass;
extern Class UpdateMaxesVCClass;
extern int UIApplicationMain(int, char *[], CFStringRef, CFStringRef);
void containerView_deinit(id, SEL);
void statusView_deinit(id, SEL);
void inputView_deinit(id, SEL);
id inputVC_init(id, SEL, VCacheRef, CCacheRef);
void inputVC_deinit(id, SEL);
void inputVC_viewDidLoad(id, SEL);
void inputVC_viewDidAppear(id, SEL, bool);
void inputVC_dismissKeyboard(id, SEL);
void inputVC_fieldBeganEditing(id, SEL, id);
void inputVC_fieldStoppedEditing(id, SEL, id);
bool inputVC_fieldChanged(id, SEL, id, CFRange, CFStringRef);
void homeVC_viewDidLoad(id, SEL);
void homeVC_workoutButtonTapped(id, SEL, id);
void homeVC_customButtonTapped(id, SEL, id);
void historyVC_viewDidLoad(id, SEL);
void historyVC_updateSegment(id, SEL, id);
CFStringRef historyVC_stringForValue(id, SEL, double);
void settingsVC_viewDidLoad(id, SEL);
void settingsVC_buttonTapped(id, SEL, id);
void setupWorkoutVC_deinit(id, SEL);
void setupWorkoutVC_viewDidLoad(id, SEL);
void setupWorkoutVC_tappedButton(id, SEL, id);
long setupWorkoutVC_numberOfRows(id, SEL, id, long);
void workoutVC_deinit(id, SEL);
void workoutVC_viewDidLoad(id, SEL);
void workoutVC_willDisappear(id, SEL, bool);
void workoutVC_startEndWorkout(id, SEL, id);
void workoutVC_handleTap(id, SEL, id);
void updateMaxesVC_deinit(id, SEL);
void updateMaxesVC_viewDidLoad(id, SEL);
void updateMaxesVC_updatedStepper(id, SEL);
void updateMaxesVC_tappedFinish(id, SEL);
bool appDelegate_didFinishLaunching(AppDelegate*, SEL, id, id);
int appDelegate_supportedOrientations(AppDelegate*, SEL, id, id);
static Class AppDelegateClass;

int main(int argc, char *argv[]) {
    Class VC = objc_getClass("UIViewController"), View = objc_getClass("UIView");
    char const *dataKey = "data";
    char const *voidSig = "v@:", *tapSig = "v@:@", *appearSig = "v@:i", *appSig = "i@:@@";
    SEL deinit = sel_getUid("dealloc");
    SEL viewLoad = sel_getUid("viewDidLoad");
    SEL btnTap = sel_registerName("buttonTapped:");

    StatusViewClass = objc_allocateClassPair(View, "StatusView", 0);
    class_addIvar(StatusViewClass, dataKey, sizeof(StatusView), 0, "{?=@@@@}");
    class_addMethod(StatusViewClass, deinit, (IMP)statusView_deinit, voidSig);
    objc_registerClassPair(StatusViewClass);

    ContainerViewClass = objc_allocateClassPair(View, "ContainerView", 0);
    class_addIvar(ContainerViewClass, dataKey, sizeof(ContainerView), 0, "{?=@@@}");
    class_addMethod(ContainerViewClass, deinit, (IMP)containerView_deinit, voidSig);
    objc_registerClassPair(ContainerViewClass);

    InputViewClass = objc_allocateClassPair(View, "InputView", 0);
    class_addIvar(InputViewClass, dataKey, sizeof(InputView), 0, "{?=@@@iisB}");
    class_addMethod(InputViewClass, deinit, (IMP)inputView_deinit, voidSig);
    objc_registerClassPair(InputViewClass);

    InputVCClass = objc_allocateClassPair(VC, "InputVC", 0);
    class_addProtocol(InputVCClass, objc_getProtocol("UITextFieldDelegate"));
    class_addIvar(InputVCClass, "validatorData", sizeof(InputVC), 0, "{?=@@[4@]@@@@@iissB}");
    class_addMethod(InputVCClass, sel_registerName("initWithVCache:cCache:"), (IMP)inputVC_init, "@@:@@");
    class_addMethod(InputVCClass, deinit, (IMP)inputVC_deinit, voidSig);
    class_addMethod(InputVCClass, viewLoad, (IMP)inputVC_viewDidLoad, voidSig);
    class_addMethod(InputVCClass, sel_getUid("viewDidAppear:"), (IMP)inputVC_viewDidAppear, appearSig);
    class_addMethod(InputVCClass, sel_registerName("dismissKeyboard"), (IMP)inputVC_dismissKeyboard, voidSig);
    class_addMethod(InputVCClass, sel_getUid("textFieldDidBeginEditing:"),
                    (IMP)inputVC_fieldBeganEditing, tapSig);
    class_addMethod(InputVCClass, sel_getUid("textFieldDidEndEditing:"),
                    (IMP)inputVC_fieldStoppedEditing, tapSig);
    class_addMethod(InputVCClass, sel_getUid("textField:shouldChangeCharactersInRange:replacementString:"),
                    (IMP)inputVC_fieldChanged, "i@:@{?=qq}@");
    objc_registerClassPair(InputVCClass);

    SettingsVCClass = objc_allocateClassPair(InputVCClass, "SettingsVC", 0);
    class_addIvar(SettingsVCClass, dataKey, sizeof(SettingsVC), 0, "{?=@@@[4s]}");
    class_addMethod(SettingsVCClass, viewLoad, (IMP)settingsVC_viewDidLoad, voidSig);
    class_addMethod(SettingsVCClass, btnTap, (IMP)settingsVC_buttonTapped, tapSig);
    objc_registerClassPair(SettingsVCClass);

    SetupWorkoutVCClass = objc_allocateClassPair(InputVCClass, "SetupWorkoutVC", 0);
    class_addProtocol(SetupWorkoutVCClass, objc_getProtocol("UIPickerViewDelegate"));
    class_addProtocol(SetupWorkoutVCClass, objc_getProtocol("UIPickerViewDataSource"));
    class_addIvar(SetupWorkoutVCClass, dataKey, sizeof(SetupWorkoutVC), 0, "{?=@@@@@ii}");
    class_addMethod(SetupWorkoutVCClass, sel_getUid("dealloc"), (IMP)setupWorkoutVC_deinit, voidSig);
    class_addMethod(SetupWorkoutVCClass, viewLoad, (IMP)setupWorkoutVC_viewDidLoad, voidSig);
    class_addMethod(SetupWorkoutVCClass, btnTap, (IMP)setupWorkoutVC_tappedButton, tapSig);
    class_addMethod(SetupWorkoutVCClass, sel_getUid("pickerView:numberOfRowsInComponent:"),
                    (IMP)setupWorkoutVC_numberOfRows, "q@:@q");
    objc_registerClassPair(SetupWorkoutVCClass);

    UpdateMaxesVCClass = objc_allocateClassPair(InputVCClass, "UpdateMaxesVC", 0);
    class_addIvar(UpdateMaxesVCClass, dataKey, sizeof(UpdateMaxesVC), 0, "{?=@@@@{?=qq}is}");
    class_addMethod(UpdateMaxesVCClass, deinit, (IMP)updateMaxesVC_deinit, voidSig);
    class_addMethod(UpdateMaxesVCClass, viewLoad, (IMP)updateMaxesVC_viewDidLoad, voidSig);
    class_addMethod(UpdateMaxesVCClass, sel_registerName("stepperChanged"),
                    (IMP)updateMaxesVC_updatedStepper, voidSig);
    class_addMethod(UpdateMaxesVCClass, sel_registerName("tappedFinish"),
                    (IMP)updateMaxesVC_tappedFinish, voidSig);
    objc_registerClassPair(UpdateMaxesVCClass);

    HomeVCClass = objc_allocateClassPair(VC, "HomeVC", 0);
    class_addIvar(HomeVCClass, dataKey, sizeof(HomeVC), 0, "{?=@@@@i}");
    class_addMethod(HomeVCClass, viewLoad, (IMP)homeVC_viewDidLoad, voidSig);
    class_addMethod(HomeVCClass, btnTap, (IMP)homeVC_workoutButtonTapped, tapSig);
    class_addMethod(HomeVCClass, sel_registerName("customButtonTapped:"),
                    (IMP)homeVC_customButtonTapped, tapSig);
    objc_registerClassPair(HomeVCClass);

    HistoryVCClass = objc_allocateClassPair(VC, "HistoryVC", 0);
    class_addProtocol(HistoryVCClass, objc_getProtocol("Charts.ValueFormatter"));
    class_addIvar(HistoryVCClass, dataKey, sizeof(HistoryVC), 0,
                  "{?=@@{?={?=@@@[3f][3f]}{?=[5@][5@]@[3[4i]][3f]}"
                  "{?=[4@][4@]@[3[4f]][3f]}@[3i][3i]}@[3@]}");
    class_addMethod(HistoryVCClass, viewLoad, (IMP)historyVC_viewDidLoad, voidSig);
    class_addMethod(HistoryVCClass, btnTap, (IMP)historyVC_updateSegment, tapSig);
    class_addMethod(HistoryVCClass, sel_getUid("stringForValue:"), (IMP)historyVC_stringForValue, "@@:d");
    objc_registerClassPair(HistoryVCClass);

    WorkoutVCClass = objc_allocateClassPair(VC, "WorkoutVC", 0);
    class_addIvar(WorkoutVCClass, dataKey, sizeof(WorkoutVC), 0,
                  "{?=@@[2{?={_opaque_pthread_mutex_t=q[56c]}{_opaque_pthread_cond_t=q[40c]}@qiiI"
                  "{?=CCC}}][2^{_opaque_pthread_t}]@@@{?=i{?=ii}}[4s]B}");
    class_addMethod(WorkoutVCClass, deinit, (IMP)workoutVC_deinit, voidSig);
    class_addMethod(WorkoutVCClass, viewLoad, (IMP)workoutVC_viewDidLoad, voidSig);
    class_addMethod(WorkoutVCClass, sel_registerName("startEndWorkout:"),
                    (IMP)workoutVC_startEndWorkout, tapSig);
    class_addMethod(WorkoutVCClass, sel_getUid("viewWillDisappear:"),
                    (IMP)workoutVC_willDisappear, appearSig);
    class_addMethod(WorkoutVCClass, btnTap, (IMP)workoutVC_handleTap, tapSig);
    objc_registerClassPair(WorkoutVCClass);

    AppDelegateClass = objc_allocateClassPair(objc_getClass("UIResponder"), "AppDelegate", 0);
    class_addIvar(AppDelegateClass, "window", sizeof(id), 0, "@");
    class_addIvar(AppDelegateClass, "context", sizeof(id), 0, "@");
    class_addIvar(AppDelegateClass, "children", 3 * sizeof(id), 0, "[3@]");
    class_addIvar(AppDelegateClass, "clr", sizeof(ColorCache), 0, "{?=#:?}");
    class_addIvar(AppDelegateClass, "tbl", sizeof(VCache), 0,
                  "{?={?=#::::????}{?=:::::::::?????????}{?=:::???}{?=:::???}{?=::::????}{?=::???}}");
    class_addIvar(AppDelegateClass, "userData", sizeof(UserInfo), 0, "{?=qq[4s]CCC}");
    class_addMethod(AppDelegateClass, sel_getUid("application:didFinishLaunchingWithOptions:"),
                    (IMP)appDelegate_didFinishLaunching, appSig);
    class_addMethod(AppDelegateClass, sel_getUid("application:supportedInterfaceOrientationsForWindow:"),
                    (IMP)appDelegate_supportedOrientations, appSig);
    objc_registerClassPair(AppDelegateClass);
    return UIApplicationMain(argc, argv, nil, CFSTR("AppDelegate"));
}
