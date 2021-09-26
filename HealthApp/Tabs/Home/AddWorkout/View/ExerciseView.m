//
//  ExerciseView.m
//  HealthApp
//
//  Created by Christopher Ray on 8/22/21.
//

#import "ExerciseView.h"
#include "ViewControllerHelpers.h"

@implementation ExerciseView
- (id) initWithEntry: (ExerciseEntry *)e tag: (uint)tag target: (id)target action: (SEL)action {
    if (!(self = [super initWithButtonText:NULL hideHeader:false hideBox:false tag:(int)tag
                                    target:target action:action])) return nil;
    [self configureWithEntry:e];
    return self;
}

- (void) configureWithEntry: (ExerciseEntry *)e {
    CFStringRef setsStr = exerciseEntry_createSetsTitle(e);
    CFStringRef title = exerciseEntry_createTitle(e);
    setButtonTitle(button, title, 0);
    setLabelText(headerLabel, setsStr);

    switch (e->state) {
        case ExerciseStateDisabled:
            setBackground(checkbox, UIColor.systemGrayColor);
            enableButton(button, false);
            break;
        case ExerciseStateActive:
            if (e->type == ExerciseTypeDuration)
                button.userInteractionEnabled = false;
        case ExerciseStateResting:
            enableButton(button, true);
            setBackground(checkbox, UIColor.systemOrangeColor);
            break;
        case ExerciseStateCompleted:
            enableButton(button, false);
            setBackground(checkbox, UIColor.systemGreenColor);
    }

    CFRelease(title);
    if (setsStr)
        CFRelease(setsStr);
}
@end
