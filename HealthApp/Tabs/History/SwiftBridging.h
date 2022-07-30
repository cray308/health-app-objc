#ifndef SwiftBridging_h
#define SwiftBridging_h

#include "Views.h"

bool setupCharts(void);
void populateChartsSelsAndFuncs(IMP *impArr, Class **classes, SEL **selArr);
id createChartView(id formatter, long *colors, int count, uint8_t options);

#endif /* SwiftBridging_h */
