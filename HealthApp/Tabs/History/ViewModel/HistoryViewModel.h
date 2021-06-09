//
//  HistoryViewModel.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef HistoryViewModel_h
#define HistoryViewModel_h

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#import "HealthApp-Swift.h"
#pragma clang diagnostic pop
#include "array.h"

#define freeChartDataEntry(x) [(x) release]
gen_array(chartData, ChartDataEntry*, DSDefault_shallowCopy, freeChartDataEntry)

typedef struct HistoryViewModel HistoryViewModel;
typedef struct HistoryTabCoordinator HistoryTabCoordinator;
typedef struct HistoryGradientChartViewModel HistoryGradientChartViewModel;
typedef struct HistoryAreaChartViewModel HistoryAreaChartViewModel;
typedef struct HistoryPieChartViewModel HistoryPieChartViewModel;

struct HistoryViewModel {
    HistoryTabCoordinator *delegate;
    HistoryGradientChartViewModel *gradientChartViewModel;
    HistoryAreaChartViewModel *areaChartViewModel;
    HistoryPieChartViewModel *pieChartViewModel;
};

void historyViewModel_clear(HistoryViewModel *model);
void historyViewModel_fetchData(HistoryViewModel *model, int index);

#endif /* HistoryViewModel_h */
