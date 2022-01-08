#import "SwiftBridging.h"
#import "UIKit/UIKit.h"
@import Charts;

void toggleDarkModeForCharts(bool enabled) { [ChartUtility setDarkModeWithEnabled:enabled]; }

void setLegendLabel(LineChart *v, int index, CFStringRef text) {
    [v setLegendLabel:index text:(__bridge NSString*)text];
}

void setLineLimit(LineChart *v, float limit) { [v setLineLimit:limit]; }

Protocol *getValueFormatterType(void) { return @protocol(ValueFormatter); }

SEL getValueFormatterAction(void) { return @selector(stringForValue:); }

void disableLineChartView(LineChart *v) { [v setData:nil axisMax:0]; }

void updateChart(LineChart *v, ChartData *data, float max) { [v setData:data axisMax:max]; }

int getOSVersion(void) {
    if (@available(iOS 15, *)) return 15;
    else if (@available(iOS 14, *)) return 14;
    else if (@available(iOS 13, *)) return 13;
    return 12;
}

id createChartView(id formatter, HAInt *colors, int count, uint8_t options) {
    return [[LineChart alloc] initWithColors:colors count:count xFormatter:formatter options:options];
}

id createDataSet(int color, DataSet *fillSet) {
    return [[DataSet alloc] initWithColorVal:color fillSet:fillSet];
}

id createChartData(CFArrayRef dataSets, int lineWidth, uint8_t options) {
    return [[ChartData alloc] initWithDataSets:(__bridge NSArray*)dataSets lineWidth:lineWidth options:options];
}

void replaceDataSetEntries(DataSet *dataSet, CGPoint *entries, int count) {
    [dataSet replaceEntries:entries count:count];
}
