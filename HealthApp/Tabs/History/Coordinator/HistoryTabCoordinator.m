//
//  HistoryTabCoordinator.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "HistoryTabCoordinator.h"
#import "AppCoordinator.h"
#import "HistoryViewModel.h"
#import "HistoryViewController.h"
#import "HistoryAreaChartViewModel.h"
#import "HistoryPieChartViewModel.h"
#import "HistoryGradientChartViewModel.h"
#import "ActivityType.h"
#import "CalendarDateHelpers.h"

#define UpdateRange 0x1
#define UpdateCharts 0x2

static inline HistoryViewController *getHistoryViewController(UINavigationController *controller) {
    return (HistoryViewController *) controller.viewControllers[0];
}

HistoryViewModel *historyViewModel_init(HistoryTabCoordinator *delegate);
void historyViewModel_free(HistoryViewModel *model);
void historyCoordinator_updateUI(HistoryTabCoordinator *coordinator, unsigned char update, bool resetViewModel);

HistoryTabCoordinator *historyCoordinator_init(UINavigationController *navVC, AppCoordinator *delegate) {
    HistoryTabCoordinator *coordinator = calloc(1, sizeof(HistoryTabCoordinator));
    if (!coordinator) return NULL;
    coordinator->viewModel = historyViewModel_init(coordinator);
    if (!coordinator->viewModel) {
        free(coordinator);
        return NULL;
    }
    coordinator->delegate = delegate;
    coordinator->navigationController = navVC;
    return coordinator;
}

void historyCoordinator_free(HistoryTabCoordinator *coordinator) {
    historyViewModel_free(coordinator->viewModel);
    free(coordinator);
}

void historyCoordinator_start(HistoryTabCoordinator *coordinator) {
    HistoryViewController *vc = [[HistoryViewController alloc] initWithViewModel:coordinator->viewModel];
    [coordinator->navigationController setViewControllers:@[vc]];
    [vc release];
}

void historyCoordinator_updateNavBarTokens(HistoryTabCoordinator *coordinator, NSString *label) {
    HistoryViewController *vc = getHistoryViewController(coordinator->navigationController);
    [vc updateNavBarCoins:label];
}

void historyCoordinator_performForegroundUpdate(HistoryTabCoordinator *coordinator) {
    historyCoordinator_updateUI(coordinator, UpdateRange, false);
}

void historyCoordinator_handleUserInfoChange(HistoryTabCoordinator *coordinator) {
    historyCoordinator_updateUI(coordinator, UpdateCharts, false);
}

void historyCoordinator_handleDataDeletion(HistoryTabCoordinator *coordinator) {
    historyCoordinator_updateUI(coordinator, UpdateCharts, true);
}

#pragma mark - Helpers

void historyCoordinator_updateUI(HistoryTabCoordinator *coordinator, unsigned char update, bool resetViewModel) {
    HistoryViewController *vc = getHistoryViewController(coordinator->navigationController);
    if (resetViewModel) historyViewModel_clear(coordinator->viewModel);
    if (update == UpdateRange) {
        [vc performForegroundUpdate];
    } else {
        [vc updateChartsWithAnimation:false];
    }
}

HistoryViewModel *historyViewModel_init(HistoryTabCoordinator *delegate) {
    HistoryViewModel *model = malloc(sizeof(HistoryViewModel));
    if (!model) return NULL;
    model->areaChartViewModel = calloc(1, sizeof(HistoryAreaChartViewModel));
    model->pieChartViewModel = calloc(1, sizeof(HistoryPieChartViewModel));
    model->gradientChartViewModel = calloc(1, sizeof(HistoryGradientChartViewModel));
    if (!(model->areaChartViewModel && model->gradientChartViewModel && model->pieChartViewModel)) {
        if (model->areaChartViewModel) free(model->areaChartViewModel);
        if (model->gradientChartViewModel) free(model->gradientChartViewModel);
        if (model->pieChartViewModel) free(model->pieChartViewModel);
        free(model);
        return NULL;
    }

    {
        HistoryAreaChartViewModel *vm = model->areaChartViewModel;
        for (int i = 0; i < 4; ++i) {
            vm->entries[i] = array_new(chartData);
        }
        for (int i = 0; i < 3; ++i) {
            NSString *suffix = activityType_getString(i, false);
            vm->legendLabelPrefixes[i] = [[NSString alloc] initWithFormat:@"%@ (Avg: %%@)", suffix];
            [suffix release];
        }
    }
    {
        HistoryPieChartViewModel *vm = model->pieChartViewModel;
        NSString **dayLabels = calendar_getWeekDaySymbols(false);
        for (int i = 0; i < 7; ++i) {
            vm->entries[i] = [[PieChartDataEntry alloc] initWithValue:0];
            vm->legendLabelFormats[i] = [[NSString alloc] initWithFormat:@"%@ (Avg: %%@)", dayLabels[i]];
            [dayLabels[i] release];
        }
        free(dayLabels);
    }
    {
        HistoryGradientChartViewModel *vm = model->gradientChartViewModel;
        vm->legendLabelFormats[0] = [[NSString alloc] initWithFormat:@"Average Tokens (%%.2f)"];
        vm->legendLabelFormats[1] = [[NSString alloc] initWithFormat:@"Token Goal (%%d)"];
        vm->entries = array_new(chartData);
    }

    model->delegate = delegate;
    return model;
}

void historyViewModel_free(HistoryViewModel *model) {
    {
        HistoryAreaChartViewModel *vm = model->areaChartViewModel;
        for (int i = 0; i < 3; ++i) { [vm->legendLabelPrefixes[i] release]; }
        for (int i = 0; i < 4; ++i) {  array_free(chartData, vm->entries[i]); }
        free(vm);
    }
    {
        HistoryPieChartViewModel *vm = model->pieChartViewModel;
        for (int i = 0; i < 7; ++i) {
            [vm->entries[i] release];
            [vm->legendLabelFormats[i] release];
        }
        free(vm);
    }
    {
        HistoryGradientChartViewModel *vm = model->gradientChartViewModel;
        for (int i = 0; i < 2; ++i) { [vm->legendLabelFormats[i] release]; }
        array_free(chartData, vm->entries);
        free(vm);
    }
    free(model);
}
