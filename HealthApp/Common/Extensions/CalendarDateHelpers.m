//
//  CalendarDateHelpers.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "CalendarDateHelpers.h"
#import "AppUserData.h"
#import "CustomOperators.h"

static inline double getStartOfDay(CFCalendarRef calendar, double date) {
    double result = 0;
    CFCalendarGetTimeRangeOfUnit(calendar, kCFCalendarUnitDay, date, &result, NULL);
    return result;
}

static inline double date_calcEndOfWeek(double startOfWeek) {
    return startOfWeek + (double) (WeekSeconds - 1);
}

NSString **calendar_getWeekDaySymbols(bool shortSymbols) {
    NSString **days = malloc(7 * sizeof(NSString*));
    NSCalendar *calendar = [NSCalendar currentCalendar];
    NSArray<NSString*> *daySymbols = shortSymbols ? [calendar shortWeekdaySymbols] : [calendar weekdaySymbols];

    int dayIdx = 0;
    for (int i = 1; i < 8; ++i) {
        days[dayIdx++] = [[NSString alloc] initWithString:daySymbols[mod(i, 7)]];
    }
    return days;
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

int date_indexForDate(double date, CFCalendarRef calendar) {
    return date_indexForWeekday(date_getDayOfWeek(date, calendar));
}

int date_indexForWeekday(int dayIndex) {
    return mod(dayIndex - 2, 7);
}
