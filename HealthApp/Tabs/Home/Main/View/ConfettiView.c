//
//  ConfettiView.c
//  HealthApp
//
//  Created by Christopher Ray on 8/22/21.
//

#include "ConfettiView.h"
#include <CoreFoundation/CFString.h>
#include <CoreGraphics/CGColor.h>
#include <objc/message.h>
#include <math.h>
#include <stdlib.h>
#include "CocoaHelpers.h"
#include "Views.h"

extern id kCAEmitterLayerLine;

static inline void setFloat(id obj, SEL _cmd, float arg) {
    ((void(*)(id,SEL,float))objc_msgSend)(obj, _cmd, arg);
}

id createConfettiView(CGRect frame) {
    id view = createObjectWithFrame("UIView", frame);
    id bg = getObjectWithFloat(createColor(ColorGray),
                               sel_getUid("colorWithAlphaComponent:"), 0.8);
    setBackground(view, bg);

    id shapeColors[] = {
        createColor(ColorRed), createColor(ColorBlue),
        createColor(ColorGreen), getColorRef(1, 0.84, 0.04, 1)
    };
    id images[4];
    for (int i = 0; i < 4; ++i) {
        CFStringRef name = CFStringCreateWithFormat(NULL, NULL, CFSTR("confetti%d"), i);
        images[i] = createImage(name);
        CFRelease(name);
    }
    int const velocities[] = {100, 90, 150, 200};

    id cells[16];
    for (int i = 0; i < 16; ++i) {
        cells[i] = getObject(allocClass("CAEmitterCell"), sel_getUid("init"));
        setFloat(cells[i], sel_getUid("setBirthRate:"), 4);
        setFloat(cells[i], sel_getUid("setLifetime:"), 14);
        setCGFloat(cells[i], sel_getUid("setVelocity:"), velocities[arc4random_uniform(4)]);
        setCGFloat(cells[i], sel_getUid("setEmissionLongitude:"), M_PI);
        setCGFloat(cells[i], sel_getUid("setEmissionRange:"), 0.5);
        setCGFloat(cells[i], sel_getUid("setSpin:"), 3.5);
        CGColorRef color = ((CGColorRef(*)(id,SEL))objc_msgSend)(shapeColors[i / 4],
                                                                 sel_getUid("CGColor"));
        ((void(*)(id,SEL,CGColorRef))objc_msgSend)(cells[i], sel_getUid("setColor:"), color);
        id image = (id)((CGImageRef(*)(id,SEL))objc_msgSend)(images[i % 4], sel_getUid("CGImage"));
        setObject(cells[i], sel_getUid("setContents:"), image);
        setCGFloat(cells[i], sel_getUid("setScaleRange:"), 0.25);
        setCGFloat(cells[i], sel_getUid("setScale:"), 0.1);
    }

    CFArrayRef array = CFArrayCreate(NULL, (const void **)cells, 16, &retainedArrCallbacks);
    id particleLayer = getObject(allocClass("CAEmitterLayer"), sel_getUid("init"));
    setObject(getLayer(view), sel_getUid("addSublayer:"), particleLayer);
    ((void(*)(id,SEL,CGPoint))objc_msgSend)(particleLayer, sel_getUid("setEmitterPosition:"),
                                            (CGPoint){frame.size.width / 2, 0});
    setObject(particleLayer, sel_getUid("setEmitterShape:"), kCAEmitterLayerLine);
    ((void(*)(id,SEL,CGSize))objc_msgSend)(particleLayer, sel_getUid("setEmitterSize:"),
                                           (CGSize){frame.size.width - 16, 1});
    setArray(particleLayer, sel_getUid("setEmitterCells:"), array);

    CFRelease(array);
    releaseObj(particleLayer);
    for (int i = 0; i < 16; ++i)
        releaseObj(cells[i]);
    return view;
}
