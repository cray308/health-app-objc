//
//  HistoryTabCoordinator.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "HistoryTabCoordinator.h"
#include "HistoryDataManager.h"
#import "HistoryViewController.h"

void historyCoordinator_free(HistoryTabCoordinator *this) {
    historyViewModel_free(&this->viewModel);
    free(this);
}

void historyCoordinator_start(HistoryTabCoordinator *this) {
    historyViewModel_init(&this->viewModel);
    UIViewController *vc = [[HistoryViewController alloc] initWithDelegate:this];
    [this->navigationController setViewControllers:@[vc]];
    [vc release];
}

void historyCoordinator_updateUI(HistoryTabCoordinator *this) {
    HistoryViewController *vc = (HistoryViewController *) this->navigationController.viewControllers[0];
    historyDataManager_fetchData(&this->viewModel);
    [vc performForegroundUpdate];
}
