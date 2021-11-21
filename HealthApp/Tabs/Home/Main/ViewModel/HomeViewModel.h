#ifndef HomeViewModel_h
#define HomeViewModel_h

#include <CoreFoundation/CFBase.h>

typedef struct {
    CFStringRef workoutNames[7];
    CFStringRef stateNames[2];
    CFStringRef timeNames[3];
    enum { Morning, Afternoon, Evening } timeOfDay;
} HomeViewModel;

void homeViewModel_fetchData(HomeViewModel *model);
bool homeViewModel_updateTimeOfDay(HomeViewModel *model);

#endif /* HomeViewModel_h */
