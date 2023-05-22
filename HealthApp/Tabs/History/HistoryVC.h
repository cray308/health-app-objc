#ifndef HistoryVC_h
#define HistoryVC_h

#include <CoreFoundation/CoreFoundation.h>
#include <objc/objc.h>
#include "AppDelegate.h"
#include "ColorCache.h"
#include "ViewCache.h"

extern Class HistoryVCClass;

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
    CCacheRef clr;
    VCacheRef tbl;
    HistoryViewModel model;
    id picker;
    id charts[3];
} HistoryVC;

id historyVC_init(void **model, FetchHandler *handler, VCacheRef tbl, CCacheRef clr);
void historyVC_updateSegment(id self, SEL _cmd, id picker);
void historyVC_viewDidLoad(id self, SEL _cmd);
CFStringRef historyVC_stringForValue(id self, SEL _cmd, double value);

void historyVC_clearData(id self);
void historyVC_updateColors(id self, unsigned char darkMode);

#endif /* HistoryVC_h */
