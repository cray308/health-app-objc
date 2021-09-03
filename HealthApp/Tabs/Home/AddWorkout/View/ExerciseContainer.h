//
//  ExerciseContainer.h
//  HealthApp
//
//  Created by Christopher Ray on 8/22/21.
//

#ifndef ExerciseContainer_h
#define ExerciseContainer_h

#include "Exercise.h"
#import "ExerciseView.h"

@interface ExerciseContainer: UIView
- (id) initWithGroup: (ExerciseGroup *)g tag: (uint)idx target: (id)target action: (SEL)action;
@end

@interface ExerciseContainer() {
    @public UIView *divider;
    @public UILabel *headerLabel;
    @public ExerciseView **viewsArr;
    unsigned size;
}
@end

#endif /* ExerciseContainer_h */
