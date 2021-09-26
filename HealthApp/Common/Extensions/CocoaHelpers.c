//
//  CocoaHelpers.c
//  HealthApp
//
//  Created by Christopher Ray on 7/24/21.
//

#include "CocoaHelpers.h"

CFArrayCallBacks kCocoaArrCallbacks = {0};

id objc_staticMethod(Class _self, SEL _cmd) {
    return ((id(*)(Class,SEL))objc_msgSend)(_self, _cmd);
}

void objc_singleArg(id obj, SEL _cmd) {
    ((void(*)(id,SEL))objc_msgSend)(obj, _cmd);
}

id allocClass(const char *name) {
    return objc_staticMethod(objc_getClass(name), sel_getUid("alloc"));
}

void releaseObj(id obj) {
    objc_singleArg(obj, sel_getUid("release"));
}

void getViewBounds(id view, CGRect *bounds) {
    ((void(*)(CGRect*,id,SEL))objc_msgSend_stret)(bounds, view, sel_getUid("bounds"));
}

void getViewFrame(id view, CGRect *frame) {
    ((void(*)(CGRect*,id,SEL))objc_msgSend_stret)(frame, view, sel_getUid("frame"));
}

void getScreenBounds(CGRect *result) {
    getViewBounds(objc_staticMethod(objc_getClass("UIScreen"), sel_getUid("mainScreen")), result);
}

id getBundle(void) {
    return objc_staticMethod(objc_getClass("NSBundle"), sel_getUid("mainBundle"));
}

id getUserDefaults(void) {
    return objc_staticMethod(objc_getClass("NSUserDefaults"), sel_getUid("standardUserDefaults"));
}

id getNotificationCenter(void) {
    return objc_staticMethod(objc_getClass("UNUserNotificationCenter"),
                             sel_getUid("currentNotificationCenter"));
}

id createColor(const char *name) {
    return objc_staticMethod(objc_getClass("UIColor"), sel_getUid(name));
}

id createImage(CFStringRef name) {
    return ((id(*)(Class,SEL,CFStringRef))objc_msgSend)(objc_getClass("UIImage"),
                                                        sel_getUid("systemImageNamed:"), name);
}

CFStringRef localize(CFStringRef key) {
    return ((CFStringRef(*)(id,SEL,CFStringRef,CFStringRef,CFStringRef))objc_msgSend)
    (getBundle(), sel_getUid("localizedStringForKey:value:table:"), key, NULL, NULL);
}
