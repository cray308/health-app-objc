//
//  HistoryTabCoordinator.c
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#include "HistoryTabCoordinator.h"
#include "ViewControllerHelpers.h"

void historyCoordinator_start(HistoryTabCoordinator *this) {
    historyViewModel_init(&this->viewModel);
    setupNavVC(this->navVC, createVCWithDelegate("HistoryViewController", this));
}

void historyCoordinator_fetchData(HistoryTabCoordinator *this) {
    historyViewModel_fetchData(&this->viewModel);
}

void historyCoordinator_updateUI(HistoryTabCoordinator *this, bool callVC) {
    array_clear(weekData, this->viewModel.data);
    if (callVC)
        objc_singleArg(getFirstVC(this->navVC), sel_getUid("performForegroundUpdate"));
}
