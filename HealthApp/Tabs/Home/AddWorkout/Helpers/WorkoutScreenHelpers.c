//
//  WorkoutScreenHelpers.c
//  HealthApp
//
//  Created by Christopher Ray on 8/22/21.
//

#include "WorkoutScreenHelpers.h"

pthread_mutex_t timerLock;

void scheduleNotification(int secondsFromNow, CFStringRef message) {
    static int identifier = 0;
    CFStringRef idString = CFStringCreateWithFormat(NULL, NULL, CFSTR("%d"), identifier++);

    id content = ((id(*)(id,SEL))objc_msgSend)(allocClass("UNMutableNotificationContent"),
                                               sel_getUid("init"));
    ((void(*)(id,SEL,CFStringRef))objc_msgSend)(content, sel_getUid("setTitle:"),
                                                CFSTR("Workout Update"));
    ((void(*)(id,SEL,CFStringRef))objc_msgSend)(content, sel_getUid("setSubtitle:"), message);
    id sound = objc_staticMethod(objc_getClass("UNNotificationSound"), sel_getUid("defaultSound"));
    ((void(*)(id,SEL,id))objc_msgSend)(content, sel_getUid("setSound:"), sound);

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

void startWorkoutTimer(WorkoutTimer *t, int duration, int container, int exercise) {
    pthread_mutex_lock(&t->lock);
    t->refTime = time(NULL);
    t->duration = duration;
    if (container >= 0)
        t->container = container;
    else
        t->exercise = exercise;
    t->info.stop = !duration;
    t->info.active = 1;
    pthread_cond_signal(&t->cond);
    pthread_mutex_unlock(&t->lock);
}
