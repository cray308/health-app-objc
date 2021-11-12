#ifndef SwiftBridging_h
#define SwiftBridging_h

#include "ViewControllerHelpers.h"

id createChartView(id parent, id xAxisFormatter, id *legendEntries, int count, int height);
void disableLineChartView(id v);
void updateDataSet(bool isSmall, int count, id dataSet, id *entries);
void updateChart(bool isSmall, id view, id data, float axisMax);

#endif /* SwiftBridging_h */
