#include "AppDelegate.h"
#include <CoreFoundation/CFAttributedString.h>
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CFPreferences.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "ExerciseManager.h"
#include "HomeVC.h"
#include "InputVC.h"
#include "SetupWorkoutVC.h"
#include "Views.h"

#define WeekSeconds 604800

#if TARGET_OS_SIMULATOR
void exerciseManager_setCurrentWeek(int);
#endif
extern CGFloat UIFontWeightSemibold;
void initViewData(VCache *cacheRef);
void initNSData(bool modern, ColorCache *cacheRef, Class **clsRefs, size_t **sizeRefs);
void setupAppColors(unsigned char darkMode, bool deleteOld);
void toggleDarkModeForCharts(bool);
void initValidatorStrings(CFBundleRef);
void initExerciseData(int, CFBundleRef);
void initWorkoutStrings(CFBundleRef);
void initStatVData(void);
id homeVC_init(CFBundleRef, VCacheRef, CCacheRef, time_t);
void homeVC_updateWorkoutsList(HomeVC *self, unsigned char completed);
void homeVC_createWorkoutsList(id self, const UserInfo *info);
void homeVC_updateColors(id self);
id historyVC_init(CFBundleRef, void **, FetchHandler*, VCacheRef, CCacheRef);
void historyVC_clearData(id self);
void historyVC_updateColors(id vc, unsigned char darkMode);
id settingsVC_init(VCacheRef, CCacheRef);
void settingsVC_updateColors(id self, unsigned char darkMode);
long setupWorkoutVC_numberOfComponents(id, SEL, id);
long setupWorkoutVC_numberOfComponentsLegacy(id, SEL, id);
CFAttributedStringRef setupWorkoutVC_attrTitleForRow(id, SEL, id, long, long);
CFStringRef setupWorkoutVC_titleForRow(id, SEL, id, long, long);
void setupWorkoutVC_didSelectRow(id, SEL, id, long, long);
void setupWorkoutVC_didSelectRowLegacy(id, SEL, id, long, long);
id colorCreateLegacy(id self, SEL _cmd, int type);
id colorCreate(id self, SEL _cmd, int type);
id barColorCreateLegacy(id self, SEL _cmd, int type);
id barColorCreate(id self, SEL _cmd, int type);

enum {
    WorkoutPlanBaseBuilding, WorkoutPlanContinuation
};

enum {
    IWeekStart, IPlanStart, ITzOffset, ICurrentPlan, ICompletedWorkouts, IDarkMode, ILiftArray
};

#if DEBUG
static bool FIRST_LAUNCH;
#endif
static id service;
static CFStringRef const dictKey = CFSTR("userinfo");
static const void *DictKeys[] = {
    CFSTR("weekStart"), CFSTR("planStart"), CFSTR("tzOffset"),
    CFSTR("currentPlan"), CFSTR("completedWorkouts"), CFSTR("darkMode"),
    CFSTR("squatMax"), CFSTR("pullUpMax"), CFSTR("benchMax"), CFSTR("deadliftMax")
};
static const char *const liftGets[] = {"bestSquat", "bestPullup", "bestBench", "bestDeadlift"};
static const char *const timeGets[] = {"timeStrength", "timeSE", "timeEndurance", "timeHIC"};
static char const *const timeSets[] = {"setTimeStrength:", "setTimeSE:", "setTimeEndurance:", "setTimeHIC:"};
static char const *const liftSets[] = {
    "setBestSquat:", "setBestPullup:", "setBestBench:", "setBestDeadlift:"
};
static int orientations;
static int barStyle;
static Class DMNavVC;
static Class UIApplication;
static SEL sharedApplication;
static SEL delegate;
static id (*sharedAppImp)(Class,SEL);
static id (*delegateImp)(id,SEL);

static AppDelegate *getAppDel(void) {
    id app = sharedAppImp(UIApplication, sharedApplication);
    return (AppDelegate *)delegateImp(app, delegate);
}

#pragma mark - User Defaults Helpers

static CFMutableDictionaryRef createMutableDict(void) {
    CFDictionaryRef saved = CFPreferencesCopyAppValue(dictKey, kCFPreferencesCurrentApplication);
    CFMutableDictionaryRef newDict = CFDictionaryCreateMutableCopy(NULL, 10, saved);
    CFRelease(saved);
    return newDict;
}

static void saveChanges(CFMutableDictionaryRef dict CF_CONSUMED,
                        CFStringRef *keys, CFNumberRef *values, int count) {
    for (int i = 0; i < count; ++i) {
        CFDictionaryReplaceValue(dict, keys[i], values[i]);
    }
    CFPreferencesSetAppValue(dictKey, dict, kCFPreferencesCurrentApplication);
    CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
    CFRelease(dict);
    for (int i = 0; i < count; ++i) {
        CFRelease(values[i]);
    }
}

#pragma mark - Core Data Helpers

#define weekData_setTotalWorkouts(d, val) msg1(void,int16_t,d,sel_getUid("setTotalWorkouts:"),val)

static void context_saveChanges(id context) {
    if (msg0(bool, context, sel_getUid("hasChanges")))
        msg1(void, id, context, sel_getUid("save:"), nil);
}

static CFArrayRef context_fetchData(id context, int options, int *count) {
    id req = clsF1(id, CFStringRef, objc_getClass("NSFetchRequest"),
                   sel_getUid("fetchRequestWithEntityName:"), CFSTR("WeeklyData"));
    if (options & 0x2)
        msg1(void, unsigned long, req, sel_getUid("setFetchLimit:"), 1);
    CFArrayRef descriptorArr = createSortDescriptors(CFSTR("weekStart"), options & 0x1);
    msg1(void, CFArrayRef, req, sel_getUid("setSortDescriptors:"), descriptorArr);
    CFRelease(descriptorArr);

    int len = 0;
    CFArrayRef data = msg2(CFArrayRef, id, id, context, sel_getUid("executeFetchRequest:error:"), req, nil);
    if (!(data && (len = (int)(CFArrayGetCount(data)))))
        data = NULL;
    *count = len;
    return data;
}

#if DEBUG
static void createDummyData(id context) {
    ((void(*)(id,SEL,Callback))objc_msgSend)(context, sel_getUid("performBlock:"), ^{
        int16_t lifts[] = {300, 20, 185, 235};
        int i = 0, r = 0;
        unsigned char plan = 0;
        time_t endpts[] = {time(NULL) - 126489600, time(NULL) - 2678400};
        for (int x = 0; x < 2; ++x) {
            time_t date = endpts[x];
            struct tm tmInfo;
            localtime_r(&date, &tmInfo);
            if (tmInfo.tm_wday != 1) {
                date = date - WeekSeconds + (((8 - tmInfo.tm_wday) % 7) * 86400);
                localtime_r(&date, &tmInfo);
            }
            endpts[x] = date - ((tmInfo.tm_hour * 3600) + (tmInfo.tm_min * 60) + tmInfo.tm_sec);
        }
        while (endpts[0] < endpts[1]) {
            int16_t totalWorkouts = 0;
            int16_t times[4] = {0};
            id data = msg1(id, id, clsF0(id, objc_getClass("WeekData"), Sels.alo),
                           sel_getUid("initWithContext:"), context);
            int addnlTime = 0, addnlWk = 1;
            if (r == 0 || (i < 24)) {
                addnlTime = rand() % 40;
                addnlWk += rand() % 5;
            }
            msg1(void, int64_t, data, sel_getUid("setWeekStart:"), endpts[0]);

            if (plan == 0) {
                for (int j = 0; j < 6; ++j) {
                    int extra = 10;
                    bool didSE = true;
                    switch (j) {
                        case 1:
                        case 2:
                        case 5:
                            times[2] += ((rand() % 30) + 30 + addnlTime);
                            totalWorkouts += addnlWk;
                            break;
                        case 4:
                            if ((didSE = (rand() % 10 >= 5))) extra = 0;
                        case 0:
                        case 3:
                            if (didSE) {
                                times[1] += ((rand() % 20) + extra + addnlTime);
                                totalWorkouts += addnlWk;
                            }
                        default:
                            break;
                    }
                }
            } else {
                for (int j = 0; j < 6; ++j) {
                    switch (j) {
                        case 0:
                        case 2:
                        case 4:
                            times[0] += ((rand() % 20) + 20 + addnlTime);
                            totalWorkouts += addnlWk;
                            break;
                        case 1:
                        case 3:
                            times[3] += ((rand() % 20) + 15 + addnlTime);
                            totalWorkouts += addnlWk;
                            break;
                        case 5:
                            times[2] += ((rand() % 30) + 60 + addnlTime);
                            totalWorkouts += addnlWk;
                        default:
                            break;
                    }
                }
            }

            if (i == 7) {
                plan = 1;
            } else if (i == 20 || i == 32 || i == 44) {
                lifts[2] = 185 + (rand() % 50);
                lifts[1] = 20 + (rand() % 20);
                lifts[0] = 300 + (rand() % 80);
                lifts[3] = 235 + (rand() % 50);
            }
            msg1(void, int16_t, data, sel_getUid("setTotalWorkouts:"), totalWorkouts);
            for (int x = 0; x < 4; ++x) {
                msg1(void, int16_t, data, sel_getUid(liftSets[x]), lifts[x]);
                msg1(void, int16_t, data, sel_getUid(timeSets[x]), times[x]);
            }
            msg0(void, data, Sels.rel);

            if (++i == 52) {
                i = 0;
                plan = 0;
                r += 1;
            }
            endpts[0] += WeekSeconds;
        }
        context_saveChanges(context);
    });
}
#endif

static void fetchHistory(id context, void *model, FetchHandler handler) {
    ((void(*)(id,SEL,Callback))objc_msgSend)(context, sel_getUid("performBlock:"), ^{
        struct tm tm;
        int count = 0;
        CFArrayRef data = context_fetchData(context, 1, &count);
        if (data && count > 1) {
            SEL liftSels[4], timeSels[4];
            SEL getTotal = sel_getUid("totalWorkouts"), getStart = sel_getUid("weekStart");
            for (int i = 0; i < 4; ++i) {
                liftSels[i] = sel_getUid(liftGets[i]);
                timeSels[i] = sel_getUid(timeGets[i]);
            }

            CFMutableArrayRef strs = CFArrayCreateMutable(NULL, --count, &kCFTypeArrayCallBacks);
            WeekDataModel *results = malloc((unsigned)count * sizeof(WeekDataModel));
            customAssert(count > 0)
            for (int i = 0; i < count; ++i) {
                id d = (id)CFArrayGetValueAtIndex(data, i);
                WeekDataModel *r = &results[i];
                localtime_r(&(time_t){msg0(int64_t, d, getStart)}, &tm);
                CFStringRef str = formatStr(CFSTR("%d/%d/%d"), tm.tm_mon + 1, tm.tm_mday, tm.tm_year % 100);
                CFArrayAppendValue(strs, str);
                r->totalWorkouts = msg0(int16_t, d, getTotal);
                for (int j = 0; j < 4; ++j) {
                    r->durationByType[j] = msg0(int16_t, d, timeSels[j]);
                    r->weightArray[j] = msg0(int16_t, d, liftSels[j]);
                }

                r->cumulativeDuration[0] = results[i].durationByType[0];
                for (int j = 1; j < 4; ++j) {
                    r->cumulativeDuration[j] = r->cumulativeDuration[j - 1] + r->durationByType[j];
                }
                CFRelease(str);
            }
            handler(model, strs, results, count);
        }
    });
}

static void runStartupDataJob(id *cRef, void *model, FetchHandler handler, time_t weekStart, int tzDiff) {
    service = msg1(id, CFStringRef, Sels.alloc(objc_getClass("NSPersistentContainer"), Sels.alo),
                   sel_getUid("initWithName:"), CFSTR("HealthApp"));
    msg1(void, void(^)(id,id), service,
         sel_getUid("loadPersistentStoresWithCompletionHandler:"), ^(id desc _U_, id err _U_){});
    id context = msg0(id, service, sel_getUid("newBackgroundContext"));
    msg1(void, bool, context, sel_getUid("setAutomaticallyMergesChangesFromParent:"), true);
    *cRef = context;
#if DEBUG
    if (FIRST_LAUNCH) createDummyData(context);
#endif
    ((void(*)(id,SEL,Callback))objc_msgSend)(context, sel_getUid("performBlock:"), ^{
        Class WeekData = objc_getClass("WeekData");
        SEL liftSels[4];
        SEL getStart = sel_getUid("weekStart"), delObj = sel_getUid("deleteObject:");
        SEL weekInit = sel_getUid("initWithContext:"), setStart = sel_getUid("setWeekStart:");
        const time_t endPt = weekStart - 63244800;
        time_t start;
        int count = 0;
        CFArrayRef data = context_fetchData(context, 1, &count);
        if (!data) {
            id first = msg1(id, id, clsF0(id, WeekData, Sels.alo), weekInit, context);
            msg1(void, int64_t, first, setStart, weekStart);
            msg0(void, first, Sels.rel);
            context_saveChanges(context);
            goto cleanup;
        }

        if (tzDiff) {
            for (int i = 0; i < count; ++i) {
                id d = (id)CFArrayGetValueAtIndex(data, i);
                start = msg0(int64_t, d, getStart) + tzDiff;
                msg1(void, int64_t, d, setStart, start);
            }
        }

        id last = (id)CFArrayGetValueAtIndex(data, count - 1);
        int16_t lastLifts[4];
        for (int i = 0; i < 4; ++i) {
            liftSels[i] = sel_getUid(liftSets[i]);
            lastLifts[i] = msg0(int16_t, last, sel_getUid(liftGets[i]));
        }
        start = msg0(int64_t, last, getStart);
        if (start != weekStart) {
            id currWeek = msg1(id, id, clsF0(id, WeekData, Sels.alo), weekInit, context);
            msg1(void, int64_t, currWeek, setStart, weekStart);
            for (int j = 0; j < 4; ++j) {
                msg1(void, int16_t, currWeek, liftSels[j], lastLifts[j]);
            }
            msg0(void, currWeek, Sels.rel);
        }

        for (int i = 0; i < count; ++i) {
            id d = (id)CFArrayGetValueAtIndex(data, i);
            time_t tmpStart = msg0(int64_t, d, getStart);
            if (tmpStart < endPt)
                msg1(void, id, context, delObj, d);
        }

        start += WeekSeconds;
        for (; start < weekStart; start += WeekSeconds) {
            id curr = msg1(id, id, clsF0(id, WeekData, Sels.alo), weekInit, context);
            msg1(void, int64_t, curr, setStart, start);
            for (int j = 0; j < 4; ++j) {
                msg1(void, int16_t, curr, liftSels[j], lastLifts[j]);
            }
            msg0(void, curr, Sels.rel);
        }
        context_saveChanges(context);

    cleanup:
        fetchHistory(context, model, handler);
    });
}

#pragma mark - VC Helpers

static int getStatusBarStyle(id self _U_, SEL _cmd _U_) { return barStyle; }

static void setupBarGeneric(id bar, Class appearanceClass, id color) {
    if (appearanceClass) {
        SEL scrollEdge = sel_getUid("setScrollEdgeAppearance:");
        id appearance = Sels.new(appearanceClass, Sels.nw);
        msg0(void, appearance, sel_getUid("configureWithOpaqueBackground"));
        msg1(void, id, appearance, sel_getUid("setBackgroundColor:"), color);
        msg1(void, id, bar, sel_getUid("setStandardAppearance:"), appearance);
        if (msg1(bool, SEL, bar, sel_getUid("respondsToSelector:"), scrollEdge))
            msg1(void, id, bar, scrollEdge, appearance);
        Sels.objRel(appearance, Sels.rel);
    } else {
        msg1(void, bool, bar, sel_getUid("setTranslucent:"), false);
        msg1(void, id, bar, sel_getUid("setBarTintColor:"), color);
    }
}

static void setupTabVC(AppDelegate *self, id vc, Class TabAppear) {
    const void *keys[] = {NSForegroundColorAttributeName};
    const void *vals[] = {self->clr.getColor(self->clr.cls, self->clr.sc, ColorLabel)};
    CFDictionaryRef dict = createDict(keys, vals, 1, NULL);
    Class NavAppear = objc_getClass("UINavigationBarAppearance");
    id tabBar = msg0(id, vc, sel_getUid("tabBar"));
    id barColor = clsF1(id, int, self->clr.cls, sel_getUid("getBarColorWithType:"), 0);
    SEL nb = sel_getUid("navigationBar");
    setupBarGeneric(tabBar, TabAppear, barColor);
    if (!TabAppear)
        msg1(void, id, tabBar, sel_getUid("setUnselectedItemTintColor:"),
             self->clr.getColor(self->clr.cls, self->clr.sc, ColorGray));
    CFArrayRef ctrls = msg0(CFArrayRef, vc, sel_getUid("viewControllers"));
    for (int i = 0; i < 3; ++i) {
        id navVC = (id)CFArrayGetValueAtIndex(ctrls, i);
        id navBar = msg0(id, navVC, nb);
        setupBarGeneric(navBar, NavAppear, barColor);
        if (!TabAppear) {
            msg1(void, CFDictionaryRef, navBar, sel_getUid("setTitleTextAttributes:"), dict);
            msg0(void, navVC, sel_getUid("setNeedsStatusBarAppearanceUpdate"));
        }
    }
    CFRelease(dict);
}

void presentVC(id child) {
    id window = getAppDel()->window;
    msg1(void, id, window, sel_getUid("setTintColor:"), nil);
    msg3(void, id, bool, id, msg0(id, window, sel_getUid("rootViewController")),
         sel_getUid("presentViewController:animated:completion:"), child, true, nil);
}

void presentModalVC(id modal) {
    id nav = msg1(id, id, Sels.alloc(DMNavVC, Sels.alo), sel_getUid("initWithRootViewController:"), modal);
    setupBarGeneric(msg0(id, nav, sel_getUid("navigationBar")), objc_getClass("UINavigationBarAppearance"),
                    clsF1(id, int, objc_getClass("UIColor"), sel_getUid("getBarColorWithType:"), 1));
    presentVC(nav);
    Sels.vcRel(nav, Sels.rel);
    Sels.vcRel(modal, Sels.rel);
}

void dismissPresentedVC(Callback handler) {
    AppDelegate *self = getAppDel();
    id window = self->window;
    msg2(void, bool, Callback, msg0(id, window, sel_getUid("rootViewController")),
         sel_getUid("dismissViewControllerAnimated:completion:"), true, ^{
        if (handler)
            handler();
        msg1(void, id, window, sel_getUid("setTintColor:"),
             self->clr.getColor(self->clr.cls, self->clr.sc, ColorRed));
    });
}

id createAlertController(CFStringRef title, CFStringRef message) {
    id ctrl = clsF2(id, CFStringRef, CFStringRef, objc_getClass("UIAlertController"),
                    sel_getUid("getCtrlWithTitle:message:"), title, message);
    CFRelease(title);
    CFRelease(message);
    return ctrl;
}

static id alertCtrlCreate(id self _U_, SEL _cmd _U_, CFStringRef title, CFStringRef message) {
    return clsF3(id, CFStringRef, CFStringRef, long, objc_getClass("UIAlertController"),
                 sel_getUid("alertControllerWithTitle:message:preferredStyle:"), title, message, 1);
}

static id alertCtrlCreateLegacy(id self, SEL _cmd _U_, CFStringRef title, CFStringRef message) {
    AppDelegate *del = getAppDel();
    id vc = alertCtrlCreate(self, nil, title, message);
    id fg = del->clr.getColor(del->clr.cls, del->clr.sc, ColorLabel);
    const void *keys[] = {NSForegroundColorAttributeName, NSFontAttributeName};
    Class Font = objc_getClass("UIFont");
    SEL getFont = sel_getUid("systemFontOfSize:weight:");
    const void *titleVals[] = {fg, clsF2(id, CGFloat, CGFloat, Font, getFont, 17, UIFontWeightSemibold)};
    const void *msgVals[] = {fg, clsF2(id, CGFloat, CGFloat, Font, getFont, 13, UIFontWeightRegular)};
    CFDictionaryRef titleDict = createDict(keys, titleVals, 2, &kCFTypeDictionaryValueCallBacks);
    CFDictionaryRef msgDict = createDict(keys, msgVals, 2, &kCFTypeDictionaryValueCallBacks);
    CFAttributedStringRef titleString = CFAttributedStringCreate(NULL, title, titleDict);
    CFAttributedStringRef msgString = CFAttributedStringCreate(NULL, message, msgDict);
    SEL sKV = sel_getUid("setValue:forKey:"), gsv = sel_getUid("subviews");
    msg2(void, id, CFStringRef, vc, sKV, (id)titleString, CFSTR("attributedTitle"));
    msg2(void, id, CFStringRef, vc, sKV, (id)msgString, CFSTR("attributedMessage"));

    id view = msg0(id, vc, sel_getUid("view"));
    for (int i = 0; i < 3; ++i) {
        view = (id)CFArrayGetValueAtIndex(msg0(CFArrayRef, view, gsv), 0);
    }
    del->tbl.view.setBG(view, del->tbl.view.sbg,
                        del->clr.getColor(del->clr.cls, del->clr.sc, ColorTertiaryBG));

    CFRelease(titleDict);
    CFRelease(msgDict);
    CFRelease(titleString);
    CFRelease(msgString);
    return vc;
}

void addAlertAction(id ctrl, CFStringRef title, int style, Callback handler) {
    id action = clsF3(id, CFStringRef, long, void(^)(id), objc_getClass("UIAlertAction"),
                      sel_getUid("actionWithTitle:style:handler:"), title, style, ^(id hdlr _U_) {
        if (handler)
            handler();
        AppDelegate *self = getAppDel();
        msg1(void, id, self->window, sel_getUid("setTintColor:"),
             self->clr.getColor(self->clr.cls, self->clr.sc, ColorRed));
    });
    msg1(void, id, ctrl, sel_getUid("addAction:"), action);
    CFRelease(title);
}

#pragma mark - App Delegate Main Funcs

static void handleFirstLaunch(AppDelegate *self, CFStringRef key, time_t start, int tzOffset, bool modern) {
    CFNumberRef hasLaunched = CFNumberCreate(NULL, kCFNumberCharType, &(bool){true});
    UserInfo data = {start, start, {0}, modern ? 0xff : 0, 0xff, 0};
    const void *values[] = {
        CFNumberCreate(NULL, kCFNumberLongType, &start),
        CFNumberCreate(NULL, kCFNumberLongType, &start),
        CFNumberCreate(NULL, kCFNumberIntType, &tzOffset),
        CFNumberCreate(NULL, kCFNumberCharType, &(unsigned char){0xff}),
        CFNumberCreate(NULL, kCFNumberCharType, &(unsigned char){0}),
        CFNumberCreate(NULL, kCFNumberCharType, &data.darkMode),
        CFNumberCreate(NULL, kCFNumberShortType, &(short){0}),
        CFNumberCreate(NULL, kCFNumberShortType, &(short){0}),
        CFNumberCreate(NULL, kCFNumberShortType, &(short){0}),
        CFNumberCreate(NULL, kCFNumberShortType, &(short){0})
    };
    CFDictionaryRef dict = createDict(DictKeys, values, 10, &kCFTypeDictionaryValueCallBacks);
    CFPreferencesSetAppValue(key, hasLaunched, kCFPreferencesCurrentApplication);
    CFPreferencesSetAppValue(dictKey, dict, kCFPreferencesCurrentApplication);
    CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
    CFRelease(dict);
    CFRelease(hasLaunched);
    for (int i = 0; i < 10; ++i) {
        CFRelease(values[i]);
    }
    memcpy(&self->userData, &data, sizeof(UserInfo));
    id center = clsF0(id, objc_getClass("UNUserNotificationCenter"), sel_getUid("currentNotificationCenter"));
    msg2(void, unsigned long, void(^)(bool,id), center,
         sel_getUid("requestAuthorizationWithOptions:completionHandler:"), 6, ^(bool g _U_, id e _U_){});
}

bool appDelegate_didFinishLaunching(AppDelegate *self, SEL _cmd _U_, id application _U_, id options _U_) {
    DMNavVC = objc_allocateClassPair(objc_getClass("UINavigationController"), "DMNavVC", 0);
    objc_registerClassPair(DMNavVC);
    Class TabAppear = objc_getClass("UITabBarAppearance");
    initNSData(TabAppear, &self->clr, (Class*[]){&View, &VC}, (size_t*[]){&ViewSize, &VCSize});
    initViewData(&self->tbl);
    UIApplication = objc_getClass("UIApplication");
    sharedApplication = sel_getUid("sharedApplication");
    delegate = sel_getUid("delegate");
    sharedAppImp = (id(*)(Class,SEL))getImpC(UIApplication, sharedApplication);
    delegateImp = (id(*)(id,SEL))getImpO(UIApplication, delegate);
    orientations = msg0(long, clsF0(id, objc_getClass("UIDevice"), sel_getUid("currentDevice")),
                        sel_getUid("userInterfaceIdiom")) == 1 ? 26 : 2;

    time_t now = time(NULL);
    struct tm tmInfo;
    localtime_r(&now, &tmInfo);
    int tzOffset = (int)tmInfo.tm_gmtoff, tzDiff = 0, week = 0;
    if (tmInfo.tm_wday != 1) {
        now = now - WeekSeconds + (((8 - tmInfo.tm_wday) % 7) * 86400);
        localtime_r(&now, &tmInfo);
    }
    time_t weekStart = now - ((tmInfo.tm_hour * 3600) + (tmInfo.tm_min * 60) + tmInfo.tm_sec);
    unsigned char dm;

    CFStringRef hlKey = CFSTR("hasLaunched");
    CFBooleanRef saved = CFPreferencesCopyAppValue(hlKey, kCFPreferencesCurrentApplication);
    if (saved != NULL) {
        CFRelease(saved);
        const int planLengths[] = {8, 13};
        int savedTzOffset;
        time_t savedWeekStart, planStart;
        unsigned char changes = 0, plan, completed;
        CFDictionaryRef dict = CFPreferencesCopyAppValue(dictKey, kCFPreferencesCurrentApplication);
        CFNumberGetValue(CFDictionaryGetValue(dict, DictKeys[0]), kCFNumberLongType, &savedWeekStart);
        CFNumberGetValue(CFDictionaryGetValue(dict, DictKeys[1]), kCFNumberLongType, &planStart);
        CFNumberGetValue(CFDictionaryGetValue(dict, DictKeys[2]), kCFNumberIntType, &savedTzOffset);
        CFNumberGetValue(CFDictionaryGetValue(dict, DictKeys[3]), kCFNumberCharType, &plan);
        CFNumberGetValue(CFDictionaryGetValue(dict, DictKeys[4]), kCFNumberCharType, &completed);
        CFNumberGetValue(CFDictionaryGetValue(dict, DictKeys[5]), kCFNumberCharType, &dm);

        if ((tzDiff = savedTzOffset - tzOffset)) {
            planStart += tzDiff;
            if (weekStart != savedWeekStart) {
                changes = 7;
                savedWeekStart += tzDiff;
            } else {
                changes = 6;
                tzDiff = 0;
            }
        }

        week = (int)((weekStart - planStart) / WeekSeconds);
        if (weekStart != savedWeekStart) {
            changes |= 17;
            completed = 0;

            if (!(plan & 128) && week >= planLengths[plan]) {
                if (plan == WorkoutPlanBaseBuilding) {
                    plan = WorkoutPlanContinuation;
                    changes |= 8;
                }
                planStart = weekStart;
                changes |= 2;
                week = 0;
            }
        }

        if (dm < 2 && TabAppear) {
            dm = 0xff;
            changes |= 32;
        }

        if (changes) {
            CFStringRef keys[6];
            CFNumberRef values[6];
            int nChanges = 0;

            if (changes & 1) {
                keys[0] = DictKeys[IWeekStart];
                values[nChanges++] = CFNumberCreate(NULL, kCFNumberLongType, &weekStart);
            }
            if (changes & 2) {
                keys[nChanges] = DictKeys[IPlanStart];
                values[nChanges++] = CFNumberCreate(NULL, kCFNumberLongType, &planStart);
            }
            if (changes & 4) {
                keys[nChanges] = DictKeys[ITzOffset];
                values[nChanges++] = CFNumberCreate(NULL, kCFNumberIntType, &tzOffset);
            }
            if (changes & 8) {
                keys[nChanges] = DictKeys[ICurrentPlan];
                values[nChanges++] = CFNumberCreate(NULL, kCFNumberCharType, &plan);
            }
            if (changes & 16) {
                keys[nChanges] = DictKeys[ICompletedWorkouts];
                values[nChanges++] = CFNumberCreate(NULL, kCFNumberCharType, &completed);
            }
            if (changes & 32) {
                keys[nChanges] = DictKeys[IDarkMode];
                values[nChanges++] = CFNumberCreate(NULL, kCFNumberCharType, &dm);
            }
            CFMutableDictionaryRef updates = CFDictionaryCreateMutableCopy(NULL, 10, dict);
            saveChanges(updates, keys, values, nChanges);
        }

        UserInfo data = {planStart, weekStart, {0}, dm, plan, completed};
        for (int i = 0; i < 4; ++i) {
            CFNumberGetValue(CFDictionaryGetValue(dict, DictKeys[6 + i]),
                             kCFNumberShortType, &data.liftMaxes[i]);
        }
        memcpy(&self->userData, &data, sizeof(UserInfo));
        CFRelease(dict);
    } else {
#if DEBUG
        FIRST_LAUNCH = true;
#endif
        handleFirstLaunch(self, hlKey, weekStart, tzOffset, TabAppear);
        dm = self->userData.darkMode;
    }

    CFBundleRef bundle = CFBundleGetMainBundle();
    initStatVData();
    initValidatorStrings(bundle);
    initExerciseData(week, bundle);
    initWorkoutStrings(bundle);

    IMP imps[4];
    if (TabAppear) {
        imps[0] = (IMP)setupWorkoutVC_numberOfComponents;
        imps[1] = (IMP)setupWorkoutVC_didSelectRow;
        imps[2] = (IMP)setupWorkoutVC_titleForRow;
        imps[3] = (IMP)alertCtrlCreate;
    } else {
        barStyle = dm ? 2 : 0;
        class_addMethod(DMNavVC, sel_getUid("preferredStatusBarStyle"), (IMP)getStatusBarStyle, "i@:");
        imps[0] = (IMP)setupWorkoutVC_numberOfComponentsLegacy;
        imps[1] = (IMP)setupWorkoutVC_didSelectRowLegacy;
        imps[2] = (IMP)setupWorkoutVC_attrTitleForRow;
        imps[3] = (IMP)alertCtrlCreateLegacy;
        setupAppColors(dm, false);
        toggleDarkModeForCharts(dm);
    }
    class_addMethod(SetupWorkoutVCClass, sel_getUid("numberOfComponentsInPickerView:"), imps[0], "q@:@");
    class_addMethod(SetupWorkoutVCClass, sel_getUid("pickerView:didSelectRow:inComponent:"),
                    imps[1], "v@:@qq");
    class_addMethod(SetupWorkoutVCClass, sel_getUid("pickerView:titleForRow:forComponent:"),
                    imps[2], "@@:@qq");
    class_addMethod(objc_getMetaClass("UIAlertController"),
                    sel_registerName("getCtrlWithTitle:message:"), imps[3], "@@:@@");

    void *fetchArg; FetchHandler fetchHandler;
    self->children[0] = homeVC_init(bundle, &self->tbl, &self->clr, weekStart + 43200);
    self->children[1] = historyVC_init(bundle, &fetchArg, &fetchHandler, &self->tbl, &self->clr);
    self->children[2] = settingsVC_init(&self->tbl, &self->clr);

    Class Item = objc_getClass("UITabBarItem"), Image = objc_getClass("UIImage");
    SEL itemInit = sel_getUid("initWithTitle:image:tag:"), setItem = sel_getUid("setTabBarItem:");
    SEL imgInit = sel_getUid("imageNamed:"), navInit = sel_getUid("initWithRootViewController:");
    id controllers[3];
    CFStringRef imgNames[] = {CFSTR("ico_house"), CFSTR("ico_chart"), CFSTR("ico_gear")};
    CFStringRef titles[3];
    fillStringArray(bundle, titles, CFSTR("tabs%d"), 3);

    for (int i = 0; i < 3; ++i) {
        id img = clsF1(id, CFStringRef, Image, imgInit, imgNames[i]);
        id item = msg3(id, CFStringRef, id, long, Sels.alloc(Item, Sels.alo), itemInit, titles[i], img, i);
        controllers[i] = msg1(id, id, Sels.alloc(DMNavVC, Sels.alo), navInit, self->children[i]);
        msg1(void, id, controllers[i], setItem, item);
        Sels.objRel(item, Sels.rel);
        CFRelease(titles[i]);
    }

    CGRect bounds;
    getScreenBounds(&bounds);
    self->window = msg1(id, CGRect, Sels.alloc(objc_getClass("UIWindow"), Sels.alo),
                        sel_getUid("initWithFrame:"), bounds);
    msg1(void, id, self->window, sel_getUid("setTintColor:"),
         self->clr.getColor(self->clr.cls, self->clr.sc, ColorRed));
    id tabVC = Sels.new(objc_getClass("UITabBarController"), Sels.nw);
    msg1(void, id, self->window, sel_getUid("setRootViewController:"), tabVC);
    CFArrayRef array = CFArrayCreate(NULL, (const void **)controllers, 3, NULL);
    msg2(void, CFArrayRef, bool, tabVC, sel_getUid("setViewControllers:animated:"), array, false);
    setupTabVC(self, tabVC, TabAppear);
    for (int i = 0; i < 3; ++i) {
        Sels.vcRel(controllers[i], Sels.rel);
    }
    CFRelease(array);
    Sels.vcRel(tabVC, Sels.rel);
    msg0(void, self->window, sel_getUid("makeKeyAndVisible"));
    runStartupDataJob(&self->context, fetchArg, fetchHandler, weekStart, tzDiff);
    return true;
}

int appDelegate_supportedOrientations(AppDelegate *self _U_, SEL _cmd _U_,
                                      id application _U_, id window _U_) { return orientations; }

#pragma mark - Child VC Callbacks

UserInfo const *getUserInfo(void) { return &getAppDel()->userData; }

static int updateWeight(short *curr, short *weights, CFStringRef *keys, CFNumberRef *values) {
    int nChanges = 0;
    for (int i = 0, keyIndex = ILiftArray; i < 4; ++i, ++keyIndex) {
        short new = weights[i];
        if (new > curr[i]) {
            curr[i] = new;
            keys[nChanges] = DictKeys[keyIndex];
            values[nChanges++] = CFNumberCreate(NULL, kCFNumberShortType, &new);
        }
    }
    return nChanges;
}

void updateUserInfo(unsigned char plan, unsigned char darkMode, short *weights) {
    AppDelegate *self = getAppDel();
    int nChanges = 0;
    CFStringRef keys[7];
    CFNumberRef values[7];
    unsigned char changes = plan != self->userData.currentPlan ? 1 : 0;
    if (changes) {
        keys[0] = DictKeys[ICurrentPlan];
        values[nChanges++] = CFNumberCreate(NULL, kCFNumberCharType, &plan);
        self->userData.currentPlan = plan;
        if (!(plan & 128)) {
#if TARGET_OS_SIMULATOR
            time_t newPlanStart = self->userData.weekStart;
            exerciseManager_setCurrentWeek(0);
#else
            time_t newPlanStart = self->userData.weekStart + WeekSeconds;
#endif
            keys[1] = DictKeys[IPlanStart];
            values[nChanges++] = CFNumberCreate(NULL, kCFNumberLongType, &newPlanStart);
            self->userData.planStart = newPlanStart;
        }
    }

    if (darkMode != self->userData.darkMode) {
        changes |= 2;
        keys[nChanges] = DictKeys[IDarkMode];
        values[nChanges++] = CFNumberCreate(NULL, kCFNumberCharType, &darkMode);
        self->userData.darkMode = darkMode;
    }

    nChanges += updateWeight(self->userData.liftMaxes, weights, &keys[nChanges], &values[nChanges]);
    if (nChanges)
        saveChanges(createMutableDict(), keys, values, nChanges);

    if (changes & 2) {
        barStyle = darkMode ? 2 : 0;
        setupAppColors(darkMode, true);
        msg1(void, id, self->window, sel_getUid("setTintColor:"),
             self->clr.getColor(self->clr.cls, self->clr.sc, ColorRed));
        setupTabVC(self, msg0(id, self->window, sel_getUid("rootViewController")),
                   objc_getClass("UITabBarAppearance"));
        toggleDarkModeForCharts(darkMode);
        homeVC_updateColors(self->children[0]);
        if (msg0(bool, self->children[1], sel_getUid("isViewLoaded")))
            historyVC_updateColors(self->children[1], darkMode);
        settingsVC_updateColors(self->children[2], darkMode);
    }
    if (changes & 1)
        homeVC_createWorkoutsList(self->children[0], &self->userData);
}

void deleteAppData(void) {
    AppDelegate *self = getAppDel();
    if (self->userData.completedWorkouts) {
        self->userData.completedWorkouts = 0;
        CFStringRef keys[] = {DictKeys[ICompletedWorkouts]};
        CFNumberRef values[] = {CFNumberCreate(NULL, kCFNumberCharType, &(unsigned char){0})};
        saveChanges(createMutableDict(), keys, values, 1);
        homeVC_updateWorkoutsList((HomeVC *)((char *)self->children[0] + VCSize), 0);
    }
    historyVC_clearData(self->children[1]);
    id context = self->context;
    ((void(*)(id,SEL,Callback))objc_msgSend)(context, sel_getUid("performBlock:"), ^{
        SEL delObj = sel_getUid("deleteObject:");
        int count = 0;
        CFArrayRef data = context_fetchData(context, 1, &count);
        if (data) {
            int end = count - 1;
            for (int i = 0; i < end; ++i) {
                msg1(void, id, context, delObj, (id)CFArrayGetValueAtIndex(data, i));
            }
            id currWeek = (id)CFArrayGetValueAtIndex(data, end);
            weekData_setTotalWorkouts(currWeek, 0);
            for (int i = 0; i < 4; ++i) {
                msg1(void, int16_t, currWeek, sel_getUid(timeSets[i]), 0);
            }
            context_saveChanges(context);
        }
    });
}

unsigned char addWorkoutData(unsigned char day, int type, int16_t duration, short *weights) {
    AppDelegate *self = getAppDel();
    unsigned char completed = 0;
    CFStringRef keys[5];
    CFNumberRef values[5];
    int nChanges = 0;
    if (weights)
        nChanges = updateWeight(self->userData.liftMaxes, weights, keys, values);
    if (day != 0xff) {
        completed = self->userData.completedWorkouts;
        completed |= (1 << day);
        self->userData.completedWorkouts = completed;
        keys[nChanges] = DictKeys[ICompletedWorkouts];
        values[nChanges++] = CFNumberCreate(NULL, kCFNumberCharType, &completed);
    }
    if (nChanges) {
        saveChanges(createMutableDict(), keys, values, nChanges);
        id vc = self->children[2];
        if (msg0(bool, vc, sel_getUid("isViewLoaded")))
            inputVC_updateFields((InputVC *)((char *)vc + VCSize), self->userData.liftMaxes);
    }

    id context = self->context;
    ((void(*)(id,SEL,Callback))objc_msgSend)(context, sel_getUid("performBlock:"), ^{
        id data = (id)CFArrayGetValueAtIndex(context_fetchData(context, 2, &(int){0}), 0);
        int16_t newDuration = duration + msg0(int16_t, data, sel_getUid(timeGets[type]));
        msg1(void, int16_t, data, sel_getUid(timeSets[type]), newDuration);
        int16_t totalWorkouts = msg0(int16_t, data, sel_getUid("totalWorkouts")) + 1;
        weekData_setTotalWorkouts(data, totalWorkouts);
        if (weights) {
            for (int i = 0; i < 4; ++i) {
                msg1(void, int16_t, data, sel_getUid(liftSets[i]), weights[i]);
            }
            free(weights);
        }
        context_saveChanges(context);
    });
    return completed;
}
