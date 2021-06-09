//
//  AppUserData.m
//  HealthApp
//
//  Created by Christopher Ray on 3/20/21.
//

#import "AppUserData.h"
#import "AppDelegate.h"
#import "CalendarDateHelpers.h"

UserInfo *appUserDataShared = NULL;

UserInfo *userInfo_initFromStorage(void) {
    NSDictionary<NSString *, id> *savedInfo = [NSUserDefaults.standardUserDefaults dictionaryForKey:@"userinfo"];
    if (!savedInfo) return NULL;

    UserInfo *info = malloc(sizeof(UserInfo));
    if (!info) return NULL;
    info->planStart = [savedInfo[@"planStart"] doubleValue];
    info->currentPlan = (signed char) [savedInfo[@"currentPlan"] charValue];
    info->completedWorkouts = (uint8_t) [savedInfo[@"completedWorkouts"] unsignedCharValue];

    info->squatMax = [savedInfo[@"squatMax"] unsignedShortValue];
    info->pullUpMax = [savedInfo[@"pullUpMax"] unsignedShortValue];
    info->benchMax = [savedInfo[@"benchMax"] unsignedShortValue];
    info->deadliftMax = [savedInfo[@"deadliftMax"] unsignedShortValue];
    return info;
}

void userInfo_saveData(UserInfo *info) {
    NSDictionary<NSString *, id> *dict = @{
        @"planStart": [NSNumber numberWithDouble:info->planStart],
        @"currentPlan": [NSNumber numberWithChar:info->currentPlan],
        @"completedWorkouts": [NSNumber numberWithUnsignedChar:(unsigned char) info->completedWorkouts],
        @"squatMax": [NSNumber numberWithUnsignedShort:info->squatMax],
        @"pullUpMax": [NSNumber numberWithUnsignedShort: info->pullUpMax],
        @"benchMax": [NSNumber numberWithUnsignedShort:info->benchMax],
        @"deadliftMax": [NSNumber numberWithUnsignedShort:info->deadliftMax],
    };

    [NSUserDefaults.standardUserDefaults setObject:dict forKey:@"userinfo"];
    [NSUserDefaults.standardUserDefaults synchronize];
}

void appUserData_free(void) {
    if (appUserDataShared) free(appUserDataShared);
    appUserDataShared = NULL;
}

void appUserData_setWorkoutPlan(signed char plan) {
    appUserDataShared->currentPlan = plan;
    if (plan >= 0) {
        CFCalendarRef calendar = CFCalendarCopyCurrent();
        appUserDataShared->planStart = date_calcStartOfWeek(CFAbsoluteTimeGetCurrent(), calendar, DateSearchDirection_Next, true);
        CFRelease(calendar);
    }
    userInfo_saveData(appUserDataShared);
}

bool appUserData_hasWorkoutPlan(void) {
    if (appUserDataShared->currentPlan < 0) return false;
    double now = CFAbsoluteTimeGetCurrent();
    CFCalendarRef calendar = CFCalendarCopyCurrent();
    double start = date_calcStartOfWeek(now, calendar, DateSearchDirection_Previous, true);
    CFRelease(calendar);
    return ((int) start <= (int) now);
}
