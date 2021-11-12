#import "LiftingChart.h"
#import "SwiftBridging.h"
@implementation LiftingChart @end

id liftingChart_init(LiftChartViewModel *model, id formatter) {
    LiftingChart *this = [[LiftingChart alloc] initWithFrame:CGRectZero];
    this->model = model;
    for (int i = 0; i < 4; ++i)
        ((LineChartDataSet *) model->dataSets[i]).lineWidth = 3;
    this->chartView = createChartView(this, formatter, model->legendEntries, 4, 550);
    return this;
}

void liftingChart_update(LiftingChart *this, int count, bool isSmall) {
    for (int i = 0; i < 4; ++i) {
        updateDataSet(isSmall, count, this->model->dataSets[i], this->model->entries[i]->arr);
    }
    updateChart(isSmall, this->chartView, this->model->chartData, this->model->yMax);
}
