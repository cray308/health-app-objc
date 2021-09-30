//
//  ExerciseContainer.m
//  HealthApp
//
//  Created by Christopher Ray on 8/22/21.
//

#import "ExerciseContainer.h"
#include "ViewControllerHelpers.h"

void exerciseView_configure(StatusButton *v, ExerciseEntry *e) {
    CFStringRef setsStr = exerciseEntry_createSetsTitle(e);
    CFStringRef title = exerciseEntry_createTitle(e);
    setButtonTitle(v->button, title, 0);
    setLabelText(v->headerLabel, setsStr);

    switch (e->state) {
        case ExerciseStateDisabled:
            setBackground(v->box, UIColor.systemGrayColor);
            enableButton(v->button, false);
            break;
        case ExerciseStateActive:
            if (e->type == ExerciseTypeDuration)
                v->button.userInteractionEnabled = false;
        case ExerciseStateResting:
            enableButton(v->button, true);
            setBackground(v->box, UIColor.systemOrangeColor);
            break;
        case ExerciseStateCompleted:
            enableButton(v->button, false);
            setBackground(v->box, UIColor.systemGreenColor);
    }

    CFRelease(title);
    if (setsStr)
        CFRelease(setsStr);
}

id exerciseContainer_init(ExerciseGroup *g, int idx, id target, SEL action) {
    ExerciseContainer *this = [[ExerciseContainer alloc] initWithFrame:CGRectZero];
    this->size = g->exercises->size;
    this->viewsArr = calloc(this->size, sizeof(StatusButton *));

    this->divider = createDivider();
    CFStringRef headerStr = exerciseGroup_createHeader(g);
    this->headerLabel = createLabel(headerStr, TextTitle3, 4);
    UIStackView *exerciseStack = createStackView(NULL, 0, 1, 5, (Padding){.top = 5});
    UIStackView *vStack = createStackView((id []){this->divider, this->headerLabel, exerciseStack},
                                          3, 1, 0, (Padding){0, 8, 0, 8});
    [this addSubview:vStack];
    [vStack setCustomSpacing:20 afterView:this->divider];
    pin(vStack, this, (Padding){0}, 0);
    setHeight(this->headerLabel, 20);

    for (unsigned i = 0; i < this->size; ++i) {
        this->viewsArr[i] = statusButton_init(NULL, false, (idx << 8) | i, target, action);
        exerciseView_configure(this->viewsArr[i], &g->exercises->arr[i]);
        [exerciseStack addArrangedSubview:this->viewsArr[i]];
    }
    if (headerStr)
        CFRelease(headerStr);
    [vStack release];
    [exerciseStack release];
    return this;
}

@implementation ExerciseContainer
- (void) dealloc {
    for (unsigned i = 0; i < size; ++i)
        [viewsArr[i] release];
    free(viewsArr);
    [divider release];
    [headerLabel release];
    [super dealloc];
}
@end
