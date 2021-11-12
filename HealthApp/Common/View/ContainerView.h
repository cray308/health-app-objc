//
//  ContainerView.h
//  HealthApp
//
//  Created by Christopher Ray on 10/3/21.
//

#ifndef ContainerView_h
#define ContainerView_h

#include <CoreFoundation/CFBase.h>
#include "ObjectArray.h"

enum {
    HideDivider = 0x1,
    HideLabel = 0x2
};

typedef struct {
    id view;
    id divider;
    id headerLabel;
    id stack;
    Array_object *views;
} Container;

id createContainer(Container *c, CFStringRef title, int hidden, int spacing, bool margins);
void containers_free(Container *c, int size);
void container_add(Container *c, id v);
void container_clear(Container *c);
void container_refresh(Container *c);

#endif /* ContainerView_h */
