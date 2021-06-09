//
//  NavBarCoinsView.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef NavBarCoinsView_h
#define NavBarCoinsView_h

#import <UIKit/UIKit.h>

@interface NavBarCoinsView: UIView

- (id) init;
- (void) runAnimation;
- (void) updateTokens: (NSString *)tokenString;

@end

#endif /* NavBarCoinsView_h */
