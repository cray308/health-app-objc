//
//  HistoryViewModel.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef HistoryViewModel_h
#define HistoryViewModel_h

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#import "HealthApp-Swift.h"
#pragma clang diagnostic pop
#include <CoreFoundation/CoreFoundation.h>

typedef struct HistoryViewModel HistoryViewModel;
typedef struct HistoryTabCoordinator HistoryTabCoordinator;
typedef struct Array_weekData Array_weekData;
typedef struct Array_chartData Array_chartData;
typedef struct HistoryGradientChartViewModel HistoryGradientChartViewModel;
typedef struct HistoryAreaChartViewModel HistoryAreaChartViewModel;
typedef struct HistoryLiftChartViewModel HistoryLiftChartViewModel;

struct HistoryGradientChartViewModel {
    Array_chartData *entries;
    CFStringRef legendLabelFormat;
    int totalWorkouts;
    int maxWorkouts;
    double avgWorkouts;
};

struct HistoryAreaChartViewModel {
    Array_chartData *entries[5];
    CFStringRef legendLabelFormats[4];
    int maxActivityTime;
    int totalByType[4];
};

struct HistoryLiftChartViewModel {
    Array_chartData *entries[4];
    CFStringRef legendLabelFormats[4];
    int maxWeight;
    int totalByExercise[4];
};

struct HistoryViewModel {
    HistoryTabCoordinator *delegate;
    HistoryGradientChartViewModel *gradientChartViewModel;
    HistoryAreaChartViewModel *areaChartViewModel;
    HistoryLiftChartViewModel *liftChartViewModel;
    Array_weekData *data;
};

HistoryViewModel *historyViewModel_init(void);
void historyViewModel_free(HistoryViewModel *model);
void historyViewModel_fetchData(HistoryViewModel *model);
void historyViewModel_formatDataForTimeRange(HistoryViewModel *model, int index);

unsigned char historyViewModel_shouldShowCharts(HistoryViewModel *model);
void historyViewModel_applyUpdatesForTotalWorkouts(HistoryGradientChartViewModel *model, LineChartView *view, LineChartData *data, LineChartDataSet *dataSet, ChartLimitLine *limitLine, NSArray<ChartLegendEntry*> *legendEntries);
void historyViewModel_applyUpdatesForDurations(HistoryAreaChartViewModel *model, LineChartView *view, LineChartData *data, LineChartDataSet **dataSets, NSArray<ChartLegendEntry*> *legendEntries);
void historyViewModel_applyUpdatesForLifts(HistoryLiftChartViewModel *model, LineChartView *view, LineChartData *data, LineChartDataSet **dataSets, NSArray<ChartLegendEntry*> *legendEntries);

#endif /* HistoryViewModel_h */
