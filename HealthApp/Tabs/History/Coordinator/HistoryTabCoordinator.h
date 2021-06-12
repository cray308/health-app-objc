//
//  HistoryTabCoordinator.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef HistoryTabCoordinator_h
#define HistoryTabCoordinator_h

#import <UIKit/UIKit.h>

typedef struct AppCoordinator AppCoordinator;
typedef struct HistoryTabCoordinator HistoryTabCoordinator;
typedef struct HistoryViewModel HistoryViewModel;
@class HistoryViewController;

struct HistoryTabCoordinator {
    UINavigationController *navigationController;
    AppCoordinator *delegate;
    HistoryViewModel *viewModel;
};

HistoryTabCoordinator *historyCoordinator_init(UINavigationController *navVC, AppCoordinator *delegate);
void historyCoordinator_free(HistoryTabCoordinator *coordinator);
void historyCoordinator_start(HistoryTabCoordinator *coordinator);

void historyCoordinator_performForegroundUpdate(HistoryTabCoordinator *coordinator);
void historyCoordinator_handleDataDeletion(HistoryTabCoordinator *coordinator);

#endif /* HistoryTabCoordinator_h */
