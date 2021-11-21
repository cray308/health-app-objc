#include "AddWorkoutCoordinator.h"
#include "AppCoordinator.h"
#include "AppUserData.h"
#include "PersistenceService.h"
#include "ViewControllerHelpers.h"
#include "WorkoutVC.h"
#include "UpdateMaxesModal.h"

extern void homeCoordinator_didFinishAddingWorkout(void *parent, int totalCompleted, bool popStack);

static bool checkEnduranceDuration(Workout *w) {
    if (w->type != WorkoutEndurance) return false;
    int planDuration = w->activities->arr[0].exercises->arr[0].reps / 60;
    return w->duration >= planDuration;
}

static void setDuration(Workout *w) {
    w->duration = ((int16_t) ((time(NULL) - w->startTime) / 60.f)) + 1;
#if TARGET_OS_SIMULATOR
    w->duration *= 10;
#endif
}

static void updateStoredDataAndCleanup(AddWorkoutCoordinator *this, short *lifts) {
    unsigned char type = this->workout->type;
    int16_t duration = this->workout->duration;

    CFRelease(this->workout->title);
    array_free(circuit, this->workout->activities);
    free(this->workout);
    free(this);

    if (duration < 15) return;

    runInBackground((^{
        id data = persistenceService_getCurrentWeek();
        int16_t newDuration = duration + weekData_getWorkoutTimeForType(data, type);
        weekData_setWorkoutTimeForType(data, type, newDuration);
        int16_t totalWorkouts = weekData_getTotalWorkouts(data);
        totalWorkouts += 1;
        weekData_setTotalWorkouts(data, totalWorkouts);
        if (lifts) {
            weekData_setLiftingMaxArray(data, lifts);
            free(lifts);
        }
        persistenceService_saveContext();
    }));
}

static void handleFinishedWorkout(AddWorkoutCoordinator *this,
                                  bool dismissVC, bool goBack, short *lifts) {
    const signed char day = this->workout->day;
    unsigned char totalCompleted = 0;
    bool longEnough = this->workout->duration >= 15;
    short *liftsCopy = NULL;
    if (longEnough) {
        if (day >= 0)
            totalCompleted = appUserData_addCompletedWorkout(day);
        if (lifts) {
            liftsCopy = malloc(sizeof(short) << 2);
            memcpy(liftsCopy, lifts, sizeof(short) << 2);
        }
    }

    id navVC = this->navVC;
    void *parent = this->parent;
    updateStoredDataAndCleanup(this, lifts);
    if (dismissVC) {
        dismissPresentedVC(navVC, ^{
            appCoordinator_updateMaxWeights(liftsCopy);
            homeCoordinator_didFinishAddingWorkout(parent, totalCompleted, true);
        });
    } else {
        if (liftsCopy)
            free(liftsCopy);
        homeCoordinator_didFinishAddingWorkout(parent, totalCompleted, goBack);
    }
}

void addWorkoutCoordinator_start(AddWorkoutCoordinator *this) {
    id vc = workoutVC_init(this);
    ((void(*)(id,SEL,id,bool))objc_msgSend)(this->navVC,
                                            sel_getUid("pushViewController:animated:"), vc, true);
    releaseObj(vc);
}

void addWorkoutCoordinator_stoppedWorkout(AddWorkoutCoordinator *this) {
    setDuration(this->workout);
    void *parent = this->parent;
    if (checkEnduranceDuration(this->workout)) {
        handleFinishedWorkout(this, false, true, NULL);
    } else {
        updateStoredDataAndCleanup(this, NULL);
        homeCoordinator_didFinishAddingWorkout(parent, 0, true);
    }
}

void addWorkoutCoordinator_completedWorkout(AddWorkoutCoordinator *this, bool dismissVC,
                                            bool showModalIfRequired, short *lifts) {
    CFStringRef title = this->workout->title;
    if (showModalIfRequired)
        setDuration(this->workout);
    if (showModalIfRequired &&
        CFStringCompareWithOptions(title, localize(CFSTR("workoutTitleTestDay")),
                                   CFRangeMake(0, CFStringGetLength(title)),
                                   kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
        presentModalVC(this->navVC, updateMaxesVC_init(this));
    } else {
        handleFinishedWorkout(this, dismissVC, true, lifts);
    }
}

void addWorkoutCoordinator_stopWorkoutFromBackButtonPress(AddWorkoutCoordinator *this) {
    if (this->workout->startTime) {
        setDuration(this->workout);
        if (checkEnduranceDuration(this->workout)) {
            handleFinishedWorkout(this, false, false, NULL);
            return;
        }
    }
    updateStoredDataAndCleanup(this, NULL);
}
