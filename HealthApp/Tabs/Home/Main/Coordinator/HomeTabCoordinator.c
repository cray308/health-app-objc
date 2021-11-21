#include "HomeTabCoordinator.h"
#include <CoreGraphics/CGColor.h>
#include <dispatch/queue.h>
#include <math.h>
#include "AddWorkoutCoordinator.h"
#include "AppUserData.h"
#include "HomeVC.h"
#include "SetupWorkoutModal.h"
#include "ViewControllerHelpers.h"

#define setFloat(_o, _cmd, _arg) (((void(*)(id,SEL,float))objc_msgSend)((_o), (_cmd), (_arg)))

extern id kCAEmitterLayerLine;

enum {
    CustomWorkoutIndexTestMax,
    CustomWorkoutIndexEndurance,
    CustomWorkoutIndexStrength,
    CustomWorkoutIndexSE,
    CustomWorkoutIndexHIC
};

static void navigateToAddWorkout(HomeTabCoordinator *this, bool dismissVC, Workout *workout) {
    HomeTabCoordinator *coord = this;
    AddWorkoutCoordinator *child = malloc(sizeof(AddWorkoutCoordinator));
    child->navVC = this->navVC;
    child->parent = this;
    child->workout = workout;

    if (dismissVC) {
        dismissPresentedVC(getFirstVC(this->navVC), ^{
            coord->childCoordinator = child;
            addWorkoutCoordinator_start(child);
        });
    } else {
        coord->childCoordinator = child;
        addWorkoutCoordinator_start(child);
    }
}

static void showConfetti(id vc) {
    id view = getView(vc);
    CGRect frame;
    getRect(view, &frame, 0);
    id confettiView = createObjectWithFrame(objc_getClass("UIView"), frame);
    id grayColor = createColor(ColorGray);
    id bg = getObjectWithFloat(grayColor, sel_getUid("colorWithAlphaComponent:"), 0.8);
    setBackground(confettiView, bg);

    Class cellClass = objc_getClass("CAEmitterCell");
    SEL cgImg = sel_getUid("CGImage"), cgCol = sel_getUid("CGColor"), init = sel_getUid("init");
    SEL sColor = sel_getUid("setColor:"), sImg = sel_getUid("setContents:");
    SEL sRate = sel_getUid("setBirthRate:"), sLife = sel_getUid("setLifetime:");
    SEL sVel = sel_getUid("setVelocity:"), sELong = sel_getUid("setEmissionLongitude:");
    SEL sERange = sel_getUid("setEmissionRange:"), sSpin = sel_getUid("setSpin:");
    SEL sScale = sel_getUid("setScale:"), sScaleR = sel_getUid("setScaleRange:");

    int const velocities[] = {100, 90, 150, 200};
    id _colors[] = {
        createColor(ColorRed), createColor(ColorBlue),
        createColor(ColorGreen), getColorRef(1, 0.84, 0.04, 1)
    };
    CGColorRef shapeColors[4];
    id images[4];
    for (int i = 0; i < 4; ++i) {
        CFStringRef name = CFStringCreateWithFormat(NULL, NULL, CFSTR("confetti%d"), i);
        id img = createImage(name);
        CFRelease(name);
        images[i] = (id)((CGImageRef(*)(id,SEL))objc_msgSend)(img, cgImg);
        shapeColors[i] = ((CGColorRef(*)(id,SEL))objc_msgSend)(_colors[i], cgCol);
    }

    id cells[16];
    for (int i = 0; i < 16; ++i) {
        cells[i] = getObject(allocClass(cellClass), init);
        setFloat(cells[i], sRate, 4);
        setFloat(cells[i], sLife, 14);
        int velocity = velocities[arc4random_uniform(4)];
        setCGFloat(cells[i], sVel, velocity);
        setCGFloat(cells[i], sELong, M_PI);
        setCGFloat(cells[i], sERange, 0.5);
        setCGFloat(cells[i], sSpin, 3.5);
        ((void(*)(id,SEL,CGColorRef))objc_msgSend)(cells[i], sColor, shapeColors[i >> 2]);
        setObject(cells[i], sImg, images[i % 4]);
        setCGFloat(cells[i], sScaleR, 0.25);
        setCGFloat(cells[i], sScale, 0.1);
    }

    CFArrayRef array = CFArrayCreate(NULL, (const void **)cells, 16, &retainedArrCallbacks);
    id _layer = allocClass(objc_getClass("CAEmitterLayer"));
    id particleLayer = getObject(_layer, init);
    id viewLayer = getLayer(confettiView);
    setObject(viewLayer, sel_getUid("addSublayer:"), particleLayer);
    ((void(*)(id,SEL,CGPoint))objc_msgSend)(particleLayer, sel_getUid("setEmitterPosition:"),
                                            (CGPoint){frame.size.width / 2, 0});
    setObject(particleLayer, sel_getUid("setEmitterShape:"), kCAEmitterLayerLine);
    ((void(*)(id,SEL,CGSize))objc_msgSend)(particleLayer, sel_getUid("setEmitterSize:"),
                                           (CGSize){frame.size.width - 16, 1});
    setArray(particleLayer, sel_getUid("setEmitterCells:"), array);

    CFRelease(array);
    releaseObj(particleLayer);
    for (int i = 0; i < 16; ++i) {
        releaseObj(cells[i]);
    }
    addSubview(view, confettiView);

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC * 5),
                   dispatch_get_main_queue(), ^(void) {
        removeView(confettiView);
        releaseObj(confettiView);
        id ctrl = createAlertController(localize(CFSTR("homeAlertTitle")),
                                        localize(CFSTR("homeAlertMessage")));
        addAlertAction(ctrl, localize(CFSTR("ok")), 0, NULL);
        presentVC(vc, ctrl);
    });
}

void homeCoordinator_start(HomeTabCoordinator *this) {
    fillStringArray(this->model.stateNames, CFSTR("homeState%d"), 2);
    fillStringArray(this->model.timeNames, CFSTR("timesOfDay%d"), 3);
    homeViewModel_updateTimeOfDay(&this->model);
    setupNavVC(this->navVC, homeVC_init(this));
}

void homeCoordinator_didFinishAddingWorkout(HomeTabCoordinator *this,
                                            int totalCompleted, bool popStack) {
    id homeVC = getFirstVC(this->navVC);
    homeVC_updateWorkoutsList(homeVC);

    if (popStack) {
        setBool(this->navVC, sel_getUid("popViewControllerAnimated:"), true);
        this->childCoordinator = NULL;
    }

    if (!totalCompleted) return;

    int nWorkouts = 0;
    for (int i = 0; i < 7; ++i) {
        if (this->model.workoutNames[i])
            ++nWorkouts;
    }

    if (nWorkouts == totalCompleted) {
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC * 0.75),
                       dispatch_get_main_queue(), ^(void) {
            showConfetti(homeVC);
        });
    }
}

void homeCoordinator_addWorkoutFromPlan(HomeTabCoordinator *this, int index) {
    Workout *w = exerciseManager_getWeeklyWorkoutAtIndex(userData->currentPlan,
                                                         appUserData_getWeekInPlan(), index);
    if (w)
        navigateToAddWorkout(this, false, w);
}

void homeCoordinator_addWorkoutFromCustomButton(HomeTabCoordinator *this, int index) {
    unsigned char type = WorkoutStrength;
    switch (index) {
        case CustomWorkoutIndexSE:
            type = WorkoutSE;
            break;
        case CustomWorkoutIndexHIC:
            type = WorkoutHIC;
            break;
        case CustomWorkoutIndexTestMax: ;
            WorkoutParams params = {-1, WorkoutStrength, 2, 1, 1, 100};
            Workout *w = exerciseManager_getWorkoutFromLibrary(&params);
            if (w)
                navigateToAddWorkout(this, false, w);
            return;
        case CustomWorkoutIndexEndurance:
            type = WorkoutEndurance;
        default:
            break;
    }

    CFArrayRef names = exerciseManager_createWorkoutNames(type);
    if (!names) return;
    else if (!CFArrayGetCount(names)) {
        CFRelease(names);
        return;
    }

    presentModalVC(getFirstVC(this->navVC), setupWorkoutVC_init(this, type, names));
}

void homeCoordinator_finishedSettingUpCustomWorkout(HomeTabCoordinator *this, void *params) {
    Workout *w = exerciseManager_getWorkoutFromLibrary(params);
    if (w)
        navigateToAddWorkout(this, true, w);
}

void homeCoordinator_checkForChildCoordinator(HomeTabCoordinator *this) {
    AddWorkoutCoordinator *child = this->childCoordinator;
    if (child) {
        addWorkoutCoordinator_stopWorkoutFromBackButtonPress(child);
        this->childCoordinator = NULL;
    }
}
