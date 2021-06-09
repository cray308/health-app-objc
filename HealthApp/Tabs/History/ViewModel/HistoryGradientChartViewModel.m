//
//  HistoryGradientChartViewModel.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "HistoryGradientChartViewModel.h"
#import "AppUserData.h"

void historyGradientChartViewModel_updateLeftAxisData(HistoryGradientChartViewModel *model, ChartLimitLine **limitLines,
                                                      NSArray<ChartLegendEntry *> *legendEntries, double *axisMax) {
    *axisMax = max(1.1 * model->maxTokens, 6.6);
    limitLines[0].limit = model->avgTokens;
    limitLines[1].limit = 6;
    legendEntries[0].label = [NSString stringWithFormat:model->legendLabelFormats[0], model->avgTokens];
    legendEntries[1].label = [NSString stringWithFormat:model->legendLabelFormats[1], 6.0];
}
