//
//  HomePieChartViewModel.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef HomePieChartViewModel_h
#define HomePieChartViewModel_h

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#import "HealthApp-Swift.h"
#pragma clang diagnostic pop

typedef struct HomePieChartViewModel HomePieChartViewModel;

struct HomePieChartViewModel {
    int totalMinutes;
    int activityTypes[3];
    PieChartDataEntry *entries[3];
    NSString *legendLabelFormats[3];
};

void homePieChartViewModel_formatEntries(HomePieChartViewModel *model);
void homePieChartViewModel_updateLegend(HomePieChartViewModel *model, NSArray<ChartLegendEntry*> *entries);

#endif /* HomePieChartViewModel_h */
