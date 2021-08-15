//
//  CocoaBridging.h
//  HealthApp
//
//  Created by Christopher Ray on 7/24/21.
//

#ifndef CocoaBridging_h
#define CocoaBridging_h

#include <CoreFoundation/CoreFoundation.h>
#include <objc/runtime.h>
#include <objc/message.h>

id objc_staticMethod(Class _self, SEL _cmd);
void objc_singleArg(id obj, SEL _cmd);

#endif /* CocoaBridging_h */
