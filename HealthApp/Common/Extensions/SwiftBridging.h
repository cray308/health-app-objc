#ifndef SwiftBridging_h
#define SwiftBridging_h

#include "ViewControllerHelpers.h"

id createChartView(id formatter, long *colors, int count, uint8_t options);
void replaceDataSetEntries(id dataSet, CGPoint *entries, int count);
void updateChart(id view, id data, float axisMax);

#endif /* SwiftBridging_h */
