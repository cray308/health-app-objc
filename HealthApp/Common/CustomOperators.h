//
//  CustomOperators.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef CustomOperators_h
#define CustomOperators_h

#import <UIKit/UIKit.h>

static inline int mod(int lhs, int rhs) {
    int remainder = lhs % rhs;
    return (remainder >= 0) ? remainder : (rhs + remainder);
}

#endif /* CustomOperators_h */
