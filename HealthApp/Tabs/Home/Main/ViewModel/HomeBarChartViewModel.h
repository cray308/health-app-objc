//
//  HomeBarChartViewModel.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef HomeBarChartViewModel_h
#define HomeBarChartViewModel_h

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#import "HealthApp-Swift.h"
#pragma clang diagnostic pop

typedef struct HomeBarChartViewModel HomeBarChartViewModel;

struct HomeBarChartViewModel {
    int totalTokens;
    BarChartDataEntry *cumulativeArray[7];
};

void homeBarChartViewModel_updateLeftAxisData(HomeBarChartViewModel *model, ChartLimitLine *limitLine, double *axisMax);

#endif /* HomeBarChartViewModel_h */
