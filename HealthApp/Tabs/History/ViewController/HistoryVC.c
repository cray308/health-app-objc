#include "HistoryVC.h"
#include <stdio.h>
#include "AppCoordinator.h"
#include "ContainerView.h"
#include "ViewControllerHelpers.h"
#include "LiftingView.h"
#include "TotalWorkoutsView.h"
#include "WorkoutTypeView.h"

extern id createChartEntry(int x, int y);
extern void setLegendLabel(id entry, CFStringRef text);
extern void disableLineChartView(id v);

enum {
    HistoryTimeRange6Months,
    HistoryTimeRange1Year
};

Class HistoryVCClass;
Ivar HistoryVCDataRef;

static inline void createNewEntry(Array_object *arr, int x, int y) {
    array_push_back(object, arr, createChartEntry(x, y));
}

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

    data->charts[0] = totalWorkoutsView_init(&data->model->totalWorkoutsModel, self);
    data->charts[1] = workoutTypeView_init(&data->model->workoutTypeModel, self);
    data->charts[2] = liftingView_init(&data->model->liftModel, self);

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

    int size = 0;
    if (!(size = (model->data->size))) {
        TotalWorkoutsViewData *twData =
        (TotalWorkoutsViewData *) object_getIvar(data->charts[0], TotalWorkoutsViewDataRef);
        WorkoutTypeViewData *aData =
        (WorkoutTypeViewData *) object_getIvar(data->charts[1], WorkoutTypeViewDataRef);
        LiftViewData *lData = (LiftViewData *) object_getIvar(data->charts[2], LiftViewDataRef);
        disableLineChartView(twData->chart);
        disableLineChartView(aData->chart);
        disableLineChartView(lData->chart);
        return;
    }

    int index = getInt(picker, sel_getUid("selectedSegmentIndex"));
    int startIndex = 0;
    if (index == HistoryTimeRange6Months) {
        startIndex = size - 26;
    } else if (index == HistoryTimeRange1Year) {
        startIndex = size - 52;
    }

    if (startIndex < 0)
        startIndex = 0;
    int nEntries = size - startIndex;
    bool isSmall = nEntries < 7;

    model->totalWorkoutsModel.avgWorkouts = 0;
    model->totalWorkoutsModel.yMax = 0;
    model->workoutTypeModel.yMax = 0;
    model->liftModel.yMax = 0;
    memset(model->liftModel.totalByExercise, 0, sizeof(int) << 2);
    memset(model->workoutTypeModel.totalByType, 0, sizeof(int) << 2);
    array_clear(object, model->totalWorkoutsModel.entries);
    for (int i = 0; i < 4; ++i) {
        array_clear(object, model->workoutTypeModel.entries[i]);
        array_clear(object, model->liftModel.entries[i]);
    }
    array_clear(object, model->workoutTypeModel.entries[4]);

    HistoryWeekDataModel *arr = model->data->arr;
    int totalWorkouts = 0, maxWorkouts = 0, maxActivityTime = 0, maxWeight = 0;

    for (int i = startIndex; i < size; ++i) {
        HistoryWeekDataModel *e = &arr[i];

        totalWorkouts += e->totalWorkouts;
        if (e->totalWorkouts > maxWorkouts)
            maxWorkouts = e->totalWorkouts;
        createNewEntry(model->totalWorkoutsModel.entries, i, e->totalWorkouts);

        for (int j = 0; j < 4; ++j) {
            model->workoutTypeModel.totalByType[j] += e->durationByType[j];

            model->liftModel.totalByExercise[j] += e->weightArray[j];
            if (e->weightArray[j] > maxWeight)
                maxWeight = e->weightArray[j];
            createNewEntry(model->liftModel.entries[j], i, e->weightArray[j]);
        }

        if (e->cumulativeDuration[3] > maxActivityTime)
            maxActivityTime = e->cumulativeDuration[3];
        createNewEntry(model->workoutTypeModel.entries[0], i, 0);
        for (int j = 1; j < 5; ++j)
            createNewEntry(model->workoutTypeModel.entries[j], i, e->cumulativeDuration[j - 1]);
    }

    model->totalWorkoutsModel.avgWorkouts = (float) totalWorkouts / nEntries;
    model->totalWorkoutsModel.yMax = maxWorkouts < 7 ? 7 : 1.1 * maxWorkouts;
    model->workoutTypeModel.yMax = 1.1 * maxActivityTime;
    model->liftModel.yMax = 1.1 * maxWeight;

    char buf[10];
    CFStringRef label = CFStringCreateWithFormat(NULL, NULL, model->totalWorkoutsModel.legendFormat,
                                                 model->totalWorkoutsModel.avgWorkouts);
    setLegendLabel(model->totalWorkoutsModel.legendEntries[0], label);
    CFRelease(label);

    for (int i = 0; i < 4; ++i) {
        float liftAverage = (float) model->liftModel.totalByExercise[i] / nEntries;
        int typeAverage = model->workoutTypeModel.totalByType[i] / nEntries;
        if (typeAverage > 59) {
            sprintf(buf, "%d h %d m", typeAverage / 60, typeAverage % 60);
        } else {
            sprintf(buf, "%d m", typeAverage);
        }
        label = CFStringCreateWithFormat(NULL, NULL, model->workoutTypeModel.legendFormat,
                                         model->workoutTypeModel.names[i], buf);
        setLegendLabel(model->workoutTypeModel.legendEntries[i], label);
        CFRelease(label);

        label = CFStringCreateWithFormat(NULL, NULL, model->liftModel.legendFormat,
                                         model->liftModel.names[i], liftAverage);
        setLegendLabel(model->liftModel.legendEntries[i], label);
        CFRelease(label);
    }

    totalWorkoutsView_update(data->charts[0], nEntries, isSmall);
    workoutTypeView_update(data->charts[1], nEntries, isSmall);
    liftingView_update(data->charts[2], nEntries, isSmall);
}

CFStringRef historyVC_stringForValue(id self, SEL _cmd _U_, double value, id axis _U_) {
    HistoryViewModel *model = ((HistoryVCData *) object_getIvar(self, HistoryVCDataRef))->model;
    int index = (int) value;
    CFRelease(model->formatter.currString);
    HistoryWeekDataModel *data = &model->data->arr[index];
    model->formatter.currString = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@/%d/%d"),
                                                           model->formatter.months[data->month],
                                                           data->day, data->year);
    return model->formatter.currString;
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
