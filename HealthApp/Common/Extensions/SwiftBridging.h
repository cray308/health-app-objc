#ifndef SwiftBridging_h
#define SwiftBridging_h

#include "ViewControllerHelpers.h"

#if defined(__LP64__)
id createChartView(id formatter, long *colors, int count, uint8_t options);
#else
id createChartView(id formatter, int *colors, int count, uint8_t options);
#endif

void replaceDataSetEntries(id dataSet, CGPoint *entries, int count);
void updateChart(id view, id data, float axisMax);

#endif /* SwiftBridging_h */
