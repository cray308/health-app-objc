#import "SwiftBridging.h"
#import "TotalWorkoutsChart.h"
#import "WorkoutTypeChart.h"
#import "LiftingChart.h"
#include "AppCoordinator.h"
#include "HistoryTabCoordinator.h"

@interface HistoryViewController: UIViewController<AxisValueFormatter> @end
@interface HistoryViewController() {
    @public HistoryViewModel *model;
    @public UISegmentedControl *picker;
    Container containers[3];
    TotalWorkoutsChart *totalWorkoutsChart;
    WorkoutTypeChart *workoutTypeChart;
    LiftingChart *liftChart;
}
@end
@implementation HistoryViewController
- (void) viewDidLoad {
    [super viewDidLoad];
    setBackground(self.view, UIColor.systemBackgroundColor);
    self.navigationItem.title = _nsstr(localize(CFSTR("titles1")));

    CFStringRef titles[3]; fillStringArray(titles, CFSTR("chartHeader%d"), 3);
    totalWorkoutsChart = totalWorkoutsChart_init(&model->totalWorkoutsModel, self);
    workoutTypeChart = workoutTypeChart_init(&model->workoutTypeModel, self);
    liftChart = liftingChart_init(&model->liftModel, self);
    id charts[] = {totalWorkoutsChart, workoutTypeChart, liftChart};
    UIStackView *vStack = createStackView(nil, 0, 1, 5, (Padding){10, 8, 10, 8});

    picker = createSegmentedControl(CFSTR("historySegment%d"),3,0,self,@selector(updateSegment:),-1);
    self.navigationItem.titleView = picker;

    for (int i = 0; i < 3; ++i) {
        [vStack addArrangedSubview:createContainer(&containers[i], titles[i], !i ? 1 : 0, 0, 0)];
        container_add(&containers[i], charts[i]);
    }

    UIScrollView *scrollView = createScrollView();

    [self.view addSubview:scrollView];
    pin(scrollView, self.view.safeAreaLayoutGuide, (Padding){0}, 0);
    addVStackToScrollView(vStack, scrollView);

    [vStack release];
    [scrollView release];

    [self updateSegment:picker];
    appCoordinator->loadedViewControllers |= LoadedViewController_History;
}

- (void) updateSegment: (UISegmentedControl *)sender {
    historyViewModel_formatDataForTimeRange(model, (int) sender.selectedSegmentIndex);
    [self updateCharts];
}

- (void) updateCharts {
    if (!model->data->size) {
        disableLineChartView(totalWorkoutsChart->chartView);
        disableLineChartView(workoutTypeChart->chartView);
        disableLineChartView(liftChart->chartView);
        return;
    }

    const int count = model->totalWorkoutsModel.entries->size;
    totalWorkoutsChart_update(totalWorkoutsChart, count, model->isSmall);
    workoutTypeChart_update(workoutTypeChart, count, model->isSmall);
    liftingChart_update(liftChart, count, model->isSmall);
}

- (NSString *) stringForValue: (double)value axis: (AxisBase *)axis {
    return _nsstr(historyViewModel_getXAxisLabel(model, (int) value));
}
@end

id historyVC_init(HistoryTabCoordinator *delegate) {
    HistoryViewController *this = [[HistoryViewController alloc] initWithNibName:nil bundle:nil];
    this->model = &delegate->model;
    return this;
}

void historyVC_refresh(HistoryViewController *vc) {
    vc->picker.selectedSegmentIndex = 0;
    [vc updateSegment:vc->picker];
}
