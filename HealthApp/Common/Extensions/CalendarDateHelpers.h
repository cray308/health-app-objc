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

long date_calcStartOfWeek(long date, CFCalendarRef calendar, DateSearchDirection direction, bool considerToday);

#define date_twoYears (appUserDataShared->weekStart - 63244800)

#endif /* CalendarDateHelpers_h */
