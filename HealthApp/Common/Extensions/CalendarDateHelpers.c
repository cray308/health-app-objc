//
//  CalendarDateHelpers.c
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#include "CalendarDateHelpers.h"

static inline long getStartOfDay(CFCalendarRef calendar, long date) {
    double result = 0;
    CFCalendarGetTimeRangeOfUnit(calendar, kCFCalendarUnitDay, date, &result, NULL);
    return result;
}

static inline int getDayOfWeek(long date, CFCalendarRef calendar) {
    return (int) CFCalendarGetOrdinalityOfUnit(calendar, kCFCalendarUnitWeekday, kCFCalendarUnitWeekOfYear, date);
}

long date_calcStartOfWeek(long date, CFCalendarRef calendar, DateSearchDirection direction, bool considerToday) {
    int weekday = getDayOfWeek(date, calendar);

    if (considerToday && weekday == 2) return getStartOfDay(calendar, date);

    long result = direction == DateSearchDirectionNext ? date + WeekSeconds : date - WeekSeconds;
    weekday = getDayOfWeek(result, calendar);

    switch (direction) {
        case DateSearchDirectionNext:
            while (weekday != 2) {
                result -= DaySeconds;
                weekday = weekday == 1 ? 7 : weekday - 1;
            }
            break;
        case DateSearchDirectionPrev:
            while (weekday != 2) {
                result += DaySeconds;
                weekday = weekday == 7 ? 1 : weekday + 1;
            }
            break;
    }
    return getStartOfDay(calendar, result);
}
