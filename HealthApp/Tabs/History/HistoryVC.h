#ifndef HistoryVC_h
#define HistoryVC_h

#include <CoreFoundation/CFArray.h>
#include <CoreGraphics/CGGeometry.h>
#include <objc/runtime.h>

extern Class HistoryVCClass;
extern Ivar HistoryVCDataRef;

typedef struct {
    CGPoint *entries;
    CFStringRef legendFormat;
    id dataSet;
    id chartData;
    float avgs[3];
    float maxes[3];
} TotalWorkoutsChartModel;

typedef struct {
    CGPoint *entries[5];
    CFStringRef legendFormat;
    id dataSets[5];
    id chartData;
    int avgs[3][4];
    float maxes[3];
    CFStringRef names[4];
} WorkoutTypeChartModel;

typedef struct {
    CGPoint *entries[4];
    CFStringRef legendFormat;
    id dataSets[4];
    id chartData;
    float avgs[3][4];
    float maxes[3];
    CFStringRef names[4];
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
} HistoryVCData;

id historyVC_init(void **model, void (**handler)(void*));
void historyVC_clearData(id self);
void historyVC_updateColors(id vc);
void historyVC_viewDidLoad(id self, SEL _cmd);
void historyVC_updateSegment(id self, SEL _cmd, id picker);
CFStringRef historyVC_stringForValue(id self, SEL _cmd, double value);

#endif /* HistoryVC_h */
