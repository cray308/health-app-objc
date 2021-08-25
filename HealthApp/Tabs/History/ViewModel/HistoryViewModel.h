//
//  HistoryViewModel.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef HistoryViewModel_h
#define HistoryViewModel_h

#include "array.h"
#include "CocoaHelpers.h"

typedef struct {
    int year;
    int month;
    int day;
    int totalWorkouts;
    int durationByType[4];
    int cumulativeDuration[4];
    int weightArray[4];
} HistoryWeekDataModel;

gen_array_headers(weekData, HistoryWeekDataModel)
gen_array_headers(chartData, id)

typedef struct {
    char wordMonths[12][4];
    char numMonths[12][3];
    enum {
        FormatShort,
        FormatLong
    } formatType;
    CFStringRef currString;
} HistoryXAxisFormatter;

typedef struct {
    Array_chartData *entries;
    id legendEntries[1];
    id dataSet;
    id chartData;
    double avgWorkouts;
    double yMax;
} HistoryTotalWorkoutsChartViewModel;

typedef struct {
    Array_chartData *entries[5];
    id legendEntries[4];
    id dataSets[5];
    id chartData;
    int totalByType[4];
    double yMax;
    CFStringRef durationStr;
    char names[4][10];
} HistoryWorkoutTypeChartViewModel;

typedef struct {
    Array_chartData *entries[4];
    id legendEntries[4];
    id dataSets[4];
    id chartData;
    int totalByExercise[4];
    double yMax;
    char names[4][9];
} HistoryLiftChartViewModel;

typedef struct {
    HistoryTotalWorkoutsChartViewModel totalWorkoutsViewModel;
    HistoryWorkoutTypeChartViewModel workoutTypeViewModel;
    HistoryLiftChartViewModel liftViewModel;
    HistoryXAxisFormatter formatter;
    Array_weekData *data;
} HistoryViewModel;

void historyViewModel_init(HistoryViewModel *model);
void historyViewModel_fetchData(HistoryViewModel *this);
void historyViewModel_formatDataForTimeRange(HistoryViewModel *this, int index);

CFStringRef historyViewModel_getXAxisLabel(HistoryViewModel *this, int index);
CFStringRef workoutTypeViewModel_getDuration(HistoryWorkoutTypeChartViewModel *this, int minutes);

#endif /* HistoryViewModel_h */
