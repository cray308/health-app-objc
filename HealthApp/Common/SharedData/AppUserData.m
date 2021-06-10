//
//  AppUserData.m
//  HealthApp
//
//  Created by Christopher Ray on 3/20/21.
//

#import "AppUserData.h"
#import "AppDelegate.h"
#import "CalendarDateHelpers.h"
#import "Exercise.h"

UserInfo *appUserDataShared = NULL;

UserInfo *userInfo_initFromStorage(void) {
    NSDictionary<NSString *, id> *savedInfo = [NSUserDefaults.standardUserDefaults dictionaryForKey:@"userinfo"];
    if (!savedInfo) return NULL;

    UserInfo *info = malloc(sizeof(UserInfo));
    if (!info) return NULL;
    info->planStart = [savedInfo[@"planStart"] doubleValue];
    info->weekStart = [savedInfo[@"weekStart"] doubleValue];
    info->currentPlan = (signed char) [savedInfo[@"currentPlan"] charValue];
    info->completedWorkouts = [savedInfo[@"completedWorkouts"] unsignedCharValue];

    info->squatMax = [savedInfo[@"squatMax"] unsignedShortValue];
    info->pullUpMax = [savedInfo[@"pullUpMax"] unsignedShortValue];
    info->benchMax = [savedInfo[@"benchMax"] unsignedShortValue];
    info->deadliftMax = [savedInfo[@"deadliftMax"] unsignedShortValue];
    return info;
}

void userInfo_saveData(UserInfo *info) {
    NSDictionary<NSString *, id> *dict = @{
        @"planStart": [NSNumber numberWithDouble:info->planStart],
        @"weekStart": [NSNumber numberWithDouble:info->weekStart],
        @"currentPlan": [NSNumber numberWithChar:info->currentPlan],
        @"completedWorkouts": [NSNumber numberWithUnsignedChar:info->completedWorkouts],
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
    if (plan >= 0 && plan != appUserDataShared->currentPlan) {
        CFCalendarRef calendar = CFCalendarCopyCurrent();
        NSLog(@"Change to the commented out line!!!\n");
        appUserDataShared->planStart = date_calcStartOfWeek(CFAbsoluteTimeGetCurrent(), calendar, DateSearchDirection_Previous, 1); // remove this line
        //appUserDataShared->planStart = date_calcStartOfWeek(CFAbsoluteTimeGetCurrent(), calendar, DateSearchDirection_Next, 1);
        CFRelease(calendar);
    }
    appUserDataShared->currentPlan = plan;
    userInfo_saveData(appUserDataShared);
}

unsigned char appUserData_hasWorkoutPlan(void) {
    if (appUserDataShared->currentPlan < 0) return 0;
    return ((int) appUserDataShared->weekStart <= (int) CFAbsoluteTimeGetCurrent());
}

void appUserData_deleteSavedData(void) {
    appUserDataShared->completedWorkouts = 0;
    userInfo_saveData(appUserDataShared);
}

void appUserData_handleNewWeek(double weekStart) {
    appUserDataShared->completedWorkouts = 0;
    appUserDataShared->weekStart = weekStart;

    signed char plan = appUserDataShared->currentPlan;
    if (plan >= 0) {
        unsigned int difference = (unsigned int) (weekStart - appUserDataShared->planStart);
        const unsigned int nWeeks = plan == FitnessPlanBaseBuilding ? 8 : 13;
        if ((difference / WeekSeconds) >= nWeeks) {
            if (plan == FitnessPlanBaseBuilding) {
                appUserDataShared->currentPlan = FitnessPlanContinuation;
            }
            appUserDataShared->planStart = weekStart;
        }
    }
    userInfo_saveData(appUserDataShared);
}

unsigned char appUserData_addCompletedWorkout(unsigned char day) { // returns new number of completed workouts
    unsigned char total = 0;
    appUserDataShared->completedWorkouts |= (1 << day);
    userInfo_saveData(appUserDataShared);
    const unsigned char completedMask = appUserDataShared->completedWorkouts;
    for (unsigned char i = 0; i < 7; ++i) {
        if ((1 << i) & completedMask) ++total;
    }
    return total;
}

unsigned int appUserData_getWeekInPlan(void) {
    return ((unsigned int) (appUserDataShared->weekStart - appUserDataShared->planStart)) / WeekSeconds;
}
