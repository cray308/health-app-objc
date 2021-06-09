//
//  SingleActivityViewModel.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef SingleActivityViewModel_h
#define SingleActivityViewModel_h

#import <UIKit/UIKit.h>

typedef struct AddSingleActivityViewModel AddSingleActivityViewModel;
typedef struct AddWorkoutCoordinator AddWorkoutCoordinator;
typedef struct AddWorkoutViewModel AddWorkoutViewModel;

struct AddSingleActivityViewModel {
    int intensityIndex;
    int duration;
    AddWorkoutViewModel *parentViewModel;
    AddWorkoutCoordinator *delegate;
};

void addSingleActivityViewModel_tappedAddActivity(AddSingleActivityViewModel *model, UIViewController *presenter);

#endif /* SingleActivityViewModel_h */
