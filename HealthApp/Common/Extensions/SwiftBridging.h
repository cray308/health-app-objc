#ifndef SwiftBridging_h
#define SwiftBridging_h

#include "ViewControllerHelpers.h"

id createDataSet(int color, id fillSet);
id createChartData(CFArrayRef dataSets, int lineWidth, uint8_t options);
id createChartView(id formatter, long *colors, int count, uint8_t options);
void replaceDataSetEntries(id dataSet, CGPoint *entries, int count);
void setLegendLabel(id v, int index, CFStringRef text);
void disableLineChartView(id v);
void updateChart(id view, id data, float axisMax);

#endif /* SwiftBridging_h */
