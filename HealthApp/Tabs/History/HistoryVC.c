#include "HistoryVC.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AppDelegate.h"
#include "ContainerView.h"
#include "SwiftBridging.h"

void populateChartsSelsAndFuncs(Class *classes, IMP *impArr, SEL *selArr);

struct HistCache {
    const SEL slglb, slilm, sdt, rpe;
    void (*setLegendLabel)(id,SEL,long,id);
    void (*setLineLimit)(id,SEL,CGFloat);
    void (*setData)(id,SEL,id,CGFloat);
    void (*replaceEntries)(id,SEL,CGPoint*,long);
};

Class HistoryVCClass;

static CFStringRef totalWorkoutsFormat;
static CFStringRef workoutTypeFormat;
static CFStringRef liftFormat;
static CFStringRef workoutTypeNames[4];
static CFStringRef liftNames[4];
static struct HistCache cache;

#pragma mark - Load Data

static void historyData_populate(void *_model, CFArrayRef strs, WeekDataModel *results, int size) {
    HistoryViewModel *m = _model;
    m->axisStrings = strs;
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
    m->totalWorkouts.entries = malloc((unsigned)size * sizeof(CGPoint));
    m->lifts.entries[0] = malloc((unsigned)size * sizeof(CGPoint));
    m->lifts.entries[1] = malloc((unsigned)size * sizeof(CGPoint));
    m->lifts.entries[2] = malloc((unsigned)size * sizeof(CGPoint));
    m->lifts.entries[3] = malloc((unsigned)size * sizeof(CGPoint));
    m->workoutTypes.entries[0] = malloc((unsigned)size * sizeof(CGPoint));
    m->workoutTypes.entries[1] = malloc((unsigned)size * sizeof(CGPoint));
    m->workoutTypes.entries[2] = malloc((unsigned)size * sizeof(CGPoint));
    m->workoutTypes.entries[3] = malloc((unsigned)size * sizeof(CGPoint));
    m->workoutTypes.entries[4] = malloc((unsigned)size * sizeof(CGPoint));

    for (int section = 3, index = 0; section > 0; --section) {
        int limit = refIndices[section - 1];
        int jEnd = innerLimits[section - 1];
        for (int i = refIndices[section]; i < limit; ++i, ++index) {
            WeekDataModel *e = &results[i];

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
        m->totalWorkouts.avgs[i] = (float)totalWorkouts[i] / m->nEntries[i];
        m->totalWorkouts.maxes[i] = maxWorkouts[i] < 7 ? 7 : 1.1f * maxWorkouts[i];
        m->workoutTypes.maxes[i] = 1.1f * maxTime[i];
        m->lifts.maxes[i] = 1.1f * maxWeight[i];

        for (int j = 0; j < 4; ++j) {
            m->workoutTypes.avgs[i][j] = totalByType[i][j] / m->nEntries[i];
            m->lifts.avgs[i][j] = (float)totalByExercise[i][j] / m->nEntries[i];
        }
    }
    free(results);
}

#pragma mark - Main Functions

id historyVC_init(CFBundleRef bundle, void **model, FetchHandler *handler, VCacheRef tbl, CCacheRef clr) {
    id self = Sels.new(HistoryVCClass, Sels.nw);
    HistoryVC *data = (HistoryVC *)((char *)self + VCSize);
    data->tbl = tbl;
    data->clr = clr;
    *model = &data->model;
    *handler = &historyData_populate;
    HistoryViewModel *m = &data->model;
    int colors[] = {0, 1, 2, 3};
    Class classes[2];
    SEL selArr[6];
    IMP impArr[6];
    populateChartsSelsAndFuncs(classes, impArr, selArr);
    memcpy(&cache, &(struct HistCache){selArr[0], selArr[1], selArr[2], selArr[3],
        (void(*)(id,SEL,long,id))impArr[0], (void(*)(id,SEL,CGFloat))impArr[1],
        (void(*)(id,SEL,id,CGFloat))impArr[2], (void(*)(id,SEL,CGPoint*,long))impArr[3]
    }, sizeof(struct HistCache));
    Class SetCls = classes[0], DataCls = classes[1];
    SEL iSet = selArr[4], iData = selArr[5];
    id (*setInit)(id,SEL,long,id) = (id(*)(id,SEL,long,id))impArr[4];
    id (*dataInit)(id,SEL,CFArrayRef,long,uint8_t) = (id(*)(id,SEL,CFArrayRef,long,uint8_t))impArr[5];

    m->totalWorkouts.dataSet = setInit(Sels.alloc(SetCls, Sels.alo), iSet, 5, nil);
    m->workoutTypes.dataSets[0] = setInit(Sels.alloc(SetCls, Sels.alo), iSet, -1, nil);
    fillStringArray(bundle, workoutTypeNames, CFSTR("workoutTypes%d"), 4);
    fillStringArray(bundle, liftNames, CFSTR("exNames%02d"), 4);
    totalWorkoutsFormat = localize(bundle, CFSTR("totalWorkoutsLegend"));
    liftFormat = localize(bundle, CFSTR("liftLegend"));
    workoutTypeFormat = localize(bundle, CFSTR("workoutTypeLegend"));

    for (int i = 0; i < 4; ++i) {
        id boundary = m->workoutTypes.dataSets[i];
        m->workoutTypes.dataSets[i + 1] = setInit(Sels.alloc(SetCls, Sels.alo), iSet, colors[i], boundary);
        m->lifts.dataSets[i] = setInit(Sels.alloc(SetCls, Sels.alo), iSet, colors[i], nil);
    }

    CFArrayRef dataArr = CFArrayCreate(NULL, (const void *[]){m->totalWorkouts.dataSet}, 1, NULL);
    m->totalWorkouts.chartData = dataInit(Sels.alloc(DataCls, Sels.alo), iData, dataArr, 1, 3);
    CFRelease(dataArr);
    id *areaSets = m->workoutTypes.dataSets;
    dataArr = CFArrayCreate(NULL, (const void *[]){areaSets[4], areaSets[3], areaSets[2], areaSets[1]},
                            4, NULL);
    m->workoutTypes.chartData = dataInit(Sels.alloc(DataCls, Sels.alo), iData, dataArr, 1, 5);
    CFRelease(dataArr);
    dataArr = CFArrayCreate(NULL, (const void **)m->lifts.dataSets, 4, NULL);
    m->lifts.chartData = dataInit(Sels.alloc(DataCls, Sels.alo), iData, dataArr, 3, 0);
    CFRelease(dataArr);
    return self;
}

void historyVC_updateSegment(id self, SEL _cmd _U_, id picker) {
    HistoryVC *data = (HistoryVC *)((char *)self + VCSize);
    id totalsChart = data->charts[0], typesChart = data->charts[1], liftsChart = data->charts[2];
    TotalWorkoutsChartModel *m1 = &data->model.totalWorkouts;
    WorkoutTypeChartModel *m2 = &data->model.workoutTypes;
    LiftChartModel *m3 = &data->model.lifts;

    int index = (int)msg0(long, picker, sel_getUid("selectedSegmentIndex"));
    int count = data->model.nEntries[index];
    if (!count) {
        cache.setData(totalsChart, cache.sdt, nil, 0);
        cache.setData(typesChart, cache.sdt, nil, 0);
        cache.setData(liftsChart, cache.sdt, nil, 0);
        msg3(void, id, SEL, unsigned long, data->picker,
             sel_getUid("removeTarget:action:forControlEvents:"), nil, nil, 4096);
        return;
    }

    char buf[16];
    CFStringRef label = formatStr(totalWorkoutsFormat, m1->avgs[index]);
    cache.setLegendLabel(totalsChart, cache.slglb, 0, (id)label);
    CFRelease(label);

    for (int i = 0; i < 4; ++i) {
        int typeAverage = m2->avgs[index][i];
        if (typeAverage > 59) {
            sprintf(buf, "%d h %d m", typeAverage / 60, typeAverage % 60);
        } else {
            sprintf(buf, "%d m", typeAverage);
        }
        label = formatStr(workoutTypeFormat, workoutTypeNames[i], buf);
        cache.setLegendLabel(typesChart, cache.slglb, i, (id)label);
        CFRelease(label);

        label = formatStr(liftFormat, liftNames[i], m3->avgs[index][i]);
        cache.setLegendLabel(liftsChart, cache.slglb, i, (id)label);
        CFRelease(label);
    }

    int ref = data->model.refIndices[index];
    cache.setLineLimit(totalsChart, cache.slilm, m1->avgs[index]);
    cache.replaceEntries(m1->dataSet, cache.rpe, &m1->entries[ref], count);
    for (int i = 0; i < 4; ++i) {
        cache.replaceEntries(m2->dataSets[i], cache.rpe, &m2->entries[i][ref], count);
        cache.replaceEntries(m3->dataSets[i], cache.rpe, &m3->entries[i][ref], count);
    }
    cache.replaceEntries(m2->dataSets[4], cache.rpe, &m2->entries[4][ref], count);
    cache.setData(totalsChart, cache.sdt, m1->chartData, m1->maxes[index]);
    cache.setData(typesChart, cache.sdt, m2->chartData, m2->maxes[index]);
    cache.setData(liftsChart, cache.sdt, m3->chartData, m3->maxes[index]);
}

void historyVC_viewDidLoad(id self, SEL _cmd) {
    msgSup0(void, (&(struct objc_super){self, VC}), _cmd);

    CFBundleRef bundle = CFBundleGetMainBundle();
    const unsigned char darkMode = getUserInfo()->darkMode;
    HistoryVC *data = (HistoryVC *)((char *)self + VCSize);
    VCacheRef tbl = data->tbl;
    id view = msg0(id, self, sel_getUid("view"));
    tbl->view.setBG(view, tbl->view.sbg, data->clr->getColor(data->clr->cls, data->clr->sc, ColorPrimaryBG));

    data->charts[0] = createChartView(self, (long []){4}, 1, 1);
    data->charts[1] = createChartView(self, (long []){0, 1, 2, 3}, 4, 6);
    data->charts[2] = createChartView(self, (long []){0, 1, 2, 3}, 4, 0);
    data->picker = createSegmentedControl(bundle, CFSTR("historySegment%d"), 0);
    tbl->button.addTarget(data->picker, tbl->button.atgt, self, sel_getUid("buttonTapped:"), 4096);
    if (darkMode < 2)
        updateSegmentedControl(data->clr, data->picker, darkMode);
    msg1(void, id, msg0(id, self, sel_getUid("navigationItem")), sel_getUid("setTitleView:"), data->picker);

    CFStringRef titles[3]; id containers[3]; int heights[] = {390, 425, 550};
    ContainerView *c;
    fillStringArray(bundle, titles, CFSTR("chartHeader%d"), 3);
    for (int i = 0; i < 3; ++i) {
        containers[i] = containerView_init(tbl, data->clr, titles[i], &c);
        setHeight(&tbl->cc, data->charts[i], heights[i], false, false);
        tbl->stack.addSub(c->stack, tbl->stack.asv, data->charts[i]);
        tbl->view.hide(c->divider, tbl->view.shd, !i);
    }

    id vStack = createVStack(containers, 3);
    tbl->stack.setSpace(vStack, tbl->stack.ssp, 4);
    tbl->stack.setMargins(vStack, tbl->stack.smr, (HAInsets){16, 8, 16, 8});
    id scrollView = createScrollView();
    addVStackToScrollView(tbl, view, vStack, scrollView);
    Sels.viewRel(vStack, Sels.rel);
    Sels.viewRel(scrollView, Sels.rel);
    for (int i = 0; i < 3; ++i) {
        Sels.viewRel(containers[i], Sels.rel);
    }

    historyVC_updateSegment(self, nil, data->picker);
}

void historyVC_clearData(id self) {
    HistoryVC *data = (HistoryVC *)((char *)self + VCSize);
    HistoryViewModel *model = &data->model;
    if (!model->nEntries[2]) return;

    CFRelease(model->axisStrings);
    memset(model->nEntries, 0, 3 * sizeof(int));
    free(model->totalWorkouts.entries);
    cache.replaceEntries(model->totalWorkouts.dataSet, cache.rpe, NULL, 0);
    for (int i = 0; i < 4; ++i) {
        free(model->workoutTypes.entries[i]);
        free(model->lifts.entries[i]);
        cache.replaceEntries(model->workoutTypes.dataSets[i], cache.rpe, NULL, 0);
        cache.replaceEntries(model->lifts.dataSets[i], cache.rpe, NULL, 0);
    }
    free(model->workoutTypes.entries[4]);
    cache.replaceEntries(model->workoutTypes.dataSets[4], cache.rpe, NULL, 0);

    if (msg0(bool, self, sel_getUid("isViewLoaded"))) {
        msg1(void, long, data->picker, sel_getUid("setSelectedSegmentIndex:"), 0);
        historyVC_updateSegment(self, nil, data->picker);
    }
}

CFStringRef historyVC_stringForValue(id self, SEL _cmd _U_, double value) {
    CFArrayRef strs = ((HistoryVC *)((char *)self + VCSize))->model.axisStrings;
    return CFArrayGetValueAtIndex(strs, (int)value);
}

void historyVC_updateColors(id self, unsigned char darkMode) {
    HistoryVC *data = (HistoryVC *)((char *)self + VCSize);
    VCacheRef tbl = data->tbl;
    id view = msg0(id, self, sel_getUid("view"));
    tbl->view.setBG(view, tbl->view.sbg, data->clr->getColor(data->clr->cls, data->clr->sc, ColorPrimaryBG));
    updateSegmentedControl(data->clr, data->picker, darkMode);
    view = (id)CFArrayGetValueAtIndex(msg0(CFArrayRef, view, sel_getUid("subviews")), 0);
    view = (id)CFArrayGetValueAtIndex(msg0(CFArrayRef, view, sel_getUid("subviews")), 0);
    CFArrayRef views = tbl->stack.getSub(view, tbl->stack.gsv);
    for (int i = 0; i < 3; ++i) {
        containerView_updateColors(
          (ContainerView *)((char *)CFArrayGetValueAtIndex(views, i) + ViewSize), tbl, data->clr);
    }
}
