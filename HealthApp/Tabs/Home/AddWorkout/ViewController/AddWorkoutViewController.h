//
//  AddWorkoutViewController.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef AddWorkoutViewController_h
#define AddWorkoutViewController_h

#import "Constants.h"

@interface AddWorkoutViewController: UIViewController<UITableViewDataSource, UITableViewDelegate>

- (id) initWithViewModel: (AddWorkoutViewModel *)model;
- (void) insertEntryInRow: (int)row;
- (void) showTokenPopup;

@end

#endif /* AddWorkoutViewController_h */
