#include "StatusCell.h"
#include "CustomButton.h"
#include "Views.h"

struct CellCache CellTable;
Class Cell;
Class RootCell;
Class BasicCell;
Class FullCell;
CFStringRef const BasicCellID = CFSTR("BasicCell");
CFStringRef const FullCellID = CFSTR("FullCell");

static CFStringRef separatorFormat;
static SEL cv;
static id (*getContentView)(id, SEL);
static SEL sbv;
static void (*setBackgroundView)(id, SEL, id);
static SEL ssbv;
static void (*setSelectedBackgroundView)(id, SEL, id);

void initCellData(void) {
    Cell = objc_getClass("UICollectionViewCell");
    SEL gbv = sel_getUid("backgroundView"), gsbv = sel_getUid("selectedBackgroundView");
    memcpy(&CellTable, &(struct CellCache){
        class_getInstanceSize(Cell), gbv, gsbv,
        (id(*)(id, SEL))class_getMethodImplementation(Cell, gbv),
        (id(*)(id, SEL))class_getMethodImplementation(Cell, gsbv)
    }, sizeof(struct CellCache));

    separatorFormat = localize(CFSTR("separator"));
    cv = sel_getUid("contentView");
    getContentView = (id(*)(id, SEL))class_getMethodImplementation(Cell, cv);
    sbv = sel_getUid("setBackgroundView:");
    setBackgroundView = (void(*)(id, SEL, id))class_getMethodImplementation(Cell, sbv);
    ssbv = sel_getUid("setSelectedBackgroundView:");
    setSelectedBackgroundView = (void(*)(id, SEL, id))class_getMethodImplementation(Cell, ssbv);
}

id rootCell_initWithFrame(id self, SEL _cmd, CGRect frame) {
    self = msgSupV(supSig(id, CGRect), self, Cell, _cmd, frame);
    StatusCell *v = getIVC(self);

    v->button = createCustomButton(CustomButton);
    setHeight(v->button, 50, true, false);
    setUserInteractionEnabled(v->button, false);

    id backgroundView = new(View);
    setBackgroundColor(backgroundView, getColor(ColorSecondaryBGGrouped));
    addCornerRadius(backgroundView);
    setBackgroundView(self, sbv, backgroundView);
    releaseView(backgroundView);

    id selectedBackgroundView = new(View);
    setBackgroundColor(selectedBackgroundView, colorWithAlphaComponent(ColorLabel, 0.5));
    addCornerRadius(selectedBackgroundView);
    setSelectedBackgroundView(self, ssbv, selectedBackgroundView);
    releaseView(selectedBackgroundView);
    return self;
}

void rootCell_deinit(id self, SEL _cmd) {
    StatusCell *v = getIVC(self);
    if (v->header) releaseView(v->header);
    releaseView(v->box);
    msgSupV(supSig(void), self, Cell, _cmd);
}

static void setupBox(StatusCell *v) {
    v->box = new(View);
    addCornerRadius(v->box);
    id boxWidthConstraint = makeConstraint(v->box, LayoutAttributeWidth, 0, nil, 0, 20);
    lowerPriority(boxWidthConstraint);
    setActive(boxWidthConstraint);
    setHeight(v->box, 20, false, true);
}

id basicCell_initWithFrame(id self, SEL _cmd, CGRect frame) {
    self = rootCell_initWithFrame(self, _cmd, frame);
    StatusCell *v = getIVC(self);
    setupBox(v);

    id hStack = createHStack((id []){v->button, v->box});
    useStackConstraints(hStack);
    setLayoutMargins(hStack, (HAInsets){0, 4, 0, 4});
    id contentView = getContentView(self, cv);
    addSubview(contentView, hStack);
    pin(hStack, contentView);
    releaseView(hStack);
    return self;
}

id fullCell_initWithFrame(id self, SEL _cmd, CGRect frame) {
    self = rootCell_initWithFrame(self, _cmd, frame);
    StatusCell *v = getIVC(self);
    setupBox(v);
    v->header = createLabel(nil, UIFontTextStyleSubheadline, ColorLabel);
    setIsAccessibilityElement(v->header, false);

    id hStack = createHStack((id []){v->button, v->box});
    id vStack = createVStack((id []){v->header, hStack}, 2);
    useStackConstraints(vStack);
    setLayoutMargins(vStack, (HAInsets){4, 4, 0, 4});
    setSpacing(vStack, ViewSpacing);
    id contentView = getContentView(self, cv);
    addSubview(contentView, vStack);
    pin(vStack, contentView);
    releaseView(hStack);
    releaseView(vStack);
    return self;
}

void statusCell_updateAccessibility(StatusCell *v) {
    CFStringRef title = getCurrentTitle(v->button);
    CFStringRef label = formatStr(NULL, separatorFormat, getText(v->header), title);
    setAccessibilityLabel(v->button, label);
    CFRelease(label);
}
