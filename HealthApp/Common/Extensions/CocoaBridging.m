//
//  CocoaBridging.m
//  HealthApp
//
//  Created by Christopher Ray on 7/24/21.
//

#import "CocoaBridging.h"
#import <Foundation/Foundation.h>

id objc_staticMethod(Class _self, SEL _cmd) {
    return ((id (*)(Class, SEL)) objc_msgSend)(_self, _cmd);
}

void objc_singleArg(id obj, SEL _cmd) {
    ((void (*)(id, SEL)) objc_msgSend)(obj, _cmd);
}
