//
//  AppDelegate.h
//  HealthApp
//
//  Created by Christopher Ray on 3/20/21.
//

#import <UIKit/UIKit.h>

@class WorkoutViewController;
typedef struct AppCoordinator AppCoordinator;

@interface AppDelegate: UIResponder <UIApplicationDelegate>
@end

@interface AppDelegate() {
    @public AppCoordinator *coordinator;
    UIWindow *window;
    @public WorkoutViewController *workoutVC;
}
@end
