//
//  HomeWorkoutDayButton.h
//  HealthApp
//
//  Created by Christopher Ray on 6/8/21.
//

#ifndef HomeWorkoutDayButton_h
#define HomeWorkoutDayButton_h

#import <UIKit/UIKit.h>

@interface HomeWorkoutDayButton: UIView

- (id) initWithTitle: (NSString *)title day: (NSString *)day;
- (UIButton *) getButton;
- (void) setEnabled: (bool)enabled;

@end

#endif /* HomeWorkoutDayButton_h */
