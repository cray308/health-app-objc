#ifndef HistoryVC_h
#define HistoryVC_h

#include "HistoryModel.h"

extern Class HistoryVCClass;

typedef struct {
    HistoryModel model;
    id rangeControl;
    id charts[3];
} HistoryVC;

id historyVC_init(HistoryModel **ref);
void historyVC_changedSegment(id self, SEL _cmd, id control);
void historyVC_viewDidLoad(id self, SEL _cmd);
CFStringRef historyVC_stringForValue(id self, SEL _cmd, double value);

void historyVC_clearData(id self);

#endif /* HistoryVC_h */
