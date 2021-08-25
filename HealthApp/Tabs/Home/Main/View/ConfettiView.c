//
//  ConfettiView.c
//  HealthApp
//
//  Created by Christopher Ray on 8/22/21.
//

#include "ConfettiView.h"
#include "ViewControllerHelpers.h"

id createConfettiView(CGRect frame) {
    id this = createObjectWithFrame("UIView", frame);
    id bg = ((id(*)(id,SEL,CGFloat))objc_msgSend)(createColor("systemGrayColor"),
                                                  sel_getUid("colorWithAlphaComponent:"), 0.8);
    setBackground(this, bg);

    id const colors[] = {
        createColor("systemRedColor"), createColor("systemBlueColor"),
        createColor("systemGreenColor"), createColor("systemYellowColor")
    };
    id const images[] = {
        createImage(CFSTR("Box")), createImage(CFSTR("Triangle")),
        createImage(CFSTR("Circle")), createImage(CFSTR("Spiral"))
    };
    int const velocities[] = {100, 90, 150, 200};
    id cells[16];

    for (int i = 0; i < 16; ++i) {
        cells[i] = ((id(*)(id,SEL))objc_msgSend)(allocClass("CAEmitterCell"), sel_getUid("init"));
        ((void(*)(id,SEL,float))objc_msgSend)(cells[i], sel_getUid("setBirthRate:"), 4);
        ((void(*)(id,SEL,float))objc_msgSend)(cells[i], sel_getUid("setLifetime:"), 14);
        ((void(*)(id,SEL,CGFloat))objc_msgSend)(cells[i], sel_getUid("setVelocity:"),
                                                velocities[arc4random_uniform(4)]);
        ((void(*)(id,SEL,CGFloat))objc_msgSend)(cells[i], sel_getUid("setEmissionLongitude:"),
                                                M_PI);
        ((void(*)(id,SEL,CGFloat))objc_msgSend)(cells[i], sel_getUid("setEmissionRange:"), 0.5);
        ((void(*)(id,SEL,CGFloat))objc_msgSend)(cells[i], sel_getUid("setSpin:"), 3.5);
        CGColorRef color = ((CGColorRef(*)(id,SEL))objc_msgSend)(colors[i / 4],
                                                                 sel_getUid("CGColor"));
        ((void(*)(id,SEL,CGColorRef))objc_msgSend)(cells[i], sel_getUid("setColor:"), color);
        id image = (id)((CGImageRef(*)(id,SEL))objc_msgSend)(images[i % 4], sel_getUid("CGImage"));
        ((void(*)(id,SEL,id))objc_msgSend)(cells[i], sel_getUid("setContents:"), image);
        ((void(*)(id,SEL,CGFloat))objc_msgSend)(cells[i], sel_getUid("setScaleRange:"), 0.25);
        ((void(*)(id,SEL,CGFloat))objc_msgSend)(cells[i], sel_getUid("setScale:"), 0.1);
    }

    CFArrayRef array = CFArrayCreate(NULL, (const void **)cells, 16, &kCocoaArrCallbacks);
    id particleLayer = ((id(*)(id,SEL))objc_msgSend)(allocClass("CAEmitterLayer"),
                                                     sel_getUid("init"));
    id layer = ((id(*)(id,SEL))objc_msgSend)(this, sel_getUid("layer"));
    ((void(*)(id,SEL,id))objc_msgSend)(layer, sel_getUid("addSublayer:"), particleLayer);
    ((void(*)(id,SEL,CGPoint))objc_msgSend)(particleLayer, sel_getUid("setEmitterPosition:"),
                                            (CGPoint){frame.size.width / 2, 0});
    ((void(*)(id,SEL,CFStringRef))objc_msgSend)(particleLayer, sel_getUid("setEmitterShape:"),
                                                CFSTR("line"));
    ((void(*)(id,SEL,CGSize))objc_msgSend)(particleLayer, sel_getUid("setEmitterSize:"),
                                           (CGSize){frame.size.width - 16, 1});
    ((void(*)(id,SEL,CFArrayRef))objc_msgSend)(particleLayer, sel_getUid("setEmitterCells:"),
                                               array);

    for (int i = 0; i < 16; ++i)
        releaseObj(cells[i]);
    CFRelease(array);
    releaseObj(particleLayer);
    return this;
}
