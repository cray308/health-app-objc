//
//  WorkoutTimer.c
//  HealthApp
//
//  Created by Christopher Ray on 10/4/21.
//

#include "WorkoutTimer.h"
#include <CoreFoundation/CFString.h>
#include <objc/message.h>
#include "CocoaHelpers.h"

static CFStringRef notifTitle;
static CFStringRef notificationMessages[2];

void initTimerStrings(void) {
    notifTitle = localize(CFSTR("workoutNotificationTitle"));
    notificationMessages[0] = localize(CFSTR("notifications0"));
    notificationMessages[1] = localize(CFSTR("notifications1"));
}

void scheduleNotification(int secondsFromNow, TimerType type) {
    static int identifier = 0;
    CFStringRef idString = getNumberString(identifier++);

    id content = getObject(allocClass("UNMutableNotificationContent"), sel_getUid("init"));
    setString(content, sel_getUid("setTitle:"), notifTitle);
    setString(content, sel_getUid("setSubtitle:"), notificationMessages[type]);
    id sound = staticMethod(objc_getClass("UNNotificationSound"), sel_getUid("defaultSound"));
    setObject(content, sel_getUid("setSound:"), sound);

    id trigger = ((id(*)(Class,SEL,double,bool))objc_msgSend)
    (objc_getClass("UNTimeIntervalNotificationTrigger"),
     sel_getUid("triggerWithTimeInterval:repeats:"), secondsFromNow, false);

    id req = ((id(*)(Class,SEL,CFStringRef,id,id))objc_msgSend)
    (objc_getClass("UNNotificationRequest"), sel_getUid("requestWithIdentifier:content:trigger:"),
     idString, content, trigger);

    ((void(*)(id,SEL,id,void(^)(id)))objc_msgSend)
    (getNotificationCenter(), sel_getUid("addNotificationRequest:withCompletionHandler:"),
     req, ^(id error _U_) {});
    CFRelease(idString);
    releaseObj(content);
}

void startWorkoutTimer(WorkoutTimer *t, int duration) {
    pthread_mutex_lock(&t->lock);
    t->refTime = time(NULL);
    t->duration = duration;
    t->info.stop = !duration;
    t->info.active = 1;
    pthread_cond_signal(&t->cond);
    pthread_mutex_unlock(&t->lock);
}
