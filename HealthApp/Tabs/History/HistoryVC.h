#ifndef HistoryVC_h
#define HistoryVC_h

#include "ColorCache.h"
#include "HistoryModel.h"
#include "ViewCache.h"

extern Class HistoryVCClass;

typedef struct {
    CCacheRef clr;
    VCacheRef tbl;
    HistoryModel model;
    id picker;
    id charts[3];
} HistoryVC;

id historyVC_init(HistoryModel **ref, VCacheRef tbl, CCacheRef clr);
void historyVC_updateSegment(id self, SEL _cmd, id picker);
void historyVC_viewDidLoad(id self, SEL _cmd);
CFStringRef historyVC_stringForValue(id self, SEL _cmd, double value);

void historyVC_clearData(id self);
void historyVC_updateColors(id self, bool darkMode);

#endif /* HistoryVC_h */
