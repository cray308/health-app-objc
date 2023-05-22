#ifndef StatusView_h
#define StatusView_h

#include <objc/objc.h>
#include "ExerciseManager.h"
#include "ColorCache.h"
#include "ViewCache.h"

extern Class StatusViewClass;

typedef struct {
    id button;
    id header;
    id box;
    ExerciseEntry *entry;
} StatusView;

typedef struct {
    VCacheRef tbl;
    CCacheRef clr;
    StatusView **v;
} SVArgs;

void initStatVData(Class Button);

id statusView_init(SVArgs *args,
                   CFStringRef header, CFStringRef title CF_CONSUMED, int tag, id target, SEL action);
void statusView_deinit(id self, SEL _cmd);
void statusView_updateAccessibility(StatusView *v, VCacheRef tbl);

#endif /* StatusView_h */
