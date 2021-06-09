//
//  HomePieChartViewModel.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "HomePieChartViewModel.h"
#import "StringHelpers.h"

void homePieChartViewModel_formatEntries(HomePieChartViewModel *model) {
    if (!model->totalMinutes) return;
    for (int i = 0; i < 3; ++i) {
        model->entries[i].value = (double) model->activityTypes[i] / model->totalMinutes;
    }
}

void homePieChartViewModel_updateLegend(HomePieChartViewModel *model, NSArray<ChartLegendEntry*> *entries) {
    for (int i = 0; i < 3; ++i) {
        NSString *suffix = durationStringCreate_fromNumber(model->activityTypes[i]);
        entries[i].label = [NSString stringWithFormat:model->legendLabelFormats[i], suffix];
        [suffix release];
    }
}
