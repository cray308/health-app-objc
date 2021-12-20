#include "HistoryVC.h"
#include <stdio.h>
#include "AppCoordinator.h"
#include "AppUserData.h"
#include "ContainerView.h"
#include "ExerciseManager.h"
#include "ViewControllerHelpers.h"
#include "LiftingView.h"
#include "PersistenceService.h"
#include "SwiftBridging.h"
#include "TotalWorkoutsView.h"
#include "WorkoutTypeView.h"

gen_array_source(pt, CGPoint, DSDefault_shallowCopy, DSDefault_shallowDelete)

extern void setLegendLabel(id v, int index, CFStringRef text);
extern void disableLineChartView(id v);
extern id createDataSet(int color, id fillSet);
extern id createChartData(CFArrayRef dataSets, int lineWidth, uint8_t options);

struct WeekDataModel {
    struct HistTimeData timeData;
    int totalWorkouts;
    int durationByType[4];
    int cumulativeDuration[4];
    short weightArray[4];
};

Class HistoryVCClass;
Ivar HistoryVCDataRef;

#pragma mark - Load Data

static inline void createNewEntry(Array_pt *arr, int x, int y) {
    CGPoint point = {x, y};
    array_push_back(pt, arr, point);
}

static void historyData_populate(HistoryViewModel *m, struct WeekDataModel *results, int size) {
    int innerLimits[] = {-1, 0, 1};
    int refIndices[] = {size, size - 26, size - 52, 0};
    if (refIndices[2] < 0)
        refIndices[2] = 0;
    if (refIndices[1] < 0)
        refIndices[1] = 0;

    memcpy(m->nEntries, (int[]){size - refIndices[1], size - refIndices[2], size}, 3 * sizeof(int));

    int totalWorkouts[3] = {0}, maxWorkouts[3] = {0}, maxTime[3] = {0}, maxWeight[3] = {0};
    int totalByType[3][4] = {{0},{0},{0}}, totalByExercise[3][4] = {{0},{0},{0}};

    for (int section = 3; section > 0; --section) {
        int limit = refIndices[section - 1];
        int jEnd = innerLimits[section - 1];
        for (int i = refIndices[section]; i < limit; ++i) {
            struct WeekDataModel *e = &results[i];

            for (int j = 2; j > jEnd; --j) {
                totalWorkouts[j] += e->totalWorkouts;
                if (e->totalWorkouts > maxWorkouts[j])
                    maxWorkouts[j] = e->totalWorkouts;
            }
            createNewEntry(m->totalWorkouts.entries, i, e->totalWorkouts);

            for (int x = 0; x < 4; ++x) {
                for (int j = 2; j > jEnd; --j) {
                    totalByType[j][x] += e->durationByType[x];
                    totalByExercise[j][x] += e->weightArray[x];
                    if (e->weightArray[x] > maxWeight[j])
                        maxWeight[j] = e->weightArray[x];
                }
                createNewEntry(m->lifts.entries[x], i, e->weightArray[x]);
            }

            for (int j = 2; j > jEnd; --j) {
                if (e->cumulativeDuration[3] > maxTime[j])
                    maxTime[j] = e->cumulativeDuration[3];
            }
            createNewEntry(m->workoutTypes.entries[0], i, 0);
            for (int x = 1; x < 5; ++x) {
                createNewEntry(m->workoutTypes.entries[x], i, e->cumulativeDuration[x - 1]);
            }
        }
    }

    for (int i = 0; i < 3; ++i) {
        int refIdx = refIndices[i + 1];
        m->totalWorkouts.avgs[i] = (float) totalWorkouts[i] / m->nEntries[i];
        m->totalWorkouts.maxes[i] = maxWorkouts[i] < 7 ? 7 : 1.1 * maxWorkouts[i];
        m->workoutTypes.maxes[i] = 1.1 * maxTime[i];
        m->lifts.maxes[i] = 1.1 * maxWeight[i];
        m->totalWorkouts.dataArrays[i] = &m->totalWorkouts.entries->arr[refIdx];
        m->workoutTypes.dataArrays[i][0] = &m->workoutTypes.entries[0]->arr[refIdx];

        for (int j = 0; j < 4; ++j) {
            m->workoutTypes.avgs[i][j] = totalByType[i][j] / m->nEntries[i];
            m->lifts.avgs[i][j] = (float) totalByExercise[i][j] / m->nEntries[i];
            m->workoutTypes.dataArrays[i][j + 1] = &m->workoutTypes.entries[j + 1]->arr[refIdx];
            m->lifts.dataArrays[i][j] = &m->lifts.entries[j]->arr[refIdx];
        }
    }
    free(results);
}

static void historyData_fetch(void *_model) {
    HistoryViewModel *model = _model;
    runInBackground((^{
        struct tm localInfo;
        int count = 0;

        id request = fetchRequest(createPredicate(CFSTR("weekStart < %lld"), userData->weekStart));
        CFArrayRef data = persistenceService_executeFetchRequest(request, &count, true);
        if (data) {
            struct WeekDataModel *results = malloc(sizeof(struct WeekDataModel) << 7);
            for (int i = 0; i < count; ++i) {
                id d = (id) CFArrayGetValueAtIndex(data, i);
                struct WeekDataModel *r = &results[i];
                time_t timestamp = (time_t) weekData_getWeekStart(d);
                localtime_r(&timestamp, &localInfo);
                r->timeData.year = localInfo.tm_year % 100;
                r->timeData.month = localInfo.tm_mon;
                r->timeData.day = localInfo.tm_mday;
                r->totalWorkouts = weekData_getTotalWorkouts(d);
                r->durationByType[0] = weekData_getWorkoutTimeForType(d, WorkoutStrength);
                r->durationByType[1] = weekData_getWorkoutTimeForType(d, WorkoutHIC);
                r->durationByType[2] = weekData_getWorkoutTimeForType(d, WorkoutSE);
                r->durationByType[3] = weekData_getWorkoutTimeForType(d, WorkoutEndurance);
                weekData_getLiftingLimits(d, r->weightArray);

                r->cumulativeDuration[0] = results[i].durationByType[0];
                for (int j = 1; j < 4; ++j) {
                    r->cumulativeDuration[j] = r->cumulativeDuration[j - 1] + r->durationByType[j];
                }

                memcpy(&model->formatter.data[i], &r->timeData, sizeof(struct HistTimeData));
            }
            historyData_populate(model, results, count);
        }
    }));
}

#pragma mark - Main Functions

id historyVC_init(void **model, void (**handler)(void*)) {
    id self = createVC(HistoryVCClass);
    HistoryVCData *data = calloc(1, sizeof(HistoryVCData));
    *model = &data->model;
    *handler = &historyData_fetch;
    HistoryViewModel *m = &data->model;
    int chartColors[] = {0, 1, 2, 3};
    id *areaDataSets = m->workoutTypes.dataSets;

    m->totalWorkouts.entries = array_new(pt);
    m->totalWorkouts.dataSet = createDataSet(5, nil);
    m->workoutTypes.dataSets[0] = createDataSet(-1, nil);
    fillStringArray(m->workoutTypes.names, CFSTR("workoutTypes%d"), 4);
    fillStringArray(m->lifts.names, CFSTR("liftTypes%d"), 4);
    fillStringArray(m->formatter.months, CFSTR("months%02d"), 12);
    m->formatter.currString = CFStringCreateCopy(NULL, CFSTR(""));
    m->totalWorkouts.legendFormat = localize(CFSTR("totalWorkoutsLegend"));
    m->lifts.legendFormat = localize(CFSTR("liftLegend"));
    m->workoutTypes.legendFormat = localize(CFSTR("workoutTypeLegend"));

    for (int i = 0; i < 4; ++i) {
        m->workoutTypes.entries[i] = array_new(pt);
        m->lifts.entries[i] = array_new(pt);

        id boundary = m->workoutTypes.dataSets[i];
        m->workoutTypes.dataSets[i + 1] = createDataSet(chartColors[i], boundary);
        m->lifts.dataSets[i] = createDataSet(chartColors[i], nil);
    }
    m->workoutTypes.entries[4] = array_new(pt);

    CFArrayRef dataArr = CFArrayCreate(NULL, (const void **)(id []){m->totalWorkouts.dataSet},
                                       1, &(CFArrayCallBacks){0});
    m->totalWorkouts.chartData = createChartData(dataArr, 1, 3);
    CFRelease(dataArr);
    dataArr = CFArrayCreate(NULL, (const void **)(id []){
        areaDataSets[4], areaDataSets[3], areaDataSets[2], areaDataSets[1]
    }, 4, &(CFArrayCallBacks){0});
    m->workoutTypes.chartData = createChartData(dataArr, 1, 5);
    CFRelease(dataArr);
    dataArr = CFArrayCreate(NULL, (const void **)m->lifts.dataSets, 4, &(CFArrayCallBacks){0});
    m->lifts.chartData = createChartData(dataArr, 3, 0);
    CFRelease(dataArr);
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

    data->charts[0] = totalWorkoutsView_init(&data->model.totalWorkouts, self);
    data->charts[1] = workoutTypeView_init(&data->model.workoutTypes, self);
    data->charts[2] = liftingView_init(&data->model.lifts, self);

    data->picker = createSegmentedControl(CFSTR("historySegment%d"), 3, 0,
                                          self, sel_getUid("buttonTapped:"), -1);
    id navItem = getNavItem(self);
    setObject(navItem, sel_getUid("setTitleView:"), data->picker);

    id containers[3];
    for (int i = 0; i < 3; ++i) {
        containers[i] = containerView_init(titles[i], 0, false);
        containerView_add(containers[i], data->charts[i]);
    }
    ContainerViewData *firstC = (ContainerViewData *) object_getIvar(containers[0],
                                                                     ContainerViewDataRef);
    hideView(firstC->divider, true);

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

    historyVC_updateSegment(self, nil, data->picker);
    appCoordinator->loadedViewControllers |= LoadedVC_History;
}

void historyVC_updateSegment(id self, SEL _cmd _U_, id picker) {
    HistoryVCData *data = (HistoryVCData *) object_getIvar(self, HistoryVCDataRef);
    HistoryViewModel *model = &data->model;
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
    char buf[16];
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

void historyVC_clearData(id self, bool updateUI) {
    HistoryVCData *data = (HistoryVCData *) object_getIvar(self, HistoryVCDataRef);
    HistoryViewModel *model = &data->model;
    if (!model->nEntries[2]) return;

    memset(model->nEntries, 0, 3 * sizeof(int));
    memset(model->totalWorkouts.dataArrays, 0, 3 * sizeof(id*));
    memset(model->totalWorkouts.avgs, 0, 3 * sizeof(float));
    memset(model->totalWorkouts.maxes, 0, 3 * sizeof(float));
    memset(model->lifts.maxes, 0, 3 * sizeof(float));
    memset(model->workoutTypes.maxes, 0, 3 * sizeof(float));

    for (int i = 0; i < 3; ++i) {
        memset(model->workoutTypes.dataArrays[i], 0, 5 * sizeof(id*));
        memset(model->lifts.dataArrays[i], 0, sizeof(id*) << 2);
        memset(model->workoutTypes.avgs[i], 0, sizeof(int) << 2);
        memset(model->lifts.avgs[i], 0, sizeof(float) << 2);
    }

    array_clear(pt, model->totalWorkouts.entries);
    for (int i = 0; i < 4; ++i) {
        array_clear(pt, model->workoutTypes.entries[i]);
        array_clear(pt, model->lifts.entries[i]);
    }
    array_clear(pt, model->workoutTypes.entries[4]);

    replaceDataSetEntries(model->totalWorkouts.dataSet, model->totalWorkouts.entries->arr, 0);
    for (int i = 0; i < 4; ++i) {
        replaceDataSetEntries(model->workoutTypes.dataSets[i],
                              model->workoutTypes.entries[0]->arr, 0);
        replaceDataSetEntries(model->lifts.dataSets[i], model->lifts.entries[0]->arr, 0);
    }
    replaceDataSetEntries(model->workoutTypes.dataSets[4], model->workoutTypes.entries[0]->arr, 0);

    if (updateUI) {
        setInt(data->picker, sel_getUid("setSelectedSegmentIndex:"), 0);
        historyVC_updateSegment(self, nil, data->picker);
    }
}

CFStringRef historyVC_stringForValue(id self, SEL _cmd _U_, double value) {
    struct HistFormatter *formatter = &((HistoryVCData *)
                                        object_getIvar(self, HistoryVCDataRef))->model.formatter;
    struct HistTimeData *data = &formatter->data[(int) value];
    CFRelease(formatter->currString);
    formatter->currString = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@/%d/%d"),
                                                     formatter->months[data->month],
                                                     data->day, data->year);
    return formatter->currString;
}

void historyVC_updateColors(id self) {
    id picker = ((HistoryVCData *) object_getIvar(self, HistoryVCDataRef))->picker;
    id view = getView(self);
    setBackground(view, createColor(ColorPrimaryBG));
    updateSegmentedControl(picker);
}
