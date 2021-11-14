#import "SwiftBridging.h"
#import "UIKit/UIKit.h"
@import Charts;

void toggleDarkModeForCharts(bool enabled) { [ChartUtility setDarkModeWithEnabled:enabled]; }

id createChartEntry(int x, int y) { return [[ChartDataEntry alloc] initWithX:x y:y]; }

void setLegendLabel(LegendEntry *entry, CFStringRef text) { entry.label = _nsstr(text); }

void setLayoutMargins(UIView *v, Padding margins) {
    v.layoutMargins = (UIEdgeInsets){margins.top, margins.left, margins.bottom, margins.right};
}

int getOSVersion(void) {
    if (@available(iOS 14, *)) return 14;
    else if (@available(iOS 13, *)) return 13;
    return 12;
}

void setScrollViewInsets(UIScrollView *v, Padding margins) {
    UIEdgeInsets insets = (UIEdgeInsets){margins.top, margins.left, margins.bottom, margins.right};
    v.contentInset = insets;
    v.scrollIndicatorInsets = insets;
}

id createChartView(UIView *parent, id formatter, id *legendEntries, int count, int height) {
    CFArrayRef arr = CFArrayCreate(NULL, (const void **)legendEntries, count, &(CFArrayCallBacks){0});
    LineChartView *view = [[LineChartView alloc] initWithLegendEntries:_nsarr(arr)];
    view.translatesAutoresizingMaskIntoConstraints = false;
    view.xAxis.valueFormatter = (id<AxisValueFormatter>)formatter;
    [parent addSubview:view];
    pin(view, parent, (Padding){0, 8, 0, 8}, 0);
    setHeight(view, height);
    CFRelease(arr);
    return view;
}

id createDataSet(int color, bool withFill) {
    CFArrayRef entries = CFArrayCreate(NULL, (const void *[]){}, 0, &(CFArrayCallBacks){0});
    LineChartDataSet *dataSet = [[LineChartDataSet alloc] initWithEntries:_nsarr(entries) colorVal:color withFill:withFill];
    CFRelease(entries);
    return dataSet;
}

id createChartData(id *dataSets, int count) {
    CFArrayRef arr = CFArrayCreate(NULL, (const void **)dataSets, count, &(CFArrayCallBacks){0});
    id data = [[LineChartData alloc] initWithDataSets:_nsarr(arr)];
    CFRelease(arr);
    return data;
}

void setupLegendEntries(id *entries, int *colors, int count) {
    for (int i = 0; i < count; ++i)
        entries[i] = [[LegendEntry alloc] initWithLabel:@"" colorType:colors[i]];
}

void disableLineChartView(LineChartView *v) {
    v.legend.enabled = false;
    v.data = nil;
    [v notifyDataSetChanged];
}

void updateDataSet(bool isSmall, int count, LineChartDataSet *set, id *entries) {
    set.drawCirclesEnabled = isSmall;
    CFArrayRef array = CFArrayCreate(NULL, (const void **)entries, count, &(CFArrayCallBacks){0});
    [set replaceEntries:_nsarr(array)];
    CFRelease(array);
}

void updateChart(bool isSmall, LineChartView *v, LineChartData *data, float axisMax) {
    v.leftAxis.axisMaximum = axisMax;
    v.legend.enabled = true;
    [data setDrawValues:isSmall];
    v.data = data;
    [v.data notifyDataChanged];
    [v notifyDataSetChanged];
    [v animateWithXAxisDuration:isSmall ? 1.5 : 2.5];
}
