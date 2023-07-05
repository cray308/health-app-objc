#ifndef HomeVC_h
#define HomeVC_h

#include <objc/objc.h>
#include "UserData.h"

extern Class HomeVCClass;

typedef struct {
    int indexMapping[6];
    int selectedIndex;
    uint8_t completedWorkouts;
} HomeVC;

id homeVC_init(void);
void homeVC_viewDidLoad(id self, SEL _cmd);
id homeVC_cellForItemAtIndexPath(id self, SEL _cmd, id collectionView, id indexPath);
bool homeVC_shouldSelectItem(id self, SEL _cmd, id collectionView, id indexPath);
void homeVC_didSelectItemAtIndexPath(id self, SEL _cmd, id collectionView, id indexPath);

void homeVC_createWorkoutsList(id self, UserData const *data);
void homeVC_updateWorkoutsList(id self, uint8_t completedWorkouts);

void homeVC_handleFinishedWorkout(id self, uint8_t completedWorkouts);

#endif /* HomeVC_h */
