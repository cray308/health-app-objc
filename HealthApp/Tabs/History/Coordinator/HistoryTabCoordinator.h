#ifndef HistoryTabCoordinator_h
#define HistoryTabCoordinator_h

#include <CoreFoundation/CFBase.h>
#include <objc/runtime.h>
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
} HistoryViewModel;

typedef struct {
    id navVC;
    HistoryViewModel model;
} HistoryTabCoordinator;

void historyCoordinator_start(HistoryTabCoordinator *this);

#endif /* HistoryTabCoordinator_h */
