//
//  ExerciseContainer.h
//  HealthApp
//
//  Created by Christopher Ray on 8/22/21.
//

#ifndef ExerciseContainer_h
#define ExerciseContainer_h

#include "Exercise.h"
#import "StatusButton.h"

@interface ExerciseContainer: UIView @end
@interface ExerciseContainer() {
    @public UIView *divider;
    @public UILabel *headerLabel;
    @public StatusButton **viewsArr;
    @public unsigned size;
}
@end

id exerciseContainer_init(ExerciseGroup *g, int idx, id target, SEL action);
void exerciseView_configure(id v, ExerciseEntry *e);

#endif /* ExerciseContainer_h */
