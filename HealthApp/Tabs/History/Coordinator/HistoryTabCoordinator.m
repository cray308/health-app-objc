//
//  HistoryTabCoordinator.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "HistoryTabCoordinator.h"
#import "AppCoordinator.h"
#import "ChartHelpers.h"
#import "HistoryViewModel.h"
#import "HistoryViewController.h"

static inline HistoryViewController *getHistoryViewController(UINavigationController *controller) {
    return (HistoryViewController *) controller.viewControllers[0];
}

void historyCoordinator_updateUI(HistoryTabCoordinator *coordinator);

HistoryTabCoordinator *historyCoordinator_init(UINavigationController *navVC, AppCoordinator *delegate) {
    HistoryTabCoordinator *coordinator = calloc(1, sizeof(HistoryTabCoordinator));
    if (!coordinator) return NULL;
    sharedHistoryXAxisFormatter_setup();
    coordinator->viewModel = historyViewModel_init();
    if (!coordinator->viewModel) {
        free(coordinator);
        return NULL;
    }
    coordinator->viewModel->delegate = coordinator;
    coordinator->delegate = delegate;
    coordinator->navigationController = navVC;
    return coordinator;
}

void historyCoordinator_free(HistoryTabCoordinator *coordinator) {
    historyViewModel_free(coordinator->viewModel);
    sharedHistoryXAxisFormatter_free();
    free(coordinator);
}

void historyCoordinator_start(HistoryTabCoordinator *coordinator) {
    HistoryViewController *vc = [[HistoryViewController alloc] initWithViewModel:coordinator->viewModel];
    [coordinator->navigationController setViewControllers:@[vc]];
    [vc release];
}

void historyCoordinator_performForegroundUpdate(HistoryTabCoordinator *coordinator) {
    historyCoordinator_updateUI(coordinator);
}

void historyCoordinator_handleDataDeletion(HistoryTabCoordinator *coordinator) {
    historyCoordinator_updateUI(coordinator);
}

#pragma mark - Helpers

void historyCoordinator_updateUI(HistoryTabCoordinator *coordinator) {
    HistoryViewController *vc = getHistoryViewController(coordinator->navigationController);
    historyViewModel_fetchData(coordinator->viewModel);
    [vc performForegroundUpdate];
}
