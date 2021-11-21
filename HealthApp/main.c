#include <CoreFoundation/CFString.h>
#include <objc/message.h>
#include "StatusView.h"
#include "TotalWorkoutsView.h"
#include "WorkoutTypeView.h"
#include "LiftingView.h"
#include "InputVC.h"
#include "HomeVC.h"
#include "HistoryVC.h"
#include "SettingsVC.h"
#include "WorkoutVC.h"
#include "SetupWorkoutVC.h"
#include "UpdateMaxesVC.h"
#include "AppDelegate.h"

extern int UIApplicationMain(int, char *[], CFStringRef, CFStringRef);
extern Protocol *getValueFormatterType(void);
extern SEL getValueFormatterAction(void);

int main(int argc, char *argv[]) {
    Class viewClass = objc_getClass("UIView");
    Class VCClass = objc_getClass("UIViewController");
    char const *validatorKey = "validatorData", *dataKey = "data";
    char const *voidSig = "v@:", *tapSig = "v@:@", *appearSig = "v@:i", *appSig = "i@:@@";
    SEL deinit = sel_getUid("dealloc");
    SEL viewLoad = sel_getUid("viewDidLoad");
    SEL btnTap = sel_getUid("buttonTapped:");

    StatusViewClass = objc_allocateClassPair(viewClass, "StatusView", 0);
    class_addIvar(StatusViewClass, dataKey, sizeof(StatusViewData*), 0, "^{__statusVData=@@@@}");
    class_addMethod(StatusViewClass, deinit, (IMP) statusView_deinit, voidSig);
    objc_registerClassPair(StatusViewClass);
    StatusViewDataRef = class_getInstanceVariable(StatusViewClass, dataKey);

    ContainerViewClass = objc_allocateClassPair(viewClass, "ContainerView", 0);
    class_addIvar(ContainerViewClass, dataKey,
                  sizeof(ContainerViewData*), 0, "^{__containerVData=@@@@}");
    class_addMethod(ContainerViewClass, deinit, (IMP) containerView_deinit, voidSig);
    objc_registerClassPair(ContainerViewClass);
    ContainerViewDataRef = class_getInstanceVariable(ContainerViewClass, dataKey);

    TotalWorkoutsViewClass = objc_allocateClassPair(viewClass, "TotalWorkoutsView", 0);
    class_addIvar(TotalWorkoutsViewClass, dataKey,
                  sizeof(TotalWorkoutsViewData*), 0, "^{__totalWorkoutsVData=@@}");
    objc_registerClassPair(TotalWorkoutsViewClass);
    TotalWorkoutsViewDataRef = class_getInstanceVariable(TotalWorkoutsViewClass, dataKey);

    WorkoutTypeViewClass = objc_allocateClassPair(viewClass, "WorkoutTypeView", 0);
    class_addIvar(WorkoutTypeViewClass, dataKey,
                  sizeof(WorkoutTypeViewData*), 0, "^{__workoutTypeVData=@@}");
    objc_registerClassPair(WorkoutTypeViewClass);
    WorkoutTypeViewDataRef = class_getInstanceVariable(WorkoutTypeViewClass, dataKey);

    LiftViewClass = objc_allocateClassPair(viewClass, "LiftView", 0);
    class_addIvar(LiftViewClass, dataKey, sizeof(LiftViewData*), 0, "^{__liftVData=@@}");
    objc_registerClassPair(LiftViewClass);
    LiftViewDataRef = class_getInstanceVariable(LiftViewClass, dataKey);

    InputViewClass = objc_allocateClassPair(viewClass, "InputView", 0);
    class_addIvar(InputViewClass, dataKey, sizeof(InputViewData*), 0, "^{__inputVData=sssB@@@@@}");
    class_addMethod(InputViewClass, deinit, (IMP) inputView_deinit, voidSig);
    objc_registerClassPair(InputViewClass);
    InputViewDataRef = class_getInstanceVariable(InputViewClass, dataKey);

    InputVCClass = objc_allocateClassPair(VCClass, "InputVC", 0);
    class_addProtocol(InputVCClass, objc_getProtocol("UITextFieldDelegate"));
    class_addIvar(InputVCClass, validatorKey,
                  sizeof(InputVCData*), 0, "^{__inputVCData=ii[4@]@@@@@@}");
    class_addMethod(InputVCClass, sel_getUid("initWithNibName:bundle:"),
                    (IMP) inputVC_init, "@@:@@");
    class_addMethod(InputVCClass, deinit, (IMP) inputVC_deinit, voidSig);
    class_addMethod(InputVCClass, viewLoad, (IMP) inputVC_viewDidLoad, voidSig);
    class_addMethod(InputVCClass, sel_getUid("viewDidAppear:"),
                    (IMP) inputVC_viewDidAppear, appearSig);
    class_addMethod(InputVCClass, sel_getUid("dismissKeyboard"),
                    (IMP) inputVC_dismissKeyboard, voidSig);
    class_addMethod(InputVCClass, sel_getUid("textFieldShouldReturn:"),
                    (IMP) inputVC_fieldShouldReturn, "i@:@");
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
    class_addIvar(SettingsVCClass, dataKey,
                  sizeof(SettingsVCData*), 0, "^{__settingsVCData=@@}");
    class_addMethod(SettingsVCClass, viewLoad, (IMP) settingsVC_viewDidLoad, voidSig);
    class_addMethod(SettingsVCClass, btnTap, (IMP) settingsVC_buttonTapped, tapSig);
    objc_registerClassPair(SettingsVCClass);
    SettingsVCDataRef = class_getInstanceVariable(SettingsVCClass, dataKey);

    SetupWorkoutVCClass = objc_allocateClassPair(InputVCClass, "SetupWorkoutVC", 0);
    class_addProtocol(SetupWorkoutVCClass, objc_getProtocol("UIPickerViewDelegate"));
    class_addProtocol(SetupWorkoutVCClass, objc_getProtocol("UIPickerViewDataSource"));
    class_addIvar(SetupWorkoutVCClass, dataKey, sizeof(SetupWorkoutVCData*), 0,
                  "^{__setupWorkoutVCData=@@@{__workoutParams=cCiiii}}");
    class_addMethod(SetupWorkoutVCClass, deinit, (IMP) setupWorkoutVC_deinit, voidSig);
    class_addMethod(SetupWorkoutVCClass, viewLoad, (IMP) setupWorkoutVC_viewDidLoad, voidSig);
    class_addMethod(SetupWorkoutVCClass, btnTap, (IMP) setupWorkoutVC_tappedButton, tapSig);
    class_addMethod(SetupWorkoutVCClass, sel_getUid("numberOfComponentsInPickerView:"),
                    (IMP) setupWorkoutVC_numberOfComponents, "q@:@");
    class_addMethod(SetupWorkoutVCClass, sel_getUid("pickerView:numberOfRowsInComponent:"),
                    (IMP) setupWorkoutVC_numberOfRows, "q@:@q");
    class_addMethod(SetupWorkoutVCClass, sel_getUid("pickerView:titleForRow:forComponent:"),
                    (IMP) setupWorkoutVC_titleForRow, "@@:@qq");
    class_addMethod(SetupWorkoutVCClass, sel_getUid("pickerView:didSelectRow:inComponent:"),
                    (IMP) setupWorkoutVC_didSelectRow, "v@:@qq");
    objc_registerClassPair(SetupWorkoutVCClass);
    SetupWorkoutVCDataRef = class_getInstanceVariable(SetupWorkoutVCClass, dataKey);

    UpdateMaxesVCClass = objc_allocateClassPair(InputVCClass, "UpdateMaxesVC", 0);
    class_addIvar(UpdateMaxesVCClass, dataKey, sizeof(void*), 0, "@");
    class_addMethod(UpdateMaxesVCClass, viewLoad, (IMP) updateMaxesVC_viewDidLoad, voidSig);
    class_addMethod(UpdateMaxesVCClass, sel_getUid("tappedFinish"),
                    (IMP) updateMaxesVC_tappedFinish, voidSig);
    objc_registerClassPair(UpdateMaxesVCClass);
    UpdateMaxesVCDataRef = class_getInstanceVariable(UpdateMaxesVCClass, dataKey);

    HomeVCClass = objc_allocateClassPair(VCClass, "HomeVC", 0);
    class_addIvar(HomeVCClass, dataKey, sizeof(HomeVCData*), 0, "^{__homeVCData=@@@@}");
    class_addMethod(HomeVCClass, viewLoad, (IMP) homeVC_viewDidLoad, voidSig);
    class_addMethod(HomeVCClass, sel_getUid("viewWillAppear:"),
                    (IMP) homeVC_viewWillAppear, appearSig);
    class_addMethod(HomeVCClass, btnTap, (IMP) homeVC_workoutButtonTapped, tapSig);
    class_addMethod(HomeVCClass, sel_getUid("customButtonTapped:"),
                    (IMP) homeVC_customButtonTapped, tapSig);
    objc_registerClassPair(HomeVCClass);
    HomeVCDataRef = class_getInstanceVariable(HomeVCClass, dataKey);

    HistoryVCClass = objc_allocateClassPair(VCClass, "HistoryVC", 0);
    class_addProtocol(HistoryVCClass, getValueFormatterType());
    class_addIvar(HistoryVCClass, dataKey, sizeof(HistoryVCData*), 0, "^{__historyVCData=@@[3@]}");
    class_addMethod(HistoryVCClass, viewLoad, (IMP) historyVC_viewDidLoad, voidSig);
    class_addMethod(HistoryVCClass, btnTap, (IMP) historyVC_updateSegment, tapSig);
    class_addMethod(HistoryVCClass, getValueFormatterAction(),
                    (IMP) historyVC_stringForValue, "@@:d@");
    objc_registerClassPair(HistoryVCClass);
    HistoryVCDataRef = class_getInstanceVariable(HistoryVCClass, dataKey);

    WorkoutVCClass = objc_allocateClassPair(VCClass, "WorkoutVC", 0);

#if defined(__LP64__)
    char const *layout = "^{__workoutVCData="
    "@@@[10@][2@][2@]{__savedWorkoutInfo=I{__exerciseInfo=II}}[2{__workoutTimer="
    "@{__timerInfo=CCC}{_opaque_pthread_mutex_t=q[56c]}{_opaque_pthread_cond_t=q[40c]}IIiq}]}";
    class_addIvar(WorkoutVCClass, dataKey, sizeof(WorkoutVCData*), 0, layout);
#else
    char const *layout = "^{__workoutVCData="
    "@@@[10@][2@][2@]{__savedWorkoutInfo=I{__exerciseInfo=II}}[2{__workoutTimer="
    "@{__timerInfo=CCC}{_opaque_pthread_mutex_t=q[40c]}{_opaque_pthread_cond_t=q[24c]}IIiq}]}";
    class_addIvar(WorkoutVCClass, dataKey, sizeof(WorkoutVCData*), 0, layout);
#endif

    class_addMethod(WorkoutVCClass, deinit, (IMP) workoutVC_deinit, voidSig);
    class_addMethod(WorkoutVCClass, viewLoad, (IMP) workoutVC_viewDidLoad, voidSig);
    class_addMethod(WorkoutVCClass, sel_getUid("startEndWorkout:"),
                    (IMP) workoutVC_startEndWorkout, tapSig);
    class_addMethod(WorkoutVCClass, btnTap, (IMP) workoutVC_handleTap, tapSig);
    objc_registerClassPair(WorkoutVCClass);
    WorkoutVCDataRef = class_getInstanceVariable(WorkoutVCClass, dataKey);

    AppDelegateClass = objc_allocateClassPair(objc_getClass("UIResponder"), "AppDelegate", 0);
    class_addIvar(AppDelegateClass, "window", sizeof(id), 0, "@");
    class_addMethod(AppDelegateClass, sel_getUid("application:didFinishLaunchingWithOptions:"),
                    (IMP) appDelegate_didFinishLaunching, appSig);
    class_addMethod(AppDelegateClass,
                    sel_getUid("application:supportedInterfaceOrientationsForWindow:"),
                    (IMP) appDelegate_supportedOrientations, appSig);
    objc_registerClassPair(AppDelegateClass);
    return UIApplicationMain(argc, argv, nil, CFSTR("AppDelegate"));
}
