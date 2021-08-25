//
//  SwiftBridging.m
//  HealthApp
//
//  Created by Christopher Ray on 8/22/21.
//

#import "SwiftBridging.h"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#import "HealthApp-Swift.h"
#pragma clang diagnostic pop

id createChartEntry(int x, int y) {
    return [[ChartDataEntry alloc] initWithX:x y:y];
}

void setLegendLabel(id entry, CFStringRef text) {
    ((ChartLegendEntry *)entry).label = (__bridge NSString*) text;
}

void setLayoutMargins(id view, HAEdgeInsets *margins) {
    UIEdgeInsets insets = {margins->top, margins->left, margins->bottom, margins->right};
    ((void(*)(id,SEL,UIEdgeInsets))objc_msgSend)(view, sel_getUid("setLayoutMargins:"), insets);
}

id createChartView(id parent, id xAxisFormatter, id *legendEntries, int count, int height) {
    CGRect bounds = UIScreen.mainScreen.bounds;
    CFArrayRef legendArr = CFArrayCreate(NULL, (const void **)legendEntries,
                                         count, &kCocoaArrCallbacks);
    LineChartView *view = [[LineChartView alloc] initWithFrame:CGRectZero];
    view.translatesAutoresizingMaskIntoConstraints = false;
    view.noDataText = @"No data is available";
    view.leftAxis.axisMinimum = 0;
    view.rightAxis.enabled = false;
    view.legend.horizontalAlignment = ChartLegendHorizontalAlignmentCenter;
    view.legend.orientation = ChartLegendOrientationVertical;
    view.legend.yEntrySpace = 10;
    view.legend.font = [UIFont systemFontOfSize:16];
    [view.legend setCustomWithEntries:(__bridge NSArray*)legendArr];
    view.legend.enabled = false;
    view.legend.xEntrySpace = bounds.size.width - 48;
    view.xAxis.labelPosition = XAxisLabelPositionBottom;
    view.xAxis.gridLineWidth = 1.5;
    view.xAxis.labelFont = [UIFont systemFontOfSize:12];
    view.xAxis.granularityEnabled = true;
    view.xAxis.avoidFirstLastClippingEnabled = true;
    view.xAxis.labelRotationAngle = 45;
    view.xAxis.valueFormatter = (NSObject<ChartAxisValueFormatter> *)xAxisFormatter;

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
    LineChartDataSet *dataSet = [[LineChartDataSet alloc]
                                 initWithEntries:(__bridge NSArray*)entries];
    CFRelease(entries);
    return dataSet;
}

id createDataSet(id color) {
    CFArrayRef colors = CFArrayCreate(NULL, (const void **)((id[]){color}), 1, &kCocoaArrCallbacks);
    LineChartDataSet *dataSet = createEmptyDataSet();
    dataSet.colors = (__bridge NSArray*)colors;
    dataSet.axisDependency = AxisDependencyLeft;
    dataSet.valueFont = [UIFont boldSystemFontOfSize:11];
    dataSet.valueTextColor = UIColor.labelColor;
    [dataSet setCircleColor:color];
    dataSet.circleHoleColor = color;
    dataSet.circleRadius = 2;
    CFRelease(colors);
    return dataSet;
}

id createChartData(id *dataSets, int count) {
    LineChartData *data;
    if (count > 1) {
        CFArrayRef arr = CFArrayCreate(NULL, (const void **) dataSets, count, &kCocoaArrCallbacks);
        data = [[LineChartData alloc] initWithDataSets:(__bridge NSArray*)arr];
        CFRelease(arr);
    } else {
        data = [[LineChartData alloc] initWithDataSet:dataSets[0]];
    }
    return data;
}

void setupLegendEntries(id *entries, id *colors, int count) {
    id legendEntries[4];
    for (int i = 0; i < count; ++i) {
        ChartLegendEntry *entry = [[ChartLegendEntry alloc] initWithLabel:@""];
        entry.formColor = colors[i];
        entry.formLineDashPhase = NAN;
        entry.formSize = 20;
        entry.labelColor = UIColor.labelColor;
        legendEntries[i] = entry;
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
    [set replaceEntries:(__bridge NSArray*)array];
    CFRelease(array);
}

void updateChart(bool isSmall, int count, id v, id data, double axisMax) {
    LineChartView *view = v;
    LineChartData *chartData = data;
    view.leftAxis.axisMaximum = axisMax;
    view.xAxis.forceLabelsEnabled = isSmall;
    view.xAxis.labelCount = isSmall ? count : 6;
    view.legend.enabled = true;
    [chartData setDrawValues:isSmall];
    view.data = chartData;
    [view.data notifyDataChanged];
    [view notifyDataSetChanged];
    [view animateWithXAxisDuration:isSmall ? 1.5 : 2.5];
}
