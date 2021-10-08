//
//  ConfettiView.h
//  HealthApp
//
//  Created by Christopher Ray on 8/22/21.
//

#ifndef ConfettiView_h
#define ConfettiView_h

#include <CoreGraphics/CGGeometry.h>
#include <objc/objc.h>

typedef struct {
    CGRect frame;
    id view;
    id cells[16];
} ConfettiContainer;

void setupConfettiView(ConfettiContainer *c);

#endif /* ConfettiView_h */
