//
//  HistoryTabCoordinator.c
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#include "HistoryTabCoordinator.h"
#include "ViewControllerHelpers.h"

extern id historyVC_init(void *delegate);
extern void historyVC_refresh(id vc);

void historyCoordinator_start(HistoryTabCoordinator *this) {
    historyViewModel_init(&this->model);
    setupNavVC(this->navVC, historyVC_init(this));
}

void historyCoordinator_fetchData(HistoryTabCoordinator *this) {
    historyViewModel_fetchData(&this->model);
}

void historyCoordinator_updateUI(HistoryTabCoordinator *this, bool callVC) {
    array_clear(weekData, this->model.data);
    if (callVC)
        historyVC_refresh(getFirstVC(this->navVC));
}
