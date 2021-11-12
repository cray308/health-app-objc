//
//  HistoryTabCoordinator.c
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#include "HistoryTabCoordinator.h"
#include "ViewControllerHelpers.h"
#include "AppUserData.h"

extern id historyVC_init(void *delegate);
extern void historyVC_refresh(id vc);
extern void historyVC_refreshUI(id);
extern void toggleDarkModeForCharts(bool);
extern void updateLegendColor(id entry, id color);

void historyCoordinator_start(HistoryTabCoordinator *this) {
    toggleDarkModeForCharts(userData->darkMode);
    historyViewModel_init(&this->model);
    setupNavVC(this->navVC, historyVC_init(this));
}

void historyCoordinator_fetchData(HistoryTabCoordinator *this) {
    historyViewModel_fetchData(&this->model);
}

void historyCoordinator_reloadUI(HistoryTabCoordinator *this, bool callVC) {
    updateNavBar(this->navVC);
    toggleDarkModeForCharts(userData->darkMode);
    updateLegendColor(this->model.totalWorkoutsModel.legendEntries[0], getLimitLineColor());
    if (callVC) {
        id vc = getFirstVC(this->navVC);
        setBackground(getView(vc), createColor(ColorSystemBackground));
        historyVC_refreshUI(vc);
    }
}

void historyCoordinator_updateUI(HistoryTabCoordinator *this, bool callVC) {
    array_clear(weekData, this->model.data);
    if (callVC)
        historyVC_refresh(getFirstVC(this->navVC));
}
