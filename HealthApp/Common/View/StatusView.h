#ifndef StatusView_h
#define StatusView_h

#include "ExerciseManager.h"
#include "ColorCache.h"
#include "ViewCache.h"

typedef struct {
    id button;
    id headerLabel;
    id box;
    ExerciseEntry *entry;
} StatusView;

id statusView_init(VCacheRef tbl, CCacheRef clr, StatusView **v, int tag, id target, SEL action);
void statusView_updateAccessibility(StatusView *v, VCacheRef tbl);

#endif /* StatusView_h */
