//
//  Constants.h
//  HealthApp
//
//  Created by Christopher Ray on 3/21/21.
//

#ifndef Constants_h
#define Constants_h

#define TokenIconImageName @"token1"
#define _U_ __attribute__((__unused__))
#define MAX_USERNAME_LEN 32
#define TotalActivityLevels 3
#define TotalDays 7

typedef enum {
    ActivityType_Low,
    ActivityType_Medium,
    ActivityType_High
} ActivityType;

typedef struct {
    double date;
    ActivityType intensity;
    int duration;
    int tokens;
} SingleActivityModel;

typedef struct {
    double weekStart;
    double weekEnd;
    int tokensEarned;
    int durationByIntensity[3];
    int durationByDay[7];
} WeeklyActivitySummaryModel;

#endif /* Constants_h */
