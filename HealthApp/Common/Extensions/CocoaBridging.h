//
//  CocoaBridging.h
//  HealthApp
//
//  Created by Christopher Ray on 7/24/21.
//

#ifndef CocoaBridging_h
#define CocoaBridging_h

#include <CoreFoundation/CoreFoundation.h>

CFDictionaryRef getUserInfoDictionary(void);
void writeUserInfoDictionary(CFDictionaryRef dict);
CFDictionaryRef workoutJsonDictionaryCreate(void);

#endif /* CocoaBridging_h */
