//
//  HistoryPieChartViewModel.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "HistoryPieChartViewModel.h"
#import "StringHelpers.h"

void historyPieChartViewModel_updateLegend(HistoryPieChartViewModel *model, NSArray<ChartLegendEntry*> *legendEntries) {
    double entryCount = model->entryCount;
    for (int i = 0; i < 7; ++i) {
        double avg = (double) model->totalDailyActivity[i] / entryCount;
        NSString *durationStr = durationStringCreate_fromNumber(avg);
        legendEntries[i].label = [NSString stringWithFormat:model->legendLabelFormats[i], durationStr];
        [durationStr release];
    }
}
