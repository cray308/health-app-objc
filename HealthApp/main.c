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
    ViewClass = objc_getClass("UIView");
    VCClass = objc_getClass("UIViewController");
    ColorClass = objc_getClass("UIColor");
    ViewSize = class_getInstanceSize(ViewClass);
    VCSize = class_getInstanceSize(VCClass);
    InputVCSize = VCSize + sizeof(InputVC);
    char const *dataKey = "data";
    char const *voidSig = "v@:", *tapSig = "v@:@", *appearSig = "v@:i", *appSig = "i@:@@";
    SEL deinit = sel_getUid("dealloc");
    SEL viewLoad = sel_getUid("viewDidLoad");
    SEL btnTap = sel_getUid("buttonTapped:");

    DMNavVC = objc_allocateClassPair(objc_getClass("UINavigationController"), "DMNavVC", 0);

    StatusViewClass = objc_allocateClassPair(ViewClass, "StatusView", 0);
    class_addIvar(StatusViewClass, dataKey, sizeof(StatusView), 0, "{?=@@@@}");
    class_addMethod(StatusViewClass, deinit, (IMP) statusView_deinit, voidSig);
    objc_registerClassPair(StatusViewClass);

    ContainerViewClass = objc_allocateClassPair(ViewClass, "ContainerView", 0);
    class_addIvar(ContainerViewClass, dataKey, sizeof(ContainerView), 0, "{?=@@@}");
    class_addMethod(ContainerViewClass, deinit, (IMP) containerView_deinit, voidSig);
    objc_registerClassPair(ContainerViewClass);

    TotalWorkoutsViewClass = objc_allocateClassPair(ViewClass, "TotalWorkoutsView", 0);
    class_addIvar(TotalWorkoutsViewClass, dataKey, sizeof(TotalWorkoutsView), 0, "{?=@@}");
    objc_registerClassPair(TotalWorkoutsViewClass);

    WorkoutTypeViewClass = objc_allocateClassPair(ViewClass, "WorkoutTypeView", 0);
    class_addIvar(WorkoutTypeViewClass, dataKey, sizeof(WorkoutTypeView), 0, "{?=@@}");
    objc_registerClassPair(WorkoutTypeViewClass);

    LiftViewClass = objc_allocateClassPair(ViewClass, "LiftView", 0);
    class_addIvar(LiftViewClass, dataKey, sizeof(LiftView), 0, "{?=@@}");
    objc_registerClassPair(LiftViewClass);

    InputViewClass = objc_allocateClassPair(ViewClass, "InputView", 0);
    class_addIvar(InputViewClass, dataKey, sizeof(InputView), 0, "{?=@@@iisB}");
    class_addMethod(InputViewClass, deinit, (IMP) inputView_deinit, voidSig);
    objc_registerClassPair(InputViewClass);

    InputVCClass = objc_allocateClassPair(VCClass, "InputVC", 0);
    class_addProtocol(InputVCClass, objc_getProtocol("UITextFieldDelegate"));
    class_addIvar(InputVCClass, "validatorData", sizeof(InputVC), 0, "{=[4@]@@@@@iissB}");
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

    SettingsVCClass = objc_allocateClassPair(InputVCClass, "SettingsVC", 0);
    class_addIvar(SettingsVCClass, dataKey, sizeof(SettingsVC), 0, "{?=@@@[4s]}");
    class_addMethod(SettingsVCClass, viewLoad, (IMP) settingsVC_viewDidLoad, voidSig);
    class_addMethod(SettingsVCClass, btnTap, (IMP) settingsVC_buttonTapped, tapSig);
    objc_registerClassPair(SettingsVCClass);

    SetupWorkoutVCClass = objc_allocateClassPair(InputVCClass, "SetupWorkoutVC", 0);
    class_addProtocol(SetupWorkoutVCClass, objc_getProtocol("UIPickerViewDelegate"));
    class_addProtocol(SetupWorkoutVCClass, objc_getProtocol("UIPickerViewDataSource"));
    class_addIvar(SetupWorkoutVCClass, dataKey, sizeof(SetupWorkoutVC), 0, "{?=@@@ii}");
    class_addMethod(SetupWorkoutVCClass, sel_getUid("dealloc"),
                    (IMP) setupWorkoutVC_deinit, voidSig);
    class_addMethod(SetupWorkoutVCClass, viewLoad, (IMP) setupWorkoutVC_viewDidLoad, voidSig);
    class_addMethod(SetupWorkoutVCClass, sel_getUid("buttonTapped:"),
                    (IMP) setupWorkoutVC_tappedButton, tapSig);
    class_addMethod(SetupWorkoutVCClass, sel_getUid("pickerView:numberOfRowsInComponent:"),
                    (IMP) setupWorkoutVC_numberOfRows, "q@:@q");

    UpdateMaxesVCClass = objc_allocateClassPair(InputVCClass, "UpdateMaxesVC", 0);
    class_addIvar(UpdateMaxesVCClass, dataKey, sizeof(UpdateMaxesVC), 0, "{?=@@@@is}");
    class_addMethod(UpdateMaxesVCClass, deinit, (IMP) updateMaxesVC_deinit, voidSig);
    class_addMethod(UpdateMaxesVCClass, viewLoad, (IMP) updateMaxesVC_viewDidLoad, voidSig);
    class_addMethod(UpdateMaxesVCClass, sel_getUid("stepperChanged"),
                    (IMP) updateMaxesVC_updatedStepper, voidSig);
    class_addMethod(UpdateMaxesVCClass, sel_getUid("tappedFinish"),
                    (IMP) updateMaxesVC_tappedFinish, voidSig);
    objc_registerClassPair(UpdateMaxesVCClass);

    HomeVCClass = objc_allocateClassPair(VCClass, "HomeVC", 0);
    class_addIvar(HomeVCClass, dataKey, sizeof(HomeVC), 0, "{?=@@[2@]i}");
    class_addMethod(HomeVCClass, viewLoad, (IMP) homeVC_viewDidLoad, voidSig);
    class_addMethod(HomeVCClass, btnTap, (IMP) homeVC_workoutButtonTapped, tapSig);
    class_addMethod(HomeVCClass, sel_getUid("customButtonTapped:"),
                    (IMP) homeVC_customButtonTapped, tapSig);
    objc_registerClassPair(HomeVCClass);

    HistoryVCClass = objc_allocateClassPair(VCClass, "HistoryVC", 0);
    class_addProtocol(HistoryVCClass, getValueFormatterType());
    class_addIvar(HistoryVCClass, dataKey, sizeof(HistoryVC), 0,
                  "{?={?={?=@@@@[3f][3f]}{?=[5@][4@]@[5@]@[3[4i]][3f]}"
                  "{?=[4@][4@]@[4@]@[3[4f]][3f]}@[3i][3i]}@[3@]}");
    class_addMethod(HistoryVCClass, viewLoad, (IMP) historyVC_viewDidLoad, voidSig);
    class_addMethod(HistoryVCClass, btnTap, (IMP) historyVC_updateSegment, tapSig);
    class_addMethod(HistoryVCClass, getValueFormatterAction(),
                    (IMP) historyVC_stringForValue, "@@:d");
    objc_registerClassPair(HistoryVCClass);

    WorkoutVCClass = objc_allocateClassPair(VCClass, "WorkoutVC", 0);
    class_addIvar(WorkoutVCClass, dataKey, sizeof(WorkoutVC), 0,
                  "{?=[2{?={_opaque_pthread_mutex_t=q[56c]}{_opaque_pthread_cond_t=q[40c]}@qiiI"
                  "{?=CCC}}][2^{_opaque_pthread_t}]@@@[2@]{?=i{?=ii}}[4s]B}");
    class_addMethod(WorkoutVCClass, deinit, (IMP) workoutVC_deinit, voidSig);
    class_addMethod(WorkoutVCClass, viewLoad, (IMP) workoutVC_viewDidLoad, voidSig);
    class_addMethod(WorkoutVCClass, sel_getUid("startEndWorkout:"),
                    (IMP) workoutVC_startEndWorkout, tapSig);
    class_addMethod(WorkoutVCClass, sel_getUid("viewWillDisappear:"),
                    (IMP) workoutVC_willDisappear, appearSig);
    class_addMethod(WorkoutVCClass, btnTap, (IMP) workoutVC_handleTap, tapSig);
    objc_registerClassPair(WorkoutVCClass);

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
