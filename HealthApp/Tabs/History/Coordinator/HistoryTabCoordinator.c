//
//  HistoryTabCoordinator.c
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#include "HistoryTabCoordinator.h"

void historyCoordinator_start(HistoryTabCoordinator *this) {
    historyViewModel_init(&this->viewModel);
    setupNavVC(this->navVC, createVCWithDelegate("HistoryViewController", this));
}

void historyCoordinator_updateUI(HistoryTabCoordinator *this) {
    historyViewModel_fetchData(&this->viewModel);
    objc_singleArg(getFirstVC(this->navVC), sel_getUid("performForegroundUpdate"));
}
