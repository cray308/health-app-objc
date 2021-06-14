//
//  CalendarDateHelpers.c
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#include "CalendarDateHelpers.h"

static inline double getStartOfDay(CFCalendarRef calendar, double date) {
    double result = 0;
    CFCalendarGetTimeRangeOfUnit(calendar, kCFCalendarUnitDay, date, &result, NULL);
    return result;
}

static inline double date_calcEndOfWeek(double startOfWeek) {
    return startOfWeek + (double) (WeekSeconds - 1);
}

void date_calcWeekEndpoints(double date, CFCalendarRef calendar, DateSearchDirection direction, bool considerToday, double *start, double *end) {
    *start = date_calcStartOfWeek(date, calendar, direction, considerToday);
    *end = date_calcEndOfWeek(*start);
}

double date_calcStartOfWeek(double date, CFCalendarRef calendar, DateSearchDirection direction, bool considerToday) {
    int weekday = date_getDayOfWeek(date, calendar);

    if (considerToday && weekday == 2) {
        return getStartOfDay(calendar, date);
    }

    double result = direction == DateSearchDirection_Next ? date + WeekSeconds : date - WeekSeconds;
    weekday = date_getDayOfWeek(result, calendar);

    switch (direction) {
        case DateSearchDirection_Next:
            while (weekday != 2) {
                result -= DaySeconds;
                weekday = weekday == 1 ? 7 : weekday - 1;
            }
            break;
        case DateSearchDirection_Previous:
            while (weekday != 2) {
                result += DaySeconds;
                weekday = weekday == 7 ? 1 : weekday + 1;
            }
            break;
    }
    return getStartOfDay(calendar, result);
}
