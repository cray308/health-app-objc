#ifndef StatusView_h
#define StatusView_h

#include <CoreFoundation/CFBase.h>
#include <objc/objc.h>
#include "ExerciseManager.h"

typedef struct {
    id button;
    id headerLabel;
    id box;
    ExerciseEntry *entry;
} StatusView;

id statusView_init(CFStringRef text CF_CONSUMED, StatusView **ref, int tag, id target, SEL action);
void statusView_updateAccessibility(StatusView *ptr, CFStringRef stateText);

#endif /* StatusView_h */
