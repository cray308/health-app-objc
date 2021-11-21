#ifndef SwiftBridging_h
#define SwiftBridging_h

#include "ViewControllerHelpers.h"

id createChartView(id xAxisFormatter, CFArrayRef legendArr, uint8_t options);
void replaceDataSetEntries(id dataSet, id *entries, int count);
void updateDataSet(bool isSmall, int count, id dataSet, id *entries);
void updateChart(bool isSmall, id view, id data, float axisMax);

#endif /* SwiftBridging_h */
