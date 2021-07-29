//
//  HistoryDataManager.m
//  HealthApp
//
//  Created by Christopher Ray on 7/25/21.
//

#import <Foundation/Foundation.h>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#import "HealthApp-Swift.h"
#pragma clang diagnostic pop
#include "HistoryDataManager.h"
#include "AppUserData.h"
#include "CalendarDateHelpers.h"
#include "PersistenceService.h"

void freeChartDataEntry(void *entry) {
    [(id)entry release];
}

void historyDataManager_fetchData(HistoryViewModel *model) {
    array_clear(weekData, model->data);
    int count = 0;
    NSPredicate *pred = [NSPredicate predicateWithFormat:@"weekStart > %lld AND weekStart < %lld", date_twoYears,
                         appUserDataShared->weekStart];
    NSSortDescriptor *descriptor = [[NSSortDescriptor alloc] initWithKey:@"weekStart" ascending:true];
    NSArray<WeeklyData *> *data = persistenceService_executeFetchRequest(WeeklyData.fetchRequest, pred, descriptor,
                                                                         &count);
    [descriptor release];
    if (!data) return;

    for (int i = 0; i < count; ++i) {
        WeeklyData *d = data[i];
        HistoryWeekDataModel m = {.weekStart = d.weekStart, .totalWorkouts = d.totalWorkouts,
            .weightArray = {d.bestSquat, d.bestPullup, d.bestBench, d.bestDeadlift},
            .durationByType = {d.timeStrength, d.timeHIC, d.timeSE, d.timeEndurance},
            .cumulativeDuration = {[0] = d.timeStrength}
        };

        for (int j = 1; j < 4; ++j) {
            m.cumulativeDuration[j] = m.cumulativeDuration[j - 1] + m.durationByType[j];
        }
        array_push_back(weekData, model->data, m);
    }
}

void historyDataManager_createNewEntry(Array_chartData *arr, long x, int y) {
    array_push_back(chartData, arr, [[ChartDataEntry alloc] initWithX:x y:y]);
}
