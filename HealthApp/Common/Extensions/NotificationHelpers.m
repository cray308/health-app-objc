//
//  NotificationHelpers.m
//  HealthApp
//
//  Created by Christopher Ray on 6/16/21.
//

#import "NotificationHelpers.h"
#import <UserNotifications/UserNotifications.h>

#define _U_ __attribute__((__unused__))

static CFStringRef idFormat = CFSTR("%u");
static CFStringRef notificationMsgs[] = {CFSTR("Finished exercise!"), CFSTR("Finished AMRAP circuit!")};

void notifications_requestAccess(void) {
    [UNUserNotificationCenter.currentNotificationCenter requestAuthorizationWithOptions:UNAuthorizationOptionAlert | UNAuthorizationOptionSound completionHandler:^(BOOL granted _U_, NSError *_Nullable error _U_) {}];
}

void notifications_cleanup(void) {
    [UNUserNotificationCenter.currentNotificationCenter removeAllPendingNotificationRequests];
    [UNUserNotificationCenter.currentNotificationCenter removeAllDeliveredNotifications];
}

void notifications_schedule(unsigned int secondsFromNow, unsigned char messageType) {
    static unsigned int identifier = 0;
    CFStringRef idString = CFStringCreateWithFormat(NULL, NULL, idFormat, identifier++);

    UNMutableNotificationContent *content = [[UNMutableNotificationContent alloc] init];
    content.title = @"Workout Update";
    content.subtitle = (__bridge NSString*) notificationMsgs[messageType];
    content.sound = UNNotificationSound.defaultSound;

    UNTimeIntervalNotificationTrigger* trigger = [UNTimeIntervalNotificationTrigger triggerWithTimeInterval:secondsFromNow repeats:0];
    UNNotificationRequest *req = [UNNotificationRequest requestWithIdentifier:(__bridge NSString*)idString content:content trigger:trigger];

    [UNUserNotificationCenter.currentNotificationCenter addNotificationRequest:req withCompletionHandler:^(NSError *_Nullable error _U_) {}];
    CFRelease(idString);
    [content release];
}
