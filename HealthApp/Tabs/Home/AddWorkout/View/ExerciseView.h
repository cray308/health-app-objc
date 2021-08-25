//
//  ExerciseView.h
//  HealthApp
//
//  Created by Christopher Ray on 8/22/21.
//

#ifndef ExerciseView_h
#define ExerciseView_h

#include "WorkoutScreenHelpers.h"
#include "Exercise.h"
#import <UIKit/UIKit.h>

@interface ExerciseView: UIView
- (id) initWithExercise: (ExerciseEntry *)exercise tag: (int)tag
                 target: (id)target action: (SEL)action timer: (WorkoutTimer *)timer;
- (bool) handleTap;
- (void) reset;
@end

@interface ExerciseView() {
    @public ExerciseEntry *exercise;
    WorkoutTimer *timer;
    int completedSets;
    bool resting;
    CFStringRef restStr;
    UILabel *setsLabel;
    UIView *checkbox;
    UIButton *button;
}
@end

#endif /* ExerciseView_h */
