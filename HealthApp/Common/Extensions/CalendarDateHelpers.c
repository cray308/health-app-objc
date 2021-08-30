//
//  CalendarDateHelpers.c
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#include "CalendarDateHelpers.h"

int date_getOffsetFromGMT(time_t date) {
    struct tm gmtInfo;
    gmtime_r(&date, &gmtInfo);
    gmtInfo.tm_isdst = -1;
    return (int) (date - mktime(&gmtInfo));
}
