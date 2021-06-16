//
//  WorkoutFinder.m
//  HealthApp
//
//  Created by Christopher Ray on 6/7/21.
//

#import "WorkoutFinder.h"
#import "AppUserData.h"
#import <Foundation/Foundation.h>

static CFStringRef libraryKeys[] = {CFSTR("st"), CFSTR("se"), CFSTR("en"), CFSTR("hi")};
static CFStringRef libraryKey = CFSTR("library");
static CFStringRef planKeys[] = {CFSTR("bb"), CFSTR("cc")};
static CFStringRef plansKey = CFSTR("plans");
static CFStringRef indexKey = CFSTR("index");
static CFStringRef setsKey = CFSTR("sets");
static CFStringRef repsKey = CFSTR("reps");
static CFStringRef weightKey = CFSTR("weight");
static CFStringRef typeKey = CFSTR("type");
static CFStringRef titleKey = CFSTR("title");

CFArrayRef getLibraryArrayForType(CFDictionaryRef libDict, unsigned char type) {
    if (type > 3) return NULL;
    CFArrayRef arr = CFDictionaryGetValue(libDict, libraryKeys[type]);
    return arr;
}

void workoutFinder_setWeeklyWorkoutNames(unsigned char plan, unsigned int week, CFStringRef *names) {
    if (plan > 1) return;

    CFDictionaryRef root = (__bridge CFDictionaryRef) [[NSDictionary alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"WorkoutData" ofType:@"plist"]];
    CFDictionaryRef lib = CFDictionaryGetValue(root, libraryKey);
    CFDictionaryRef plans = CFDictionaryGetValue(root, plansKey);
    CFArrayRef weeks = CFDictionaryGetValue(plans, planKeys[plan]);

    if (week >= CFArrayGetCount(weeks)) {
        CFRelease(root);
        return;
    }

    CFArrayRef currWeek = CFArrayGetValueAtIndex(weeks, week);
    unsigned int index = 0, tempInt = 0;

    for (int i = 0; i < 7; ++i) {
        CFDictionaryRef day = CFArrayGetValueAtIndex(currWeek, i);

        CFNumberRef number = CFDictionaryGetValue(day, typeKey);
        CFNumberGetValue(number, kCFNumberIntType, &tempInt);
        unsigned char type = (unsigned char) tempInt;
        number = CFDictionaryGetValue(day, indexKey);
        CFNumberGetValue(number, kCFNumberIntType, &index);

        CFArrayRef libArr = getLibraryArrayForType(lib, type);
        if (!libArr) continue;

        CFDictionaryRef foundWorkout = CFArrayGetValueAtIndex(libArr, index);
        CFStringRef str = CFDictionaryGetValue(foundWorkout, titleKey);
        names[i] = CFStringCreateCopy(NULL, str);
    }
    CFRelease(root);
}

Workout *workoutFinder_getWeeklyWorkoutAtIndex(unsigned char plan, unsigned int week, unsigned int index) {
    CFDictionaryRef root = (__bridge CFDictionaryRef) [[NSDictionary alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"WorkoutData" ofType:@"plist"]];
    CFDictionaryRef lib = CFDictionaryGetValue(root, libraryKey);
    CFDictionaryRef plans = CFDictionaryGetValue(root, plansKey);
    CFArrayRef weeks = CFDictionaryGetValue(plans, planKeys[plan]);

    if (week >= CFArrayGetCount(weeks)) {
        CFRelease(root);
        return NULL;
    }

    CFArrayRef currWeek = CFArrayGetValueAtIndex(weeks, week);
    CFDictionaryRef day = CFArrayGetValueAtIndex(currWeek, index);
    unsigned int idx = 0, sets = 0, reps = 0, weight = 0;
    unsigned char type = 0;

    CFNumberRef number = CFDictionaryGetValue(day, typeKey);
    CFNumberGetValue(number, kCFNumberIntType, &idx);
    type = (unsigned char) idx;

    CFArrayRef libArr = getLibraryArrayForType(lib, type);
    if (!libArr) {
        CFRelease(root);
        return NULL;
    }

    number = CFDictionaryGetValue(day, indexKey);
    CFNumberGetValue(number, kCFNumberIntType, &idx);
    number = CFDictionaryGetValue(day, setsKey);
    CFNumberGetValue(number, kCFNumberIntType, &sets);
    number = CFDictionaryGetValue(day, repsKey);
    CFNumberGetValue(number, kCFNumberIntType, &reps);
    number = CFDictionaryGetValue(day, weightKey);
    CFNumberGetValue(number, kCFNumberIntType, &weight);

    Workout *w = calloc(1, sizeof(Workout));
    w->day = index;

    CFDictionaryRef foundWorkout = CFArrayGetValueAtIndex(libArr, idx);
    workout_buildFromDictionary(foundWorkout, idx, type, sets, reps, weight, w);
    if (!w->title) {
        free(w);
        w = NULL;
    }
    CFRelease(root);
    return w;
}

CFStringRef *workoutFinder_get_workout_names(unsigned char type, unsigned int *size) {
    CFDictionaryRef root = (__bridge CFDictionaryRef) [[NSDictionary alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"WorkoutData" ofType:@"plist"]];
    CFDictionaryRef lib = CFDictionaryGetValue(root, libraryKey);

    long len = 0;
    CFArrayRef libArr = getLibraryArrayForType(lib, type);
    if (!(libArr && (len = CFArrayGetCount(libArr)))) {
        CFRelease(root);
        return NULL;
    }

    if (type == WorkoutTypeStrength) len = 2;

    CFStringRef *results = calloc(len, sizeof(CFStringRef));
    *size = (unsigned int) len;
    for (long i = 0; i < len; ++i) {
        CFDictionaryRef week = CFArrayGetValueAtIndex(libArr, i);
        CFStringRef title = CFDictionaryGetValue(week, titleKey);
        results[i] = CFStringCreateCopy(NULL, title);
    }
    CFRelease(root);
    return results;
}

Workout *workoutFinder_get_workout_from_library(unsigned char type, unsigned int index, unsigned int reps, unsigned int sets, unsigned int weight) {
    CFDictionaryRef root = (__bridge CFDictionaryRef) [[NSDictionary alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"WorkoutData" ofType:@"plist"]];
    CFDictionaryRef lib = CFDictionaryGetValue(root, libraryKey);
    CFArrayRef libArr = getLibraryArrayForType(lib, type);
    if (!libArr) {
        CFRelease(root);
        return NULL;
    }
    Workout *w = calloc(1, sizeof(Workout));
    w->day = -1;
    CFDictionaryRef foundWorkout = CFArrayGetValueAtIndex(libArr, index);
    workout_buildFromDictionary(foundWorkout, index, type, sets, reps, weight, w);
    if (!w->title) {
        free(w);
        w = NULL;
    }
    CFRelease(root);
    return w;
}
