//
//  HistoryTabCoordinator.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef HistoryTabCoordinator_h
#define HistoryTabCoordinator_h

#import <UIKit/UIKit.h>
#include "HistoryViewModel.h"

typedef struct {
    UINavigationController *navigationController;
    HistoryViewModel viewModel;
} HistoryTabCoordinator;

void historyCoordinator_free(HistoryTabCoordinator *this);
void historyCoordinator_start(HistoryTabCoordinator *this);
void historyCoordinator_updateUI(HistoryTabCoordinator *this);

#endif /* HistoryTabCoordinator_h */
