//
//  ChartHelpers.m
//  HealthApp
//
//  Created by Christopher Ray on 3/28/21.
//

#import "ChartHelpers.h"
#import "CalendarDateHelpers.h"

DateXAxisFormatter *sharedHistoryXAxisFormatter = nil;

ChartLegendEntry *createLegendEntry(NSString *label, UIColor *color) {
    ChartLegendEntry *entry = [[ChartLegendEntry alloc] initWithLabel:label];
    entry.formColor = color;
    entry.formLineDashPhase = NAN;
    entry.formSize = 20;
    entry.labelColor = UIColor.labelColor;
    return entry;
}

void sharedHistoryXAxisFormatter_setup(void) {
    sharedHistoryXAxisFormatter = [[DateXAxisFormatter alloc] init];
}

void sharedHistoryXAxisFormatter_free(void) {
    if (sharedHistoryXAxisFormatter) [sharedHistoryXAxisFormatter release];
    sharedHistoryXAxisFormatter = nil;
}

@interface DateXAxisFormatter() {
    double referenceTime;
    NSDateFormatter *formatter;
}

@end

@implementation DateXAxisFormatter

- (id) init {
    if (!(self = [super init])) return nil;
    referenceTime = 0;
    formatter = [[NSDateFormatter alloc] init];
    return self;
}

- (void) dealloc {
    [formatter release];
    [super dealloc];
}

- (void) update: (double)newReferenceTime dateFormat: (NSString *)dateFormat {
    referenceTime = newReferenceTime;
    formatter.dateFormat = dateFormat;
}

- (NSString *_Nonnull) stringForValue: (double)value axis: (ChartAxisBase *_Nullable)axis {
    NSDate *date = [[NSDate alloc] initWithTimeIntervalSinceReferenceDate:value * DaySeconds + referenceTime];
    NSString *result = [formatter stringFromDate:date];
    [date release];
    return result;
}

@end
