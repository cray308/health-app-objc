//
//  DayWorkoutButton.h
//  HealthApp
//
//  Created by Christopher Ray on 8/23/21.
//

#ifndef DayWorkoutButton_h
#define DayWorkoutButton_h

#import <UIKit/UIKit.h>

@interface DayWorkoutButton: UIView
- (id) initWithTitle: (CFStringRef)title day: (CFStringRef)day
                 tag: (int)tag target: (id)target action: (SEL)action;
@end

@interface DayWorkoutButton() {
    @public UIView *checkbox;
    @public UIButton *button;
}
@end

#endif /* DayWorkoutButton_h */
