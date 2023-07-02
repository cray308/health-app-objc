#include "WorkoutVC_Ext.h"
#include "Views.h"

static CFStringRef notificationMessages[2];
static CFStringRef notificationTitle;
static struct WorkoutCache {
    const struct {
        Class cls;
        SEL sb, ss;
        void (*sTitle)(id, SEL, CFStringRef);
        void (*setBody)(id, SEL, CFStringRef);
        void (*setSound)(id, SEL, id);
    } content;
    const struct {
        Class cls;
        SEL cr;
        id (*create)(Class, SEL, double, bool);
    } trigger;
    const struct {
        Class cls;
        SEL cr;
        id (*create)(Class, SEL, CFStringRef, id, id);
    } request;
    const struct {
        Class cls;
        SEL ds;
        id (*defaultSound)(Class, SEL);
    } sound;
} wc;

void initNotificationCache(void) {
    fillStringArray(notificationMessages, CFSTR("notifications%d"), 2);
    notificationTitle = localize(CFSTR("workoutNotificationTitle"));

    Class Content = objc_getClass("UNMutableNotificationContent");
    SEL csb = sel_getUid("setBody:"), css = sel_getUid("setSound:");

    Class Trigger = objc_getClass("UNTimeIntervalNotificationTrigger");
    SEL cTri = sel_getUid("triggerWithTimeInterval:repeats:");

    Class Request = objc_getClass("UNNotificationRequest");
    SEL cReq = sel_getUid("requestWithIdentifier:content:trigger:");

    Class Sound = objc_getClass("UNNotificationSound");
    SEL sds = sel_getUid("defaultSound");

    memcpy(&wc, &(struct WorkoutCache){
        {
            Content, csb, css,
            (void(*)(id, SEL, CFStringRef))class_getMethodImplementation(Content, SetTitleSel),
            (void(*)(id, SEL, CFStringRef))class_getMethodImplementation(Content, csb),
            (void(*)(id, SEL, id))class_getMethodImplementation(Content, css)
        },
        {Trigger, cTri, (id(*)(Class, SEL, double, bool))getClassMethodImp(Trigger, cTri)},
        {Request, cReq, (id(*)(Class, SEL, CFStringRef, id, id))getClassMethodImp(Request, cReq)},
        {Sound, sds, (id(*)(Class, SEL))getClassMethodImp(Sound, sds)}
    }, sizeof(struct WorkoutCache));
}

void workoutTimer_start(WorkoutTimer *t, int duration, bool scheduleNotification) {
    static int identifiers[] = {0, 1};
    t->refTime = time(NULL);
    t->duration = duration;
    t->active = 1;
    if (!scheduleNotification) return;

    identifiers[t->type] += 2;
    t->identifier = identifiers[t->type];
    CFStringRef idStr = formatStr(NULL, CFSTR("%d"), t->identifier);
    id content = new(wc.content.cls);
    wc.content.sTitle(content, SetTitleSel, notificationTitle);
    wc.content.setBody(content, wc.content.sb, notificationMessages[t->type]);
    wc.content.setSound(content, wc.content.ss, wc.sound.defaultSound(wc.sound.cls, wc.sound.ds));
    id trigger = wc.trigger.create(wc.trigger.cls, wc.trigger.cr, duration, false);
    id request = wc.request.create(wc.request.cls, wc.request.cr, idStr, content, trigger);
    addNotificationRequest(getNotificationCenter(), request);
    CFRelease(idStr);
    releaseObject(content);
}

bool exercise_cycle(Exercise *e, WorkoutTimer *timers) {
    switch (e->state) {
        case ExerciseStateDisabled:
            e->state = ExerciseStateActive;
            if (e->type == ExerciseDuration)
                workoutTimer_start(&timers[TimerExercise], e->reps, true);
            break;

        case ExerciseStateActive ... ExerciseStateActiveMult:
            if (e->rest) {
                e->state = ExerciseStateResting;
                break;
            }
        case ExerciseStateResting:
            if (++e->completedSets == e->sets) {
                e->state = ExerciseStateCompleted;
                ++timers[TimerExercise].row;
                return true;
            } else {
                e->state = ExerciseStateActiveMult;
                CFLocaleRef locale = CFLocaleCopyCurrent();
                CFStringRef sets = formatStr(locale, CFSTR("%d"), e->completedSets + 1);
                updateRange(e->header, &e->headerRange, sets, locale);
                if (e->type == ExerciseDuration)
                    workoutTimer_start(&timers[TimerExercise], e->reps, true);
            }
        default:
            break;
    }
    return false;
}

void exerciseView_configure(StatusView *v) {
    switch (v->exercise->state) {
        case ExerciseStateDisabled:
            setBackgroundColor(v->box, getColor(ColorGray));
            break;

        case ExerciseStateActive:
            setEnabled(v->button, true);
            setBackgroundColor(v->box, getColor(ColorOrange));
        case ExerciseStateActiveMult:
            if (v->exercise->type == ExerciseDuration) setUserInteractionEnabled(v->button, false);
            if (v->exercise->completedSets) {
                setTitle(v->button, v->exercise->title, ControlStateNormal);
                setText(v->header, v->exercise->header);
                statusView_updateAccessibility(v);
            }
            break;

        case ExerciseStateResting:
            setTitle(v->button, v->exercise->rest, ControlStateNormal);
            if (v->exercise->sets > 1) statusView_updateAccessibility(v);
            break;

        case ExerciseStateCompleted:
            setEnabled(v->button, false);
            setBackgroundColor(v->box, getColor(ColorGreen));
            if (v->exercise->rest) {
                setTitle(v->button, v->exercise->title, ControlStateNormal);
                if (v->exercise->sets > 1) statusView_updateAccessibility(v);
            }
    }
}

void circuit_start(Circuit *c, WorkoutTimer *timers, bool startTimer) {
    timers[TimerExercise].row = c->index = 0;

    Exercise *end = &c->exercises[c->size];
    for (Exercise *e = c->exercises; e < end; ++e) {
        e->state = ExerciseStateDisabled;
        e->completedSets = 0;
    }

    if (c->type == CircuitAMRAP && startTimer)
        workoutTimer_start(&timers[TimerCircuit], 60 * c->reps, true);

    exercise_cycle(c->exercises, timers);
}

int circuit_increment(Circuit *c, WorkoutTimer *timers, Exercise *exercise) {
    if (++c->index != c->size) {
        exercise_cycle(exercise + 1, timers);
        return TransitionFinishedExercise;
    }

    int endCircuit = 0;
    switch (c->type) {
        case CircuitRounds:
            endCircuit = ++c->completedReps == c->reps;
            break;

        case CircuitDecrement:
            if ((endCircuit = (--c->completedReps == 0))) break;

            CFLocaleRef locale = CFLocaleCopyCurrent();
            CFStringRef reps = formatStr(locale, CFSTR("%d"), c->completedReps);
            Exercise *end = &c->exercises[c->size];
            for (Exercise *e = c->exercises; e < end; ++e) {
                if (e->type == ExerciseReps)
                    updateRange(e->title, &e->titleRange, CFRetain(reps), CFRetain(locale));
            }
            CFRelease(locale);
            CFRelease(reps);
        default:
            break;
    }

    if (!endCircuit) circuit_start(c, timers, false);
    return TransitionFinishedCircuit + endCircuit;
}

int workout_increment(Workout *w, WorkoutTimer *timers) {
    if (++w->index == w->size) return TransitionCompletedWorkout;

    timers[0].section = timers[1].section = w->index;
    circuit_start(&w->circuits[w->index], timers, true);
    return TransitionFinishedCircuitDeleteFirst;
}

bool workout_isCompleted(Workout *w) {
    workout_setDuration(w);
    Circuit *circuit = &w->circuits[w->index];
    if (w->index != w->size - 1 || circuit->index != circuit->size - 1) return false;
    if (w->type == WorkoutEndurance) return w->duration >= (int)(circuit->exercises[0].reps / 60);

    if (circuit->type == CircuitRounds && circuit->completedReps == circuit->reps - 1) {
        Exercise *ex = &circuit->exercises[circuit->index];
        return ex->state == ExerciseStateResting && ex->completedSets == ex->sets - 1;
    }
    return false;
}
