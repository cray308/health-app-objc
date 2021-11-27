#ifndef HistoryVC_h
#define HistoryVC_h

#include "HistoryTabCoordinator.h"

extern Class HistoryVCClass;
extern Ivar HistoryVCDataRef;

typedef struct __historyVCData {
    HistoryViewModel *model;
    id picker;
    id charts[3];
} HistoryVCData;

id historyVC_init(void *delegate);
void historyVC_refresh(id vc);
void historyVC_updateColors(id vc);
void historyVC_viewDidLoad(id self, SEL _cmd);
void historyVC_updateSegment(id self, SEL _cmd, id picker);
CFStringRef historyVC_stringForValue(id self, SEL _cmd, double value);

#endif /* HistoryVC_h */
