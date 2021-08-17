//
//  SettingsTabCoordinator.c
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#include "SettingsTabCoordinator.h"

void settingsCoordinator_start(SettingsTabCoordinator *this) {
    id vc = ((id (*)(id, SEL)) objc_msgSend)
        (objc_staticMethod(objc_getClass("SettingsViewController"), sel_getUid("alloc")),
         sel_getUid("init"));
    id array = ((id (*)(Class, SEL, id, ...)) objc_msgSend)
        (objc_getClass("NSArray"), sel_getUid("arrayWithObjects:"), vc, nil);
    ((void (*)(id, SEL, id)) objc_msgSend)(this->navVC, sel_getUid("setViewControllers:"), array);
    objc_singleArg(vc, sel_getUid("release"));
}

void settingsCoordinator_updateWeightText(SettingsTabCoordinator *this) {
    id ctrls = ((id (*)(id, SEL)) objc_msgSend)(this->navVC, sel_getUid("viewControllers"));
    id vc = ((id (*)(id, SEL, int)) objc_msgSend)(ctrls, sel_getUid("objectAtIndex:"), 0);
    objc_singleArg(vc, sel_getUid("updateWeightFields"));
}
