//
//  ObjectArray.c
//  HealthApp
//
//  Created by Christopher Ray on 10/7/21.
//

#include "ObjectArray.h"

extern void releaseObj(id obj);

gen_array_source(object, id, DSDefault_shallowCopy, releaseObj)
