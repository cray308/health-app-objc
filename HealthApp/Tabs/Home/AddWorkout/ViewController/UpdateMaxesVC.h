#ifndef UpdateMaxesVC_h
#define UpdateMaxesVC_h

#include <objc/runtime.h>

extern Class UpdateMaxesVCClass;
extern Ivar UpdateMaxesVCDataRef;

id updateMaxesVC_init(void *delegate);
void updateMaxesVC_viewDidLoad(id self, SEL _cmd);
void updateMaxesVC_tappedFinish(id self, SEL _cmd);

#endif /* UpdateMaxesVC_h */
