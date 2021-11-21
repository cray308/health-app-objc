#ifndef UpdateMaxesModal_h
#define UpdateMaxesModal_h

#include <objc/runtime.h>

extern Class UpdateMaxesClass;
extern Ivar UpdateMaxesDataRef;

id updateMaxesVC_init(void *delegate);
void updateMaxesVC_viewDidLoad(id self, SEL _cmd);
void updateMaxesVC_tappedFinish(id self, SEL _cmd);

#endif /* UpdateMaxesModal_h */
