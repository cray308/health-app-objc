//
//  HistoryViewModel.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef HistoryViewModel_h
#define HistoryViewModel_h

#include <CoreFoundation/CFBase.h>
#include "ObjectArray.h"

typedef struct {
    int year;
    int month;
    int day;
    int totalWorkouts;
    int durationByType[4];
    int cumulativeDuration[4];
    short weightArray[4];
} HistoryWeekDataModel;

gen_array_headers(weekData, HistoryWeekDataModel)

typedef struct {
    Array_object *entries;
    CFStringRef legendFormat;
    id legendEntries[1];
    id dataSet;
    id chartData;
    float avgWorkouts;
    float yMax;
} TotalWorkoutsChartViewModel;

typedef struct {
    Array_object *entries[5];
    CFStringRef legendFormat;
    id legendEntries[4];
    id dataSets[5];
    id chartData;
    int totalByType[4];
    float yMax;
    CFStringRef durationStr;
    CFStringRef names[4];
} WorkoutTypeChartViewModel;

typedef struct {
    Array_object *entries[4];
    CFStringRef legendFormat;
    id legendEntries[4];
    id dataSets[4];
    id chartData;
    int totalByExercise[4];
    float yMax;
    CFStringRef names[4];
} LiftChartViewModel;

typedef struct {
    TotalWorkoutsChartViewModel totalWorkoutsModel;
    WorkoutTypeChartViewModel workoutTypeModel;
    LiftChartViewModel liftModel;
    struct XAxisFormatter {
        CFStringRef months[12];
        CFStringRef currString;
    } formatter;
    Array_weekData *data;
    bool isSmall;
} HistoryViewModel;

void historyViewModel_init(HistoryViewModel *model);
void historyViewModel_fetchData(HistoryViewModel *this);
void historyViewModel_formatDataForTimeRange(HistoryViewModel *this, int index);
id getLimitLineColor(void);

CFStringRef historyViewModel_getXAxisLabel(HistoryViewModel *this, int index);
CFStringRef workoutTypeViewModel_getDuration(WorkoutTypeChartViewModel *this, int minutes);

#endif /* HistoryViewModel_h */
