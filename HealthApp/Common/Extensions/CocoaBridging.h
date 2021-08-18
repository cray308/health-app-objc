//
//  CocoaBridging.h
//  HealthApp
//
//  Created by Christopher Ray on 7/24/21.
//

#ifndef CocoaBridging_h
#define CocoaBridging_h

#include <CoreFoundation/CoreFoundation.h>
#include <objc/runtime.h>
#include <objc/message.h>

id objc_staticMethod(Class _self, SEL _cmd);
void objc_singleArg(id obj, SEL _cmd);
id createChartEntry(int x, int y);

id createColor(const char *name);
id allocClass(const char *name);
void releaseObj(id obj);

id createArray(id *arr, int count);
id getObjectAtIndex(id arr, int i);

id createVCWithDelegate(const char *name, void *delegate);

id getFirstVC(id navVC);
void setupNavVC(id navVC, id firstVC);
id allocNavVC(void);
void presentVC(id presenter, id modal);
void dismissPresentedVC(id presenter);

void setWeekData(id weekData, const char *setter, int16_t value);
int16_t getWeekData(id weekData, const char *getter);
int16_t getLiftingLimitForType(id weekData, unsigned char type);
int16_t getWorkoutTimeForType(id weekData, unsigned char type);
int16_t getTotalWorkouts(id weekData);
void addToWorkoutType(id weekData, unsigned char type, int16_t duration);
void setLiftingMaxes(id weekData, short *weights);

#endif /* CocoaBridging_h */
