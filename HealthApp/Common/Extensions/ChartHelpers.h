//
//  ChartHelpers.h
//  HealthApp
//
//  Created by Christopher Ray on 3/28/21.
//

#ifndef ChartHelpers_h
#define ChartHelpers_h

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#import "HealthApp-Swift.h"
#pragma clang diagnostic pop

#define ActivityLevelLowColorName @"homePieChartLow"
#define ActivityLevelMediumColorName @"homePieChartMedium"
#define ActivityLevelHighColorName @"homePieChartHigh"

@interface DateXAxisFormatter: NSObject<ChartAxisValueFormatter>

- (id) init;
- (void) update: (double)newReferenceTime dateFormat: (NSString *)dateFormat;

@end

extern DateXAxisFormatter *sharedHistoryXAxisFormatter;

void sharedHistoryXAxisFormatter_setup(void);
void sharedHistoryXAxisFormatter_free(void);

#endif /* ChartHelpers_h */
