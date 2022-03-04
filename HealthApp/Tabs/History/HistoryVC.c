#include "HistoryVC.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AppUserData.h"
#include "ContainerView.h"
#include "ExerciseManager.h"
#include "PersistenceService.h"
#include "SwiftBridging.h"

void setLineLimit(id v, float limit);

struct WeekDataModel {
    short totalWorkouts;
    short durationByType[4];
    short cumulativeDuration[4];
    short weightArray[4];
};

Class HistoryVCClass;

static CFStringRef totalWorkoutsFormat;
static CFStringRef workoutTypeFormat;
static CFStringRef liftFormat;
static CFStringRef workoutTypeNames[4];
static CFStringRef liftNames[4];

#pragma mark - Load Data

static void historyData_populate(HistoryViewModel *m, struct WeekDataModel *results, int size) {
    int innerLimits[] = {-1, 0, 1};
    int refIndices[] = {size, size - 26, size - 52, 0};
    if (refIndices[2] < 0)
        refIndices[2] = 0;
    if (refIndices[1] < 0)
        refIndices[1] = 0;

    memcpy(m->nEntries, (int[]){size - refIndices[1], size - refIndices[2], size}, 3 * sizeof(int));
    memcpy(m->refIndices, &refIndices[1], 3 * sizeof(int));

    int totalWorkouts[3] = {0};
    int totalByType[3][4] = {{0},{0},{0}}, totalByExercise[3][4] = {{0},{0},{0}};
    short maxWorkouts[3] = {0}, maxTime[3] = {0}, maxWeight[3] = {0};
    m->totalWorkouts.entries = malloc((unsigned) size * sizeof(CGPoint));
    m->lifts.entries[0] = malloc((unsigned) size * sizeof(CGPoint));
    m->lifts.entries[1] = malloc((unsigned) size * sizeof(CGPoint));
    m->lifts.entries[2] = malloc((unsigned) size * sizeof(CGPoint));
    m->lifts.entries[3] = malloc((unsigned) size * sizeof(CGPoint));
    m->workoutTypes.entries[0] = malloc((unsigned) size * sizeof(CGPoint));
    m->workoutTypes.entries[1] = malloc((unsigned) size * sizeof(CGPoint));
    m->workoutTypes.entries[2] = malloc((unsigned) size * sizeof(CGPoint));
    m->workoutTypes.entries[3] = malloc((unsigned) size * sizeof(CGPoint));
    m->workoutTypes.entries[4] = malloc((unsigned) size * sizeof(CGPoint));

    for (int section = 3, index = 0; section > 0; --section) {
        int limit = refIndices[section - 1];
        int jEnd = innerLimits[section - 1];
        for (int i = refIndices[section]; i < limit; ++i, ++index) {
            struct WeekDataModel *e = &results[i];

            for (int j = 2; j > jEnd; --j) {
                totalWorkouts[j] += e->totalWorkouts;
                if (e->totalWorkouts > maxWorkouts[j])
                    maxWorkouts[j] = e->totalWorkouts;
            }
            m->totalWorkouts.entries[index] = (CGPoint){i, e->totalWorkouts};

            for (int x = 0; x < 4; ++x) {
                for (int j = 2; j > jEnd; --j) {
                    totalByType[j][x] += e->durationByType[x];
                    totalByExercise[j][x] += e->weightArray[x];
                    if (e->weightArray[x] > maxWeight[j])
                        maxWeight[j] = e->weightArray[x];
                }
                m->lifts.entries[x][index] = (CGPoint){i, e->weightArray[x]};
            }

            for (int j = 2; j > jEnd; --j) {
                if (e->cumulativeDuration[3] > maxTime[j])
                    maxTime[j] = e->cumulativeDuration[3];
            }
            m->workoutTypes.entries[0][index] = (CGPoint){i, 0};
            for (int x = 1; x < 5; ++x) {
                m->workoutTypes.entries[x][index] = (CGPoint){i, e->cumulativeDuration[x - 1]};
            }
        }
    }

    for (int i = 0; i < 3; ++i) {
        m->totalWorkouts.avgs[i] = (float) totalWorkouts[i] / m->nEntries[i];
        m->totalWorkouts.maxes[i] = maxWorkouts[i] < 7 ? 7 : 1.1f * maxWorkouts[i];
        m->workoutTypes.maxes[i] = 1.1f * maxTime[i];
        m->lifts.maxes[i] = 1.1f * maxWeight[i];

        for (int j = 0; j < 4; ++j) {
            m->workoutTypes.avgs[i][j] = totalByType[i][j] / m->nEntries[i];
            m->lifts.avgs[i][j] = (float) totalByExercise[i][j] / m->nEntries[i];
        }
    }
    free(results);
}

static void historyData_fetch(void *_model) {
    HistoryViewModel *model = _model;
    id context = backgroundContext;
    msg1(void, void(^)(void), context, sel_getUid("performBlock:"), ^{
        struct tm localInfo;
        int count = 0;

        CFArrayRef data = persistenceService_fetchData(context, 1, &count);
        if (data && count > 1) {
            count -= 1;
            CFMutableArrayRef strs = CFArrayCreateMutable(NULL, count, &kCFTypeArrayCallBacks);
            struct WeekDataModel *results = malloc((unsigned) count * sizeof(struct WeekDataModel));
            customAssert(count > 0)
            for (int i = 0; i < count; ++i) {
                id d = (id) CFArrayGetValueAtIndex(data, i);
                struct WeekDataModel *r = &results[i];
                time_t timestamp = (time_t) weekData_getWeekStart(d);
                localtime_r(&timestamp, &localInfo);
                CFStringRef str = CFStringCreateWithFormat(NULL, NULL, CFSTR("%d/%d/%d"),
                                                           localInfo.tm_mon + 1, localInfo.tm_mday,
                                                           localInfo.tm_year % 100);
                CFArrayAppendValue(strs, str);
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
                CFRelease(str);
            }
            model->axisStrings = strs;
            historyData_populate(model, results, count);
        }
    });
}

#pragma mark - Main Functions

id historyVC_init(CFBundleRef bundle, void **model, void (**handler)(void*)) {
    id self = createNew(HistoryVCClass);
    HistoryVC *data = (HistoryVC *) ((char *)self + VCSize);
    *model = &data->model;
    *handler = &historyData_fetch;
    HistoryViewModel *m = &data->model;
    int chartColors[] = {0, 1, 2, 3};
    id *areaDataSets = m->workoutTypes.dataSets;

    m->totalWorkouts.dataSet = createDataSet(5, nil);
    m->workoutTypes.dataSets[0] = createDataSet(-1, nil);
    fillStringArray(bundle, workoutTypeNames, CFSTR("workoutTypes%d"), 4);
    fillStringArray(bundle, liftNames, CFSTR("liftTypes%d"), 4);
    totalWorkoutsFormat = CFBundleCopyLocalizedString(bundle, CFSTR("totalWorkoutsLegend"),
                                                      NULL, NULL);
    liftFormat = CFBundleCopyLocalizedString(bundle, CFSTR("liftLegend"), NULL, NULL);
    workoutTypeFormat = CFBundleCopyLocalizedString(bundle, CFSTR("workoutTypeLegend"), NULL, NULL);

    for (int i = 0; i < 4; ++i) {
        id boundary = m->workoutTypes.dataSets[i];
        m->workoutTypes.dataSets[i + 1] = createDataSet(chartColors[i], boundary);
        m->lifts.dataSets[i] = createDataSet(chartColors[i], nil);
    }

    CFArrayRef dataArr = CFArrayCreate(NULL, (const void **)(id []){m->totalWorkouts.dataSet},
                                       1, NULL);
    m->totalWorkouts.chartData = createChartData(dataArr, 1, 3);
    CFRelease(dataArr);
    dataArr = CFArrayCreate(NULL, (const void **)(id []){
        areaDataSets[4], areaDataSets[3], areaDataSets[2], areaDataSets[1]
    }, 4, NULL);
    m->workoutTypes.chartData = createChartData(dataArr, 1, 5);
    CFRelease(dataArr);
    dataArr = CFArrayCreate(NULL, (const void **)m->lifts.dataSets, 4, NULL);
    m->lifts.chartData = createChartData(dataArr, 3, 0);
    CFRelease(dataArr);
    return self;
}

void historyVC_viewDidLoad(id self, SEL _cmd) {
    struct objc_super super = {self, VCClass};
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);

    CFBundleRef bundle = CFBundleGetMainBundle();
    const unsigned char darkMode = userData->darkMode;
    HistoryVC *data = (HistoryVC *) ((char *)self + VCSize);
    id view = getView(self);
    setBackground(view, createColor(ColorPrimaryBG));

    CFStringRef titles[3];
    fillStringArray(bundle, titles, CFSTR("chartHeader%d"), 3);

    data->charts[0] = createChartView(self, (long []){4}, 1, 1);
    setHeight(data->charts[0], 390, false);
    data->charts[1] = createChartView(self, (long []){0, 1, 2, 3}, 4, 6);
    setHeight(data->charts[1], 425, false);
    data->charts[2] = createChartView(self, (long []){0, 1, 2, 3}, 4, 0);
    setHeight(data->charts[2], 550, false);

    data->picker = createSegmentedControl(bundle, CFSTR("historySegment%d"), 0);
    addTarget(data->picker, self, sel_getUid("buttonTapped:"), 4096);
    if (!(darkMode & 128))
        updateSegmentedControl(data->picker, createColor(ColorLabel), darkMode);
    msg1(void, id, getNavItem(self), sel_getUid("setTitleView:"), data->picker);

    id containers[3];
    ContainerView *c;
    for (int i = 0; i < 3; ++i) {
        containers[i] = containerView_init(titles[i], &c, 0, false);
        addArrangedSubview(c->stack, data->charts[i]);
        hideView(c->divider, !i);
    }

    id vStack = createStackView(containers, 3, 1, 5, (Padding){10, 8, 10, 8});
    id scrollView = createScrollView();
    addVStackToScrollView(view, vStack, scrollView);
    releaseObj(vStack);
    releaseObj(scrollView);
    for (int i = 0; i < 3; ++i) {
        releaseObj(containers[i]);
    }

    historyVC_updateSegment(self, nil, data->picker);
}

void historyVC_updateSegment(id self, SEL _cmd _U_, id picker) {
    HistoryVC *data = (HistoryVC *) ((char *)self + VCSize);
    id totalsChart = data->charts[0], typesChart = data->charts[1], liftsChart = data->charts[2];
    HistoryViewModel *model = &data->model;
    TotalWorkoutsChartModel *m1 = &model->totalWorkouts;
    WorkoutTypeChartModel *m2 = &model->workoutTypes;
    LiftChartModel *m3 = &model->lifts;

    int index = (int) getSelectedSegment(picker);
    int count = model->nEntries[index];
    int ref = model->refIndices[index];
    if (!count) {
        disableLineChartView(totalsChart);
        disableLineChartView(typesChart);
        disableLineChartView(liftsChart);
        msg3(void, id, SEL, unsigned long, data->picker,
             sel_getUid("removeTarget:action:forControlEvents:"), nil, nil, 4096);
        return;
    }

    char buf[16];
    CFStringRef label = CFStringCreateWithFormat(NULL, NULL, totalWorkoutsFormat, m1->avgs[index]);
    setLegendLabel(totalsChart, 0, label);
    CFRelease(label);

    for (int i = 0; i < 4; ++i) {
        int typeAverage = m2->avgs[index][i];
        if (typeAverage > 59) {
            sprintf(buf, "%d h %d m", typeAverage / 60, typeAverage % 60);
        } else {
            sprintf(buf, "%d m", typeAverage);
        }
        label = CFStringCreateWithFormat(NULL, NULL, workoutTypeFormat, workoutTypeNames[i], buf);
        setLegendLabel(typesChart, i, label);
        CFRelease(label);

        label = CFStringCreateWithFormat(NULL, NULL, liftFormat, liftNames[i], m3->avgs[index][i]);
        setLegendLabel(liftsChart, i, label);
        CFRelease(label);
    }

    setLineLimit(totalsChart, m1->avgs[index]);
    replaceDataSetEntries(m1->dataSet, &m1->entries[ref], count);
    updateChart(totalsChart, m1->chartData, m1->maxes[index]);

    replaceDataSetEntries(m2->dataSets[0], &m2->entries[0][ref], count);
    for (int i = 1; i < 5; ++i) {
        replaceDataSetEntries(m2->dataSets[i], &m2->entries[i][ref], count);
    }
    updateChart(typesChart, m2->chartData, m2->maxes[index]);

    for (int i = 0; i < 4; ++i) {
        replaceDataSetEntries(m3->dataSets[i], &m3->entries[i][ref], count);
    }
    updateChart(liftsChart, m3->chartData, m3->maxes[index]);
}

void historyVC_clearData(id self) {
    HistoryVC *data = (HistoryVC *) ((char *)self + VCSize);
    HistoryViewModel *model = &data->model;
    if (!model->nEntries[2]) return;

    CFRelease(model->axisStrings);
    memset(model->nEntries, 0, 3 * sizeof(int));
    free(model->totalWorkouts.entries);
    replaceDataSetEntries(model->totalWorkouts.dataSet, NULL, 0);
    for (int i = 0; i < 4; ++i) {
        free(model->workoutTypes.entries[i]);
        free(model->lifts.entries[i]);
        replaceDataSetEntries(model->workoutTypes.dataSets[i], NULL, 0);
        replaceDataSetEntries(model->lifts.dataSets[i], NULL, 0);
    }
    free(model->workoutTypes.entries[4]);
    replaceDataSetEntries(model->workoutTypes.dataSets[4], NULL, 0);

    if (isViewLoaded(self)) {
        setSelectedSegment(data->picker, 0);
        historyVC_updateSegment(self, nil, data->picker);
    }
}

CFStringRef historyVC_stringForValue(id self, SEL _cmd _U_, double value) {
    CFArrayRef strs = ((HistoryVC *) ((char *)self + VCSize))->model.axisStrings;
    return CFArrayGetValueAtIndex(strs, (int) value);
}

void historyVC_updateColors(id self, unsigned char darkMode) {
    id picker = ((HistoryVC *) ((char *)self + VCSize))->picker;
    id view = getView(self);
    setBackground(view, createColor(ColorPrimaryBG));
    id label = createColor(ColorLabel), divColor = createColor(ColorSeparator);
    updateSegmentedControl(picker, label, darkMode);
    view = (id) CFArrayGetValueAtIndex(getSubviews(view), 0);
    view = (id) CFArrayGetValueAtIndex(getSubviews(view), 0);
    CFArrayRef views = getArrangedSubviews(view);
    for (int i = 0; i < 3; ++i) {
        id c = (id) CFArrayGetValueAtIndex(views, i);
        containerView_updateColors((ContainerView *) ((char *)c + ViewSize), label, divColor);
    }
}
