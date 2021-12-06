#ifndef StatusView_h
#define StatusView_h

#include <CoreFoundation/CFBase.h>
#include <objc/runtime.h>
#include "ExerciseManager.h"

extern Class StatusViewClass;
extern Ivar StatusViewDataRef;

typedef struct __statusVData {
    id button;
    id headerLabel;
    id box;
    ExerciseEntry *entry;
} StatusViewData;

id statusView_init(CFStringRef text, int tag, id target, SEL action);
void statusView_deinit(id self, SEL _cmd);
void statusView_updateAccessibility(id self, CFStringRef stateText);

#endif /* StatusView_h */
