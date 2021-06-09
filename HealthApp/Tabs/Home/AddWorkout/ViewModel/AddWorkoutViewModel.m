//
//  AddWorkoutViewModel.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "AddWorkoutViewModel.h"
#import "ActivityEntry+CoreDataClass.h"
#import "PersistenceService.h"
#import "AddWorkoutCoordinator.h"

void addWorkoutViewModel_tappedSaveButton(AddWorkoutViewModel *model) {
    SingleActivityModel *activity;
    array_iter(model->workouts, activity) {
        activity->date = CFAbsoluteTimeGetCurrent();
        model->newTokens += activity->tokens;
        model->durations[activity->intensity] += activity->duration;

        ActivityEntry *entry = [[ActivityEntry alloc] initWithContext:persistenceService_sharedContainer.viewContext];
        [entry setProperties:activity];
        [entry release];
    }
    persistenceService_saveContext();
    addWorkoutCoordinator_didFinishAddingWorkouts(model->delegate);
}
