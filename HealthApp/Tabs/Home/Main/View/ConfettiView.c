//
//  ConfettiView.c
//  HealthApp
//
//  Created by Christopher Ray on 8/22/21.
//

#include "ConfettiView.h"
#include "ViewControllerHelpers.h"

void setupConfettiView(ConfettiContainer *c) {
    c->view = createObjectWithFrame("UIView", c->frame);
    id bg = getObjectWithFloat(createColor("systemGrayColor"),
                               sel_getUid("colorWithAlphaComponent:"), 0.8);
    setBackground(c->view, bg);

    id shapeColors[] = {
        createColor("systemRedColor"), createColor("systemBlueColor"),
        createColor("systemGreenColor"), createColor("systemYellowColor")
    };
    id images[4];
    for (int i = 0; i < 4; ++i) {
        CFStringRef name = CFStringCreateWithFormat(NULL, NULL, CFSTR("confetti%d"), i);
        images[i] = createImage(name, false);
        CFRelease(name);
    }
    int const velocities[] = {100, 90, 150, 200};

    for (int i = 0; i < 16; ++i) {
        c->cells[i] = getObject(allocClass("CAEmitterCell"), sel_getUid("init"));
        setFloat(c->cells[i], sel_getUid("setBirthRate:"), 4);
        setFloat(c->cells[i], sel_getUid("setLifetime:"), 14);
        setCGFloat(c->cells[i], sel_getUid("setVelocity:"), velocities[arc4random_uniform(4)]);
        setCGFloat(c->cells[i], sel_getUid("setEmissionLongitude:"), M_PI);
        setCGFloat(c->cells[i], sel_getUid("setEmissionRange:"), 0.5);
        setCGFloat(c->cells[i], sel_getUid("setSpin:"), 3.5);
        CGColorRef color = ((CGColorRef(*)(id,SEL))objc_msgSend)(shapeColors[i / 4],
                                                                 sel_getUid("CGColor"));
        ((void(*)(id,SEL,CGColorRef))objc_msgSend)(c->cells[i], sel_getUid("setColor:"), color);
        id image = (id)((CGImageRef(*)(id,SEL))objc_msgSend)(images[i % 4], sel_getUid("CGImage"));
        setObject(c->cells[i], sel_getUid("setContents:"), image);
        setCGFloat(c->cells[i], sel_getUid("setScaleRange:"), 0.25);
        setCGFloat(c->cells[i], sel_getUid("setScale:"), 0.1);
    }

    CFArrayRef array = CFArrayCreate(NULL, (const void **)c->cells, 16, &kCocoaArrCallbacks);
    id particleLayer = getObject(allocClass("CAEmitterLayer"), sel_getUid("init"));
    setObject(getLayer(c->view), sel_getUid("addSublayer:"), particleLayer);
    ((void(*)(id,SEL,CGPoint))objc_msgSend)(particleLayer, sel_getUid("setEmitterPosition:"),
                                            (CGPoint){c->frame.size.width / 2, 0});
    setString(particleLayer, sel_getUid("setEmitterShape:"), CFSTR("line"));
    ((void(*)(id,SEL,CGSize))objc_msgSend)(particleLayer, sel_getUid("setEmitterSize:"),
                                           (CGSize){c->frame.size.width - 16, 1});
    setArray(particleLayer, sel_getUid("setEmitterCells:"), array);

    CFRelease(array);
    releaseObj(particleLayer);
}
