//
//  CalendarDateHelpers.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef CalendarDateHelpers_h
#define CalendarDateHelpers_h

#include <time.h>

#define DaySeconds 86400
#define WeekSeconds 604800

time_t date_calcStartOfWeek(time_t date);
int date_getOffsetFromGMT(time_t date);

#define date_twoYears (appUserDataShared->weekStart - 63244800)

#endif /* CalendarDateHelpers_h */
