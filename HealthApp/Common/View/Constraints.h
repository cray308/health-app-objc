#ifndef Constraints_h
#define Constraints_h

#include <objc/objc.h>
#include <CoreGraphics/CGGeometry.h>

typedef struct {
    CGFloat top, left, bottom, right;
} HAInsets;

typedef struct {
    short top, left, bottom, right;
} Padding;

enum {
    EdgeTop = 0x1,
    EdgeLeft = 0x2,
    EdgeBottom = 0x4,
    EdgeRight = 0x8
};

void setWidth(id v, int width, bool priority);
void setMinHeight(id v, int height, bool priority);
void setHeight(id v, int height, bool priority);
void setEqualWidths(id v, id container);
void pin(id v, id container, Padding margins, unsigned excluded);

#endif /* Constraints_h */
