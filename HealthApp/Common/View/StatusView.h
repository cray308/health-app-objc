#ifndef StatusView_h
#define StatusView_h

#include <CoreFoundation/CFBase.h>
#include <objc/runtime.h>
#include "ExerciseManager.h"

extern Class StatusViewClass;

typedef struct {
    id button;
    id headerLabel;
    id box;
    ExerciseEntry *entry;
} StatusView;

id statusView_init(CFStringRef text CF_CONSUMED, int tag, id target, SEL action);
void statusView_deinit(id self, SEL _cmd);
void statusView_updateAccessibility(StatusView *ptr, CFStringRef stateText);

#endif /* StatusView_h */
