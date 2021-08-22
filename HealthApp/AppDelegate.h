//
//  AppDelegate.h
//  HealthApp
//
//  Created by Christopher Ray on 3/20/21.
//

#include "CocoaBridging.h"

extern Class AppDelegateClass;

typedef struct {
    Class isa;
    id window;
} AppDelegate;

bool appDelegate_didFinishLaunching(AppDelegate *self, SEL _cmd, id application, id options);
int appDelegate_supportedOrientations(AppDelegate *self, SEL _cmd, id application, id window);
