#ifndef HistoryModel_h
#define HistoryModel_h

#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <objc/objc.h>

enum {
    ChartTW,
    ChartWT,
    ChartLifts
};

typedef struct {
    short totalWorkouts;
    short durationByType[4];
    short cumulativeDuration[4];
    short weightArray[4];
} WeekDataModel;

typedef struct {
    CGPoint *entries;
    id dataSet;
    float avgs[3];
} TotalWorkoutsChartModel;

typedef struct {
    CGPoint *entries[5];
    id dataSets[5];
    int avgs[3][4];
} WorkoutTypeChartModel;

typedef struct {
    CGPoint *entries[4];
    id dataSets[4];
    float avgs[3][4];
} LiftChartModel;

typedef struct {
    TotalWorkoutsChartModel tw;
    WorkoutTypeChartModel wt;
    LiftChartModel lifts;
    CFArrayRef axisStrings;
    id data[3];
    float maxes[3][3];
    int nEntries[3];
    int refIndices[3];
} HistoryModel;

void historyModel_populate(HistoryModel *m,
                           CFArrayRef strs, WeekDataModel *results, int size, bool ltr);

#endif /* HistoryModel_h */
