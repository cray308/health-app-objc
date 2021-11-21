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
#include "SetupWorkoutModal.h"
#include "UpdateMaxesModal.h"
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
    class_addIvar(StatusViewClass, dataKey, sizeof(StatusViewData*), 0, "^{__statusViewData=@@@@}");
    class_addMethod(StatusViewClass, deinit, (IMP) statusView_deinit, voidSig);
    objc_registerClassPair(StatusViewClass);
    StatusViewDataRef = class_getInstanceVariable(StatusViewClass, dataKey);

    ContainerViewClass = objc_allocateClassPair(viewClass, "ContainerView", 0);
    class_addIvar(ContainerViewClass, dataKey,
                  sizeof(ContainerViewData*), 0, "^{__containerViewData=@@@@}");
    class_addMethod(ContainerViewClass, deinit, (IMP) containerView_deinit, voidSig);
    objc_registerClassPair(ContainerViewClass);
    ContainerViewDataRef = class_getInstanceVariable(ContainerViewClass, dataKey);

    TotalWorkoutsClass = objc_allocateClassPair(viewClass, "TotalWorkoutsView", 0);
    class_addIvar(TotalWorkoutsClass, dataKey,
                  sizeof(TotalWorkoutsData*), 0, "^{__totalWorkoutsData=@@}");
    objc_registerClassPair(TotalWorkoutsClass);
    TotalWorkoutsDataRef = class_getInstanceVariable(TotalWorkoutsClass, dataKey);

    WorkoutTypeClass = objc_allocateClassPair(viewClass, "WorkoutTypeView", 0);
    class_addIvar(WorkoutTypeClass, dataKey,
                  sizeof(WorkoutTypeData*), 0, "^{__workoutTypeData=@@}");
    objc_registerClassPair(WorkoutTypeClass);
    WorkoutTypeDataRef = class_getInstanceVariable(WorkoutTypeClass, dataKey);

    LiftVClass = objc_allocateClassPair(viewClass, "LiftView", 0);
    class_addIvar(LiftVClass, dataKey, sizeof(LiftVData*), 0, "^{__liftVData=@@}");
    objc_registerClassPair(LiftVClass);
    LiftVDataRef = class_getInstanceVariable(LiftVClass, dataKey);

    InputViewClass = objc_allocateClassPair(viewClass, "InputView", 0);
    class_addIvar(InputViewClass, dataKey,
                  sizeof(InputViewData*), 0, "^{__inputViewData=sssB@@@@@}");
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

    SetupWorkoutModalClass = objc_allocateClassPair(InputVCClass, "SetupWorkoutVC", 0);
    class_addProtocol(SetupWorkoutModalClass, objc_getProtocol("UIPickerViewDelegate"));
    class_addProtocol(SetupWorkoutModalClass, objc_getProtocol("UIPickerViewDataSource"));
    class_addIvar(SetupWorkoutModalClass, dataKey, sizeof(SetupWorkoutModalData*), 0,
                  "^{__setupWorkoutModalData=@@@{__workoutParams=cCiiii}}");
    class_addMethod(SetupWorkoutModalClass, deinit, (IMP) setupWorkoutVC_deinit, voidSig);
    class_addMethod(SetupWorkoutModalClass, viewLoad, (IMP) setupWorkoutVC_viewDidLoad, voidSig);
    class_addMethod(SetupWorkoutModalClass, btnTap, (IMP) setupWorkoutVC_tappedButton, tapSig);
    class_addMethod(SetupWorkoutModalClass, sel_getUid("numberOfComponentsInPickerView:"),
                    (IMP) setupWorkoutVC_numberOfComponents, "q@:@");
    class_addMethod(SetupWorkoutModalClass, sel_getUid("pickerView:numberOfRowsInComponent:"),
                    (IMP) setupWorkoutVC_numberOfRows, "q@:@q");
    class_addMethod(SetupWorkoutModalClass, sel_getUid("pickerView:titleForRow:forComponent:"),
                    (IMP) setupWorkoutVC_titleForRow, "@@:@qq");
    class_addMethod(SetupWorkoutModalClass, sel_getUid("pickerView:didSelectRow:inComponent:"),
                    (IMP) setupWorkoutVC_didSelectRow, "v@:@qq");
    objc_registerClassPair(SetupWorkoutModalClass);
    SetupWorkoutModalDataRef = class_getInstanceVariable(SetupWorkoutModalClass, dataKey);

    UpdateMaxesClass = objc_allocateClassPair(InputVCClass, "UpdateMaxesVC", 0);
    class_addIvar(UpdateMaxesClass, dataKey, sizeof(void*), 0, "@");
    class_addMethod(UpdateMaxesClass, viewLoad, (IMP) updateMaxesVC_viewDidLoad, voidSig);
    class_addMethod(UpdateMaxesClass, sel_getUid("tappedFinish"),
                    (IMP) updateMaxesVC_tappedFinish, voidSig);
    objc_registerClassPair(UpdateMaxesClass);
    UpdateMaxesDataRef = class_getInstanceVariable(UpdateMaxesClass, dataKey);

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
    class_addIvar(WorkoutVCClass, dataKey, sizeof(WorkoutVCData*), 0,
                  "^{__workoutVCData=@@@[10@][2@][2@]{__savedWorkoutInfo=I{__exerciseInfo=II}}[2{__workoutTimer=@{__timerInfo=CCC}{_opaque_pthread_mutex_t=q[56c]}{_opaque_pthread_cond_t=q[40c]}IIiq}]}");
#else
    class_addIvar(WorkoutVCClass, dataKey, sizeof(WorkoutVCData*), 0,
                  "^{__workoutVCData=@@@[10@][2@][2@]{__savedWorkoutInfo=I{__exerciseInfo=II}}[2{__workoutTimer=@{__timerInfo=CCC}{_opaque_pthread_mutex_t=q[40c]}{_opaque_pthread_cond_t=q[24c]}IIiq}]}");
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
