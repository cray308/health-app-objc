//
//  AddWorkoutViewModel.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef AddWorkoutViewModel_h
#define AddWorkoutViewModel_h

#import "Constants.h"
#import <Foundation/Foundation.h>
#import "HomeViewModel.h"

typedef struct AddWorkoutViewModel AddWorkoutViewModel;
typedef struct AddWorkoutCoordinator AddWorkoutCoordinator;

struct AddWorkoutViewModel {
    int newTokens;
    int durations[3];
    NSString *intensityStrings[3];
    Array_SingleActivityModel *workouts;
    AddWorkoutCoordinator *delegate;
};

void addWorkoutViewModel_tappedSaveButton(AddWorkoutViewModel *model);

#endif /* AddWorkoutViewModel_h */
