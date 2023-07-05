#include "WorkoutVC.h"
#include "AppDelegate.h"
#include "CollectionVC.h"
#include "HeaderView.h"
#include "UpdateMaxesVC.h"
#include "WorkoutVC_Ext.h"

extern CFStringRef UIApplicationDidBecomeActiveNotification;
extern uint32_t UIAccessibilityLayoutChangedNotification;

Class WorkoutVCClass;

enum {
    EventFinishCircuit = 1,
    EventFinishExercise
};

static void handleEvent(id, CollectionVC *, WorkoutVC *, int, id);
static void restartTimers(CFNotificationCenterRef, void *,
                          CFStringRef, const void *, CFDictionaryRef);

void initWorkoutData(int week) {
    initNotificationCache();
    initExerciseData(week);
}

#pragma mark - VC init/free

id workoutVC_init(Workout *workout, CFMutableStringRef *headers) {
    int *itemCounts = malloc((unsigned)workout->size * sizeof(int));
    for (int i = 0; i < workout->size; ++i) {
        itemCounts[i] = workout->circuits[i].size;
    }
    id self = collectionVC_init(WorkoutVCClass, headers, workout->size, itemCounts);
    WorkoutVC *d = getIVVCC(WorkoutVC, CollectionVC, self);
    d->workout = workout;
    d->timers[0].type = 0;
    d->timers[1].type = 1;
    return self;
}

void workoutVC_deinit(id self, SEL _cmd) {
    WorkoutVC *d = getIVVCC(WorkoutVC, CollectionVC, self);

    for (int i = 0; i < d->workout->size; ++i) {
        Circuit *c = &d->workout->circuits[i];
        Exercise *eEnd = &c->exercises[c->size];
        for (Exercise *e = c->exercises; e < eEnd; ++e) {
            CFRelease(e->title);
            if (e->rest) CFRelease(e->rest);
            if (e->header) CFRelease(e->header);
            if (e->hint) CFRelease(e->hint);
        }
        free(c->exercises);
    }
    free(d->workout->circuits);
    free(d->workout);
    collectionVC_deinit(self, _cmd);
}

static void handleFinishedWorkout(WorkoutVC *d, bool pop) {
    int *lifts = NULL;
    if (d->weights[3]) {
        lifts = malloc(sizeof(int) << 2);
        memcpy(lifts, d->weights, sizeof(int) << 2);
        d->workout->duration = max(d->workout->duration, MinWorkoutDuration);
    }
    addWorkout(d->workout, d->workout->day, lifts, pop);
}

static void cleanupNotifications(id self, WorkoutVC *d) {
    id unCenter = getNotificationCenter();
    msgV(objSig(void), unCenter, sel_getUid("removeAllPendingNotificationRequests"));
    msgV(objSig(void), unCenter, sel_getUid("removeAllDeliveredNotifications"));
    CFNotificationCenterRemoveEveryObserver(CFNotificationCenterGetLocalCenter(), self);
    d->done = true;
    d->timers[0].active = d->timers[1].active = 0;
}

#pragma mark - Lifecycle

void workoutVC_viewDidLoad(id self, SEL _cmd) {
    collectionVC_viewDidLoad(self, _cmd);

    CollectionVC *p = getIVVC(CollectionVC, self);
    WorkoutVC *d = getIVVCS(WorkoutVC, p);
    CFStringRef titleKey = createWorkoutTitleKey(d->workout->type, d->workout->nameIdx);
    id startButton = createButton(localize(CFSTR("start")), ColorGreen, true, self, getTapSel());
    setupNavItem(self, titleKey, (id []){nil, startButton});
    CFRelease(titleKey);

    for (int i = 0; i < p->totalSections; ++i) {
        Circuit *c = &d->workout->circuits[i];
        for (int j = 0; j < c->size; ++j) {
            Exercise *e = &c->exercises[j];
            p->titles[i][j] = CFStringCreateCopy(NULL, e->title);
            if (e->header) p->labels[i][j] = CFStringCreateCopy(NULL, e->header);
        }
    }

    collectionVC_calculateHeights(p, -1);
    setDataSource(p->collectionView, self);
    setDelegate(p->collectionView, self);
    CFNotificationCenterAddObserver(CFNotificationCenterGetLocalCenter(), self, restartTimers,
                                    UIApplicationDidBecomeActiveNotification, NULL,
                                    CFNotificationSuspensionBehaviorDrop);
}

void workoutVC_startEndWorkout(id self, SEL _cmd _U_, id button) {
    CollectionVC *p = getIVVC(CollectionVC, self);
    WorkoutVC *d = getIVVCS(WorkoutVC, p);
    if (!getTag(button)) {
        CFStringRef newTitle = localize(CFSTR("end"));
        setTitle(button, newTitle, ControlStateNormal);
        CFRelease(newTitle);
        setTitleColor(button, getColor(ColorRed), ControlStateNormal);
        setTag(button, 1);
        d->workout->startTime = time(NULL);
        circuit_start(d->workout->circuits, d->timers, true);
        id nextView = nil;
        id indexPath = makeIndexPath(0, 0);
        id cell = cellForItem(p->collectionView, indexPath);
        if (cell) {
            StatusCell *v = getIVC(cell);
            statusCell_configure(v, d->workout->circuits->exercises);
            nextView = v->button;
        }

        if (p->headers[0]) {
            id header = supplementaryView(p->collectionView, indexPath);
            if (header) nextView = getIVR(header)->label;
        }

        if (nextView)
            UIAccessibilityPostNotification(UIAccessibilityLayoutChangedNotification, nextView);
    } else {
        cleanupNotifications(self, d);
        if (workout_isCompleted(d->workout)) {
            handleFinishedWorkout(d, true);
        } else {
            addWorkout(d->workout, UCHAR_MAX, NULL, true);
        }
    }
}

void workoutVC_viewWillDisappear(id self, SEL _cmd, bool animated) {
    msgSupV(supSig(void, bool), self, VC, _cmd, animated);

    if (msgV(objSig(bool), self, sel_getUid("isMovingFromParentViewController"))) {
        WorkoutVC *d = getIVVCC(WorkoutVC, CollectionVC, self);
        if (!d->done) {
            cleanupNotifications(self, d);
            if (d->workout->startTime) {
                if (workout_isCompleted(d->workout)) {
                    handleFinishedWorkout(d, false);
                } else {
                    addWorkout(d->workout, UCHAR_MAX, NULL, false);
                }
            }
        }
    }
}

#pragma mark - Collection Data Source

id workoutVC_cellForItemAtIndexPath(id self, SEL _cmd _U_, id collectionView, id indexPath) {
    WorkoutVC *d = getIVVCC(WorkoutVC, CollectionVC, self);
    long section = getSection(indexPath), item = getItem(indexPath);
    Exercise *exercise = &d->workout->circuits[section].exercises[item];
    CFStringRef cellId = exercise->header ? FullCellID : BasicCellID;
    id cell = dequeueReusableCell(collectionView, cellId, indexPath);
    StatusCell *v = getIVC(cell);
    if (exercise->header && d->workout->type == WorkoutEndurance)
        setTextColor(v->header, getColor(ColorGreen));

    statusCell_configure(v, exercise);
    setIDFormatted(cell, CFSTR("cell_%ld_%ld"), section, item)
    return cell;
}

#pragma mark - Collection Delegate

bool workoutVC_shouldSelectItem(id self, SEL _cmd _U_, id collectionView _U_, id indexPath) {
    WorkoutVC *d = getIVVCC(WorkoutVC, CollectionVC, self);
    return d->workout->circuits[getSection(indexPath)].exercises[getItem(indexPath)].interactive;
}

void workoutVC_didSelectItemAtIndexPath(id self, SEL _cmd _U_, id collectionView, id indexPath) {
    CollectionVC *p = getIVVC(CollectionVC, self);
    deselectItem(collectionView, indexPath);
    handleEvent(self, p, getIVVCS(WorkoutVC, p), 0, indexPath);
}

#pragma mark - Event Handling

void handleEvent(id self, CollectionVC *p, WorkoutVC *d, int type, id indexPath) {
    long section = getSection(indexPath), item = getItem(indexPath);
    Circuit *circuit = &d->workout->circuits[section];
    Exercise *exercise = &circuit->exercises[item];
    id cell = cellForItem(p->collectionView, indexPath);
    id pathToReload = nil, nextPath = nil;
    bool exerciseDone;
    int transition = 0;
    switch (type) {
        case EventFinishCircuit:
            transition = workout_increment(d->workout, d->timers);
            break;

        case EventFinishExercise:
            exercise->interactive = true;
            if (d->workout->type == WorkoutEndurance) {
                CFStringRef durationMsg = localize(CFSTR("exerciseDuration"));
                exercise->header = CFStringCreateMutableCopy(NULL, 128, durationMsg);
                p->labels[0][0] = durationMsg;
                pathToReload = nextPath = indexPath;
                break;
            }
        default:
            exerciseDone = exercise_cycle(exercise, d->timers);
            if (cell) statusCell_configure(getIVC(cell), exercise);
            if (exerciseDone) {
                transition = circuit_increment(circuit, d->timers, exercise);
                if (transition == TransitionFinishedCircuitDeleteFirst)
                    transition = workout_increment(d->workout, d->timers);
            }
    }

    bool isSectionPath = false;

    if (transition == TransitionCompletedWorkout) {
        cleanupNotifications(self, d);
        workout_setDuration(d->workout);
        if (UIAccessibilityIsVoiceOverRunning()) {
            CFStringRef msg = localize(CFSTR("workoutComplete"));
            dispatch_after(dispatch_time(0, 4000000000), dispatch_get_main_queue(), ^{
                UIAccessibilityPostNotification(UIAccessibilityAnnouncementNotification, (id)msg);
                CFRelease(msg);
            });
        }
        handleFinishedWorkout(d, true);
        return;
    } else if (transition == TransitionFinishedCircuitDeleteFirst) {
        ++p->firstSection;
        nextPath = makeIndexPath(0, section + 1);
        isSectionPath = true;
        id header = supplementaryView(p->collectionView, nextPath);
        if (header) setAlpha(getIVR(header)->divider, 0);
        cell = cellForItem(p->collectionView, nextPath);
        if (cell) statusCell_configure(getIVC(cell), (++circuit)->exercises);
    } else if (transition == TransitionFinishedCircuit) {
        if (circuit->reps > 1 && circuit->type == CircuitRounds) {
            CFLocaleRef locale = copyLocale();
            CFStringRef reps = formatStr(locale, CFSTR("%d"), circuit->completedReps + 1);
            updateRange(p->headers[section], &circuit->range, reps, locale);

            nextPath = makeIndexPath(0, section);
            isSectionPath = true;
            id header = supplementaryView(p->collectionView, nextPath);
            if (header) setText(getIVR(header)->label, p->headers[section]);
        }

        for (int i = 0; i < circuit->size; ++i) {
            cell = cellForItem(p->collectionView, makeIndexPath(i, section));
            if (cell) statusCell_configure(getIVC(cell), &circuit->exercises[i]);
        }

        if (!nextPath) nextPath = makeIndexPath(0, section);
    } else if (transition == TransitionFinishedExercise) {
        nextPath = makeIndexPath(circuit->index, section);
        cell = cellForItem(p->collectionView, nextPath);
        if (cell) statusCell_configure(getIVC(cell), &circuit->exercises[circuit->index]);
    } else if (d->workout->testMax) {
        presentModalVC(self, updateMaxesVC_init(self, (int)item));
    }

    if (transition == TransitionFinishedCircuitDeleteFirst) {
        reloadSections(p->collectionView, (u_long)section);
    } else if (pathToReload) {
        collectionVC_calculateHeights(p, -1);
        CFArrayRef paths = CFArrayCreate(NULL, (const void *[]){pathToReload}, 1, NULL);
        msgV(objSig(void, CFArrayRef), p->collectionView,
             sel_getUid("reloadItemsAtIndexPaths:"), paths);
        CFRelease(paths);
    }

    if (nextPath) {
        dispatch_after(dispatch_time(0, 100000000), dispatch_get_main_queue(), ^{
            id nextView = nil;
            if (isSectionPath) {
                id header = supplementaryView(p->collectionView, nextPath);
                if (header) nextView = getIVR(header)->label;
            } else {
                id nextCell = cellForItem(p->collectionView, nextPath);
                if (nextCell) nextView = getIVC(nextCell)->button;
            }

            if (nextView)
                UIAccessibilityPostNotification(UIAccessibilityLayoutChangedNotification, nextView);
        });
    }
}

#pragma mark - Notifications

void restartTimers(CFNotificationCenterRef center _U_, void *self,
                   CFStringRef name _U_, const void *object _U_, CFDictionaryRef userInfo _U_) {
    CollectionVC *p = getIVVC(CollectionVC, self);
    WorkoutVC *d = getIVVCS(WorkoutVC, p);
    time_t now = time(NULL);
    for (int i = 1; i >= 0; --i) {
        WorkoutTimer *t = &d->timers[i];
        if (t->active) {
            int diff = (int)(now - t->refTime);
            if (diff >= t->duration) {
                t->active = 0;
                handleEvent(self, p, d, i + 1, makeIndexPath(t->row, t->section));
            } else {
                workoutTimer_start(t, t->duration - diff, false);
            }
        }
    }
}

void workoutVC_receivedNotification(id self, int identifier) {
    CollectionVC *p = getIVVC(CollectionVC, self);
    WorkoutVC *d = getIVVCS(WorkoutVC, p);
    int type = identifier % 2;
    WorkoutTimer *timer = &d->timers[type];
    if (!timer->active || timer->identifier != identifier) return;

    timer->active = 0;
    if (type == TimerCircuit && d->timers[TimerExercise].active) {
        d->timers[TimerExercise].active = 0;
        CFStringRef idStr = formatStr(NULL, CFSTR("%d"), d->timers[TimerExercise].identifier);
        CFArrayRef ids = CFArrayCreate(NULL, (const void *[]){idStr}, 1, &kCFTypeArrayCallBacks);
        msgV(objSig(void, CFArrayRef), getNotificationCenter(),
             sel_getUid("removePendingNotificationRequestsWithIdentifiers:"), ids);
        CFRelease(idStr);
        CFRelease(ids);
    }

    handleEvent(self, p, d, type + 1, makeIndexPath(timer->row, timer->section));
}

#pragma mark - Modal Delegate

void workoutVC_finishedBottomSheet(id self, int index, int weight) {
    CollectionVC *p = getIVVC(CollectionVC, self);
    WorkoutVC *d = getIVVCS(WorkoutVC, p);
    d->weights[index] = weight;
    handleEvent(self, p, d, 0, makeIndexPath(index, 0));
}
