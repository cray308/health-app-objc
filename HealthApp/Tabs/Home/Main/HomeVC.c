#include "HomeVC.h"
#include <CoreGraphics/CoreGraphics.h>
#include "CollectionVC.h"
#include "HeaderView.h"
#include "SetupWorkoutVC.h"
#include "StatusCell.h"
#include "WorkoutVC.h"

extern id kCAEmitterLayerLine;

Class HomeVCClass;

enum {
    SectionPlan,
    SectionCustom
};

static CFStringRef cellIDs[2];

static void updateCellBox(StatusCell *v, bool enabled) {
    setEnabled(v->button, enabled);
    setBackgroundColor(v->box, enabled ? getColor(ColorGray) : getColor(ColorGreen));
}

static void updateCellColors(id cell, StatusCell *v) {
    if (v->header) setTextColor(v->header, getColor(ColorLabel));

    setBackgroundColor(getBackgroundView(cell), getColor(ColorSecondaryBGGrouped));
    setBackgroundColor(getSelectedBackgroundView(cell), colorWithAlphaComponent(ColorLabel, 0.5));
    setTitleColor(v->button, getColor(ColorLabel), ControlStateNormal);
    setTitleColor(v->button, getColor(ColorDisabled), ControlStateDisabled);
}

void updateHeaderColors(HeaderView *v) {
    setBackgroundColor(v->divider, getColor(ColorDiv));
    setTextColor(v->label, getColor(ColorLabel));
}

#pragma mark - Lifecycle

id homeVC_init(void) {
    memcpy(cellIDs, &(CFStringRef []){FullCellID, BasicCellID}, 2 * sizeof(CFStringRef));

    CFStringRef readOnlyHeaders[2];
    fillStringArray(readOnlyHeaders, CFSTR("homeHeader%d"), 2);
    CFMutableStringRef *headers = malloc(2 * sizeof(CFMutableStringRef));
    for (int i = 0; i < 2; ++i) {
        headers[i] = CFStringCreateMutableCopy(NULL, 64, readOnlyHeaders[i]);
        CFRelease(readOnlyHeaders[i]);
    }
    int *itemCounts = malloc(2 * sizeof(int));
    memcpy(itemCounts, &(int []){6, 5}, 2 * sizeof(int));
    return collectionVC_init(HomeVCClass, headers, 2, itemCounts);
}

static void loadSections(CollectionVC *p, HomeVC *d, UserData const *data) {
    int items = p->itemCounts[SectionPlan];
    p->itemCounts[SectionPlan] = 0;
    for (int i = 0; i < items; ++i) {
        if (p->labels[SectionPlan][i]) {
            CFRelease(p->labels[SectionPlan][i]);
            p->labels[SectionPlan][i] = NULL;
        }
        if (p->titles[SectionPlan][i]) {
            CFRelease(p->titles[SectionPlan][i]);
            p->titles[SectionPlan][i] = NULL;
        }
    }

    p->firstSection = data->plan > MaxValidChar || data->planStart > time(NULL);
    if (p->firstSection == SectionCustom) return;

    CFLocaleRef locale = copyLocale();
    CFDateFormatterRef formatter = CFDateFormatterCreate(NULL, locale, 0, 0);
    CFDateFormatterSetFormat(formatter, CFSTR("EEEE"));
    CFRelease(locale);
    CFStringRef workoutNames[6] = {0};
    getWeeklyWorkoutNames(workoutNames, data->plan);
    time_t date = data->weekStart + (DaySeconds >> 1);

    for (int i = 0, itemIndex = 0; i < 6; ++i, date += DaySeconds) {
        if (!workoutNames[i]) continue;
        ++p->itemCounts[SectionPlan];
        p->labels[SectionPlan][itemIndex] = formatDate(formatter, date);
        p->titles[SectionPlan][itemIndex] = workoutNames[i];
        d->indexMapping[itemIndex++] = i;
    }
    CFRelease(formatter);

    d->completedWorkouts = data->completedWorkouts;
    collectionVC_calculateHeights(p, SectionPlan);
}

void homeVC_viewDidLoad(id self, SEL _cmd) {
    collectionVC_viewDidLoad(self, _cmd);

    CollectionVC *p = getIVVC(CollectionVC, self);
    HomeVC *d = getIVVCS(HomeVC, p);
    setupNavItem(self, CFSTR("tabs0"), NULL);

    p->titles[SectionCustom][0] = localize(CFSTR("homeTestMax"));
    fillStringArray(&p->titles[SectionCustom][1], CFSTR("workoutTypes%d"), 4);
    d->indexMapping[5] = 5;
    collectionVC_calculateHeights(p, -1);
    loadSections(p, d, getUserData());

    setDataSource(p->collectionView, self);
    setDelegate(p->collectionView, self);
}

#pragma mark - Collection Data Source

id homeVC_cellForItemAtIndexPath(id self, SEL _cmd _U_, id collectionView, id indexPath) {
    CollectionVC *p = getIVVC(CollectionVC, self);
    HomeVC *d = getIVVCS(HomeVC, p);
    long section = getSection(indexPath), item = getItem(indexPath);
    id cell = dequeueReusableCell(collectionView, cellIDs[section], indexPath);
    StatusCell *v = getIVC(cell);
    if (section == SectionPlan) {
        setText(v->header, p->labels[SectionPlan][item]);
        updateCellBox(v, !(d->completedWorkouts & (1 << d->indexMapping[item])));
    } else {
        setHidden(v->box, true);
    }

    setTitle(v->button, p->titles[section][item], ControlStateNormal);
    if (d->refreshItemColors[section] & (1 << item)) {
        updateCellColors(cell, v);
        d->refreshItemColors[section] &= ~(1 << item);
    }

    if (v->header) statusCell_updateAccessibility(v);
    setIDFormatted(cell, CFSTR("cell_%ld_%ld"), section, item)
    return cell;
}

id homeVC_viewForSupplementaryElement(id self, SEL _cmd _U_,
                                      id collectionView, CFStringRef kind _U_, id indexPath) {
    HomeVC *d = getIVVCC(HomeVC, CollectionVC, self);
    id header = collectionVC_viewForSupplementaryElement(self, nil, collectionView, kind, indexPath);
    long section = getSection(indexPath);
    if (d->refreshHeaderColors & (1 << section)) {
        updateHeaderColors(getIVR(header));
        d->refreshHeaderColors &= ~(1 << section);
    }
    return header;
}

#pragma mark - Collection Delegate

bool homeVC_shouldSelectItem(id self, SEL _cmd _U_, id collectionView _U_, id indexPath) {
    if (getSection(indexPath) == 1) return true;

    HomeVC *d = getIVVCC(HomeVC, CollectionVC, self);
    return !(d->completedWorkouts & (1 << d->indexMapping[getItem(indexPath)]));
}

void homeVC_didSelectItemAtIndexPath(id self, SEL _cmd _U_, id collectionView, id indexPath) {
    HomeVC *d = getIVVCC(HomeVC, CollectionVC, self);
    long section = getSection(indexPath);
    uint8_t index = (uint8_t)getItem(indexPath);
    deselectItem(collectionView, indexPath);
    CFMutableStringRef *headers;
    if (section == SectionPlan) {
        d->selectedIndex = (int)index;
        UserData const *data = getUserData();
        Workout *workout = getWeeklyWorkout(d->indexMapping[index],
                                            data->plan, data->lifts, &headers);
        homeVC_navigateToWorkout(self, workout, headers);
    } else if (!index) {
        WorkoutParams params = {StrengthIndexTestMax, 1, 1, 100, WorkoutStrength, UCHAR_MAX};
        Workout *workout = getWorkoutFromLibrary(&params, getUserData()->lifts, &headers);
        homeVC_navigateToWorkout(self, workout, headers);
    } else {
        presentModalVC(self, setupWorkoutVC_init(self, --index));
    }
}

#pragma mark - Public Functions

void homeVC_navigateToWorkout(id self, Workout *workout, CFMutableStringRef *headers) {
    id workoutVC = workoutVC_init(workout, headers);
    id navVC = getNavVC(self);
    msgV(objSig(void, id, bool), navVC, sel_getUid("pushViewController:animated:"), workoutVC, true);
    releaseVC(workoutVC);
}

void homeVC_createWorkoutsList(id self, UserData const *data) {
    CollectionVC *p = getIVVC(CollectionVC, self);
    loadSections(p, getIVVCS(HomeVC, p), data);
    id customWorkoutsHeader = supplementaryView(p->collectionView, makeIndexPath(0, SectionCustom));
    if (customWorkoutsHeader)
        setAlpha(getIVR(customWorkoutsHeader)->divider, p->firstSection == SectionPlan);

    reloadSections(p->collectionView, 0);
}

void homeVC_updateWorkoutsList(id self, uint8_t completedWorkouts) {
    CollectionVC *p = getIVVC(CollectionVC, self);
    HomeVC *d = getIVVCS(HomeVC, p);
    d->completedWorkouts = completedWorkouts;
    int items = p->itemCounts[SectionPlan];
    for (int i = 0; i < items; ++i) {
        id cell = cellForItem(p->collectionView, makeIndexPath(i, SectionPlan));
        if (cell) updateCellBox(getIVC(cell), !(d->completedWorkouts & (1 << d->indexMapping[i])));
    }
}

void homeVC_updateColors(id self) {
    CollectionVC *p = getIVVC(CollectionVC, self);
    HomeVC *d = getIVVCS(HomeVC, p);
    setBackgroundColor(p->collectionView, getColor(ColorPrimaryBGGrouped));
    for (int i = p->firstSection; i < p->totalSections; ++i) {
        id indexPath = makeIndexPath(0, i);
        id header = supplementaryView(p->collectionView, indexPath);
        if (header) {
            updateHeaderColors(getIVR(header));
        } else {
            d->refreshHeaderColors |= (1 << i);
        }

        int items = p->itemCounts[i];
        for (int j = 0; j < items; ++j) {
            indexPath = makeIndexPath(j, i);
            id cell = cellForItem(p->collectionView, indexPath);
            if (cell) {
                StatusCell *v = getIVC(cell);
                if (i == SectionPlan)
                    updateCellBox(v, !(d->completedWorkouts & (1 << d->indexMapping[i])));
                updateCellColors(cell, v);
            } else {
                d->refreshItemColors[i] |= (1 << j);
            }
        }
    }
}

#pragma mark - Navigate Back From Workout

static void showConfetti(id self) {
    SEL cgColor = sel_getUid("CGColor");
    SEL iimn = sel_getUid("imageNamed:"), icgi = sel_getUid("CGImage");
    id (*imageNamed)(Class, SEL, CFStringRef) =
      (id(*)(Class, SEL, CFStringRef))getClassMethodImp(Image, iimn);
    CGImageRef (*getCGImage)(id, SEL) =
      (CGImageRef(*)(id, SEL))class_getMethodImplementation(Image, icgi);

    CGColorRef colors[] = {
        msgV(objSig(CGColorRef), getColor(ColorRed), cgColor),
        msgV(objSig(CGColorRef), getColor(ColorBlue), cgColor),
        msgV(objSig(CGColorRef), getColor(ColorGreen), cgColor),
        msgV(objSig(CGColorRef), getColor(ColorOrange), cgColor)
    };
    CGImageRef images[] = {
        getCGImage(imageNamed(Image, iimn, CFSTR("cv0")), icgi),
        getCGImage(imageNamed(Image, iimn, CFSTR("cv1")), icgi),
        getCGImage(imageNamed(Image, iimn, CFSTR("cv2")), icgi),
        getCGImage(imageNamed(Image, iimn, CFSTR("cv3")), icgi)
    };

    Class EmitterCell = objc_getClass("CAEmitterCell");
    SEL esbr = sel_getUid("setBirthRate:"), esl = sel_getUid("setLifetime:");
    SEL esv = sel_getUid("setVelocity:"), esel = sel_getUid("setEmissionLongitude:");
    SEL eser = sel_getUid("setEmissionRange:"), ess = sel_getUid("setSpin:");
    SEL escl = sel_getUid("setColor:"), escn = sel_getUid("setContents:");
    SEL esscr = sel_getUid("setScaleRange:"), essc = sel_getUid("setScale:");
    void (*setBirthRate)(id, SEL, float) =
      (void(*)(id, SEL, float))class_getMethodImplementation(EmitterCell, esbr);
    void (*setLifetime)(id, SEL, float) =
      (void(*)(id, SEL, float))class_getMethodImplementation(EmitterCell, esl);
    void (*setVelocity)(id, SEL, CGFloat) =
      (void(*)(id, SEL, CGFloat))class_getMethodImplementation(EmitterCell, esv);
    void (*setEmissionLongitude)(id, SEL, CGFloat) =
      (void(*)(id, SEL, CGFloat))class_getMethodImplementation(EmitterCell, esel);
    void (*setEmissionRange)(id, SEL, CGFloat) =
      (void(*)(id, SEL, CGFloat))class_getMethodImplementation(EmitterCell, eser);
    void (*setSpin)(id, SEL, CGFloat) =
      (void(*)(id, SEL, CGFloat))class_getMethodImplementation(EmitterCell, ess);
    void (*setColor)(id, SEL, CGColorRef) =
      (void(*)(id, SEL, CGColorRef))class_getMethodImplementation(EmitterCell, escl);
    void (*setContents)(id, SEL, id) =
      (void(*)(id, SEL, id))class_getMethodImplementation(EmitterCell, escn);
    void (*setScaleRange)(id, SEL, CGFloat) =
      (void(*)(id, SEL, CGFloat))class_getMethodImplementation(EmitterCell, esscr);
    void (*setScale)(id, SEL, CGFloat) =
      (void(*)(id, SEL, CGFloat))class_getMethodImplementation(EmitterCell, essc);

    int velocities[] = {100, 90, 150, 200};
    CFMutableArrayRef cells = CFArrayCreateMutable(NULL, 16, &kCFTypeArrayCallBacks);

    for (int i = 0; i < 16; ++i) {
        id cell = new(EmitterCell);
        setBirthRate(cell, esbr, 4);
        setLifetime(cell, esl, 14);
        setVelocity(cell, esv, velocities[arc4random_uniform(4)]);
        setEmissionLongitude(cell, esel, M_PI);
        setEmissionRange(cell, eser, 0.5);
        setSpin(cell, ess, 3.5);
        setColor(cell, escl, colors[i >> 2]);
        setContents(cell, escn, (id)images[i % 4]);
        setScaleRange(cell, esscr, 0.25);
        setScale(cell, essc, 0.1);
        CFArrayAppendValue(cells, cell);
        releaseObject(cell);
    }

    id view = getView(self);
    CGRect bounds;
    getBounds(bounds, view);

    id confetti = new(View);
    useConstraints(confetti);
    setID(confetti, CFSTR("confetti"))
    setBackgroundColor(confetti, colorWithAlphaComponent(ColorGray, 0.8));

    id layer = new(objc_getClass("CAEmitterLayer"));
    msgV(objSig(void, id), getLayer(confetti), sel_getUid("addSublayer:"), layer);
    CGPoint position = {bounds.size.width * 0.5, 0};
    msgV(objSig(void, CGPoint), layer, sel_getUid("setEmitterPosition:"), position);
    msgV(objSig(void, id), layer, sel_getUid("setEmitterShape:"), kCAEmitterLayerLine);
    CGSize size = {bounds.size.width - 16, 1};
    msgV(objSig(void, CGSize), layer, sel_getUid("setEmitterSize:"), size);
    msgV(objSig(void, CFArrayRef), layer, sel_getUid("setEmitterCells:"), cells);

    CFRelease(cells);
    msgV(objSig(void), layer, ReleaseSel);
    addSubview(view, confetti);
    pinToSafeArea(confetti, view);

    dispatch_after(dispatch_time(0, 5000000000), dispatch_get_main_queue(), ^{
        msgV(objSig(void), confetti, sel_getUid("removeFromSuperview"));
        releaseView(confetti);
        id alert = createAlert(CFSTR("homeAlert"), CFSTR("homeAlertMessage"));
        addAlertAction(alert, CFSTR("ok"), ActionStyleDefault, NULL);
        disableWindowTint();
        presentVC(self, alert);
    });
}

void homeVC_handleFinishedWorkout(id self, uint8_t completedWorkouts) {
    CollectionVC *p = getIVVC(CollectionVC, self);
    int totalCompleted = 0;
    for (int i = 0; i < 6; ++i) {
        if ((1 << i) & completedWorkouts) ++totalCompleted;
    }
    homeVC_updateWorkoutsList(self, completedWorkouts);
    if (p->itemCounts[SectionPlan] == totalCompleted) {
        dispatch_after(dispatch_time(0, 2500000000), dispatch_get_main_queue(), ^{
            showConfetti(self);
        });
    }
}
