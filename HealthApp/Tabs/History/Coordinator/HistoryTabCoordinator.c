//
//  HistoryTabCoordinator.c
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#include "HistoryTabCoordinator.h"

void historyCoordinator_start(HistoryTabCoordinator *this) {
    historyViewModel_init(&this->viewModel);
    id vc = ((id (*)(id, SEL, HistoryTabCoordinator *)) objc_msgSend)
        (objc_staticMethod(objc_getClass("HistoryViewController"), sel_getUid("alloc")),
         sel_getUid("initWithDelegate:"), this);
    id array = ((id (*)(Class, SEL, id, ...)) objc_msgSend)
        (objc_getClass("NSArray"), sel_getUid("arrayWithObjects:"), vc, nil);
    ((void (*)(id, SEL, id)) objc_msgSend)(this->navVC, sel_getUid("setViewControllers:"), array);
    objc_singleArg(vc, sel_getUid("release"));
}

void historyCoordinator_updateUI(HistoryTabCoordinator *this) {
    id ctrls = ((id (*)(id, SEL)) objc_msgSend)(this->navVC, sel_getUid("viewControllers"));
    id vc = ((id (*)(id, SEL, int)) objc_msgSend)(ctrls, sel_getUid("objectAtIndex:"), 0);
    historyViewModel_fetchData(&this->viewModel);
    objc_singleArg(vc, sel_getUid("performForegroundUpdate"));
}
