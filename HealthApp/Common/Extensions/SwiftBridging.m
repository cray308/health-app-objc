#import "SwiftBridging.h"
#import "UIKit/UIKit.h"
@import Charts;

bool setupCharts(bool enabled) { return [ChartUtility setup:enabled]; }
