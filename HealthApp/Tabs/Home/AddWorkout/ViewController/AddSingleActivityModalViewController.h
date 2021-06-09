//
//  AddSingleActivityModalViewController.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef AddSingleActivityModalViewController_h
#define AddSingleActivityModalViewController_h

#import "Constants.h"

@interface AddSingleActivityModalViewController: UIViewController<UITextFieldDelegate>

- (id) initWithViewModel: (AddSingleActivityViewModel *)model;

@end

#endif /* AddSingleActivityModalViewController_h */
