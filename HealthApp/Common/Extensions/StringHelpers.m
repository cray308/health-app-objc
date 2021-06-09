//
//  StringHelpers.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "StringHelpers.h"

NSString *durationStringCreate_fromNumber(int number) {
    if (number > 59) {
        return [[NSString alloc] initWithFormat:@"%d h %d m", number / 60, number % 60];
    }
    return [[NSString alloc] initWithFormat:@"%d m", number];
}
