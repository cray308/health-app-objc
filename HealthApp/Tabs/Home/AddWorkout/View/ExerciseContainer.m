//
//  ExerciseContainer.m
//  HealthApp
//
//  Created by Christopher Ray on 8/22/21.
//

#import "ExerciseContainer.h"
#include "ViewControllerHelpers.h"

@implementation ExerciseContainer
- (id) initWithGroup: (ExerciseGroup *)g tag: (uint)idx target: (id)target action: (SEL)action {
    if (!(self = [super initWithFrame:CGRectZero])) return nil;
    size = g->exercises->size;
    viewsArr = calloc(size, sizeof(ExerciseView *));

    divider = createDivider();
    CFStringRef headerStr = exerciseGroup_createHeader(g);
    headerLabel = createLabel(headerStr, UIFontTextStyleTitle3, NSTextAlignmentNatural);
    UIStackView *exerciseStack = createStackView(NULL, 0, 1, 5, 0, (HAEdgeInsets){.top = 5});
    UIStackView *vStack = createStackView((id []){divider, headerLabel, exerciseStack},
                                          3, 1, 0, 0, (HAEdgeInsets){0, 8, 0, 8});
    [self addSubview:vStack];
    [vStack setCustomSpacing:20 afterView:divider];
    activateConstraints((id []){
        [vStack.topAnchor constraintEqualToAnchor:self.topAnchor],
        [vStack.leadingAnchor constraintEqualToAnchor:self.leadingAnchor],
        [vStack.trailingAnchor constraintEqualToAnchor:self.trailingAnchor],
        [vStack.bottomAnchor constraintEqualToAnchor:self.bottomAnchor],
        [headerLabel.heightAnchor constraintEqualToConstant:20]
    }, 5);

    for (unsigned i = 0; i < size; ++i) {
        unsigned tag = (idx << 8) | i;
        ExerciseView *v = [[ExerciseView alloc] initWithEntry:&g->exercises->arr[i]
                                                          tag:tag target:target action:action];
        [exerciseStack addArrangedSubview:v];
        viewsArr[i] = v;
    }
    if (headerStr)
        CFRelease(headerStr);
    [vStack release];
    [exerciseStack release];
    return self;
}

- (void) dealloc {
    for (unsigned i = 0; i < size; ++i)
        [viewsArr[i] release];
    free(viewsArr);
    [divider release];
    [headerLabel release];
    [super dealloc];
}
@end
