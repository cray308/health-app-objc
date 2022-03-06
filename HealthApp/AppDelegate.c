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
#define DaySeconds 86400

#if TARGET_OS_SIMULATOR
void exerciseManager_setCurrentWeek(int);
#endif
extern CGFloat UIFontWeightSemibold;
void setupAppColors(unsigned char darkMode, bool deleteOld);
void toggleDarkModeForCharts(bool);
void initValidatorStrings(CFBundleRef);
void initExerciseData(int, CFBundleRef);
void initWorkoutStrings(CFBundleRef);
id homeVC_init(CFBundleRef);
void homeVC_updateWorkoutsList(HomeVC *self, unsigned char completed);
void homeVC_createWorkoutsList(id self, const UserInfo *info);
void homeVC_updateColors(id self);
id historyVC_init(CFBundleRef, void**, void (**)(void*, CFArrayRef, WeekDataModel*, int));
void historyVC_clearData(id self);
void historyVC_updateColors(id vc, unsigned char darkMode);
id settingsVC_init(void);
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

Class AppDelegateClass;

enum {
    WorkoutPlanBaseBuilding, WorkoutPlanContinuation
};

enum {
    IWeekStart, IPlanStart, ITzOffset, ICurrentPlan, ICompletedWorkouts, IDarkMode, ILiftArray
};

static id service;
static CFStringRef const dictKey = CFSTR("userinfo");
static const void *DictKeys[] = {
    CFSTR("weekStart"), CFSTR("planStart"), CFSTR("tzOffset"),
    CFSTR("currentPlan"), CFSTR("completedWorkouts"), CFSTR("darkMode"),
    CFSTR("squatMax"), CFSTR("pullUpMax"), CFSTR("benchMax"), CFSTR("deadliftMax")
};
static const char *const liftGets[] = {"bestSquat", "bestPullup", "bestBench", "bestDeadlift"};
static const char *const timeGets[] = {"timeStrength", "timeSE", "timeEndurance", "timeHIC"};
static char const *const timeSets[] = {
    "setTimeStrength:", "setTimeSE:", "setTimeEndurance:", "setTimeHIC:"
};
static char const *const liftSets[] = {
    "setBestSquat:", "setBestPullup:", "setBestBench:", "setBestDeadlift:"
};
static int orientations;
static Class DMNavVC;
static Class UIApplication;
static Class Context;
static Class WeekData;
static Class AlertCtrl;
static SEL sharedApplication;
static SEL delegate;

static AppDelegate *getAppDel(void) {
    id app = clsF0(id, UIApplication, sharedApplication);
    return (AppDelegate *) msg0(id, app, delegate);
}

#pragma mark - User Defaults Helpers

static inline time_t getStartOfDay(time_t date, struct tm *info) {
    const int seconds = (info->tm_hour * 3600) + (info->tm_min * 60) + info->tm_sec;
    return date - seconds;
}

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
    CFArrayRef data = msg2(CFArrayRef, id, id, context,
                           sel_getUid("executeFetchRequest:error:"), req, nil);
    if (!(data && (len = (int)(CFArrayGetCount(data)))))
        data = NULL;
    *count = len;
    return data;
}

#if DEBUG
static void createDummyData(id context) {
    msg1(void, Callback, context, sel_getUid("performBlock:"), (^{
        int16_t lifts[] = {300, 20, 185, 235};
        int i = 0, r = 0;
        unsigned char plan = 0;
        time_t endpts[] = {time(NULL) - 126489600, time(NULL) - 2678400};
        for (int x = 0; x < 2; ++x) {
            time_t date = endpts[x];
            struct tm info;
            localtime_r(&date, &info);
            int weekday = info.tm_wday;
            if (weekday == 1) {
                int seconds = (info.tm_hour * 3600) + (info.tm_min * 60) + info.tm_sec;
                endpts[x] = date - seconds;
                continue;
            }
            date -= WeekSeconds;
            while (weekday != 1) {
                date += 86400;
                weekday = weekday == 6 ? 0 : weekday + 1;
            }
            localtime_r(&date, &info);
            int seconds = ((info.tm_hour * 3600) + (info.tm_min * 60) + info.tm_sec);
            endpts[x] = date - seconds;
        }
        while (endpts[0] < endpts[1]) {
            int16_t totalWorkouts = 0;
            int16_t times[4] = {0};
            id data = msg1(id, id, allocClass(WeekData), sel_getUid("initWithContext:"), context);
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
            releaseObj(data);

            if (++i == 52) {
                i = 0;
                plan = 0;
                r += 1;
            }
            endpts[0] += WeekSeconds;
        }
        context_saveChanges(context);
    }));
}
#endif

static void fetchHistory(id context,
                         void *model, void (*handler)(void*, CFArrayRef, WeekDataModel*, int)) {
    msg1(void, Callback, context, sel_getUid("performBlock:"), (^{
        struct tm localInfo;
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
            WeekDataModel *results = malloc((unsigned) count * sizeof(WeekDataModel));
            customAssert(count > 0)
            for (int i = 0; i < count; ++i) {
                id d = (id) CFArrayGetValueAtIndex(data, i);
                WeekDataModel *r = &results[i];
                time_t timestamp = msg0(int64_t, d, getStart);
                localtime_r(&timestamp, &localInfo);
                CFStringRef str = CFStringCreateWithFormat(NULL, NULL, CFSTR("%d/%d/%d"),
                                                           localInfo.tm_mon + 1, localInfo.tm_mday,
                                                           localInfo.tm_year % 100);
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
    }));
}

#pragma mark - VC Helpers

#define getRootVC(w) msg0(id,w,sel_getUid("rootViewController"))

static int getStatusBarStyle(id self _U_, SEL _cmd _U_) { return 0; }

static int getStatusBarStyleDark(id self _U_, SEL _cmd _U_) { return 2; }

static void setupBarGeneric(id bar, Class appearanceClass, bool modal) {
    id color = getBarColor(modal);
    if (appearanceClass) {
        SEL scrollEdge = sel_getUid("setScrollEdgeAppearance:");
        id appearance = createNew(appearanceClass);
        msg0(void, appearance, sel_getUid("configureWithOpaqueBackground"));
        setBackground(appearance, color);
        msg1(void, id, bar, sel_getUid("setStandardAppearance:"), appearance);
        if (msg1(bool, SEL, bar, sel_getUid("respondsToSelector:"), scrollEdge))
            msg1(void, id, bar, scrollEdge, appearance);
        releaseObj(appearance);
    } else {
        msg1(void, bool, bar, sel_getUid("setTranslucent:"), false);
        setBarTint(bar, color);
    }
}

static void setupTabVC(id vc) {
    const void *keys[] = {(CFStringRef) NSForegroundColorAttributeName};
    const void *vals[] = {createColor(ColorLabel)};
    CFDictionaryRef dict = CFDictionaryCreate(NULL, keys, vals, 1,
                                              &kCFCopyStringDictionaryKeyCallBacks, NULL);
    Class navBarAppearance = objc_getClass("UINavigationBarAppearance");
    id tabBar = msg0(id, vc, sel_getUid("tabBar"));
    SEL sNb = sel_getUid("navigationBar");
    id (*navBarImp)(id,SEL) = (id(*)(id,SEL)) getImpO(DMNavVC, sNb);
    setupBarGeneric(tabBar, objc_getClass("UITabBarAppearance"), false);
    if (!navBarAppearance)
        msg1(void, id, tabBar, sel_getUid("setUnselectedItemTintColor:"), createColor(ColorGray));
    CFArrayRef ctrls = getViewControllers(vc);
    for (int i = 0; i < 3; ++i) {
        id navVC = (id) CFArrayGetValueAtIndex(ctrls, i);
        id navBar = navBarImp(navVC, sNb);
        setupBarGeneric(navBar, navBarAppearance, false);
        if (!navBarAppearance) {
            msg1(void, CFDictionaryRef, navBar, sel_getUid("setTitleTextAttributes:"), dict);
            msg0(void, navVC, sel_getUid("setNeedsStatusBarAppearanceUpdate"));
        }
    }
    CFRelease(dict);
}

void presentVC(id child) {
    id window = getAppDel()->window;
    setTintColor(window, nil);
    msg3(void, id, bool, id, getRootVC(window),
         sel_getUid("presentViewController:animated:completion:"), child, true, nil);
}

void presentModalVC(id modal) {
    id container = msg1(id, id, allocClass(DMNavVC),
                        sel_getUid("initWithRootViewController:"), modal);
    setupBarGeneric(msg0(id, container, sel_getUid("navigationBar")),
                    objc_getClass("UINavigationBarAppearance"), true);
    presentVC(container);
    SEL rel = sel_getUid("release");
    msg0(void, container, rel);
    msg0(void, modal, rel);
}

void dismissPresentedVC(Callback handler) {
    id window = getAppDel()->window;
    msg2(void, bool, Callback, getRootVC(window),
         sel_getUid("dismissViewControllerAnimated:completion:"), true, ^{
        if (handler)
            handler();
        setTintColor(window, createColor(ColorRed));
    });
}

id createAlertController(CFStringRef title, CFStringRef message) {
    id ctrl = clsF2(id, CFStringRef, CFStringRef, AlertCtrl,
                    sel_getUid("getCtrlWithTitle:message:"), title, message);
    CFRelease(title);
    CFRelease(message);
    return ctrl;
}

static id alertCtrlCreate(id self _U_, SEL _cmd _U_, CFStringRef title, CFStringRef message) {
    return clsF3(id, CFStringRef, CFStringRef, long, AlertCtrl,
                 sel_getUid("alertControllerWithTitle:message:preferredStyle:"), title, message, 1);
}

static id alertCtrlCreateLegacy(id self, SEL _cmd _U_, CFStringRef title, CFStringRef message) {
    SEL sColor = sel_getUid("getColorWithType:"), f = sel_getUid("systemFontOfSize:weight:");
    SEL sKV = sel_getUid("setValue:forKey:");
    void (*kvImp)(id,SEL,id,CFStringRef) = (void(*)(id,SEL,id,CFStringRef)) getImpO(AlertCtrl, sKV);
    id (*colorImp)(Class,SEL,int) = (id(*)(Class,SEL,int)) getImpC(ColorClass, sColor);
    id (*fontImp)(Class,SEL,CGFloat,CGFloat) = (id(*)(Class,SEL,CGFloat,CGFloat))getImpC(FontClass,f);
    id vc = alertCtrlCreate(self, nil, title, message);
    id foreground = colorImp(ColorClass, sColor, ColorLabel);
    const void *keys[] = {
        (CFStringRef) NSForegroundColorAttributeName, (CFStringRef) NSFontAttributeName
    };
    const void *titleVals[] = {foreground, fontImp(FontClass, f, 17, UIFontWeightSemibold)};
    const void *msgVals[] = {foreground, fontImp(FontClass, f, 13, UIFontWeightRegular)};
    CFDictionaryRef titleDict = CFDictionaryCreate(NULL, keys, titleVals, 2,
                                                   &kCFCopyStringDictionaryKeyCallBacks,
                                                   &kCFTypeDictionaryValueCallBacks);
    CFDictionaryRef msgDict = CFDictionaryCreate(NULL, keys, msgVals, 2,
                                                 &kCFCopyStringDictionaryKeyCallBacks,
                                                 &kCFTypeDictionaryValueCallBacks);
    CFAttributedStringRef titleString = CFAttributedStringCreate(NULL, title, titleDict);
    CFAttributedStringRef msgString = CFAttributedStringCreate(NULL, message, msgDict);
    kvImp(vc, sKV, (id) titleString, CFSTR("attributedTitle"));
    kvImp(vc, sKV, (id) msgString, CFSTR("attributedMessage"));

    id view = getView(vc);
    for (int i = 0; i < 3; ++i) {
        view = (id) CFArrayGetValueAtIndex(getSubviews(view), 0);
    }
    setBackground(view, colorImp(ColorClass, sColor, ColorTertiaryBG));

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
        setTintColor(getAppDel()->window, createColor(ColorRed));
    });
    msg1(void, id, ctrl, sel_getUid("addAction:"), action);
    CFRelease(title);
}

#pragma mark - App Delegate Main Funcs

bool appDelegate_didFinishLaunching(AppDelegate *self, SEL _cmd _U_,
                                    id application _U_, id options _U_) {
    DMNavVC = objc_allocateClassPair(objc_getClass("UINavigationController"), "DMNavVC", 0);
    objc_registerClassPair(DMNavVC);
    AlertCtrl = objc_getClass("UIAlertController");
    Context = objc_getClass("NSManagedObjectContext");
    WeekData = objc_getClass("WeekData");
    UIApplication = objc_getClass("UIApplication");
    sharedApplication = sel_getUid("sharedApplication");
    delegate = sel_getUid("delegate");
    SEL alo = sel_getUid("alloc"), rel = sel_getUid("release");
    id _pscontainer = clsF0(id, objc_getClass("NSPersistentContainer"), alo);
    service = msg1(id, CFStringRef, _pscontainer, sel_getUid("initWithName:"), CFSTR("HealthApp"));
    msg1(void, void(^)(id,id), service,
         sel_getUid("loadPersistentStoresWithCompletionHandler:"), ^(id desc _U_, id err _U_){});
    id context = msg0(id, service, sel_getUid("newBackgroundContext"));
    msg1(void, bool, context, sel_getUid("setAutomaticallyMergesChangesFromParent:"), true);
    self->context = context;

    time_t now = time(NULL);
    time_t weekStart;
    struct tm localInfo;
    localtime_r(&now, &localInfo);
    if (localInfo.tm_wday == 1) {
        weekStart = getStartOfDay(now, &localInfo);
    } else {
        time_t date = now - WeekSeconds;
        int weekday = localInfo.tm_wday;
        while (weekday != 1) {
            date += DaySeconds;
            weekday = weekday == 6 ? 0 : weekday + 1;
        }
        localtime_r(&date, &localInfo);
        weekStart = getStartOfDay(date, &localInfo);
    }
    struct tm gmtInfo;
    gmtime_r(&now, &gmtInfo);
    gmtInfo.tm_isdst = -1;
    int tzOffset = (int) (now - mktime(&gmtInfo));
    int tzDiff = 0, week = 0;
    unsigned char dm = 0;
    bool legacy = !objc_getClass("UITabBarAppearance");
    CFStringRef hasLaunchedKey = CFSTR("hasLaunched");
    CFBooleanRef saved = CFPreferencesCopyAppValue(hasLaunchedKey, kCFPreferencesCurrentApplication);

    if (saved == NULL) {
        CFNumberRef newValue = CFNumberCreate(NULL, kCFNumberCharType, &(bool){true});
        CFPreferencesSetAppValue(hasLaunchedKey, newValue, kCFPreferencesCurrentApplication);
        CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
        CFRelease(newValue);

        dm = legacy ? 0 : 0xff;
        UserInfo data = {weekStart, weekStart, {0}, dm, 0xff, 0};
        CFNumberRef values[] = {
            CFNumberCreate(NULL, kCFNumberLongType, &weekStart),
            CFNumberCreate(NULL, kCFNumberLongType, &weekStart),
            CFNumberCreate(NULL, kCFNumberIntType, &tzOffset),
            CFNumberCreate(NULL, kCFNumberCharType, &(unsigned char){0xff}),
            CFNumberCreate(NULL, kCFNumberCharType, &(unsigned char){0}),
            CFNumberCreate(NULL, kCFNumberCharType, &data.darkMode),
            CFNumberCreate(NULL, kCFNumberShortType, &(short){0}),
            CFNumberCreate(NULL, kCFNumberShortType, &(short){0}),
            CFNumberCreate(NULL, kCFNumberShortType, &(short){0}),
            CFNumberCreate(NULL, kCFNumberShortType, &(short){0})
        };
        CFDictionaryRef dict = CFDictionaryCreate(NULL, DictKeys, (const void **)values, 10,
                                                  &kCFCopyStringDictionaryKeyCallBacks,
                                                  &kCFTypeDictionaryValueCallBacks);
        CFPreferencesSetAppValue(dictKey, dict, kCFPreferencesCurrentApplication);
        CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
        CFRelease(dict);
        for (int i = 0; i < 10; ++i) {
            CFRelease(values[i]);
        }
        memcpy(&self->userData, &data, sizeof(UserInfo));
#if DEBUG
        createDummyData(context);
#endif
        msg2(void, unsigned long, void(^)(bool,id), getUserNotificationCenter(),
             sel_getUid("requestAuthorizationWithOptions:completionHandler:"),
             6, ^(bool granted _U_, id error _U_) {});
    } else {
        CFRelease(saved);
        const int planLengths[] = {8, 13};
        int savedTzOffset;
        time_t savedWeekStart, planStart;
        unsigned char changes = 0, plan, completed;
        CFDictionaryRef dict = CFPreferencesCopyAppValue(dictKey, kCFPreferencesCurrentApplication);

        CFNumberRef value = CFDictionaryGetValue(dict, DictKeys[0]);
        CFNumberGetValue(value, kCFNumberLongType, &savedWeekStart);
        value = CFDictionaryGetValue(dict, DictKeys[1]);
        CFNumberGetValue(value, kCFNumberLongType, &planStart);
        value = CFDictionaryGetValue(dict, DictKeys[2]);
        CFNumberGetValue(value, kCFNumberIntType, &savedTzOffset);
        value = CFDictionaryGetValue(dict, DictKeys[3]);
        CFNumberGetValue(value, kCFNumberCharType, &plan);
        value = CFDictionaryGetValue(dict, DictKeys[4]);
        CFNumberGetValue(value, kCFNumberCharType, &completed);
        value = CFDictionaryGetValue(dict, DictKeys[5]);
        CFNumberGetValue(value, kCFNumberCharType, &dm);

        if ((tzDiff = savedTzOffset - tzOffset)) {
            changes = 7;
            savedWeekStart += tzDiff;
            planStart += tzDiff;
        }

        week = (int) ((weekStart - planStart) / WeekSeconds);
        if (weekStart != savedWeekStart) {
            changes |= 17;
            completed = 0;
            savedWeekStart = weekStart;

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

        if (!(dm & 128) && !legacy) {
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
            value = CFDictionaryGetValue(dict, DictKeys[6 + i]);
            CFNumberGetValue(value, kCFNumberShortType, &data.liftMaxes[i]);
        }
        memcpy(&self->userData, &data, sizeof(UserInfo));
        CFRelease(dict);
    }

    CFBundleRef bundle = CFBundleGetMainBundle();
    initValidatorStrings(bundle);
    initExerciseData(week, bundle);
    initWorkoutStrings(bundle);
    char const *colorCreateSig = "@@:i";
    Class colorMeta = objc_getMetaClass("UIColor");
    Class alertMeta = objc_getMetaClass("UIAlertController");
    if (legacy) {
        IMP imp = (IMP) (dm ? getStatusBarStyleDark : getStatusBarStyle);
        class_addMethod(DMNavVC, sel_getUid("preferredStatusBarStyle"), imp, "i@:");
        class_addMethod(SetupWorkoutVCClass, sel_getUid("numberOfComponentsInPickerView:"),
                        (IMP) setupWorkoutVC_numberOfComponentsLegacy, "q@:@");
        class_addMethod(SetupWorkoutVCClass, sel_getUid("pickerView:didSelectRow:inComponent:"),
                        (IMP) setupWorkoutVC_didSelectRowLegacy, "v@:@qq");
        class_addMethod(SetupWorkoutVCClass,
                        sel_getUid("pickerView:attributedTitleForRow:forComponent:"),
                        (IMP) setupWorkoutVC_attrTitleForRow, "@@:@qq");
        class_addMethod(colorMeta, sel_getUid("getColorWithType:"),
                        (IMP) colorCreateLegacy, colorCreateSig);
        class_addMethod(colorMeta, sel_getUid("getBarColorWithType:"),
                        (IMP) barColorCreateLegacy, colorCreateSig);
        class_addMethod(alertMeta, sel_getUid("getCtrlWithTitle:message:"),
                        (IMP) alertCtrlCreateLegacy, "@@:@@");
        setupAppColors(dm, false);
        toggleDarkModeForCharts(dm);
    } else {
        class_addMethod(SetupWorkoutVCClass, sel_getUid("numberOfComponentsInPickerView:"),
                        (IMP) setupWorkoutVC_numberOfComponents, "q@:@");
        class_addMethod(SetupWorkoutVCClass, sel_getUid("pickerView:didSelectRow:inComponent:"),
                        (IMP) setupWorkoutVC_didSelectRow, "v@:@qq");
        class_addMethod(SetupWorkoutVCClass, sel_getUid("pickerView:titleForRow:forComponent:"),
                        (IMP) setupWorkoutVC_titleForRow, "@@:@qq");
        class_addMethod(colorMeta, sel_getUid("getColorWithType:"),
                        (IMP) colorCreate, colorCreateSig);
        class_addMethod(colorMeta, sel_getUid("getBarColorWithType:"),
                        (IMP) barColorCreate, colorCreateSig);
        class_addMethod(alertMeta, sel_getUid("getCtrlWithTitle:message:"),
                        (IMP) alertCtrlCreate, "@@:@@");
    }

    id device = clsF0(id, objc_getClass("UIDevice"), sel_getUid("currentDevice"));
    orientations = msg0(long, device, sel_getUid("userInterfaceIdiom")) == 1 ? 26 : 2;
    CGRect bounds;
    getScreenBounds(&bounds);
    self->window = msg1(id, CGRect, clsF0(id, objc_getClass("UIWindow"), alo),
                        sel_getUid("initWithFrame:"), bounds);
    setTintColor(self->window, createColor(ColorRed));
    id tabVC = createNew(objc_getClass("UITabBarController"));
    msg1(void, id, self->window, sel_getUid("setRootViewController:"), tabVC);
    void (*fetchHandler)(void*, CFArrayRef, WeekDataModel*, int);
    void *fetchArg;
    Class itemClass = objc_getClass("UITabBarItem"), imgClass = objc_getClass("UIImage");
    SEL itemInit = sel_getUid("initWithTitle:image:tag:"), setItem = sel_getUid("setTabBarItem:");
    SEL imgInit = sel_getUid("imageNamed:"), navInit = sel_getUid("initWithRootViewController:");
    id (*imgInitImp)(Class,SEL,CFStringRef) = (id(*)(Class,SEL,CFStringRef)) getImpC(imgClass, imgInit);
    id (*iInitImp)(id,SEL,CFStringRef,id,long) = (id(*)(id,SEL,CFStringRef,id,long)) getImpO(itemClass, itemInit);
    void (*iSetImp)(id,SEL,id) = (void(*)(id,SEL,id)) getImpO(DMNavVC, setItem);
    id (*navInitImp)(id,SEL,id) = (id(*)(id,SEL,id)) getImpO(DMNavVC, navInit);
    id controllers[3];
    CFStringRef imgNames[] = {CFSTR("ico_house"), CFSTR("ico_chart"), CFSTR("ico_gear")};
    CFStringRef titles[3];
    fillStringArray(bundle, titles, CFSTR("tabs%d"), 3);
    self->children[0] = homeVC_init(bundle);
    self->children[1] = historyVC_init(bundle, &fetchArg, &fetchHandler);
    self->children[2] = settingsVC_init();

    for (int i = 0; i < 3; ++i) {
        id img = imgInitImp(imgClass, imgInit, imgNames[i]);
        id item = iInitImp(clsF0(id, itemClass, alo), itemInit, titles[i], img, i);
        controllers[i] = navInitImp(clsF0(id, DMNavVC, alo), navInit, self->children[i]);
        iSetImp(controllers[i], setItem, item);
        msg0(void, item, rel);
        CFRelease(titles[i]);
    }

    CFArrayRef array = CFArrayCreate(NULL, (const void **)controllers, 3, NULL);
    msg2(void, CFArrayRef, bool, tabVC, sel_getUid("setViewControllers:animated:"), array, false);
    setupTabVC(tabVC);

    for (int i = 0; i < 3; ++i) {
        msg0(void, controllers[i], rel);
    }
    CFRelease(array);
    msg0(void, tabVC, rel);

    msg0(void, self->window, sel_getUid("makeKeyAndVisible"));
    msg1(void, Callback, context, sel_getUid("performBlock:"), (^{
        SEL liftSels[] = {sel_getUid(liftSets[0]), sel_getUid(liftSets[1]),
            sel_getUid(liftSets[2]), sel_getUid(liftSets[3])};
        SEL getStart = sel_getUid("weekStart"), delObj = sel_getUid("deleteObject:");
        SEL weekInit = sel_getUid("initWithContext:"), setStart = sel_getUid("setWeekStart:");
        void (*delImp)(id,SEL,id) = (void(*)(id,SEL,id)) getImpO(Context, delObj);
        const time_t endPt = weekStart - 63244800;
        time_t start;
        int count = 0;
        CFArrayRef data = context_fetchData(context, 1, &count);
        if (!data) {
            id first = msg1(id, id, clsF0(id, WeekData, alo), weekInit, context);
            msg1(void, int64_t, first, setStart, weekStart);
            msg0(void, first, rel);
            context_saveChanges(context);
            goto cleanup;
        }

        if (tzOffset) {
            for (int i = 0; i < count; ++i) {
                id d = (id) CFArrayGetValueAtIndex(data, i);
                start = msg0(int64_t, d, getStart) + tzOffset;
                msg1(void, int64_t, d, setStart, start);
            }
        }

        id last = (id) CFArrayGetValueAtIndex(data, count - 1);
        int16_t lastLifts[4];
        for (int i = 0; i < 4; ++i) {
            lastLifts[i] = msg0(int16_t, last, sel_getUid(liftGets[i]));
        }
        start = msg0(int64_t, last, getStart);
        if (start != weekStart) {
            id currWeek = msg1(id, id, clsF0(id, WeekData, alo), weekInit, context);
            msg1(void, int64_t, currWeek, setStart, weekStart);
            for (int j = 0; j < 4; ++j) {
                msg1(void, int16_t, currWeek, liftSels[j], lastLifts[j]);
            }
            msg0(void, currWeek, rel);
        }

        for (int i = 0; i < count; ++i) {
            id d = (id) CFArrayGetValueAtIndex(data, i);
            time_t tmpStart = msg0(int64_t, d, getStart);
            if (tmpStart < endPt)
                delImp(context, delObj, d);
        }

        start += WeekSeconds;
        for (; start < weekStart; start += WeekSeconds) {
            id curr = msg1(id, id, clsF0(id, WeekData, alo), weekInit, context);
            msg1(void, int64_t, curr, setStart, start);
            for (int j = 0; j < 4; ++j) {
                msg1(void, int16_t, curr, liftSels[j], lastLifts[j]);
            }
            msg0(void, curr, rel);
        }
        context_saveChanges(context);

    cleanup:
        fetchHistory(context, fetchArg, fetchHandler);
    }));
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
        Method method = class_getInstanceMethod(DMNavVC, sel_getUid("preferredStatusBarStyle"));
        IMP newImp = (IMP) (darkMode ? getStatusBarStyleDark : getStatusBarStyle);
        method_setImplementation(method, newImp);
        setupAppColors(darkMode, true);
        setTintColor(self->window, createColor(ColorRed));
        setupTabVC(getRootVC(self->window));
        toggleDarkModeForCharts(darkMode);
        homeVC_updateColors(self->children[0]);
        if (isViewLoaded(self->children[1]))
            historyVC_updateColors(self->children[1], darkMode);
        settingsVC_updateColors(self->children[2], darkMode);
    }
    if (changes & 1)
        homeVC_createWorkoutsList(self->children[0], &self->userData);
}

void deleteAppData(void) {
    AppDelegate *self = getAppDel();
    id *arr = self->children;
    bool updateHome = false;
    if (self->userData.completedWorkouts) {
        self->userData.completedWorkouts = 0;
        CFStringRef keys[] = {DictKeys[ICompletedWorkouts]};
        CFNumberRef values[] = {CFNumberCreate(NULL, kCFNumberCharType, &(unsigned char){0})};
        saveChanges(createMutableDict(), keys, values, 1);
        updateHome = true;
    }
    id context = self->context;
    msg1(void, Callback, context, sel_getUid("performBlock:"), ^{
        SEL delObj = sel_getUid("deleteObject:");
        void (*delImp)(id,SEL,id) = (void(*)(id,SEL,id)) getImpO(Context, delObj);
        int count = 0;
        CFArrayRef data = context_fetchData(context, 1, &count);
        if (data) {
            int end = count - 1;
            for (int i = 0; i < end; ++i) {
                delImp(context, delObj, (id) CFArrayGetValueAtIndex(data, i));
            }
            id currWeek = (id) CFArrayGetValueAtIndex(data, end);
            weekData_setTotalWorkouts(currWeek, 0);
            for (int i = 0; i < 4; ++i) {
                msg1(void, int16_t, currWeek, sel_getUid(timeSets[i]), 0);
            }
            context_saveChanges(context);
        }
    });
    if (updateHome)
        homeVC_updateWorkoutsList((HomeVC *) ((char *)arr[0] + VCSize), 0);
    historyVC_clearData(arr[1]);
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
        if (isViewLoaded(vc))
            inputVC_updateFields((InputVC *) ((char *)vc + VCSize), self->userData.liftMaxes);
    }

    id context = self->context;
    msg1(void, Callback, context, sel_getUid("performBlock:"), ^{
        CFArrayRef currentWeeks = context_fetchData(context, 2, &(int){0});
        id data = (id) CFArrayGetValueAtIndex(currentWeeks, 0);
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
