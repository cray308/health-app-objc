#import "SwiftBridging.h"
#import "UIKit/UIKit.h"
@import Charts;

bool setupCharts(void) { return [ChartUtility setup]; }

void populateChartsSelsAndFuncs(IMP *impArr, Class **classes, SEL **selArr) {
    Class Chart = LineChart.class, CSet = DataSet.class;
    Class owners[] = {Chart, Chart, Chart, CSet, CSet, ChartData.class};
    *classes[0] = CSet;
    *classes[1] = owners[5];
    SEL localSels[] = {
        @selector(setLegendLabel:text:), @selector(setLineLimit:),
        @selector(setData:axisMax:), @selector(replaceEntries:count:),
        @selector(initWithColorVal:fillSet:), @selector(initWithDataSets:lineWidth:options:)
    };
    for (int i = 0; i < 6; ++i) {
        impArr[i] = method_getImplementation(class_getInstanceMethod(owners[i], localSels[i]));
        *selArr[i] = localSels[i];
    }
}

id createChartView(id formatter, long *colors, int count, uint8_t options) {
    return [[LineChart alloc] initWithColors:colors count:count xFormatter:formatter options:options];
}
