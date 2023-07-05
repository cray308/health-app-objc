#include "AppDelegate.h"
#include "CollectionVC.h"
#include "CustomButton.h"
#include "HeaderView.h"
#include "HistoryVC.h"
#include "HomeVC.h"
#include "InputVC.h"
#include "SettingsVC.h"
#include "SetupWorkoutVC.h"
#include "StatusCell.h"
#include "UpdateMaxesVC.h"
#include "Views_VCExt.h"
#include "WorkoutVC.h"

extern int UIApplicationMain(int, char *[], CFStringRef, CFStringRef);

int main(int argc, char *argv[]) {
    checkMainArgs(argc)
    initAppData((Class *[]){&View, &VC});
    initViewData((void(*[])(void)){
        initCellData, initValidatorData, initCollectionData, initUpdateMaxesData
    }, 4);
    initVCData();
    char *superKey = "parentData", *dataKey = "data", *voidSig = "v@:", *tapSig = "v@:@";
    char *cellSig = "@@:@@", *selectSig = "v@:@@", *sizeSig = "{?=dd}@:@@@";
    char *initSig = "@@:{?={?=dd}{?=dd}}", *bool2Arg = "B@:@@";
    SEL init = getViewInitSel(), deinit = sel_getUid("dealloc");
    SEL viewLoad = sel_getUid("viewDidLoad"), tap = getTapSel();
    SEL getCell = sel_getUid("collectionView:cellForItemAtIndexPath:");
    SEL shouldSelect = sel_getUid("collectionView:shouldSelectItemAtIndexPath:");
    SEL didSelect = sel_getUid("collectionView:didSelectItemAtIndexPath:");

    CustomButton = objc_allocateClassPair(Button, "CustomButton", 0);
    class_addMethod(CustomButton, sel_getUid("accessibilityTraits"),
                    (IMP)customButton_getAccessibilityTraits, "Q@:{?={?=dd}{?=dd}}");
    objc_registerClassPair(CustomButton);

    RootCell = objc_allocateClassPair(Cell, "StatusCell", 0);
    class_addIvar(RootCell, dataKey, sizeof(StatusCell), 0, "{?=@@@}");
    class_addMethod(RootCell, init, (IMP)rootCell_initWithFrame, initSig);
    class_addMethod(RootCell, deinit, (IMP)rootCell_deinit, voidSig);
    objc_registerClassPair(RootCell);

    BasicCell = objc_allocateClassPair(RootCell, "BasicCell", 0);
    class_addMethod(BasicCell, init, (IMP)basicCell_initWithFrame, initSig);
    objc_registerClassPair(BasicCell);

    FullCell = objc_allocateClassPair(RootCell, "FullCell", 0);
    class_addMethod(FullCell, init, (IMP)fullCell_initWithFrame, initSig);
    objc_registerClassPair(FullCell);

    ReusableView = objc_getClass("UICollectionReusableView");
    ReusableViewSize = class_getInstanceSize(ReusableView);
    HeaderViewClass = objc_allocateClassPair(ReusableView, "HeaderView", 0);
    class_addIvar(HeaderViewClass, dataKey, sizeof(HeaderView), 0, "{?=@@}");
    class_addMethod(HeaderViewClass, init, (IMP)headerView_initWithFrame, initSig);
    class_addMethod(HeaderViewClass, deinit, (IMP)headerView_deinit, voidSig);
    objc_registerClassPair(HeaderViewClass);

    InputViewClass = objc_allocateClassPair(View, "InputView", 0);
    class_addIvar(InputViewClass, dataKey, sizeof(InputView), 0, "{?=@@@iifC}");
    class_addMethod(InputViewClass, deinit, (IMP)inputView_deinit, voidSig);
    objc_registerClassPair(InputViewClass);

    StepperViewClass = objc_allocateClassPair(View, "StepperView", 0);
    class_addIvar(StepperViewClass, dataKey, sizeof(StepperView), 0, "{?=@@@{?=qq}}");
    class_addMethod(StepperViewClass, deinit, (IMP)stepperView_deinit, voidSig);
    class_addMethod(StepperViewClass, getValueChangedSel(), (IMP)stepperView_changedValue, voidSig);
    class_addMethod(StepperViewClass, sel_getUid("accessibilityIncrement"),
                    (IMP)stepperView_accessibilityIncrement, voidSig);
    class_addMethod(StepperViewClass, sel_getUid("accessibilityDecrement"),
                    (IMP)stepperView_accessibilityDecrement, voidSig);
    objc_registerClassPair(StepperViewClass);

    InputVCClass = objc_allocateClassPair(VC, "InputVC", 0);
    class_addProtocol(InputVCClass, objc_getProtocol("UITextFieldDelegate"));
    class_addIvar(InputVCClass, superKey, sizeof(InputVC), 0, "{?=[4{?=@@}]@@@@ii}");
    class_addMethod(InputVCClass, deinit, (IMP)inputVC_deinit, voidSig);
    class_addMethod(InputVCClass, viewLoad, (IMP)inputVC_viewDidLoad, voidSig);
    class_addMethod(InputVCClass, getDismissKeyboardSel(), (IMP)inputVC_dismissKeyboard, voidSig);
    class_addMethod(InputVCClass, getJumpToPrevSel(), (IMP)inputVC_jumpToPrev, voidSig);
    class_addMethod(InputVCClass, getJumpToNextSel(), (IMP)inputVC_jumpToNext, voidSig);
    class_addMethod(InputVCClass, sel_getUid("textFieldDidBeginEditing:"),
                    (IMP)inputVC_textFieldDidBeginEditing, tapSig);
    class_addMethod(InputVCClass, sel_getUid("textFieldDidEndEditing:"),
                    (IMP)inputVC_textFieldDidEndEditing, tapSig);
    class_addMethod(InputVCClass, sel_getUid("textFieldShouldReturn:"),
                    (IMP)inputVC_textFieldShouldReturn, "B@:@");
    class_addMethod(InputVCClass,
                    sel_getUid("textField:shouldChangeCharactersInRange:replacementString:"),
                    (IMP)inputVC_shouldChange, "B@:@{?=qq}@");
    objc_registerClassPair(InputVCClass);

    SettingsVCClass = objc_allocateClassPair(InputVCClass, "SettingsVC", 0);
    class_addIvar(SettingsVCClass, dataKey, sizeof(SettingsVC), 0, "{?=@[4i]}");
    class_addMethod(SettingsVCClass, viewLoad, (IMP)settingsVC_viewDidLoad, voidSig);
    class_addMethod(SettingsVCClass, tap, (IMP)settingsVC_buttonTapped, tapSig);
    objc_registerClassPair(SettingsVCClass);

    SetupWorkoutVCClass = objc_allocateClassPair(InputVCClass, "SetupWorkoutVC", 0);
    class_addProtocol(SetupWorkoutVCClass, objc_getProtocol("UIPickerViewDelegate"));
    class_addProtocol(SetupWorkoutVCClass, objc_getProtocol("UIPickerViewDataSource"));
    class_addIvar(SetupWorkoutVCClass, dataKey, sizeof(SetupWorkoutVC), 0, "{?=@@@iC}");
    class_addMethod(SetupWorkoutVCClass, deinit, (IMP)setupWorkoutVC_deinit, voidSig);
    class_addMethod(SetupWorkoutVCClass, viewLoad, (IMP)setupWorkoutVC_viewDidLoad, voidSig);
    class_addMethod(SetupWorkoutVCClass, tap, (IMP)setupWorkoutVC_tappedButton, tapSig);
    class_addMethod(SetupWorkoutVCClass, sel_getUid("numberOfComponentsInPickerView:"),
                    (IMP)setupWorkoutVC_numberOfComponentsInPickerView, "q@:@");
    class_addMethod(SetupWorkoutVCClass, sel_getUid("pickerView:numberOfRowsInComponent:"),
                    (IMP)setupWorkoutVC_numberOfRows, "q@:@q");
    class_addMethod(SetupWorkoutVCClass, sel_getUid("pickerView:titleForRow:forComponent:"),
                    (IMP)setupWorkoutVC_titleForRow, "@@:@qq");
    class_addMethod(SetupWorkoutVCClass, sel_getUid("pickerView:didSelectRow:inComponent:"),
                    (IMP)setupWorkoutVC_didSelectRow, "v@:@qq");
    objc_registerClassPair(SetupWorkoutVCClass);

    UpdateMaxesVCClass = objc_allocateClassPair(InputVCClass, "UpdateMaxesVC", 0);
    class_addIvar(UpdateMaxesVCClass, dataKey, sizeof(UpdateMaxesVC), 0, "{?=@@i}");
    class_addMethod(UpdateMaxesVCClass, deinit, (IMP)updateMaxesVC_deinit, voidSig);
    class_addMethod(UpdateMaxesVCClass, viewLoad, (IMP)updateMaxesVC_viewDidLoad, voidSig);
    class_addMethod(UpdateMaxesVCClass, tap, (IMP)updateMaxesVC_tappedFinish, tapSig);
    objc_registerClassPair(UpdateMaxesVCClass);

    HistoryVCClass = objc_allocateClassPair(VC, "HistoryVC", 0);
    class_addProtocol(HistoryVCClass, objc_getProtocol("Charts.ValueFormatter"));
    class_addIvar(HistoryVCClass, dataKey, sizeof(HistoryVC), 0,
                  "{?={?={?=@@[3f]}{?=[5@][5@][3[4i]]}{?=[4@][4@][3[4f]]}"
                  "@[3@][3[3f]][3i][3i]}@[3@]}");
    class_addMethod(HistoryVCClass, viewLoad, (IMP)historyVC_viewDidLoad, voidSig);
    class_addMethod(HistoryVCClass, tap, (IMP)historyVC_changedSegment, tapSig);
    class_addMethod(HistoryVCClass, sel_getUid("stringForValue:"),
                    (IMP)historyVC_stringForValue, "@@:d");
    objc_registerClassPair(HistoryVCClass);

    CollectionVCClass = objc_allocateClassPair(VC, "CollectionVC", 0);
    class_addIvar(CollectionVCClass, superKey, sizeof(CollectionVC), 0, "{?=@@@@@@@iiii}");
    class_addMethod(CollectionVCClass, deinit, (IMP)collectionVC_deinit, voidSig);
    class_addMethod(CollectionVCClass, viewLoad, (IMP)collectionVC_viewDidLoad, voidSig);
    class_addMethod(CollectionVCClass,
                    sel_getUid("viewWillTransitionToSize:withTransitionCoordinator:"),
                    (IMP)collectionVC_viewWillTransitionToSize, "v@:{?=dd}@");
    objc_registerClassPair(CollectionVCClass);

    HomeVCClass = objc_allocateClassPair(CollectionVCClass, "HomeVC", 0);
    class_addIvar(HomeVCClass, dataKey, sizeof(HomeVC), 0, "{?=[6i]iC}");
    class_addMethod(HomeVCClass, viewLoad, (IMP)homeVC_viewDidLoad, voidSig);
    class_addMethod(HomeVCClass, getCell, (IMP)homeVC_cellForItemAtIndexPath, cellSig);
    class_addMethod(HomeVCClass, shouldSelect, (IMP)homeVC_shouldSelectItem, bool2Arg);
    class_addMethod(HomeVCClass, didSelect, (IMP)homeVC_didSelectItemAtIndexPath, selectSig);

    WorkoutVCClass = objc_allocateClassPair(CollectionVCClass, "WorkoutVC", 0);
    class_addIvar(WorkoutVCClass, dataKey, sizeof(WorkoutVC), 0, "{?=[2{?=qiiiiCC}]@[4i]B}");
    class_addMethod(WorkoutVCClass, deinit, (IMP)workoutVC_deinit, voidSig);
    class_addMethod(WorkoutVCClass, viewLoad, (IMP)workoutVC_viewDidLoad, voidSig);
    class_addMethod(WorkoutVCClass, tap, (IMP)workoutVC_startEndWorkout, tapSig);
    class_addMethod(WorkoutVCClass, sel_getUid("viewWillDisappear:"),
                    (IMP)workoutVC_viewWillDisappear, "v@:B");
    class_addMethod(WorkoutVCClass, getCell, (IMP)workoutVC_cellForItemAtIndexPath, cellSig);
    class_addMethod(WorkoutVCClass, shouldSelect, (IMP)workoutVC_shouldSelectItem, bool2Arg);
    class_addMethod(WorkoutVCClass, didSelect, (IMP)workoutVC_didSelectItemAtIndexPath, selectSig);

    Protocol *protocols[] = {
        objc_getProtocol("UICollectionViewDataSource"),
        objc_getProtocol("UICollectionViewDelegateFlowLayout")
    };
    Class collectionClasses[] = {HomeVCClass, WorkoutVCClass};
    for (int i = 0; i < 2; ++i) {
        Class class = collectionClasses[i];
        class_addMethod(class, sel_getUid("numberOfSectionsInCollectionView:"),
                        (IMP)collectionVC_numberOfSectionsInCollectionView, "q@:");
        class_addMethod(class, sel_getUid("collectionView:numberOfItemsInSection:"),
                        (IMP)collectionVC_numberOfItemsInSection, "q@:@q");
        class_addMethod(class,
                        sel_getUid("collectionView:viewForSupplementaryElementOfKind:atIndexPath:"),
                        (IMP)collectionVC_viewForSupplementaryElement, "@@:@@@");
        class_addMethod(class, sel_getUid("collectionView:layout:sizeForItemAtIndexPath:"),
                        (IMP)collectionVC_sizeForItem, sizeSig);
        class_addMethod(class, sel_getUid("collectionView:layout:insetForSectionAtIndex:"),
                        (IMP)collectionVC_insetForSection, "{?=dddd}@:@@q");
        class_addMethod(class, sel_getUid("collectionView:layout:referenceSizeForHeaderInSection:"),
                        (IMP)collectionVC_referenceSizeForHeader, sizeSig);
        class_addProtocol(class, protocols[0]);
        class_addProtocol(class, protocols[1]);
        objc_registerClassPair(class);
    }

    Class AppDelegateClass = objc_allocateClassPair(objc_getClass("UIResponder"), "AppDelegate", 0);
    class_addProtocol(AppDelegateClass, objc_getProtocol("UNUserNotificationCenterDelegate"));
    class_addIvar(AppDelegateClass, dataKey, sizeof(AppDelegate), 0, "@[3@]{?=qq[4i]CC}");
    class_addMethod(AppDelegateClass, sel_getUid("application:didFinishLaunchingWithOptions:"),
                    (IMP)appDelegate_didFinishLaunching, bool2Arg);
    class_addMethod(AppDelegateClass,
                    sel_getUid("application:supportedInterfaceOrientationsForWindow:"),
                    (IMP)appDelegate_supportedInterfaceOrientations, "Q@:@@");
    class_addMethod(AppDelegateClass, sel_getUid("userNotificationCenter:"
                                                 "willPresentNotification:withCompletionHandler:"),
                    (IMP)appDelegate_willPresentNotification, "v@:@@?");
    objc_registerClassPair(AppDelegateClass);
    return UIApplicationMain(argc, argv, nil, CFSTR("AppDelegate"));
}
