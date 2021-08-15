//
//  HistoryViewModel.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef HistoryViewModel_h
#define HistoryViewModel_h

#include "array.h"

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
gen_array_headers(chartData, void*)

typedef enum {
    FormatShort,
    FormatLong
} XAxisFormatType;

typedef struct {
    Array_chartData *entries;
    int totalWorkouts;
    int maxWorkouts;
    double avgWorkouts;
} HistoryGradientChartViewModel;

typedef struct {
    Array_chartData *entries[5];
    char names[4][10];
    int maxActivityTime;
    int totalByType[4];
} HistoryAreaChartViewModel;

typedef struct {
    Array_chartData *entries[4];
    char names[4][9];
    int maxWeight;
    int totalByExercise[4];
} HistoryLiftChartViewModel;

typedef struct {
    HistoryGradientChartViewModel gradientChartViewModel;
    HistoryAreaChartViewModel areaChartViewModel;
    HistoryLiftChartViewModel liftChartViewModel;
    Array_weekData *data;
} HistoryViewModel;

void historyViewModel_init(HistoryViewModel *model);
XAxisFormatType historyViewModel_formatDataForTimeRange(HistoryViewModel *this, int index);

#endif /* HistoryViewModel_h */
