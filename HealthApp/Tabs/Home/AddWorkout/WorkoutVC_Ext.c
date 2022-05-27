#include "WorkoutVC_Ext.h"
#include "Views.h"

static CFStringRef notificationMessages[2];
static CFStringRef notifTitle;
static struct WorkoutCache {
    const Class Center, Content, Trigger, Request, Sound;
    const SEL dc, ds, scst, scsb, scss, newTri, newReq, addReq;
    id (*getCenter)(Class,SEL);
    id (*getSound)(Class,SEL);
    void (*setTitle)(id,SEL,CFStringRef);
    void (*setBody)(id,SEL,CFStringRef);
    void (*setSound)(id,SEL,id);
    id (*createTrigger)(Class,SEL,double,bool);
    id (*createReq)(Class,SEL,CFStringRef,id,id);
    void (*addToCenter)(id,SEL,id,void(^)(id));
} wc;

void initNotificationCache(void) {
    fillStringArray(notificationMessages, CFSTR("notifications%d"), 2);
    notifTitle = localize(CFSTR("workoutNotificationTitle"));
    Class Center = objc_getClass("UNUserNotificationCenter");
    Class Content = objc_getClass("UNMutableNotificationContent");
    Class Trigger = objc_getClass("UNTimeIntervalNotificationTrigger");
    Class Req = objc_getClass("UNNotificationRequest"), Sound = objc_getClass("UNNotificationSound");
    SEL ds = sel_getUid("defaultSound"), dc = sel_getUid("currentNotificationCenter");
    SEL newTri = sel_getUid("triggerWithTimeInterval:repeats:");
    SEL newReq = sel_getUid("requestWithIdentifier:content:trigger:");
    SEL addReq = sel_getUid("addNotificationRequest:withCompletionHandler:");
    SEL scst = sel_getUid("setTitle:"), scsb = sel_getUid("setBody:"), scss = sel_getUid("setSound:");
    memcpy(&wc, &(struct WorkoutCache){Center, Content, Trigger, Req, Sound, dc, ds,
        scst, scsb, scss, newTri, newReq, addReq, (id(*)(Class,SEL))getImpC(Center, dc),
        (id(*)(Class,SEL))getImpC(Sound, ds), (void(*)(id,SEL,CFStringRef))getImpO(Content, scst),
        (void(*)(id,SEL,CFStringRef))getImpO(Content, scsb),
        (void(*)(id,SEL,id))getImpO(Content, scss),
        (id(*)(Class,SEL,double,bool))getImpC(Trigger, newTri),
        (id(*)(Class,SEL,CFStringRef,id,id))getImpC(Req, newReq),
        (void(*)(id,SEL,id,void(^)(id)))getImpO(Center, addReq)
    }, sizeof(struct WorkoutCache));
}

void workoutTimer_start(WorkoutTimer *t, int duration, bool scheduleNotif) {
    static int identifiers[] = {0, 1};
    t->refTime = time(NULL);
    t->duration = duration;
    t->active = 1;
    if (!scheduleNotif) return;

    identifiers[t->type] += 2;
    int currentId = identifiers[t->type];
    t->identifier = currentId;
    CFStringRef idString = formatStr(NULL, CFSTR("%d"), currentId);
    id content = new(wc.Content);
    wc.setTitle(content, wc.scst, notifTitle);
    wc.setBody(content, wc.scsb, notificationMessages[t->type]);
    wc.setSound(content, wc.scss, wc.getSound(wc.Sound, wc.ds));
    id trigger = wc.createTrigger(wc.Trigger, wc.newTri, duration, false);
    id req = wc.createReq(wc.Request, wc.newReq, idString, content, trigger);
    wc.addToCenter(wc.getCenter(wc.Center, wc.dc), wc.addReq, req, ^(id e _U_){});
    CFRelease(idString);
    releaseO(content);
}

bool exerciseEntry_cycle(ExerciseEntry *e, WorkoutTimer *timers) {
    switch (e->state) {
        case ExerciseStateDisabled:
            e->state = ExerciseStateActive;
            if (e->type == ExerciseDuration) workoutTimer_start(&timers[TimerEx], e->reps, true);
            break;

        case ExerciseStateActive:
            if (e->rest) {
                e->state = ExerciseStateResting;
                break;
            }
        case ExerciseStateResting:
            if (++e->completedSets == e->sets) {
                e->state = ExerciseStateCompleted;
                ++timers[TimerEx].exercise;
                return true;
            } else {
                e->state = ExerciseStateActive;
                CFLocaleRef locale = CFLocaleCopyCurrent();
                CFStringRef sets = formatStr(locale, CFSTR("%d"), e->completedSets + 1);
                CFRelease(locale);
                updateRange(e->header, &e->hRange, sets);
                if (e->type == ExerciseDuration) workoutTimer_start(&timers[TimerEx], e->reps, true);
            }
        default:
            break;
    }
    return false;
}

void exerciseView_configure(StatusView *v) {
    if (v->entry->state == ExerciseStateResting) {
        setBtnTitle(v->button, v->entry->rest);
    } else {
        setBtnTitle(v->button, v->entry->title);
    }

    switch (v->entry->state) {
        case ExerciseStateDisabled:
            setBackgroundColor(v->box, getColor(ColorGray));
            setEnabled(v->button, false);
            break;

        case ExerciseStateActive:
            setText(v->header, v->entry->header);
            if (v->entry->type == ExerciseDuration)
                msg1(void, bool, v->button, sel_getUid("setUserInteractionEnabled:"), false);
        case ExerciseStateResting:
            setEnabled(v->button, true);
            setBackgroundColor(v->box, getColor(ColorOrange));
            break;

        case ExerciseStateCompleted:
            setEnabled(v->button, false);
            setBackgroundColor(v->box, getColor(ColorGreen));
    }
    statusView_updateAccessibility(v);
}

void circuit_start(Circuit *c, WorkoutTimer *timers, bool startTimer) {
    timers[TimerEx].exercise = c->index = 0;

    ExerciseEntry *end = &c->exercises[c->size];
    for (ExerciseEntry *e = c->exercises; e < end; ++e) {
        e->state = ExerciseStateDisabled;
        e->completedSets = 0;
    }

    if (c->type == CircuitAMRAP && startTimer)
        workoutTimer_start(&timers[TimerGroup], 60 * c->reps, true);

    exerciseEntry_cycle(c->exercises, timers);
}

int circuit_increment(Circuit *c, WorkoutTimer *timers, ExerciseEntry *entry) {
    if (++c->index != c->size) {
        exerciseEntry_cycle(entry + 1, timers);
        return TransitionFinishedExercise;
    }

    int finishedGroup = 0;
    switch (c->type) {
        case CircuitRounds:
            finishedGroup = ++c->completedReps == c->reps;
            break;

        case CircuitDecrement:
            if (--c->completedReps == 0) {
                finishedGroup = 1;
                break;
            }

            CFLocaleRef locale = CFLocaleCopyCurrent();
            CFStringRef reps = formatStr(locale, CFSTR("%d"), c->completedReps);
            CFRelease(locale);
            ExerciseEntry *end = &c->exercises[c->size];
            for (ExerciseEntry *e = c->exercises; e < end; ++e) {
                if (e->type == ExerciseReps) updateRange(e->title, &e->tRange, CFRetain(reps));
            }
            CFRelease(reps);
        default:
            break;
    }

    if (!finishedGroup) circuit_start(c, timers, false);
    return TransitionFinishedCircuit + finishedGroup;
}

int workout_increment(Workout *w, WorkoutTimer *timers) {
    if (++w->index == w->size) return TransitionCompletedWorkout;

    timers[0].container = timers[1].container = w->index;
    circuit_start(++w->group, timers, true);
    return TransitionFinishedCircuitDeleteFirst;
}

bool workout_isCompleted(Workout *w) {
    workout_setDuration(w);
    Circuit *group = w->group;
    int groupIndex = group->index;
    if (w->index != w->size - 1 || groupIndex != group->size - 1) return false;
    if (w->type == WorkoutEndurance) return w->duration >= (int16_t)(group->exercises[0].reps / 60);

    if (group->type == CircuitRounds && group->completedReps == group->reps - 1) {
        ExerciseEntry *entry = &group->exercises[groupIndex];
        return entry->state == ExerciseStateResting && entry->completedSets == entry->sets - 1;
    }
    return false;
}
