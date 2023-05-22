#include "HistoryVC.h"
#include "ContainerView.h"
#include "SwiftBridging.h"

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
static CFStringRef hourMinFmt;
static CFStringRef minsFmt;
static CFStringRef workoutTypeNames[4];
static CFStringRef liftNames[4];
static struct HistCache cache;

static void populateHistory(void *, CFArrayRef, WeekDataModel *, int, bool);

id historyVC_init(void **model, FetchHandler *handler, VCacheRef tbl, CCacheRef clr) {
    id self = Sels.new(HistoryVCClass, Sels.nw);
    HistoryVC *d = (HistoryVC *)((char *)self + VCSize);
    d->tbl = tbl;
    d->clr = clr;
    HistoryViewModel *m = &d->model;
    *model = m;
    *handler = &populateHistory;
    int colors[] = {0, 1, 2, 3};
    Class classes[2];
    SEL selArr[6];
    IMP impArr[6];
    populateChartsSelsAndFuncs(classes, impArr, selArr);
    memcpy(&cache, &(struct HistCache){selArr[0], selArr[1], selArr[2], selArr[3],
        (void(*)(id,SEL,long,id))impArr[0], (void(*)(id,SEL,CGFloat))impArr[1],
        (void(*)(id,SEL,id,CGFloat))impArr[2], (void(*)(id,SEL,CGPoint*,long))impArr[3]
    }, sizeof(struct HistCache));
    Class Set = classes[0], Data = classes[1];
    SEL iSet = selArr[4], iData = selArr[5];
    id (*setInit)(id,SEL,long,id) = (id(*)(id,SEL,long,id))impArr[4];
    id (*dataInit)(id,SEL,CFArrayRef,long,uint8_t) = (id(*)(id,SEL,CFArrayRef,long,uint8_t))impArr[5];

    m->totalWorkouts.dataSet = setInit(Sels.alloc(Set, Sels.alo), iSet, 5, nil);
    m->workoutTypes.dataSets[0] = setInit(Sels.alloc(Set, Sels.alo), iSet, -1, nil);
    fillStringArray(workoutTypeNames, CFSTR("workoutTypes%d"), 4);
    fillStringArray(liftNames, CFSTR("exNames%02d"), 4);
    totalWorkoutsFormat = localize(CFSTR("totalWorkoutsLegend"));
    liftFormat = localize(CFSTR("liftLegend"));
    workoutTypeFormat = localize(CFSTR("workoutTypeLegend"));
    hourMinFmt = localize(CFSTR("hourMinFmt"));
    minsFmt = localize(CFSTR("minsFmt"));

    for (int i = 0; i < 4; ++i) {
        id bound = m->workoutTypes.dataSets[i];
        m->workoutTypes.dataSets[i + 1] = setInit(Sels.alloc(Set, Sels.alo), iSet, colors[i], bound);
        m->lifts.dataSets[i] = setInit(Sels.alloc(Set, Sels.alo), iSet, colors[i], nil);
    }

    CFArrayRef dataArr = CFArrayCreate(NULL, (const void *[]){m->totalWorkouts.dataSet}, 1, NULL);
    m->totalWorkouts.chartData = dataInit(Sels.alloc(Data, Sels.alo), iData, dataArr, 1, 3);
    CFRelease(dataArr);
    id *area = m->workoutTypes.dataSets;
    dataArr = CFArrayCreate(NULL, (const void *[]){area[4], area[3], area[2], area[1]}, 4, NULL);
    m->workoutTypes.chartData = dataInit(Sels.alloc(Data, Sels.alo), iData, dataArr, 1, 5);
    CFRelease(dataArr);
    dataArr = CFArrayCreate(NULL, (const void **)m->lifts.dataSets, 4, NULL);
    m->lifts.chartData = dataInit(Sels.alloc(Data, Sels.alo), iData, dataArr, 3, 0);
    CFRelease(dataArr);
    return self;
}

#pragma mark - Load Data

static void populateHistory(void *_m, CFArrayRef strs, WeekDataModel *results, int size, bool ltr) {
    HistoryViewModel *m = _m;
    m->axisStrings = strs;
    int incr = ltr ? 1 : -1;
    int innerLimits[] = {-1, 0, 1};
    int refIndices[] = {size, size - 26, size - 52, 0};
    refIndices[2] = max(refIndices[2], 0);
    refIndices[1] = max(refIndices[1], 0);

    memcpy(m->nEntries, (int[]){size - refIndices[1], size - refIndices[2], size}, 3 * sizeof(int));
    if (ltr) memcpy(m->refIndices, &refIndices[1], 3 * sizeof(int));

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

    for (int section = 3, index = ltr ? 0 : size - 1; section > 0; --section) {
        int limit = refIndices[section - 1];
        int jEnd = innerLimits[section - 1];
        for (int i = refIndices[section]; i < limit; ++i, index += incr) {
            WeekDataModel *e = &results[i];

            for (int j = 2; j > jEnd; --j) {
                totalWorkouts[j] += e->totalWorkouts;
                if (e->totalWorkouts > maxWorkouts[j]) maxWorkouts[j] = e->totalWorkouts;
            }
            m->totalWorkouts.entries[index] = (CGPoint){index, e->totalWorkouts};

            for (int x = 0; x < 4; ++x) {
                for (int j = 2; j > jEnd; --j) {
                    totalByType[j][x] += e->durationByType[x];
                    totalByExercise[j][x] += e->weightArray[x];
                    if (e->weightArray[x] > maxWeight[j]) maxWeight[j] = e->weightArray[x];
                }
                m->lifts.entries[x][index] = (CGPoint){index, e->weightArray[x]};
            }

            for (int j = 2; j > jEnd; --j) {
                if (e->cumulativeDuration[3] > maxTime[j]) maxTime[j] = e->cumulativeDuration[3];
            }
            m->workoutTypes.entries[0][index] = (CGPoint){index, 0};
            for (int x = 1; x < 5; ++x) {
                m->workoutTypes.entries[x][index] = (CGPoint){index, e->cumulativeDuration[x - 1]};
            }
        }
    }

    float invEntries[] = {1.f / m->nEntries[0], 1.f / m->nEntries[1], 1.f / m->nEntries[2]};
    for (int i = 0; i < 3; ++i) {
        m->totalWorkouts.avgs[i] = totalWorkouts[i] * invEntries[i];
        m->totalWorkouts.maxes[i] = maxWorkouts[i] < 7 ? 7 : 1.1f * maxWorkouts[i];
        m->workoutTypes.maxes[i] = 1.1f * maxTime[i];
        m->lifts.maxes[i] = maxWeight[i] * 1.1f;

        for (int j = 0; j < 4; ++j) {
            m->workoutTypes.avgs[i][j] = totalByType[i][j] / m->nEntries[i];
            m->lifts.avgs[i][j] = totalByExercise[i][j] * invEntries[i];
        }
    }
    if (massType) {
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < 4; ++j) m->lifts.entries[j][i].y *= 0.453592;
        }
        for (int i = 0; i < 3; ++i) {
            m->lifts.maxes[i] *= 0.453592f;
            for (int j = 0; j < 4; ++j) m->lifts.avgs[i][j] *= 0.453592f;
        }
    }
    free(results);
}

#pragma mark - Selectors/Methods

void historyVC_updateSegment(id self, SEL _cmd _U_, id picker) {
    HistoryVC *d = (HistoryVC *)((char *)self + VCSize);
    id totalsChart = d->charts[0], typesChart = d->charts[1], liftsChart = d->charts[2];
    TotalWorkoutsChartModel *m1 = &d->model.totalWorkouts;
    WorkoutTypeChartModel *m2 = &d->model.workoutTypes;
    LiftChartModel *m3 = &d->model.lifts;

    int index = (int)msg0(long, picker, sel_getUid("selectedSegmentIndex"));
    int count = d->model.nEntries[index];
    if (!count) {
        cache.setData(totalsChart, cache.sdt, nil, 0);
        cache.setData(typesChart, cache.sdt, nil, 0);
        cache.setData(liftsChart, cache.sdt, nil, 0);
        msg3(void, id, SEL, unsigned long, d->picker,
             sel_getUid("removeTarget:action:forControlEvents:"), nil, nil, 4096);
        return;
    }

    CFLocaleRef l = CFLocaleCopyCurrent();
    CFStringRef label = formatStr(l, totalWorkoutsFormat, m1->avgs[index]);
    cache.setLegendLabel(totalsChart, cache.slglb, 0, (id)label);
    CFRelease(label);

    for (int i = 0; i < 4; ++i) {
        int typeAverage = m2->avgs[index][i];
        CFStringRef duration;
        if (typeAverage > 59) {
            duration = formatStr(l, hourMinFmt, typeAverage / 60, typeAverage % 60);
        } else {
            duration = formatStr(l, minsFmt, typeAverage);
        }
        label = formatStr(l, workoutTypeFormat, workoutTypeNames[i], duration);
        CFRelease(duration);
        cache.setLegendLabel(typesChart, cache.slglb, i, (id)label);
        CFRelease(label);

        label = formatStr(l, liftFormat, liftNames[i], m3->avgs[index][i]);
        cache.setLegendLabel(liftsChart, cache.slglb, i, (id)label);
        CFRelease(label);
    }
    CFRelease(l);

    int ref = d->model.refIndices[index];
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

    const unsigned char darkMode = getUserInfo()->darkMode;
    HistoryVC *d = (HistoryVC *)((char *)self + VCSize);
    VCacheRef tbl = d->tbl;
    id view = msg0(id, self, sel_getUid("view"));
    tbl->view.setBG(view, tbl->view.sbg, d->clr->getColor(d->clr->cls, d->clr->sc, ColorPrimaryBG));

    d->charts[0] = createChartView(self, (long []){4}, 1, 1);
    d->charts[1] = createChartView(self, (long []){0, 1, 2, 3}, 4, 6);
    d->charts[2] = createChartView(self, (long []){0, 1, 2, 3}, 4, 0);
    d->picker = createSegmentedControl(CFSTR("historySegment%d"), 0);
    tbl->button.addTarget(d->picker, tbl->button.atgt, self, sel_getUid("buttonTapped:"), 4096);
    if (darkMode < 2) updateSegmentedControl(d->clr, d->picker, darkMode);
    msg1(void, id, msg0(id, self, sel_getUid("navigationItem")),
         sel_getUid("setTitleView:"), d->picker);

    CFStringRef titles[3]; id containers[3]; int heights[] = {390, 425, 550};
    ContainerView *c;
    fillStringArray(titles, CFSTR("chartHeader%d"), 3);
    for (int i = 0; i < 3; ++i) {
        containers[i] = containerView_init(tbl, d->clr, &c, titles[i]);
        setHeight(&tbl->cc, d->charts[i], heights[i], false, false);
        tbl->stack.addSub(c->stack, tbl->stack.asv, d->charts[i]);
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

    historyVC_updateSegment(self, nil, d->picker);
}

CFStringRef historyVC_stringForValue(id self, SEL _cmd _U_, double value) {
    CFArrayRef strs = ((HistoryVC *)((char *)self + VCSize))->model.axisStrings;
    return CFArrayGetValueAtIndex(strs, (int)value);
}

#pragma mark - Public Functions

void historyVC_clearData(id self) {
    HistoryVC *d = (HistoryVC *)((char *)self + VCSize);
    HistoryViewModel *model = &d->model;
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
        msg1(void, long, d->picker, sel_getUid("setSelectedSegmentIndex:"), 0);
        historyVC_updateSegment(self, nil, d->picker);
    }
}

void historyVC_updateColors(id self, unsigned char darkMode) {
    HistoryVC *d = (HistoryVC *)((char *)self + VCSize);
    VCacheRef tbl = d->tbl;
    id view = msg0(id, self, sel_getUid("view"));
    tbl->view.setBG(view, tbl->view.sbg, d->clr->getColor(d->clr->cls, d->clr->sc, ColorPrimaryBG));
    updateSegmentedControl(d->clr, d->picker, darkMode);
    view = (id)CFArrayGetValueAtIndex(msg0(CFArrayRef, view, sel_getUid("subviews")), 0);
    view = (id)CFArrayGetValueAtIndex(msg0(CFArrayRef, view, sel_getUid("subviews")), 0);
    CFArrayRef views = tbl->stack.getSub(view, tbl->stack.gsv);
    for (int i = 0; i < 3; ++i) {
        containerView_updateColors(
          (ContainerView *)((char *)CFArrayGetValueAtIndex(views, i) + ViewSize), tbl, d->clr);
    }
}
