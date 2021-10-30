//
//  main.c
//  HealthApp
//
//  Created by Christopher Ray on 3/20/21.
//

#include <CoreFoundation/CFString.h>
#include <objc/message.h>
#include "AppDelegate.h"

extern int UIApplicationMain(int, char *[], CFStringRef, CFStringRef);

int main(int argc, char *argv[]) {
    AppDelegateClass = objc_allocateClassPair(objc_getClass("UIResponder"), "AppDelegate", 0);
    class_addIvar(AppDelegateClass, "window", sizeof(id), 0, "@");
    class_addMethod(AppDelegateClass, sel_getUid("application:didFinishLaunchingWithOptions:"),
                    (IMP) appDelegate_didFinishLaunching, "i@:@@");
    class_addMethod(AppDelegateClass,
                    sel_getUid("application:supportedInterfaceOrientationsForWindow:"),
                    (IMP) appDelegate_supportedOrientations, "i@:@@");
    objc_registerClassPair(AppDelegateClass);
    return UIApplicationMain(argc, argv, nil, CFSTR("AppDelegate"));
}
