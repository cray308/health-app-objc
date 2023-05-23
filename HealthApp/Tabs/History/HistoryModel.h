#ifndef HistoryModel_h
#define HistoryModel_h

#include <CoreFoundation/CoreFoundation.h>
#include <objc/objc.h>

enum {
    ChartTotals,
    ChartTypes,
    ChartLifts
};

typedef struct {
    int totalWorkouts;
    int durationByType[4];
    int cumulativeDuration[4];
    int weights[4];
} WeeklyData;

typedef struct {
    struct {
        CGPoint *entries;
        id set;
        float avgs[3];
    } totals;
    struct {
        CGPoint *entries[5];
        id sets[5];
        int avgs[3][4];
    } types;
    struct {
        CGPoint *entries[4];
        id sets[4];
        float avgs[3][4];
    } lifts;
    CFArrayRef axisStrs;
    id data[3];
    float maxes[3][3];
    int nEntries[3];
    int refIndices[3];
} HistoryModel;

void historyModel_populate(HistoryModel *m, CFMutableArrayRef axisStrs, WeeklyData *weeks, int size);

#endif /* HistoryModel_h */
