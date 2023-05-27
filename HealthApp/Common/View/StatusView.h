#ifndef StatusView_h
#define StatusView_h

#include <objc/objc.h>
#include "ExerciseManager.h"

extern Class StatusViewClass;

typedef struct {
    id button;
    id header;
    id box;
    Exercise *exercise;
} StatusView;

void initStatusViewData(void);

id statusView_init(StatusView **ref, CFStringRef header CF_CONSUMED, CFStringRef title CF_CONSUMED,
                   int tag, id target, SEL action);
void statusView_deinit(id self, SEL _cmd);
void statusView_updateAccessibility(StatusView *v);

#endif /* StatusView_h */
