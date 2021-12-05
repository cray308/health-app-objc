#include "HistoryVC.h"
#include <stdio.h>
#include "AppCoordinator.h"
#include "ContainerView.h"
#include "ViewControllerHelpers.h"
#include "LiftingView.h"
#include "TotalWorkoutsView.h"
#include "WorkoutTypeView.h"

extern void setLegendLabel(id v, int index, CFStringRef text);
extern void disableLineChartView(id v);

Class HistoryVCClass;
Ivar HistoryVCDataRef;

id historyVC_init(void *delegate) {
    id self = createVC(HistoryVCClass);
    HistoryVCData *data = malloc(sizeof(HistoryVCData));
    data->model = &((HistoryTabCoordinator *)delegate)->model;
    object_setIvar(self, HistoryVCDataRef, (id) data);
    return self;
}

void historyVC_viewDidLoad(id self, SEL _cmd) {
    struct objc_super super = {self, objc_getClass("UIViewController")};
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);

    HistoryVCData *data = (HistoryVCData *) object_getIvar(self, HistoryVCDataRef);
    id view = getView(self);
    setBackground(view, createColor(ColorPrimaryBG));
    setVCTitle(self, localize(CFSTR("titles1")));

    CFStringRef titles[3];
    fillStringArray(titles, CFSTR("chartHeader%d"), 3);

    data->charts[0] = totalWorkoutsView_init(&data->model->totalWorkouts, self);
    data->charts[1] = workoutTypeView_init(&data->model->workoutTypes, self);
    data->charts[2] = liftingView_init(&data->model->lifts, self);

    SEL btnTap = sel_getUid("buttonTapped:");
    data->picker = createSegmentedControl(CFSTR("historySegment%d"), 3, 0, self, btnTap, -1);
    id navItem = getNavItem(self);
    setObject(navItem, sel_getUid("setTitleView:"), data->picker);

    id containers[3];
    for (int i = 0; i < 3; ++i) {
        containers[i] = containerView_init(titles[i], !i ? HideDivider : 0, 0, false);
        containerView_add(containers[i], data->charts[i]);
    }

    id vStack = createStackView(containers, 3, 1, 5, (Padding){10, 8, 10, 8});
    id scrollView = createScrollView();

    addSubview(view, scrollView);
    id guide = getLayoutGuide(view);
    pin(scrollView, guide, (Padding){0}, 0);
    addVStackToScrollView(vStack, scrollView);

    releaseObj(vStack);
    releaseObj(scrollView);
    for (int i = 0; i < 3; ++i) {
        releaseObj(containers[i]);
    }

    setObject(self, btnTap, data->picker);
    appCoordinator->loadedViewControllers |= LoadedVC_History;
}

void historyVC_updateSegment(id self, SEL _cmd _U_, id picker) {
    HistoryVCData *data = (HistoryVCData *) object_getIvar(self, HistoryVCDataRef);
    HistoryViewModel *model = data->model;
    TotalWorkoutsViewData *twData =
    (TotalWorkoutsViewData *) object_getIvar(data->charts[0], TotalWorkoutsViewDataRef);
    WorkoutTypeViewData *aData =
    (WorkoutTypeViewData *) object_getIvar(data->charts[1], WorkoutTypeViewDataRef);
    LiftViewData *lData = (LiftViewData *) object_getIvar(data->charts[2], LiftViewDataRef);
    if (!model->nEntries[2]) {
        disableLineChartView(twData->chart);
        disableLineChartView(aData->chart);
        disableLineChartView(lData->chart);
        return;
    }

    int index = getSelectedSegment(picker);
    char buf[10];
    CFStringRef label = CFStringCreateWithFormat(NULL, NULL, model->totalWorkouts.legendFormat,
                                                 model->totalWorkouts.avgs[index]);
    setLegendLabel(twData->chart, 0, label);
    CFRelease(label);

    for (int i = 0; i < 4; ++i) {
        int typeAverage = model->workoutTypes.avgs[index][i];
        if (typeAverage > 59) {
            sprintf(buf, "%d h %d m", typeAverage / 60, typeAverage % 60);
        } else {
            sprintf(buf, "%d m", typeAverage);
        }
        label = CFStringCreateWithFormat(NULL, NULL, model->workoutTypes.legendFormat,
                                         model->workoutTypes.names[i], buf);
        setLegendLabel(aData->chart, i, label);
        CFRelease(label);

        label = CFStringCreateWithFormat(NULL, NULL, model->lifts.legendFormat,
                                         model->lifts.names[i], model->lifts.avgs[index][i]);
        setLegendLabel(lData->chart, i, label);
        CFRelease(label);
    }

    totalWorkoutsView_update(data->charts[0], model->nEntries[index], index);
    workoutTypeView_update(data->charts[1], model->nEntries[index], index);
    liftingView_update(data->charts[2], model->nEntries[index], index);
}

CFStringRef historyVC_stringForValue(id self, SEL _cmd _U_, double value) {
    struct HistFormatter *formatter = &((HistoryVCData *)
                                        object_getIvar(self, HistoryVCDataRef))->model->formatter;
    struct HistTimeData *data = &formatter->data[(int) value];
    CFRelease(formatter->currString);
    formatter->currString = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@/%d/%d"),
                                                     formatter->months[data->month],
                                                     data->day, data->year);
    return formatter->currString;
}

void historyVC_refresh(id self) {
    id picker = ((HistoryVCData *) object_getIvar(self, HistoryVCDataRef))->picker;
    setInt(picker, sel_getUid("setSelectedSegmentIndex:"), 0);
    setObject(self, sel_getUid("buttonTapped:"), picker);
}

void historyVC_updateColors(id self) {
    id picker = ((HistoryVCData *) object_getIvar(self, HistoryVCDataRef))->picker;
    id view = getView(self);
    setBackground(view, createColor(ColorPrimaryBG));
    updateSegmentedControl(picker);
}
