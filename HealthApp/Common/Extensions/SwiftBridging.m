//
//  SwiftBridging.m
//  HealthApp
//
//  Created by Christopher Ray on 8/22/21.
//

#import "SwiftBridging.h"
#import "UIKit/UIKit.h"
@import Charts;

id createChartEntry(int x, int y) { return [[ChartDataEntry alloc] initWithX:x y:y]; }

void setLegendLabel(id entry, CFStringRef text) { ((LegendEntry *)entry).label = _nsstr(text); }

void setLayoutMargins(id view, Padding *margins) {
    UIEdgeInsets insets = {margins->top, margins->left, margins->bottom, margins->right};
    ((void(*)(id,SEL,UIEdgeInsets))objc_msgSend)(view, sel_getUid("setLayoutMargins:"), insets);
}

id createChartView(id parent, id xAxisFormatter, id *legendEntries, int count, int height) {
    CFArrayRef legendArr = CFArrayCreate(NULL, (const void **)legendEntries,
                                         count, &kCocoaArrCallbacks);
    LineChartView *view = [[LineChartView alloc] initWithLegendEntries:_nsarr(legendArr)];
    view.translatesAutoresizingMaskIntoConstraints = false;
    view.xAxis.valueFormatter = (id<AxisValueFormatter>)xAxisFormatter;
    [((UIView *)parent) addSubview:view];
    activateConstraints((id []){
        [view.topAnchor constraintEqualToAnchor:((UIView *)parent).topAnchor],
        [view.bottomAnchor constraintEqualToAnchor:((UIView *)parent).bottomAnchor],
        [view.leadingAnchor constraintEqualToAnchor:((UIView *)parent).leadingAnchor constant:8],
        [view.trailingAnchor constraintEqualToAnchor:((UIView *)parent).trailingAnchor constant:-8],
        [view.heightAnchor constraintEqualToConstant:height]
    }, 5);
    CFRelease(legendArr);
    return view;
}

id createEmptyDataSet(void) {
    CFArrayRef entries = CFArrayCreate(NULL, (const void **)((id []){}), 0, &kCocoaArrCallbacks);
    LineChartDataSet *dataSet = [[LineChartDataSet alloc] initWithEntries:_nsarr(entries)];
    CFRelease(entries);
    return dataSet;
}

id createDataSet(id color) {
    CFArrayRef colors = CFArrayCreate(NULL, (const void **)((id[]){color}), 1, &kCocoaArrCallbacks);
    LineChartDataSet *dataSet = createEmptyDataSet();
    dataSet.colors = _nsarr(colors);
    [dataSet setCircleColor:color];
    CFRelease(colors);
    return dataSet;
}

id createChartData(id *dataSets, int count) {
    LineChartData *data;
    CFArrayRef arr = CFArrayCreate(NULL, (const void **) dataSets, count, &kCocoaArrCallbacks);
    data = [[LineChartData alloc] initWithDataSets:_nsarr(arr)];
    CFRelease(arr);
    return data;
}

void setupLegendEntries(id *entries, id *colors, int count) {
    id legendEntries[4];
    for (int i = 0; i < count; ++i) {
        legendEntries[i] = [[LegendEntry alloc] initWithLabel:@"" color:colors[i]];
    }
    memcpy(entries, legendEntries, count * sizeof(id));
}

void disableLineChartView(id v) {
    LineChartView *view = v;
    view.legend.enabled = false;
    view.data = nil;
    [view notifyDataSetChanged];
}

void updateDataSet(bool isSmall, int count, id dataSet, id *entries) {
    LineChartDataSet *set = dataSet;
    set.drawCirclesEnabled = isSmall;
    CFArrayRef array = CFArrayCreate(NULL, (const void **)entries, count, &kCocoaArrCallbacks);
    [set replaceEntries:_nsarr(array)];
    CFRelease(array);
}

void updateChart(bool isSmall, int count, id v, id data, float axisMax) {
    LineChartView *view = v;
    LineChartData *chartData = data;
    view.leftAxis.axisMaximum = axisMax;
    view.xAxis.labelCount = isSmall ? count : 6;
    view.legend.enabled = true;
    [chartData setDrawValues:isSmall];
    view.data = chartData;
    [view.data notifyDataChanged];
    [view notifyDataSetChanged];
    [view animateWithXAxisDuration:isSmall ? 1.5 : 2.5];
}

void setTabBarItemColors(id appearance) {
    UITabBarItemAppearance *item = appearance;
    item.normal.iconColor = UIColor.systemGrayColor;
    item.normal.titleTextAttributes = @{NSForegroundColorAttributeName: UIColor.systemGrayColor};
    item.selected.iconColor = UIColor.systemRedColor;
    item.selected.titleTextAttributes = @{NSForegroundColorAttributeName: UIColor.systemRedColor};
}
