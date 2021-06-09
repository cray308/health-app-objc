//
//  CustomTokenPopupViewModel.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef CustomTokenPopupViewModel_h
#define CustomTokenPopupViewModel_h

#import "Constants.h"

struct CustomTokenPopupViewModel {
    AddWorkoutCoordinator *delegate;
    NSString *message;
    int tokens;
    int tokensForRow[2];
    CGFloat foregroundWidth;
    CGFloat imageDimension;
    CGFloat spacingForRow[2];
};

CustomTokenPopupViewModel *customTokenPopupViewModel_init(AddWorkoutCoordinator *delegate, CGFloat width, int tokens);
void customTokenPopupViewModel_tappedOkButton(CustomTokenPopupViewModel *model, UIView *popup);

#endif /* CustomTokenPopupViewModel_h */
