//
//  HistoryAreaChartViewModel.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "HistoryAreaChartViewModel.h"
#import "StringHelpers.h"

void historyAreaChartViewModel_updateLegend(HistoryAreaChartViewModel *model, NSArray<ChartLegendEntry*> *entries) {
    double entryCount = array_size(model->entries[0]);
    for (int i = 0; i < 3; ++i) {
        double average = (double) model->totalByIntensity[i] / entryCount;
        NSString *suffix = durationStringCreate_fromNumber(average);
        entries[i].label = [NSString stringWithFormat:model->legendLabelPrefixes[i], suffix];
        [suffix release];
    }
}
