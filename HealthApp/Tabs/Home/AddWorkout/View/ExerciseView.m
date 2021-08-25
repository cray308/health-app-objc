//
//  ExerciseView.m
//  HealthApp
//
//  Created by Christopher Ray on 8/22/21.
//

#import "ExerciseView.h"
#include "ViewControllerHelpers.h"

@implementation ExerciseView
- (id) initWithExercise: (ExerciseEntry *)exercise tag: (int)tag
                 target: (id)target action: (SEL)action timer: (WorkoutTimer *)timer {
    if (!(self = [super initWithFrame:CGRectZero])) return nil;
    self->exercise = exercise;
    if (exercise->rest)
        restStr = CFStringCreateWithFormat(NULL, NULL, CFSTR("Rest: %d s"), exercise->rest);

    button = createButton(CFSTR(""), UIColor.labelColor, UIColor.secondaryLabelColor,
                          UIFontTextStyleHeadline, UIColor.secondarySystemGroupedBackgroundColor,
                          true, true, tag, target, action);
    checkbox = createView(nil, true);
    setsLabel = createLabel(NULL, UIFontTextStyleSubheadline, NSTextAlignmentNatural);
    if (exercise->sets > 1) {
        CFStringRef setsStr = exerciseEntry_createSetsTitle(exercise);
        setLabelText(setsLabel, setsStr);
        CFRelease(setsStr);
    }

    [self addSubview:setsLabel];
    [self addSubview:button];
    [self addSubview:checkbox];

    activateConstraints((id []){
        [setsLabel.topAnchor constraintEqualToAnchor:self.topAnchor],
        [setsLabel.heightAnchor constraintEqualToConstant:20],
        [setsLabel.leadingAnchor constraintEqualToAnchor:self.leadingAnchor constant:8],
        [setsLabel.trailingAnchor constraintEqualToAnchor:self.trailingAnchor constant:-8],

        [button.topAnchor constraintEqualToAnchor:setsLabel.bottomAnchor constant:5],
        [button.heightAnchor constraintEqualToConstant:50],
        [button.bottomAnchor constraintEqualToAnchor:self.bottomAnchor],
        [button.leadingAnchor constraintEqualToAnchor:self.leadingAnchor constant:8],
        [button.trailingAnchor constraintEqualToAnchor:checkbox.leadingAnchor constant:-5],

        [checkbox.trailingAnchor constraintEqualToAnchor:self.trailingAnchor constant:-8],
        [checkbox.centerYAnchor constraintEqualToAnchor:button.centerYAnchor],
        [checkbox.widthAnchor constraintEqualToConstant:20],
        [checkbox.heightAnchor constraintEqualToAnchor:checkbox.widthAnchor]
    }, 13);
    [self reset];
    return self;
}

- (void) dealloc {
    [setsLabel release];
    [checkbox release];
    if (restStr) CFRelease(restStr);
    [super dealloc];
}

- (bool) handleTap {
    if (!button.enabled) {
        enableButton(button, true);
        setBackground(checkbox, UIColor.systemOrangeColor);
        if (exercise->type == ExerciseTypeDuration) { // start timer
            button.userInteractionEnabled = false;
            startWorkoutTimer(timer, exercise->reps, -1, (int) button.tag);
            scheduleNotification(exercise->reps, CFSTR("Finished exercise!"));
        }
    } else {
        button.userInteractionEnabled = true;

        if (restStr && !resting) {
            resting = true;
            setButtonTitle(button, restStr, 0);
        } else {
            resting = false;
            if (++exercise->completedSets == exercise->sets) {
                enableButton(button, false);
                setBackground(checkbox, UIColor.systemGreenColor);
                return true;
            }

            CFStringRef setsStr = exerciseEntry_createSetsTitle(exercise);
            CFStringRef title = exerciseEntry_createTitle(exercise);
            setLabelText(setsLabel, setsStr);
            setButtonTitle(button, title, 0);
            CFRelease(setsStr);
            CFRelease(title);
        }
    }
    return false;
}

- (void) reset {
    setBackground(checkbox, UIColor.systemGrayColor);
    enableButton(button, false);
    resting = false;
    exercise->completedSets = 0;
    CFStringRef title = exerciseEntry_createTitle(exercise);
    setButtonTitle(button, title, 0);
    CFRelease(title);
}
@end
