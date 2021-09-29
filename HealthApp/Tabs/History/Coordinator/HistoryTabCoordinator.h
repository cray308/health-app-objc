//
//  HistoryTabCoordinator.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef HistoryTabCoordinator_h
#define HistoryTabCoordinator_h

#include "CocoaHelpers.h"
#include "HistoryViewModel.h"

typedef struct {
    id navVC;
    HistoryViewModel model;
} HistoryTabCoordinator;

void historyCoordinator_start(HistoryTabCoordinator *this);
void historyCoordinator_fetchData(HistoryTabCoordinator *this);
void historyCoordinator_updateUI(HistoryTabCoordinator *this, bool callVC);

#endif /* HistoryTabCoordinator_h */
