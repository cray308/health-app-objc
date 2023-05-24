#include <objc/message.h>
#include "AppDelegate.h"
#include "HistoryVC.h"
#include "HomeVC.h"
#include "InputVC.h"
#include "SettingsVC.h"
#include "SetupWorkoutVC.h"
#include "StatusView.h"
#include "UpdateMaxesVC.h"
#include "WorkoutVC.h"

extern int UIApplicationMain(int, char *[], CFStringRef, CFStringRef);

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
    class_addIvar(InputViewClass, dataKey, sizeof(InputView), 0, "{?=@@@@iifB}");
    class_addMethod(InputViewClass, deinit, (IMP)inputView_deinit, voidSig);
    objc_registerClassPair(InputViewClass);

    StepperViewClass = objc_allocateClassPair(View, "StepperView", 0);
    class_addIvar(StepperViewClass, dataKey, sizeof(StepperView), 0, "{?=:?@@@{?=qq}}");
    class_addMethod(StepperViewClass, deinit, (IMP)stepperView_deinit, voidSig);
    class_addMethod(StepperViewClass, sel_registerName("stepperChanged"),
                    (IMP)stepperView_updatedStepper, voidSig);
    class_addMethod(StepperViewClass, sel_getUid("accessibilityIncrement"),
                    (IMP)stepperView_increment, voidSig);
    class_addMethod(StepperViewClass, sel_getUid("accessibilityDecrement"),
                    (IMP)stepperView_decrement, voidSig);
    objc_registerClassPair(StepperViewClass);

    InputVCClass = objc_allocateClassPair(VC, "InputVC", 0);
    class_addProtocol(InputVCClass, objc_getProtocol("UITextFieldDelegate"));
    class_addIvar(InputVCClass, "validatorData", sizeof(InputVC), 0, "{?=@@[4{?=@@}]@@@@@iiiiB}");
    class_addMethod(InputVCClass, sel_registerName("initWithVCache:cCache:"),
                    (IMP)inputVC_init, "@@:@@");
    class_addMethod(InputVCClass, deinit, (IMP)inputVC_deinit, voidSig);
    class_addMethod(InputVCClass, viewLoad, (IMP)inputVC_viewDidLoad, voidSig);
    class_addMethod(InputVCClass, sel_getUid("viewDidAppear:"),
                    (IMP)inputVC_viewDidAppear, appearSig);
    class_addMethod(InputVCClass, sel_registerName("dismissKeyboard"),
                    (IMP)inputVC_dismissKeyboard, voidSig);
    class_addMethod(InputVCClass, sel_registerName("jumpToPrev"), (IMP)inputVC_jumpToPrev, voidSig);
    class_addMethod(InputVCClass, sel_registerName("jumpToNext"), (IMP)inputVC_jumpToNext, voidSig);
    class_addMethod(InputVCClass, sel_getUid("textFieldDidBeginEditing:"),
                    (IMP)inputVC_fieldBeganEditing, tapSig);
    class_addMethod(InputVCClass, sel_getUid("textFieldDidEndEditing:"),
                    (IMP)inputVC_fieldStoppedEditing, tapSig);
    SEL tfChange = sel_getUid("textField:shouldChangeCharactersInRange:replacementString:");
    class_addMethod(InputVCClass, tfChange, (IMP)inputVC_fieldChanged, "i@:@{?=qq}@");
    objc_registerClassPair(InputVCClass);

    SettingsVCClass = objc_allocateClassPair(InputVCClass, "SettingsVC", 0);
    class_addIvar(SettingsVCClass, dataKey, sizeof(SettingsVC), 0, "{?=@@@@[4i]}");
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
    class_addIvar(UpdateMaxesVCClass, dataKey, sizeof(UpdateMaxesVC), 0, "{?=@@ii}");
    class_addMethod(UpdateMaxesVCClass, deinit, (IMP)updateMaxesVC_deinit, voidSig);
    class_addMethod(UpdateMaxesVCClass, viewLoad, (IMP)updateMaxesVC_viewDidLoad, voidSig);
    class_addMethod(UpdateMaxesVCClass, sel_registerName("tappedFinish"),
                    (IMP)updateMaxesVC_tappedFinish, voidSig);
    objc_registerClassPair(UpdateMaxesVCClass);

    HomeVCClass = objc_allocateClassPair(VC, "HomeVC", 0);
    class_addIvar(HomeVCClass, dataKey, sizeof(HomeVC), 0, "{?=@@{?=@@}@i}");
    class_addMethod(HomeVCClass, viewLoad, (IMP)homeVC_viewDidLoad, voidSig);
    class_addMethod(HomeVCClass, btnTap, (IMP)homeVC_workoutButtonTapped, tapSig);
    class_addMethod(HomeVCClass, sel_registerName("customButtonTapped:"),
                    (IMP)homeVC_customButtonTapped, tapSig);
    objc_registerClassPair(HomeVCClass);

    HistoryVCClass = objc_allocateClassPair(VC, "HistoryVC", 0);
    class_addProtocol(HistoryVCClass, objc_getProtocol("Charts.ValueFormatter"));
    class_addIvar(HistoryVCClass, dataKey, sizeof(HistoryVC), 0,
                  "{?={?={?=@@[3f]}{?=[5@][5@][3[4i]]}{?=[4@][4@][3[4f]]}"
                  "@[3@][3[3f]][3i][3i]}@[3@]}");
    class_addMethod(HistoryVCClass, viewLoad, (IMP)historyVC_viewDidLoad, voidSig);
    class_addMethod(HistoryVCClass, btnTap, (IMP)historyVC_updateSegment, tapSig);
    class_addMethod(HistoryVCClass, sel_getUid("stringForValue:"),
                    (IMP)historyVC_stringForValue, "@@:d");
    objc_registerClassPair(HistoryVCClass);

    WorkoutVCClass = objc_allocateClassPair(VC, "WorkoutVC", 0);
    class_addIvar(WorkoutVCClass, dataKey, sizeof(WorkoutVC), 0,
                  "{?=@@[2{?={_opaque_pthread_mutex_t=q[56c]}{_opaque_pthread_cond_t=q[40c]}@qiiI"
                  "{?=CCC}}][2^{_opaque_pthread_t}]@@@{?=i{?=ii}}[4i]B}");
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
    class_addIvar(AppDelegateClass, "children", 3 * sizeof(id), 0, "[3@]");
    class_addIvar(AppDelegateClass, "clr", sizeof(ColorCache), 0, "{?=#:?}");
    class_addIvar(AppDelegateClass, "tbl", sizeof(VCache), 0,
                  "{?={?=#::::????}{?=::::::::::::???????????}"
                  "{?=:::::?????}{?=::::????}{?=::::????}{?=:?}}");
    class_addIvar(AppDelegateClass, "userData", sizeof(UserData), 0, "{?=qq[4i]CCC}");
    class_addMethod(AppDelegateClass, sel_getUid("application:didFinishLaunchingWithOptions:"),
                    (IMP)appDelegate_didFinishLaunching, appSig);
    SEL orient = sel_getUid("application:supportedInterfaceOrientationsForWindow:");
    class_addMethod(AppDelegateClass, orient, (IMP)appDelegate_supportedOrientations, appSig);
    objc_registerClassPair(AppDelegateClass);
    return UIApplicationMain(argc, argv, nil, CFSTR("AppDelegate"));
}
