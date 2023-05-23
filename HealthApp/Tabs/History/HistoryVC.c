#include "HistoryVC.h"
#include "AppDelegate.h"
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

id historyVC_init(HistoryModel **ref, VCacheRef tbl, CCacheRef clr) {
    id self = Sels.new(HistoryVCClass, Sels.nw);
    HistoryVC *d = (HistoryVC *)((char *)self + VCSize);
    d->tbl = tbl;
    d->clr = clr;
    HistoryModel *m = &d->model;
    *ref = m;
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

    m->totals.set = setInit(Sels.alloc(Set, Sels.alo), iSet, 5, nil);
    m->types.sets[0] = setInit(Sels.alloc(Set, Sels.alo), iSet, -1, nil);
    fillStringArray(workoutTypeNames, CFSTR("workoutTypes%d"), 4);
    fillStringArray(liftNames, CFSTR("exNames%02d"), 4);
    totalWorkoutsFormat = localize(CFSTR("totalWorkoutsLegend"));
    liftFormat = localize(CFSTR("liftLegend"));
    workoutTypeFormat = localize(CFSTR("workoutTypeLegend"));
    hourMinFmt = localize(CFSTR("hourMinFmt"));
    minsFmt = localize(CFSTR("minsFmt"));

    for (int i = 0; i < 4; ++i) {
        m->types.sets[i + 1] = setInit(Sels.alloc(Set, Sels.alo), iSet, colors[i], m->types.sets[i]);
        m->lifts.sets[i] = setInit(Sels.alloc(Set, Sels.alo), iSet, colors[i], nil);
    }

    CFArrayRef dataArr = CFArrayCreate(NULL, (const void *[]){m->totals.set}, 1, NULL);
    m->data[ChartTotals] = dataInit(Sels.alloc(Data, Sels.alo), iData, dataArr, 1, 3);
    CFRelease(dataArr);
    dataArr = CFArrayCreate(NULL, (const void *[]){
        m->types.sets[4], m->types.sets[3], m->types.sets[2], m->types.sets[1]
    }, 4, NULL);
    m->data[ChartTypes] = dataInit(Sels.alloc(Data, Sels.alo), iData, dataArr, 1, 5);
    CFRelease(dataArr);
    dataArr = CFArrayCreate(NULL, (const void **)m->lifts.sets, 4, NULL);
    m->data[ChartLifts] = dataInit(Sels.alloc(Data, Sels.alo), iData, dataArr, 3, 0);
    CFRelease(dataArr);
    return self;
}

#pragma mark - Selectors/Methods

void historyVC_updateSegment(id self, SEL _cmd _U_, id picker) {
    HistoryVC *d = (HistoryVC *)((char *)self + VCSize);

    int index = (int)msg0(long, picker, sel_getUid("selectedSegmentIndex"));
    int count = d->model.nEntries[index];
    if (!count) {
        for (int i = 0; i < 3; ++i) {
            cache.setData(d->charts[i], cache.sdt, nil, 0);
        }
        msg3(void, id, SEL, unsigned long, d->picker,
             sel_getUid("removeTarget:action:forControlEvents:"), nil, nil, 4096);
        return;
    }

    CFLocaleRef l = CFLocaleCopyCurrent();
    CFStringRef label = formatStr(l, totalWorkoutsFormat, d->model.totals.avgs[index]);
    cache.setLegendLabel(d->charts[ChartTotals], cache.slglb, 0, (id)label);
    CFRelease(label);

    for (int i = 0; i < 4; ++i) {
        int typeAverage = d->model.types.avgs[index][i];
        CFStringRef duration;
        if (typeAverage > 59) {
            duration = formatStr(l, hourMinFmt, typeAverage / 60, typeAverage % 60);
        } else {
            duration = formatStr(l, minsFmt, typeAverage);
        }
        label = formatStr(l, workoutTypeFormat, workoutTypeNames[i], duration);
        CFRelease(duration);
        cache.setLegendLabel(d->charts[ChartTypes], cache.slglb, i, (id)label);
        CFRelease(label);

        label = formatStr(l, liftFormat, liftNames[i], d->model.lifts.avgs[index][i]);
        cache.setLegendLabel(d->charts[ChartLifts], cache.slglb, i, (id)label);
        CFRelease(label);
    }
    CFRelease(l);

    int ref = d->model.refIndices[index];
    cache.setLineLimit(d->charts[ChartTotals], cache.slilm, d->model.totals.avgs[index]);
    cache.replaceEntries(d->model.totals.set, cache.rpe, &d->model.totals.entries[ref], count);
    for (int i = 0; i < 4; ++i) {
        cache.replaceEntries(d->model.types.sets[i], cache.rpe, &d->model.types.entries[i][ref], count);
        cache.replaceEntries(d->model.lifts.sets[i], cache.rpe, &d->model.lifts.entries[i][ref], count);
    }
    cache.replaceEntries(d->model.types.sets[4], cache.rpe, &d->model.types.entries[4][ref], count);
    for (int i = 0; i < 3; ++i) {
        cache.setData(d->charts[i], cache.sdt, d->model.data[i], d->model.maxes[i][index]);
    }
}

void historyVC_viewDidLoad(id self, SEL _cmd) {
    msgSup0(void, (&(struct objc_super){self, VC}), _cmd);

    const unsigned char darkMode = getUserInfo()->darkMode;
    HistoryVC *d = (HistoryVC *)((char *)self + VCSize);
    VCacheRef tbl = d->tbl;
    id view = msg0(id, self, sel_getUid("view"));
    tbl->view.setBG(view, tbl->view.sbg, d->clr->getColor(d->clr->cls, d->clr->sc, ColorPrimaryBG));

    d->charts[ChartTotals] = createChartView(self, (long []){4}, 1, 1);
    d->charts[ChartTypes] = createChartView(self, (long []){0, 1, 2, 3}, 4, 6);
    d->charts[ChartLifts] = createChartView(self, (long []){0, 1, 2, 3}, 4, 0);
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
    CFArrayRef strs = ((HistoryVC *)((char *)self + VCSize))->model.axisStrs;
    return CFArrayGetValueAtIndex(strs, (int)value);
}

#pragma mark - Public Functions

void historyVC_clearData(id self) {
    HistoryVC *d = (HistoryVC *)((char *)self + VCSize);
    HistoryModel *model = &d->model;
    if (!model->nEntries[2]) return;

    CFRelease(model->axisStrs);
    memset(model->nEntries, 0, 3 * sizeof(int));
    free(model->totals.entries);
    cache.replaceEntries(model->totals.set, cache.rpe, NULL, 0);
    for (int i = 0; i < 4; ++i) {
        free(model->types.entries[i]);
        free(model->lifts.entries[i]);
        cache.replaceEntries(model->types.sets[i], cache.rpe, NULL, 0);
        cache.replaceEntries(model->lifts.sets[i], cache.rpe, NULL, 0);
    }
    free(model->types.entries[4]);
    cache.replaceEntries(model->types.sets[4], cache.rpe, NULL, 0);

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
