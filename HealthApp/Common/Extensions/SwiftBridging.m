#import "SwiftBridging.h"
#import "UIKit/UIKit.h"
@import Charts;

void toggleDarkModeForCharts(bool enabled) { [ChartUtility setDarkModeWithEnabled:enabled]; }

id createChartEntry(int x, int y) { return [[ChartDataEntry alloc] initWithX:x y:y]; }

void setLegendLabel(LegendEntry *entry, CFStringRef text) { entry.label = _nsstr(text); }

void setLineLimit(LineChartView *v, float limit) { [v setLineLimit:limit]; }

Protocol *getValueFormatterType(void) { return @protocol(AxisValueFormatter); }

SEL getValueFormatterAction(void) { return @selector(stringForValue:axis:); }

void disableLineChartView(LineChartView *v) { v.data = nil; }

int getOSVersion(void) {
    if (@available(iOS 14, *)) return 14;
    else if (@available(iOS 13, *)) return 13;
    return 12;
}

id createChartView(id formatter, CFArrayRef legendArr, uint8_t options) {
    LineChartView *v = [[LineChartView alloc] initWithLegendEntries:_nsarr(legendArr) xFormatter:formatter options:options];
    v.translatesAutoresizingMaskIntoConstraints = false;
    return v;
}

id createDataSet(int color, int lineWidth, uint8_t options, LineChartDataSet *fillSet) {
    return [[LineChartDataSet alloc] initWithColorVal:color lineWidth:lineWidth options:options fillSet:fillSet];
}

id createChartData(id *dataSets, int count, uint8_t options) {
    CFArrayRef arr = CFArrayCreate(NULL, (const void **)dataSets, count, &(CFArrayCallBacks){0});
    id data = [[LineChartData alloc] initWithDataSets:_nsarr(arr) options:options];
    CFRelease(arr);
    return data;
}

void setupLegendEntries(id *entries, int *colors, int count) {
    for (int i = 0; i < count; ++i) entries[i] = [[LegendEntry alloc] initWithColorType:colors[i]];
}

void replaceDataSetEntries(LineChartDataSet *dataSet, id *entries, int count) {
    CFArrayRef array = CFArrayCreate(NULL, (const void **)entries, count, &(CFArrayCallBacks){0});
    [dataSet replaceEntries:_nsarr(array)];
    CFRelease(array);
}

void updateDataSet(bool isSmall, int count, LineChartDataSet *set, id *entries) {
    set.drawCirclesEnabled = isSmall;
    replaceDataSetEntries(set, entries, count);
}

void updateChart(bool isSmall, LineChartView *v, LineChartData *data, float axisMax) {
    [v setAxisMax:axisMax];
    [data setDrawValues:isSmall];
    v.data = data;
    [v animateWithXAxisDuration:isSmall ? 1.5 : 2.5];
}
