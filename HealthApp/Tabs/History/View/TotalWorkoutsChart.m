#import "TotalWorkoutsChart.h"
#import "SwiftBridging.h"
@implementation TotalWorkoutsChart @end

id totalWorkoutsChart_init(TotalWorkoutsChartViewModel *model, id formatter) {
    TotalWorkoutsChart *this = [[TotalWorkoutsChart alloc] initWithFrame:CGRectZero];
    this->model = model;

    this->limitLine = [[ChartLimitLine alloc] initWithLimit:0];
    this->limitLine.lineColor = ((LegendEntry*) model->legendEntries[0]).formColor;

    LineChartDataSet *dataSet = model->dataSet;
    NSObject<Fill> *fill = [[LinearGradientFill alloc] initWithEndColor:UIColor.systemRedColor];
    dataSet.fill = fill;
    dataSet.drawFilledEnabled = true;
    dataSet.fillAlpha = 0.75;

    this->chartView = createChartView(this, formatter, model->legendEntries, 1, 390);
    [this->chartView.leftAxis addLimitLine:this->limitLine];
    [fill release];
    return this;
}

void totalWorkoutsChart_update(TotalWorkoutsChart *this, int count, bool isSmall) {
    this->limitLine.limit = this->model->avgWorkouts;
    updateDataSet(isSmall, count, this->model->dataSet, this->model->entries->arr);
    updateChart(isSmall, this->chartView, this->model->chartData, this->model->yMax);
    NSNumberFormatter *nFormatter = [[NSNumberFormatter alloc] init];
    DefaultValueFormatter *formatter = [[DefaultValueFormatter alloc] initWithFormatter:nFormatter];
    [((LineChartData *) this->model->chartData) setValueFormatter:formatter];
    [formatter release];
    [nFormatter release];
}
