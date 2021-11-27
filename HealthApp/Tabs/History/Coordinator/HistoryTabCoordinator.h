#ifndef HistoryTabCoordinator_h
#define HistoryTabCoordinator_h

#include <CoreGraphics/CGGeometry.h>
#include <objc/runtime.h>
#include "array.h"

gen_array_headers(pt, CGPoint)

struct HistTimeData {
    short year;
    short month;
    short day;
};

typedef struct {
    Array_pt *entries;
    CFStringRef legendFormat;
    id dataSet;
    id chartData;
    CGPoint *dataArrays[3];
    float avgs[3];
    float maxes[3];
} TotalWorkoutsChartModel;

typedef struct {
    Array_pt *entries[5];
    CFStringRef legendFormat;
    id dataSets[5];
    id chartData;
    CGPoint *dataArrays[3][5];
    int avgs[3][4];
    float maxes[3];
    CFStringRef names[4];
} WorkoutTypeChartModel;

typedef struct {
    Array_pt *entries[4];
    CFStringRef legendFormat;
    id dataSets[4];
    id chartData;
    CGPoint *dataArrays[3][4];
    float avgs[3][4];
    float maxes[3];
    CFStringRef names[4];
} LiftChartModel;

typedef struct {
    TotalWorkoutsChartModel totalWorkouts;
    WorkoutTypeChartModel workoutTypes;
    LiftChartModel lifts;
    struct HistFormatter {
        CFStringRef months[12];
        CFStringRef currString;
        struct HistTimeData data[128];
    } formatter;
    int nEntries[3];
} HistoryViewModel;

typedef struct {
    id navVC;
    HistoryViewModel model;
} HistoryTabCoordinator;

void historyCoordinator_start(HistoryTabCoordinator *this);
void historyCoordinator_clearData(HistoryTabCoordinator *this, bool callVC);

#endif /* HistoryTabCoordinator_h */
