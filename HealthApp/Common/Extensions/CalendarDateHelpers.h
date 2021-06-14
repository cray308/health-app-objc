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
    DateSearchDirection_Previous,
    DateSearchDirection_Next
} DateSearchDirection;

static inline int date_getDayOfWeek(double date, CFCalendarRef calendar) {
    return (int) CFCalendarGetOrdinalityOfUnit(calendar, kCFCalendarUnitWeekday, kCFCalendarUnitWeekOfYear, date);
}

void date_calcWeekEndpoints(double date, CFCalendarRef calendar, DateSearchDirection direction, bool considerToday, double *start, double *end);
double date_calcStartOfWeek(double date, CFCalendarRef calendar, DateSearchDirection direction, bool considerToday);

#define date_lastMonth(calendar) date_calcStartOfWeek(CFAbsoluteTimeGetCurrent() - 2678400, calendar, DateSearchDirection_Previous, true)

#define date_sixMonths(calendar) date_calcStartOfWeek(CFAbsoluteTimeGetCurrent() - 15811200, calendar, DateSearchDirection_Previous, true)

#define date_lastYear(calendar) date_calcStartOfWeek(CFAbsoluteTimeGetCurrent() - 31622400, calendar, DateSearchDirection_Previous, true)

#define date_twoYears(calendar) date_calcStartOfWeek(CFAbsoluteTimeGetCurrent() - 63244800, calendar, DateSearchDirection_Previous, true)

#endif /* CalendarDateHelpers_h */
