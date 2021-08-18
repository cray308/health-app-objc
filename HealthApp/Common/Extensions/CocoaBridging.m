//
//  CocoaBridging.m
//  HealthApp
//
//  Created by Christopher Ray on 7/24/21.
//

#import "CocoaBridging.h"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#import "HealthApp-Swift.h"
#pragma clang diagnostic pop

id objc_staticMethod(Class _self, SEL _cmd) {
    return ((id(*)(Class,SEL))objc_msgSend)(_self, _cmd);
}

void objc_singleArg(id obj, SEL _cmd) {
    ((void(*)(id,SEL))objc_msgSend)(obj, _cmd);
}

id createChartEntry(int x, int y) {
    return [[ChartDataEntry alloc] initWithX:x y:y];
}

id createColor(const char *name) {
    return objc_staticMethod(objc_getClass("UIColor"), sel_getUid(name));
}

id allocClass(const char *name) {
    return objc_staticMethod(objc_getClass(name), sel_getUid("alloc"));
}

void releaseObj(id obj) {
    objc_singleArg(obj, sel_getUid("release"));
}

id createArray(id *arr, int count) {
    return ((id(*)(Class,SEL,id*,int))objc_msgSend)
    (objc_getClass("NSArray"), sel_getUid("arrayWithObjects:count:"), arr, count);
}

id getObjectAtIndex(id arr, int i) {
    return ((id(*)(id,SEL,int))objc_msgSend)(arr, sel_getUid("objectAtIndex:"), i);
}

id createVCWithDelegate(const char *name, void *delegate) {
    return ((id(*)(id,SEL,void*))objc_msgSend)(allocClass(name),
                                               sel_getUid("initWithDelegate:"), delegate);
}

void setupNavVC(id navVC, id firstVC) {
    id array = createArray((id []){firstVC}, 1);
    ((void(*)(id,SEL,id))objc_msgSend)(navVC, sel_getUid("setViewControllers:"), array);
    releaseObj(firstVC);
}

id getFirstVC(id navVC) {
    id ctrls = ((id(*)(id,SEL))objc_msgSend)(navVC, sel_getUid("viewControllers"));
    return getObjectAtIndex(ctrls, 0);
}

id allocNavVC(void) {
    return allocClass("UINavigationController");
}

void presentVC(id presenter, id modal) {
    id container = ((id(*)(id,SEL,id))objc_msgSend)
    (allocNavVC(), sel_getUid("initWithRootViewController:"), modal);
    ((void(*)(id,SEL,id,bool,id))objc_msgSend)
    (presenter, sel_getUid("presentViewController:animated:completion:"), container, true, nil);
    releaseObj(container);
    releaseObj(modal);
}

void dismissPresentedVC(id presenter) {
    ((void(*)(id,SEL,bool,id))objc_msgSend)
    (presenter, sel_getUid("dismissViewControllerAnimated:completion:"), true, nil);
}

void setWeekData(id weekData, const char *setter, int16_t value) {
    ((void(*)(id,SEL,int16_t))objc_msgSend)(weekData, sel_getUid(setter), value);
}

int16_t getWeekData(id weekData, const char *getter) {
    return ((int16_t(*)(id,SEL))objc_msgSend)(weekData, sel_getUid(getter));
}

int16_t getLiftingLimitForType(id weekData, unsigned char type) {
    static char const *getterStrs[] = {"bestSquat", "bestPullup", "bestBench", "bestDeadlift"};
    return getWeekData(weekData, getterStrs[type]);
}

int16_t getWorkoutTimeForType(id weekData, unsigned char type) {
    static char const *getterStrs[] = {"timeStrength", "timeSE", "timeEndurance", "timeHIC"};
    return getWeekData(weekData, getterStrs[type]);
}

int16_t getTotalWorkouts(id weekData) {
    return getWeekData(weekData, "totalWorkouts");
}

void addToWorkoutType(id weekData, unsigned char type, int16_t duration) {
    static char const *setterStrs[] = {
        "setTimeStrength:", "setTimeSE:", "setTimeEndurance:", "setTimeHIC:"
    };
    setWeekData(weekData, setterStrs[type], duration + getWorkoutTimeForType(weekData, type));
}

void setLiftingMaxes(id weekData, short *weights) {
    static char const *setterStrs[] = {
        "setBestSquat:", "setBestPullup:", "setBestBench:", "setBestDeadlift:"
    };
    for (int i = 0; i < 4; ++i) {
        setWeekData(weekData, setterStrs[i], weights[i]);
    }
}
