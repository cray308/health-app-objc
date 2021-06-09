//
//  HomeBarChartViewModel.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#include "ds.h"
#import "HomeBarChartViewModel.h"
#import "CalendarDateHelpers.h"
#import "AppUserData.h"

void homeBarChartViewModel_updateLeftAxisData(HomeBarChartViewModel *model, ChartLimitLine *limitLine, double *axisMax) {
    int tokenGoal = 6; // change to number of workouts this week, only check if it's not 0
    limitLine.limit = tokenGoal;
    *axisMax = max(1.25 * model->totalTokens, 1.25 * tokenGoal);
}
