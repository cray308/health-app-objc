#import "SwiftBridging.h"
#import "UIKit/UIKit.h"
@import Charts;

void toggleDarkModeForCharts(bool enabled) { [ChartUtility setDarkModeWithEnabled:enabled]; }

void populateChartsSelsAndFuncs(Class *classes, IMP *impArr, SEL *selArr) {
    Class Chart = LineChart.class, CSet = DataSet.class, CData = ChartData.class;
    memcpy(classes, (Class []){CSet, CData}, sizeof(Class) << 1);
    SEL localSels[] = {@selector(setLegendLabel:text:), @selector(setLineLimit:),
        @selector(setData:axisMax:), @selector(replaceEntries:count:),
        @selector(initWithColorVal:fillSet:), @selector(initWithDataSets:lineWidth:options:)
    };
    impArr[0] = method_getImplementation(class_getInstanceMethod(Chart, localSels[0]));
    impArr[1] = method_getImplementation(class_getInstanceMethod(Chart, localSels[1]));
    impArr[2] = method_getImplementation(class_getInstanceMethod(Chart, localSels[2]));
    impArr[3] = method_getImplementation(class_getInstanceMethod(CSet, localSels[3]));
    impArr[4] = method_getImplementation(class_getInstanceMethod(CSet, localSels[4]));
    impArr[5] = method_getImplementation(class_getInstanceMethod(CData, localSels[5]));
    memcpy(selArr, localSels, sizeof(SEL) * 6);
}

id createChartView(id formatter, long *colors, int count, uint8_t options) {
    return [[LineChart alloc] initWithColors:colors count:count xFormatter:formatter options:options];
}
