//
//  CocoaBridging.m
//  HealthApp
//
//  Created by Christopher Ray on 7/24/21.
//

#import "CocoaBridging.h"
#import <Foundation/Foundation.h>

CFDictionaryRef getUserInfoDictionary(void) {
    return (__bridge CFDictionaryRef)[NSUserDefaults.standardUserDefaults
                                      dictionaryForKey:@"userinfo"];
}

void writeUserInfoDictionary(CFDictionaryRef dict) {
    [NSUserDefaults.standardUserDefaults setObject:(__bridge NSDictionary*)dict forKey:@"userinfo"];
    [NSUserDefaults.standardUserDefaults synchronize];
}

CFDictionaryRef workoutJsonDictionaryCreate(void) {
    NSString *path = [[NSBundle mainBundle] pathForResource:@"WorkoutData" ofType:@"plist"];
    return (__bridge CFDictionaryRef) [[NSDictionary alloc] initWithContentsOfFile:path];
}
