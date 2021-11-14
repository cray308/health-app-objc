#import <UIKit/UIKit.h>

@interface StatusButton: UIView @end
@interface StatusButton() {
    @public UIView *box;
    @public UILabel *headerLabel;
    @public UIButton *button;
}
@end

id statusButton_init(CFStringRef text, bool hideViews, int tag, id target, SEL action);
void statusButton_updateAccessibility(id b, CFStringRef stateText);
