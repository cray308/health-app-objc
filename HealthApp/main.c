#include "CocoaHelpers.h"
#include "SetupWorkoutVC.h"
#include "StatusView.h"
#include "TotalWorkoutsView.h"
#include "WorkoutTypeView.h"
#include "LiftingView.h"
#include "InputVC.h"
#include "HomeVC.h"
#include "HistoryVC.h"
#include "SettingsVC.h"
#include "WorkoutVC.h"
#include "UpdateMaxesVC.h"
#include "AppDelegate.h"
#include "ViewControllerHelpers.h"

extern int UIApplicationMain(int, char *[], CFStringRef, CFStringRef);
extern Protocol *getValueFormatterType(void);
extern SEL getValueFormatterAction(void);

int main(int argc, char *argv[]) {
    Class viewClass = objc_getClass("UIView");
    Class VCClass = objc_getClass("UIViewController");
    char const *validatorKey = "validatorData", *dataKey = "data", *colorField = "colorCode";
    char const *voidSig = "v@:", *tapSig = "v@:@", *appearSig = "v@:i", *appSig = "i@:@@";
    SEL deinit = sel_getUid("dealloc");
    SEL viewLoad = sel_getUid("viewDidLoad");
    SEL btnTap = sel_getUid("buttonTapped:");

    DMNavVC = objc_allocateClassPair(objc_getClass("UINavigationController"), "DMNavVC", 0);
    DMButtonClass = objc_allocateClassPair(objc_getClass("UIButton"), "DMButton", 0);
    class_addIvar(DMButtonClass, colorField, sizeof(int), 0, "i");
    class_addIvar(DMButtonClass, "background", sizeof(bool), 0, "c");
    DMLabelClass = objc_allocateClassPair(objc_getClass("UILabel"), "DMLabel", 0);
    class_addIvar(DMLabelClass, colorField, sizeof(int), 0, "i");
    DMTextFieldClass = objc_allocateClassPair(objc_getClass("UITextField"), "DMTextField", 0);
    DMBackgroundViewClass = objc_allocateClassPair(objc_getClass("UIView"), "DMBackgroundView", 0);
    class_addIvar(DMBackgroundViewClass, colorField, sizeof(int), 0, "i");

    StatusViewClass = objc_allocateClassPair(viewClass, "StatusView", 0);
    class_addIvar(StatusViewClass, dataKey, sizeof(StatusViewData*), 0, "^{?=@@@@}");
    class_addMethod(StatusViewClass, deinit, (IMP) statusView_deinit, voidSig);
    objc_registerClassPair(StatusViewClass);
    StatusViewDataRef = class_getInstanceVariable(StatusViewClass, dataKey);

    ContainerViewClass = objc_allocateClassPair(viewClass, "ContainerView", 0);
    class_addIvar(ContainerViewClass, dataKey, sizeof(ContainerViewData*), 0, "^{?=@@@}");
    class_addMethod(ContainerViewClass, deinit, (IMP) containerView_deinit, voidSig);
    objc_registerClassPair(ContainerViewClass);
    ContainerViewDataRef = class_getInstanceVariable(ContainerViewClass, dataKey);

    TotalWorkoutsViewClass = objc_allocateClassPair(viewClass, "TotalWorkoutsView", 0);
    class_addIvar(TotalWorkoutsViewClass, dataKey, sizeof(TotalWorkoutsViewData*), 0, "^{?=@@}");
    objc_registerClassPair(TotalWorkoutsViewClass);
    TotalWorkoutsViewDataRef = class_getInstanceVariable(TotalWorkoutsViewClass, dataKey);

    WorkoutTypeViewClass = objc_allocateClassPair(viewClass, "WorkoutTypeView", 0);
    class_addIvar(WorkoutTypeViewClass, dataKey, sizeof(WorkoutTypeViewData*), 0, "^{?=@@}");
    objc_registerClassPair(WorkoutTypeViewClass);
    WorkoutTypeViewDataRef = class_getInstanceVariable(WorkoutTypeViewClass, dataKey);

    LiftViewClass = objc_allocateClassPair(viewClass, "LiftView", 0);
    class_addIvar(LiftViewClass, dataKey, sizeof(LiftViewData*), 0, "^{?=@@}");
    objc_registerClassPair(LiftViewClass);
    LiftViewDataRef = class_getInstanceVariable(LiftViewClass, dataKey);

    InputViewClass = objc_allocateClassPair(viewClass, "InputView", 0);
    class_addIvar(InputViewClass, dataKey, sizeof(InputViewData*), 0, "^{?=sssB@@@}");
    class_addMethod(InputViewClass, deinit, (IMP) inputView_deinit, voidSig);
    objc_registerClassPair(InputViewClass);
    InputViewDataRef = class_getInstanceVariable(InputViewClass, dataKey);

    InputVCClass = objc_allocateClassPair(VCClass, "InputVC", 0);
    class_addProtocol(InputVCClass, objc_getProtocol("UITextFieldDelegate"));
    class_addIvar(InputVCClass, validatorKey, sizeof(InputVCData*), 0, "^{?=iiss[4@]@@@@@}");
    class_addMethod(InputVCClass, sel_getUid("init"), (IMP) inputVC_init, "@@:");
    class_addMethod(InputVCClass, deinit, (IMP) inputVC_deinit, voidSig);
    class_addMethod(InputVCClass, viewLoad, (IMP) inputVC_viewDidLoad, voidSig);
    class_addMethod(InputVCClass, sel_getUid("viewDidAppear:"),
                    (IMP) inputVC_viewDidAppear, appearSig);
    class_addMethod(InputVCClass, sel_getUid("dismissKeyboard"),
                    (IMP) inputVC_dismissKeyboard, voidSig);
    class_addMethod(InputVCClass, sel_getUid("keyboardShown:"),
                    (IMP) inputVC_keyboardShown, tapSig);
    class_addMethod(InputVCClass, sel_getUid("keyboardWillHide:"),
                    (IMP) inputVC_keyboardWillHide, tapSig);
    class_addMethod(InputVCClass, sel_getUid("textFieldDidBeginEditing:"),
                    (IMP) inputVC_fieldBeganEditing, tapSig);
    class_addMethod(InputVCClass, sel_getUid("textFieldDidEndEditing:"),
                    (IMP) inputVC_fieldStoppedEditing, tapSig);
    class_addMethod(InputVCClass,
                    sel_getUid("textField:shouldChangeCharactersInRange:replacementString:"),
                    (IMP) inputVC_fieldChanged, "i@:@{?=qq}@");
    objc_registerClassPair(InputVCClass);
    InputVCDataRef = class_getInstanceVariable(InputVCClass, validatorKey);

    SettingsVCClass = objc_allocateClassPair(InputVCClass, "SettingsVC", 0);
    class_addIvar(SettingsVCClass, dataKey, sizeof(SettingsVCData*), 0, "^{?=@@[4s]}");
    class_addMethod(SettingsVCClass, viewLoad, (IMP) settingsVC_viewDidLoad, voidSig);
    class_addMethod(SettingsVCClass, btnTap, (IMP) settingsVC_buttonTapped, tapSig);
    objc_registerClassPair(SettingsVCClass);
    SettingsVCDataRef = class_getInstanceVariable(SettingsVCClass, dataKey);

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
                    (IMP) setupWorkoutVC_numberOfComponents, "q@:@");
    class_addMethod(SetupWorkoutVCClass, sel_getUid("pickerView:numberOfRowsInComponent:"),
                    (IMP) setupWorkoutVC_numberOfRows, "q@:@q");
    class_addMethod(SetupWorkoutVCClass, sel_getUid("pickerView:didSelectRow:inComponent:"),
                    (IMP) setupWorkoutVC_didSelectRow, "v@:@qq");

    UpdateMaxesVCClass = objc_allocateClassPair(InputVCClass, "UpdateMaxesVC", 0);
    class_addIvar(UpdateMaxesVCClass, dataKey, sizeof(UpdateMaxesVCData*), 0, "^{?=@@@@I}");
    class_addMethod(UpdateMaxesVCClass, deinit, (IMP) updateMaxesVC_deinit, voidSig);
    class_addMethod(UpdateMaxesVCClass, viewLoad, (IMP) updateMaxesVC_viewDidLoad, voidSig);
    class_addMethod(UpdateMaxesVCClass, sel_getUid("stepperChanged"),
                    (IMP) updateMaxesVC_updatedStepper, voidSig);
    class_addMethod(UpdateMaxesVCClass, sel_getUid("tappedFinish"),
                    (IMP) updateMaxesVC_tappedFinish, voidSig);
    objc_registerClassPair(UpdateMaxesVCClass);
    UpdateMaxesVCDataRef = class_getInstanceVariable(UpdateMaxesVCClass, dataKey);

    HomeVCClass = objc_allocateClassPair(VCClass, "HomeVC", 0);
    class_addIvar(HomeVCClass, dataKey, sizeof(HomeVCData*), 0, "^{?=@@[2@]i}");
    class_addMethod(HomeVCClass, viewLoad, (IMP) homeVC_viewDidLoad, voidSig);
    class_addMethod(HomeVCClass, btnTap, (IMP) homeVC_workoutButtonTapped, tapSig);
    class_addMethod(HomeVCClass, sel_getUid("customButtonTapped:"),
                    (IMP) homeVC_customButtonTapped, tapSig);
    objc_registerClassPair(HomeVCClass);
    HomeVCDataRef = class_getInstanceVariable(HomeVCClass, dataKey);

    HistoryVCClass = objc_allocateClassPair(VCClass, "HistoryVC", 0);
    class_addProtocol(HistoryVCClass, getValueFormatterType());
    class_addIvar(HistoryVCClass, dataKey, sizeof(HistoryVCData*), 0,
                  "^{?={?={?=@@@@[3f][3f]}{?=[5@]@[5@]@[3[4i]][3f][4@]}"
                  "{?=[4@]@[4@]@[3[4f]][3f][4@]}@[3i][3i]}@[3@]}");
    class_addMethod(HistoryVCClass, viewLoad, (IMP) historyVC_viewDidLoad, voidSig);
    class_addMethod(HistoryVCClass, btnTap, (IMP) historyVC_updateSegment, tapSig);
    class_addMethod(HistoryVCClass, getValueFormatterAction(),
                    (IMP) historyVC_stringForValue, "@@:d");
    objc_registerClassPair(HistoryVCClass);
    HistoryVCDataRef = class_getInstanceVariable(HistoryVCClass, dataKey);

    WorkoutVCClass = objc_allocateClassPair(VCClass, "WorkoutVC", 0);
    class_addIvar(WorkoutVCClass, dataKey, sizeof(WorkoutVCData*), 0,
                  "^{?=@@@[2@][2@]{__savedWorkoutInfo=I{__exerciseInfo=II}}"
                  "[2{?=@{__timerInfo=CCC}{_opaque_pthread_mutex_t=q[56c]}"
                  "{_opaque_pthread_cond_t=q[40c]}IIIq}][4s]B}");
    class_addMethod(WorkoutVCClass, deinit, (IMP) workoutVC_deinit, voidSig);
    class_addMethod(WorkoutVCClass, viewLoad, (IMP) workoutVC_viewDidLoad, voidSig);
    class_addMethod(WorkoutVCClass, sel_getUid("startEndWorkout:"),
                    (IMP) workoutVC_startEndWorkout, tapSig);
    class_addMethod(WorkoutVCClass, sel_getUid("viewWillDisappear:"),
                    (IMP) workoutVC_willDisappear, appearSig);
    class_addMethod(WorkoutVCClass, btnTap, (IMP) workoutVC_handleTap, tapSig);
    objc_registerClassPair(WorkoutVCClass);
    WorkoutVCDataRef = class_getInstanceVariable(WorkoutVCClass, dataKey);

    AppDelegateClass = objc_allocateClassPair(objc_getClass("UIResponder"), "AppDelegate", 0);
    class_addIvar(AppDelegateClass, "window", sizeof(id), 0, "@");
    class_addIvar(AppDelegateClass, "children", 3 * sizeof(id), 0, "[3@]");
    class_addMethod(AppDelegateClass, sel_getUid("application:didFinishLaunchingWithOptions:"),
                    (IMP) appDelegate_didFinishLaunching, appSig);
    class_addMethod(AppDelegateClass,
                    sel_getUid("application:supportedInterfaceOrientationsForWindow:"),
                    (IMP) appDelegate_supportedOrientations, appSig);
    objc_registerClassPair(AppDelegateClass);
    return UIApplicationMain(argc, argv, nil, CFSTR("AppDelegate"));
}
