//
//  ExerciseView.h
//  HealthApp
//
//  Created by Christopher Ray on 8/22/21.
//

#ifndef ExerciseView_h
#define ExerciseView_h

#include "Exercise.h"
#import <UIKit/UIKit.h>

@interface ExerciseView: UIView
- (id) initWithEntry: (ExerciseEntry *)e tag: (uint)tag target: (id)target action: (SEL)action;
- (void) configureWithEntry: (ExerciseEntry *)e;
@end

@interface ExerciseView() {
    @public UILabel *setsLabel;
    @public UIView *checkbox;
    @public UIButton *button;
}
@end

#endif /* ExerciseView_h */
