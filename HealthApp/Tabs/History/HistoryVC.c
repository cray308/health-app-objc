#include "HistoryVC.h"
#include "ContainerView.h"
#include "SwiftBridging.h"

Class HistoryVCClass;

enum {
    ChartColorNA = -1,
    ChartColorLimitLine = 4,
    ChartColorGradient = 5
};

enum {
    DataDrawFill = 1,
    DataDrawGradient = 2,
    DataDurationFormat = 4
};

enum {
    ChartAddLimitLine = 1,
    ChartRenderArea = 2
};

static CFStringRef totalWorkoutsFormat;
static CFStringRef workoutTypeFormat;
static CFStringRef liftFormat;
static CFStringRef hourMinFmt;
static CFStringRef minsFmt;
static CFStringRef workoutTypeNames[4];
static CFStringRef liftNames[4];
static struct HistCache {
    const SEL slglb, slilm, sdt, rpe;
    void (*setLegendLabel)(id,SEL,long,id);
    void (*setLineLimit)(id,SEL,CGFloat);
    void (*setData)(id,SEL,id,CGFloat);
    void (*replaceEntries)(id,SEL,CGPoint*,long);
} hc;

id historyVC_init(HistoryModel **ref) {
    fillStringArray(workoutTypeNames, CFSTR("workoutTypes%d"), 4);
    fillStringArray(liftNames, CFSTR("exNames%02d"), 4);
    totalWorkoutsFormat = localize(CFSTR("totalWorkoutsLegend"));
    liftFormat = localize(CFSTR("liftLegend"));
    workoutTypeFormat = localize(CFSTR("workoutTypeLegend"));
    hourMinFmt = localize(CFSTR("hourMinFmt"));
    minsFmt = localize(CFSTR("minsFmt"));
    Class Set = NULL, Data = NULL;
    SEL sels[4], iSet = NULL, iData = NULL;
    SEL *args[] = {&sels[0], &sels[1], &sels[2], &sels[3], &iSet, &iData};
    IMP imps[6];
    populateChartsSelsAndFuncs(imps, (Class *[]){&Set, &Data}, args);
    memcpy(&hc, &(struct HistCache){sels[0], sels[1], sels[2], sels[3],
        (void(*)(id,SEL,long,id))imps[0], (void(*)(id,SEL,CGFloat))imps[1],
        (void(*)(id,SEL,id,CGFloat))imps[2], (void(*)(id,SEL,CGPoint*,long))imps[3]
    }, sizeof(struct HistCache));

    id self = new(HistoryVCClass);
    HistoryVC *d = (HistoryVC *)getIVVC(self);
    HistoryModel *m = &d->model;
    *ref = m;
    int colors[] = {0, 1, 2, 3};
    id (*setInit)(id,SEL,long,id) = (id(*)(id,SEL,long,id))imps[4];
    id (*dataInit)(id,SEL,CFArrayRef,long,uint8_t) = (id(*)(id,SEL,CFArrayRef,long,uint8_t))imps[5];

    m->tw.dataSet = setInit(alloc(Set), iSet, ChartColorGradient, nil);
    m->wt.dataSets[0] = setInit(alloc(Set), iSet, ChartColorNA, nil);

    for (int i = 0; i < 4; ++i) {
        m->wt.dataSets[i + 1] = setInit(alloc(Set), iSet, colors[i], m->wt.dataSets[i]);
        m->lifts.dataSets[i] = setInit(alloc(Set), iSet, colors[i], nil);
    }

    CFArrayRef dataArr = CFArrayCreate(NULL, (const void *[]){m->tw.dataSet}, 1, NULL);
    m->data[ChartTW] = dataInit(alloc(Data), iData, dataArr, 1, DataDrawFill | DataDrawGradient);
    CFRelease(dataArr);
    const void *areaSets[] = {
        m->wt.dataSets[4], m->wt.dataSets[3], m->wt.dataSets[2], m->wt.dataSets[1]
    };
    dataArr = CFArrayCreate(NULL, areaSets, 4, NULL);
    m->data[ChartWT] = dataInit(alloc(Data), iData, dataArr, 1, DataDrawFill | DataDurationFormat);
    CFRelease(dataArr);
    dataArr = CFArrayCreate(NULL, (const void **)m->lifts.dataSets, 4, NULL);
    m->data[ChartLifts] = dataInit(alloc(Data), iData, dataArr, 3, 0);
    CFRelease(dataArr);
    return self;
}

#pragma mark - Selectors/Methods

void historyVC_updateSegment(id self, SEL _cmd _U_, id picker) {
    HistoryVC *d = (HistoryVC *)getIVVC(self);
    HistoryModel *m = &d->model;

    int index = (int)msg0(long, picker, sel_getUid("selectedSegmentIndex"));
    int count = d->model.nEntries[index];
    if (!count) {
        for (int i = 0; i < 3; ++i) {
            hc.setData(d->charts[i], hc.sdt, nil, 0);
        }
        msg3(void, id, SEL, u_long, d->picker, sel_getUid("removeTarget:action:forControlEvents:"),
             nil, nil, UIControlEventValueChanged);
        return;
    }

    CFLocaleRef locale = CFLocaleCopyCurrent();
    CFStringRef label = formatStr(locale, totalWorkoutsFormat, m->tw.avgs[index]);
    hc.setLegendLabel(d->charts[ChartTW], hc.slglb, 0, (id)label);
    CFRelease(label);

    for (int i = 0; i < 4; ++i) {
        int typeAverage = m->wt.avgs[index][i];
        CFStringRef duration;
        if (typeAverage > 59) {
            duration = formatStr(locale, hourMinFmt, typeAverage / 60, typeAverage % 60);
        } else {
            duration = formatStr(locale, minsFmt, typeAverage);
        }
        label = formatStr(locale, workoutTypeFormat, workoutTypeNames[i], duration);
        CFRelease(duration);
        hc.setLegendLabel(d->charts[ChartWT], hc.slglb, i, (id)label);
        CFRelease(label);

        label = formatStr(locale, liftFormat, liftNames[i], m->lifts.avgs[index][i]);
        hc.setLegendLabel(d->charts[ChartLifts], hc.slglb, i, (id)label);
        CFRelease(label);
    }
    CFRelease(locale);

    int ref = d->model.refIndices[index];
    hc.setLineLimit(d->charts[ChartTW], hc.slilm, m->tw.avgs[index]);
    hc.replaceEntries(m->tw.dataSet, hc.rpe, &m->tw.entries[ref], count);
    for (int i = 0; i < 4; ++i) {
        hc.replaceEntries(m->wt.dataSets[i], hc.rpe, &m->wt.entries[i][ref], count);
        hc.replaceEntries(m->lifts.dataSets[i], hc.rpe, &m->lifts.entries[i][ref], count);
    }
    hc.replaceEntries(m->wt.dataSets[4], hc.rpe, &m->wt.entries[4][ref], count);
    for (int i = 0; i < 3; ++i) {
        hc.setData(d->charts[i], hc.sdt, m->data[i], m->maxes[i][index]);
    }
}

void historyVC_viewDidLoad(id self, SEL _cmd) {
    msgSup0(void, (&(struct objc_super){self, VC}), _cmd);

    HistoryVC *d = (HistoryVC *)getIVVC(self);
    d->picker = createSegmentedControl(CFSTR("historySegment%d"), 0);
    addTarget(d->picker, self, sel_getUid("buttonTapped:"), UIControlEventValueChanged);
    id navItem = msg0(id, self, sel_getUid("navigationItem"));
    msg1(void, id, navItem, sel_getUid("setTitleView:"), d->picker);

    long colors[] = {0, 1, 2, 3};
    d->charts[ChartTW] = createChartView(self, (long []){ChartColorLimitLine}, 1, ChartAddLimitLine);
    d->charts[ChartWT] = createChartView(self, colors, 4, ChartRenderArea | DataDurationFormat);
    d->charts[ChartLifts] = createChartView(self, colors, 4, 0);

    int heights[] = {390, 425, 550};
    id containers[3];
    CFStringRef titles[3];
    ContainerView *c;
    fillStringArray(titles, CFSTR("chartHeader%d"), 3);
    for (int i = 0; i < 3; ++i) {
        containers[i] = containerView_init(&c, titles[i]);
        setHeight(d->charts[i], heights[i], false, false);
        addArrangedSubview(c->stack, d->charts[i]);
        setHidden(c->divider, !i);
    }

    id vStack = createVStack(containers, 3);
    setSpacing(vStack, ViewSpacing);
    setLayoutMargins(vStack, VCMargins);
    setupHierarchy(self, vStack, createScrollView(), ColorPrimaryBG);
    for (int i = 0; i < 3; ++i) {
        releaseV(containers[i]);
    }

    historyVC_updateSegment(self, nil, d->picker);
}

CFStringRef historyVC_stringForValue(id self, SEL _cmd _U_, double value) {
    return CFArrayGetValueAtIndex(((HistoryVC *)getIVVC(self))->model.axisStrings, (int)value);
}

#pragma mark - Public Functions

void historyVC_clearData(id self) {
    HistoryVC *d = (HistoryVC *)getIVVC(self);
    if (!d->model.nEntries[2]) return;

    CFRelease(d->model.axisStrings);
    memset(d->model.nEntries, 0, 3 * sizeof(int));
    free(d->model.tw.entries);
    hc.replaceEntries(d->model.tw.dataSet, hc.rpe, NULL, 0);
    for (int i = 0; i < 4; ++i) {
        free(d->model.wt.entries[i]);
        free(d->model.lifts.entries[i]);
        hc.replaceEntries(d->model.wt.dataSets[i], hc.rpe, NULL, 0);
        hc.replaceEntries(d->model.lifts.dataSets[i], hc.rpe, NULL, 0);
    }
    free(d->model.wt.entries[4]);
    hc.replaceEntries(d->model.wt.dataSets[4], hc.rpe, NULL, 0);

    if (msg0(bool, self, sel_getUid("isViewLoaded"))) {
        msg1(void, long, d->picker, sel_getUid("setSelectedSegmentIndex:"), 0);
        historyVC_updateSegment(self, nil, d->picker);
    }
}
