//
//  SwiftBridging.h
//  HealthApp
//
//  Created by Christopher Ray on 8/22/21.
//

#ifndef SwiftBridging_h
#define SwiftBridging_h

#include "ViewControllerHelpers.h"

id createChartEntry(int x, int y);
id createEmptyDataSet(void);
id createDataSet(id color);
id createChartData(id *dataSets, int count);
id createChartView(id parent, id xAxisFormatter, id *legendEntries, int count, int height);
void setLayoutMargins(id view, Padding margins);
void setLegendLabel(id entry, CFStringRef text);
void setupLegendEntries(id *entries, id *colors, int count);
void disableLineChartView(id v);
void updateDataSet(bool isSmall, int count, id dataSet, id *entries);
void updateChart(bool isSmall, id view, id data, float axisMax);
void setTabBarItemColors(id appearance);

#endif /* SwiftBridging_h */
