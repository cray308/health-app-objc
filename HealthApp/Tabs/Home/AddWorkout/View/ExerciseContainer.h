//
//  ExerciseContainer.h
//  HealthApp
//
//  Created by Christopher Ray on 8/22/21.
//

#ifndef ExerciseContainer_h
#define ExerciseContainer_h

#include "WorkoutScreenHelpers.h"
#include "Exercise.h"
#import "ExerciseView.h"
#import <UIKit/UIKit.h>

@interface ExerciseContainer: UIView
- (id) initWithGroup: (ExerciseGroup *)exerciseGroup tag: (int)tag parent: (id)parent
          timer: (WorkoutTimer *)timer exerciseTimer: (WorkoutTimer *)exerciseTimer;
- (void) startCircuitAndTimer: (bool)startTimer;
- (void) handleTap: (UIButton *)btn;
- (void) stopExerciseAtIndex: (int)index moveToNext: (bool)moveToNext;
- (void) finishGroupAtIndex: (int)index;
- (void) restartExerciseAtIndex: (int)index moveToNext: (bool)moveToNext refTime: (time_t)refTime;
- (void) restartGroupAtIndex: (int)index refTime: (time_t)refTime;
@end

@interface ExerciseContainer() {
    id parent;
    ExerciseGroup *group;
    WorkoutTimer *timer;
    WorkoutTimer *exerciseTimer;
    UILabel *headerLabel;
    ExerciseView **viewsArr;
    int currentIndex;
    int size;
}
@end

#endif /* ExerciseContainer_h */
