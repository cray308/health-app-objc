//
//  CalendarDateHelpers.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef CalendarDateHelpers_h
#define CalendarDateHelpers_h

#include <CoreFoundation/CoreFoundation.h>

#define DaySeconds 86400
#define WeekSeconds 604800

typedef enum {
    DateSearchDirectionPrev,
    DateSearchDirectionNext
} DateSearchDirection;

static inline int date_getDayOfWeek(double date, CFCalendarRef calendar) {
    return (int) CFCalendarGetOrdinalityOfUnit(calendar, kCFCalendarUnitWeekday, kCFCalendarUnitWeekOfYear, date);
}

void date_calcWeekEndpoints(double date, CFCalendarRef calendar, DateSearchDirection direction, bool considerToday,
                            double *start, double *end);
double date_calcStartOfWeek(double date, CFCalendarRef calendar, DateSearchDirection direction, bool considerToday);

#define date_twoYears(calendar) date_calcStartOfWeek(CFAbsoluteTimeGetCurrent() - 63244800, calendar, DateSearchDirectionPrev, true)

#endif /* CalendarDateHelpers_h */
