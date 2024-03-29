#include "HistoryVC.h"
#include "ContainerView.h"
#include "Views.h"

#define getChartClass() objc_getClass("Charts.LineChart")

Class HistoryVCClass;

static CFStringRef totalWorkoutsFormat;
static CFStringRef workoutTypeFormat;
static CFStringRef liftFormat;
static CFStringRef hourMinsFormat;
static CFStringRef minsFormat;
static CFStringRef workoutTypeNames[4];
static CFStringRef liftNames[4];
static struct HistCache {
    const struct {
        SEL sllt, sd;
        void (*setLegendLabel)(id, SEL, long, id);
        void (*setData)(id, SEL, id, CGFloat);
    } view;
    const struct {
        SEL re;
        void (*replace)(id, SEL, CGPoint *, long);
    } set;
} hc;

id historyVC_init(HistoryModel **ref) {
    fillStringArray(workoutTypeNames, CFSTR("workoutTypes%d"), 4);
    fillStringArray(liftNames, CFSTR("exNames%02d"), 4);
    totalWorkoutsFormat = localize(CFSTR("totalWorkoutsLegend"));
    liftFormat = localize(CFSTR("liftLegend"));
    workoutTypeFormat = localize(CFSTR("workoutTypeLegend"));
    hourMinsFormat = localize(CFSTR("hourMinFmt"));
    minsFormat = localize(CFSTR("minsFmt"));

    Class Chart = getChartClass();
    SEL csllt = sel_getUid("setLegendLabel:text:"), csd = sel_getUid("setData:axisMax:");

    Class Set = objc_getClass("Charts.DataSet");
    SEL sre = sel_getUid("replaceEntries:count:"), iSet = sel_getUid("initWithColorVal:fillSet:");

    memcpy(&hc, &(struct HistCache){
        {
            csllt, csd,
            (void(*)(id, SEL, long, id))class_getMethodImplementation(Chart, csllt),
            (void(*)(id, SEL, id, CGFloat))class_getMethodImplementation(Chart, csd)
        },
        {sre, (void(*)(id, SEL, CGPoint *, long))class_getMethodImplementation(Set, sre)}
    }, sizeof(struct HistCache));

    id self = new(HistoryVCClass);
    HistoryModel *m = &getIVVC(HistoryVC, self)->model;
    *ref = m;

    id (*setInit)(id, SEL, long, id) =
      (id(*)(id, SEL, long, id))class_getMethodImplementation(Set, iSet);
    int colors[] = {0, 1, 2, 3};

    m->totals.set = setInit(alloc(Set), iSet, 5, nil);
    m->types.sets[0] = setInit(alloc(Set), iSet, -1, nil);

    for (int i = 0; i < 4; ++i) {
        m->types.sets[i + 1] = setInit(alloc(Set), iSet, colors[i], m->types.sets[i]);
        m->lifts.sets[i] = setInit(alloc(Set), iSet, colors[i], nil);
    }

    Class Data = objc_getClass("Charts.ChartData");
    SEL iData = sel_getUid("initWithDataSets:lineWidth:options:");
    id (*dataInit)(id, SEL, CFArrayRef, long, uint8_t) =
      (id(*)(id, SEL, CFArrayRef, long, uint8_t))class_getMethodImplementation(Data, iData);

    CFArrayRef dataSets = CFArrayCreate(NULL, (const void *[]){m->totals.set}, 1, NULL);
    m->data[ChartTotals] = dataInit(alloc(Data), iData, dataSets, 1, 3);
    CFRelease(dataSets);
    dataSets = CFArrayCreate(NULL, (const void *[]){
        m->types.sets[4], m->types.sets[3], m->types.sets[2], m->types.sets[1]
    }, 4, NULL);
    m->data[ChartTypes] = dataInit(alloc(Data), iData, dataSets, 1, 5);
    CFRelease(dataSets);
    dataSets = CFArrayCreate(NULL, (const void **)m->lifts.sets, 4, NULL);
    m->data[ChartLifts] = dataInit(alloc(Data), iData, dataSets, 3, 0);
    CFRelease(dataSets);
    return self;
}

#pragma mark - Selectors/Methods

void historyVC_changedSegment(id self, SEL _cmd _U_, id control) {
    HistoryVC *d = getIVVC(HistoryVC, self);

    long index = getSelectedSegmentIndex(control);
    int count = d->model.nEntries[index];
    if (!count) {
        for (int i = 0; i < 3; ++i) {
            hc.view.setData(d->charts[i], hc.view.sd, nil, 0);
        }
        msgV(objSig(void, id, SEL, u_long), control,
             sel_getUid("removeTarget:action:forControlEvents:"), nil, nil, ControlEventValueChanged);
        return;
    }

    CFLocaleRef locale = copyLocale();
    CFStringRef legendText = formatStr(locale, totalWorkoutsFormat, d->model.totals.avgs[index]);
    hc.view.setLegendLabel(d->charts[ChartTotals], hc.view.sllt, 0, (id)legendText);
    CFRelease(legendText);

    for (int i = 0; i < 4; ++i) {
        CFStringRef duration;
        int typeAverage = d->model.types.avgs[index][i];
        if (typeAverage > 59) {
            duration = formatStr(locale, hourMinsFormat, typeAverage / 60, typeAverage % 60);
        } else {
            duration = formatStr(locale, minsFormat, typeAverage);
        }
        legendText = formatStr(locale, workoutTypeFormat, workoutTypeNames[i], duration);
        hc.view.setLegendLabel(d->charts[ChartTypes], hc.view.sllt, i, (id)legendText);
        CFRelease(duration);
        CFRelease(legendText);

        legendText = formatStr(locale, liftFormat, liftNames[i], d->model.lifts.avgs[index][i]);
        hc.view.setLegendLabel(d->charts[ChartLifts], hc.view.sllt, i, (id)legendText);
        CFRelease(legendText);
    }
    CFRelease(locale);

    int refIdx = d->model.refIndices[index];
    msgV(objSig(void, CGFloat), d->charts[ChartTotals],
         sel_getUid("setLineLimit:"), d->model.totals.avgs[index]);
    hc.set.replace(d->model.totals.set, hc.set.re, &d->model.totals.entries[refIdx], count);
    for (int i = 0; i < 4; ++i) {
        hc.set.replace(d->model.types.sets[i], hc.set.re, &d->model.types.entries[i][refIdx], count);
        hc.set.replace(d->model.lifts.sets[i], hc.set.re, &d->model.lifts.entries[i][refIdx], count);
    }
    hc.set.replace(d->model.types.sets[4], hc.set.re, &d->model.types.entries[4][refIdx], count);
    for (int i = 0; i < 3; ++i) {
        hc.view.setData(d->charts[i], hc.view.sd, d->model.data[i], d->model.maxes[i][index]);
    }
}

void historyVC_viewDidLoad(id self, SEL _cmd) {
    msgSupV(supSig(), self, VC, _cmd);

    HistoryVC *d = getIVVC(HistoryVC, self);
    d->rangeControl = createSegmentedControl(CFSTR("historySegment%d"), 0);
    addTarget(d->rangeControl, self, getTapSel(), ControlEventValueChanged);
    msgV(objSig(void, id), getNavItem(self), sel_getUid("setTitleView:"), d->rangeControl);

    Class Chart = getChartClass();
    SEL iChart = sel_getUid("initWithColors:count:xFormatter:options:");
    id (*chartInit)(id, SEL, long *, long, id, uint8_t) =
      (id(*)(id, SEL, long *, long, id, uint8_t))class_getMethodImplementation(Chart, iChart);
    long colors[] = {0, 1, 2, 3};

    d->charts[ChartTotals] = chartInit(alloc(Chart), iChart, (long []){4}, 1, self, 1);
    d->charts[ChartTypes] = chartInit(alloc(Chart), iChart, colors, 4, self, 6);
    d->charts[ChartLifts] = chartInit(alloc(Chart), iChart, colors, 4, self, 0);

    int heights[] = {390, 425, 550};
    id containers[3];
    ContainerView *cvRefs[3];
    CFStringRef titles[3];
    fillStringArray(titles, CFSTR("chartHeader%d"), 3);
    for (int i = 0; i < 3; ++i) {
        containers[i] = containerView_init(&cvRefs[i], titles[i]);
        setHeight(d->charts[i], heights[i], false, false);
        addArrangedSubview(cvRefs[i]->stack, d->charts[i]);
    }
    setHidden(cvRefs[0]->divider, true);

    id vStack = createVStack(containers, 3);
    setSpacing(vStack, ViewSpacing);
    setupHierarchy(self, vStack, createScrollView(), ColorPrimaryBG);
    for (int i = 0; i < 3; ++i) {
        releaseView(containers[i]);
    }

    historyVC_changedSegment(self, nil, d->rangeControl);
}

CFStringRef historyVC_stringForValue(id self, SEL _cmd _U_, double value) {
    return CFArrayGetValueAtIndex(getIVVC(HistoryVC, self)->model.axisStrs, (int)value);
}

#pragma mark - Public Functions

void historyVC_clearData(id self) {
    HistoryVC *d = getIVVC(HistoryVC, self);
    if (!d->model.nEntries[2]) return;

    CFRelease(d->model.axisStrs);
    memset(d->model.nEntries, 0, 3 * sizeof(int));
    free(d->model.totals.entries);
    hc.set.replace(d->model.totals.set, hc.set.re, NULL, 0);
    for (int i = 0; i < 4; ++i) {
        free(d->model.types.entries[i]);
        free(d->model.lifts.entries[i]);
        hc.set.replace(d->model.types.sets[i], hc.set.re, NULL, 0);
        hc.set.replace(d->model.lifts.sets[i], hc.set.re, NULL, 0);
    }
    free(d->model.types.entries[4]);
    hc.set.replace(d->model.types.sets[4], hc.set.re, NULL, 0);

    if (isViewLoaded(self)) {
        setSelectedSegmentIndex(d->rangeControl, 0);
        historyVC_changedSegment(self, nil, d->rangeControl);
    }
}
