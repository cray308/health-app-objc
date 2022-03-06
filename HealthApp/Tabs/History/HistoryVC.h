#ifndef HistoryVC_h
#define HistoryVC_h

#include <CoreFoundation/CFArray.h>
#include <CoreGraphics/CGGeometry.h>
#include "AppDelegate.h"

typedef struct {
    CGPoint *entries;
    id dataSet;
    id chartData;
    float avgs[3];
    float maxes[3];
} TotalWorkoutsChartModel;

typedef struct {
    CGPoint *entries[5];
    id dataSets[5];
    id chartData;
    int avgs[3][4];
    float maxes[3];
} WorkoutTypeChartModel;

typedef struct {
    CGPoint *entries[4];
    id dataSets[4];
    id chartData;
    float avgs[3][4];
    float maxes[3];
} LiftChartModel;

typedef struct {
    TotalWorkoutsChartModel totalWorkouts;
    WorkoutTypeChartModel workoutTypes;
    LiftChartModel lifts;
    CFArrayRef axisStrings;
    int nEntries[3];
    int refIndices[3];
} HistoryViewModel;

typedef struct {
    HistoryViewModel model;
    id picker;
    id charts[3];
} HistoryVC;

#endif /* HistoryVC_h */
