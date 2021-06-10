//
//  ActivityType.m
//  HealthApp
//
//  Created by Christopher Ray on 3/21/21.
//

#import "ActivityType.h"

static char const *FullStrs[] = {"Low-Intensity", "Medium-Intensity", "High-Intensity"};

NSString *activityType_getString(ActivityType type, unsigned char shortString) {
    const char *str = NULL;
    switch (type) {
        case ActivityType_Low:
            str = FullStrs[0];
            break;
        case ActivityType_Medium:
            str = FullStrs[1];
            break;
        case ActivityType_High:
            str = FullStrs[2];
            break;
        default:
            return nil;
    }

    const char *endPtr = NULL;
    if (shortString) {
        if (!(endPtr = strchr(str, '-'))) return nil;
    } else {
        endPtr = str + strlen(str);
    }
    return [[NSString alloc] initWithBytes:str length:(int)(endPtr - str) encoding:NSUTF8StringEncoding];
}
