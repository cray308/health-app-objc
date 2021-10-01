//
//  ViewControllerHelpers.c
//  HealthApp
//
//  Created by Christopher Ray on 3/21/21.
//

#include "ViewControllerHelpers.h"
#include <CoreGraphics/CoreGraphics.h>
#include "SwiftBridging.h"

gen_uset_source(char, unsigned short, ds_cmp_num_eq, DSDefault_addrOfVal, DSDefault_sizeOfVal,
                DSDefault_shallowCopy, DSDefault_shallowDelete)

static CFStringRef inputFieldError;

extern id UIFontTextStyleTitle1;
extern id UIFontTextStyleTitle2;
extern id UIFontTextStyleTitle3;
extern id UIFontTextStyleHeadline;
extern id UIFontTextStyleSubheadline;
extern id UIFontTextStyleBody;
extern id UIFontTextStyleFootnote;

struct AnchorNames anchors = {
    "topAnchor",
    "bottomAnchor",
    "leadingAnchor",
    "trailingAnchor",
    "widthAnchor",
    "heightAnchor",
    "centerYAnchor"
};

static void disableAutoresizing(id view) {
    ((void(*)(id,SEL,bool))objc_msgSend)
    (view, sel_getUid("setTranslatesAutoresizingMaskIntoConstraints:"), false);
}

static void setAlignment(id view, int alignment) {
    ((void(*)(id,SEL,int))objc_msgSend)(view, sel_getUid("setTextAlignment:"), alignment);
}

static void setDynamicFont(id view) {
    ((void(*)(id,SEL,bool))objc_msgSend)(view, sel_getUid("setAdjustsFontSizeToFitWidth:"), true);
    ((void(*)(id,SEL,CGFloat))objc_msgSend)(view, sel_getUid("setMinimumScaleFactor:"), 0.85);
}

static void setCornerRadius(id view) {
    id layer = ((id(*)(id,SEL))objc_msgSend)(view, sel_getUid("layer"));
    ((void(*)(id,SEL,CGFloat))objc_msgSend)(layer, sel_getUid("setCornerRadius:"), 5);
}

static void setFont(id view, id font) {
    ((void(*)(id,SEL,id))objc_msgSend)(view, sel_getUid("setFont:"), font);
}

static void addTarget(id view, id target, SEL action, int event) {
    ((void(*)(id,SEL,id,SEL,int))objc_msgSend)
    (view, sel_getUid("addTarget:action:forControlEvents:"), target, action, event);
}

static void activateConstraints(id *constraints, int count) {
    CFArrayRef array = CFArrayCreate(NULL, (const void **)constraints, count, &kCocoaArrCallbacks);
    ((void(*)(Class,SEL,CFArrayRef))objc_msgSend)(objc_getClass("NSLayoutConstraint"),
                                                  sel_getUid("activateConstraints:"), array);
    CFRelease(array);
}

static id getAnchor(id view, const char *name) {
    return ((id(*)(id,SEL))objc_msgSend)(view, sel_getUid(name));
}

static id createConstraint(id a1, id a2, int constant) {
    id result;
    if (a2) {
        result = ((id(*)(id,SEL,id,CGFloat))objc_msgSend)
        (a1, sel_getUid("constraintEqualToAnchor:constant:"), a2, constant);
    } else {
        result = ((id(*)(id,SEL,CGFloat))objc_msgSend)(a1, sel_getUid("constraintEqualToConstant:"),
                                                       constant);
    }
    return result;
}

id createToolbar(id target, SEL doneSelector) {
    CGRect bounds;
    getScreenBounds(&bounds);
    CGFloat width = bounds.size.width;

    id toolbar = createObjectWithFrame("UIToolbar", (CGRect){{0}, {width, 50}});
    objc_singleArg(toolbar, sel_getUid("sizeToFit"));

    const char *btnName = "UIBarButtonItem";
    id flexSpace = ((id(*)(id,SEL,int,id,SEL))objc_msgSend)
    (allocClass(btnName), sel_getUid("initWithBarButtonSystemItem:target:action:"), 5, nil, nil);
    id doneButton = ((id(*)(id,SEL,CFStringRef,int,id,SEL))objc_msgSend)
    (allocClass(btnName),
     sel_getUid("initWithTitle:style:target:action:"), CFSTR("Done"), 0, target, doneSelector);

    CFArrayRef array = CFArrayCreate(NULL, (const void *[]){flexSpace, doneButton},
                                     2, &kCocoaArrCallbacks);
    ((void(*)(id,SEL,CFArrayRef,bool))objc_msgSend)(toolbar,
                                            sel_getUid("setItems:animated:"), array, false);
    ((void(*)(id,SEL,bool))objc_msgSend)(toolbar, sel_getUid("setUserInteractionEnabled:"), true);

    CFRelease(array);
    releaseObj(flexSpace);
    releaseObj(doneButton);
    return toolbar;
}

void setNavButton(id navItem, bool left, id button, CGFloat totalWidth) {
    ((void(*)(id,SEL,CGRect))objc_msgSend)(button, sel_getUid("setFrame:"),
                                           (CGRect){{0}, {totalWidth / 3, 30}});
    id item = ((id(*)(id,SEL,id))objc_msgSend)(allocClass("UIBarButtonItem"),
                                               sel_getUid("initWithCustomView:"), button);
    if (left)
        ((void(*)(id,SEL,id))objc_msgSend)(navItem, sel_getUid("setLeftBarButtonItem:"), item);
    else
        ((void(*)(id,SEL,id))objc_msgSend)(navItem, sel_getUid("setRightBarButtonItem:"), item);
    releaseObj(item);
}

id createDivider(void) {
    return createView(createColor("separatorColor"), false, -1, 1);
}

void fillStringArray(CFStringRef *arr, CFStringRef format, int count) {
    for (int i = 0; i < count; ++i) {
        CFStringRef key = CFStringCreateWithFormat(NULL, NULL, format, i);
        arr[i] = localize(key);
        CFRelease(key);
    }
}

void textValidator_setup(Validator *this, short margins) {
    unsigned short nums[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    this->set = uset_new_fromArray(char, nums, 10);
    memcpy(&this->padding, &(Padding){4,margins,4,margins}, sizeof(Padding));
}

void textValidator_free(Validator *this) {
    if (this->set)
        uset_free(char, this->set);
    for (int i = 0; i < 4; ++i) {
        if (this->children[i].view) {
            releaseObj(this->children[i].hintLabel);
            releaseObj(this->children[i].field);
            releaseObj(this->children[i].errorLabel);
            releaseObj(this->children[i].view);
        }
    }
}

id validator_add(Validator *v, id delegate, CFStringRef hint, int min, int max, id toolbar) {
    struct InputView *child = &v->children[v->count];
    child->view = createView(nil, false, -1, -1);
    child->minVal = min;
    child->maxVal = max;
    if (!inputFieldError)
        inputFieldError = localize(CFSTR("inputFieldError"));
    CFStringRef errorText = CFStringCreateWithFormat(NULL, NULL, inputFieldError, min, max);
    child->hintLabel = createLabel(hint, TextFootnote, 4, 20);
    child->field = createTextfield(delegate, NULL, 4, 4, v->count++, 40);
    child->errorLabel = createLabel(errorText, TextFootnote, 4, 20);
    setTextColor(child->errorLabel, createColor("systemRedColor"));
    id vStack = createStackView((id []){child->hintLabel, child->field, child->errorLabel},
                                3, 1, 4, v->padding);
    addSubview(child->view, vStack);
    pin(vStack, child->view, (Padding){0}, 0);
    releaseObj(vStack);
    CFRelease(errorText);
    hideView(child->errorLabel, true);
    ((void(*)(id,SEL,id))objc_msgSend)(child->field, sel_getUid("setInputAccessoryView:"), toolbar);
    return child->view;
}

void inputView_reset(struct InputView *this, short value) {
    this->valid = true;
    this->result = value;
    hideView(this->errorLabel, true);
}

static void showInputError(Validator *validator, struct InputView *child) {
    enableButton(validator->button, false);
    child->valid = false;
    hideView(child->errorLabel, false);
}

bool checkInput(Validator *this, id field, CFRange range, CFStringRef replacement) {
    int len = (int) CFStringGetLength(replacement);
    if (len) {
        CFStringInlineBuffer buf;
        CFStringInitInlineBuffer(replacement, &buf, CFRangeMake(0, len));
        for (int i = 0; i < len; ++i) {
            if (!uset_contains(char, this->set, CFStringGetCharacterFromInlineBuffer(&buf, i)))
                return false;
        }
    }

    int i = ((int(*)(id,SEL))objc_msgSend)(field, sel_getUid("tag"));
    if (i == this->count) return true;
    struct InputView *child = &this->children[i];

    CFStringRef text = ((CFStringRef(*)(id,SEL))objc_msgSend)(field, sel_getUid("text"));
    CFMutableStringRef newText = CFStringCreateMutableCopy(NULL, 0, text ? text : CFSTR(""));
    CFStringReplace(newText, range, replacement);

    if (!CFStringGetLength(newText)) {
        CFRelease(newText);
        showInputError(this, child);
        return true;
    }

    short newVal = CFStringGetIntValue(newText);
    CFRelease(newText);

    if (newVal < child->minVal || newVal > child->maxVal) {
        showInputError(this, child);
        return true;
    }

    inputView_reset(child, newVal);

    for (i = 0; i < this->count; ++i) {
        if (!this->children[i].valid) return true;
    }

    enableButton(this->button, true);
    return true;
}

#pragma mark - VC Functions

void setupNavVC(id navVC, id firstVC) {
    CFArrayRef array = CFArrayCreate(NULL, (const void *[]){firstVC}, 1, &kCocoaArrCallbacks);
    ((void(*)(id,SEL,CFArrayRef))objc_msgSend)(navVC, sel_getUid("setViewControllers:"), array);
    releaseObj(firstVC);
    CFRelease(array);
}

id getFirstVC(id navVC) {
    CFArrayRef ctrls = ((CFArrayRef(*)(id,SEL))objc_msgSend)(navVC, sel_getUid("viewControllers"));
    return (id) CFArrayGetValueAtIndex(ctrls, 0);
}

id allocNavVC(void) {
    return allocClass("UINavigationController");
}

void presentVC(id presenter, id child) {
    ((void(*)(id,SEL,id,bool,id))objc_msgSend)
    (presenter, sel_getUid("presentViewController:animated:completion:"), child, true, nil);
}

void presentModalVC(id presenter, id modal) {
    id container = ((id(*)(id,SEL,id))objc_msgSend)
    (allocNavVC(), sel_getUid("initWithRootViewController:"), modal);
    presentVC(presenter, container);
    releaseObj(container);
    releaseObj(modal);
}

void dismissPresentedVC(id presenter) {
    ((void(*)(id,SEL,bool,id))objc_msgSend)
    (presenter, sel_getUid("dismissViewControllerAnimated:completion:"), true, nil);
}

id getRootView(id vc) {
    return ((id(*)(id,SEL))objc_msgSend)(vc, sel_getUid("view"));
}

void addSubview(id view, id subview) {
    ((void(*)(id,SEL,id))objc_msgSend)(view, sel_getUid("addSubview:"), subview);
}

id createAlertController(CFStringRef title, CFStringRef message) {
    return ((id(*)(Class,SEL,CFStringRef,CFStringRef,int))objc_msgSend)
    (objc_getClass("UIAlertController"),
     sel_getUid("alertControllerWithTitle:message:preferredStyle:"), title, message, 1);
}

id createAlertAction(CFStringRef title, int style, CallbackBlock handler) {
    return ((id(*)(Class,SEL,CFStringRef,int,void(^)(id)))objc_msgSend)
    (objc_getClass("UIAlertAction"), sel_getUid("actionWithTitle:style:handler:"), title, style,
     ^(id action _U_) {
        if (handler)
            handler();
    });
}

void addAlertAction(id ctrl, id action) {
    ((void(*)(id,SEL,id))objc_msgSend)(ctrl, sel_getUid("addAction:"), action);
}

#pragma mark - View Functions

void setWidth(id v, int width) {
    activateConstraints((id []){createConstraint(getAnchor(v, anchors.width), nil, width)}, 1);
}

void setHeight(id v, int height) {
    activateConstraints((id []){createConstraint(getAnchor(v,anchors.height), nil, height)}, 1);
}

void setEqualWidths(id v, id v2) {
    activateConstraints((id []){
        createConstraint(getAnchor(v, anchors.width), getAnchor(v2, anchors.width), 0)
    }, 1);
}

void setEqualCenterY(id v, id v2) {
    activateConstraints((id []){
        createConstraint(getAnchor(v, anchors.centerY), getAnchor(v2, anchors.centerY), 0)
    }, 1);
}

void pinTopToTop(id v1, id v2, int offset) {
    activateConstraints((id []){
        createConstraint(getAnchor(v1, anchors.top), getAnchor(v2, anchors.top), offset)
    }, 1);
}

void pinTopToBottom(id v1, id v2, int offset) {
    activateConstraints((id []){
        createConstraint(getAnchor(v1, anchors.top), getAnchor(v2, anchors.bottom), offset)
    }, 1);
}

void pinRightToRight(id v1, id v2, int offset) {
    activateConstraints((id []){
        createConstraint(getAnchor(v1, anchors.right), getAnchor(v2, anchors.right), offset)
    }, 1);
}

void pinRightToLeft(id v1, id v2, int offset) {
    activateConstraints((id []){
        createConstraint(getAnchor(v1, anchors.right), getAnchor(v2, anchors.left), offset)
    }, 1);
}

void pin(id v, id container, Padding margins, uint excluded) {
    id constraints[4] = {0};
    const uint edges[] = {EdgeTop, EdgeLeft, EdgeBottom, EdgeRight};
    int j = 0;
    for (int i = 0; i < 4; ++i) {
        if (edges[i] & excluded) continue;
        id c;
        switch (edges[i]) {
            case EdgeTop:
                c = createConstraint(getAnchor(v, anchors.top),
                                     getAnchor(container, anchors.top), margins.top);
                break;
            case EdgeLeft:
                c = createConstraint(getAnchor(v, anchors.left),
                                     getAnchor(container, anchors.left), margins.left);
                break;
            case EdgeBottom:
                c = createConstraint(getAnchor(container, anchors.bottom),
                                     getAnchor(v, anchors.bottom), margins.bottom);
                break;
            default:
                c = createConstraint(getAnchor(container, anchors.right),
                                     getAnchor(v, anchors.right), margins.right);
                break;
        }
        constraints[j++] = c;
    }
    activateConstraints(constraints, j);
}

id createObjectWithFrame(const char *name, CGRect rect) {
    return ((id(*)(id,SEL,CGRect))objc_msgSend)(allocClass(name),
                                                sel_getUid("initWithFrame:"), rect);
}

id createView(id color, bool rounded, int width, int height) {
    id view = createObjectWithFrame("UIView", CGRectZero);
    disableAutoresizing(view);
    setBackground(view, color);
    if (rounded)
        setCornerRadius(view);
    if (width >= 0)
        setWidth(view, width);
    if (height >= 0)
        setHeight(view, height);
    return view;
}

id createStackView(id *subviews, int count, int axis, int spacing, Padding margins) {
    id view;
    const char *name = "UIStackView";
    if (count) {
        CFArrayRef array = CFArrayCreate(NULL, (const void **)subviews, count, &kCocoaArrCallbacks);
        view = ((id(*)(id,SEL,CFArrayRef))objc_msgSend)
        (allocClass(name), sel_getUid("initWithArrangedSubviews:"), array);
        CFRelease(array);
    } else {
        view = createObjectWithFrame(name, CGRectZero);
    }
    disableAutoresizing(view);
    ((void(*)(id,SEL,int))objc_msgSend)(view, sel_getUid("setAxis:"), axis);
    ((void(*)(id,SEL,CGFloat))objc_msgSend)(view, sel_getUid("setSpacing:"), spacing);
    ((void(*)(id,SEL,bool))objc_msgSend)(view, sel_getUid("setLayoutMarginsRelativeArrangement:"),
                                         true);
    setLayoutMargins(view, margins);
    return view;
}

id createScrollView(void) {
    id view = createObjectWithFrame("UIScrollView", CGRectZero);
    disableAutoresizing(view);
    ((void(*)(id,SEL,int))objc_msgSend)(view, sel_getUid("setAutoresizingMask:"), 16);
    ((void(*)(id,SEL,bool))objc_msgSend)(view, sel_getUid("setBounces:"), true);
    ((void(*)(id,SEL,bool))objc_msgSend)(view, sel_getUid("setShowsVerticalScrollIndicator:"),
                                         true);
    return view;
}

id createLabel(CFStringRef text, int style, int alignment, int height) {
    id view = createObjectWithFrame("UILabel", CGRectZero);
    disableAutoresizing(view);
    setLabelText(view, text);
    setLabelFontWithStyle(view, style);
    setDynamicFont(view);
    setTextColor(view, createColor("labelColor"));
    setAlignment(view, alignment);
    setHeight(view, height);
    return view;
}

id createContainer(SectionContainer *c, CFStringRef title, int hidden, int spacing, bool margins) {
    c->views = array_new(object);
    c->view = createView(nil, false, -1, -1);
    c->divider = createDivider();
    c->headerLabel = createLabel(title, TextTitle3, 4, 20);
    c->stack = createStackView(NULL, 0, 1, spacing, (Padding){.top = 5});
    Padding padding = {0};
    if (margins)
        padding.left = padding.right = 8;
    id vStack = createStackView((id []){c->divider, c->headerLabel, c->stack}, 3, 1, 0, padding);
    addSubview(c->view, vStack);
    ((void(*)(id,SEL,CGFloat,id))objc_msgSend)(vStack, sel_getUid("setCustomSpacing:afterView:"),
                                               20, c->divider);
    pin(vStack, c->view, (Padding){0}, 0);

    if (hidden & HideDivider)
        hideView(c->divider, true);
    if (hidden & HideLabel)
        hideView(c->headerLabel, true);

    releaseObj(vStack);
    return c->view;
}

void containers_free(SectionContainer *c, int size) {
    for (int i = 0; i < size; ++i) {
        if (c[i].view) {
            releaseObj(c[i].view);
            releaseObj(c[i].divider);
            releaseObj(c[i].headerLabel);
            releaseObj(c[i].stack);
            array_free(object, c[i].views);
        }
    }
}

void container_add(SectionContainer *c, id v) {
    array_push_back(object, c->views, v);
    ((void(*)(id,SEL,id))objc_msgSend)(c->stack, sel_getUid("addArrangedSubview:"), v);
}

id createButton(CFStringRef title, id color, int params, int tag, id target, SEL action, int h) {
    id view = ((id(*)(Class,SEL,int))objc_msgSend)(objc_getClass("UIButton"),
                                                   sel_getUid("buttonWithType:"), 1);
    disableAutoresizing(view);
    setButtonTitle(view, title, 0);
    setButtonColor(view, color, 0);
    setButtonColor(view, createColor("secondaryLabelColor"), 2);
    id label = ((id(*)(id,SEL))objc_msgSend)(view, sel_getUid("titleLabel"));
    int style = (params & BtnLargeFont) ? TextHead : TextBody;
    setLabelFontWithStyle(label, style);
    setDynamicFont(label);
    if (params & BtnBackground)
        setBackground(view, createColor("secondarySystemGroupedBackgroundColor"));
    if (params & BtnRounded)
        setCornerRadius(view);
    setTag(view, tag);
    addTarget(view, target, action, 64);
    if (h >= 0)
        setHeight(view, h);
    return view;
}

id createSegmentedControl(CFStringRef format, int count, int startIndex,
                          id target, SEL action, int height) {
    CFStringRef segments[count];
    fillStringArray(segments, format, count);
    CFArrayRef array = CFArrayCreate(NULL, (const void **)segments, count, &kCocoaArrCallbacks);
    id view = ((id(*)(id,SEL,CFArrayRef))objc_msgSend)(allocClass("UISegmentedControl"),
                                                       sel_getUid("initWithItems:"), array);
    disableAutoresizing(view);
    ((void(*)(id,SEL,int))objc_msgSend)(view, sel_getUid("setSelectedSegmentIndex:"), startIndex);
    setCornerRadius(view);
    setTintColor(view, createColor("systemGray2Color"));
    if (action)
        addTarget(view, target, action, 4096);
    if (height >= 0)
        setHeight(view, height);
    CFRelease(array);
    return view;
}

id createTextfield(id delegate, CFStringRef text, int alignment, int keyboard, int tag, int h) {
    id view = createObjectWithFrame("UITextField", CGRectZero);
    disableAutoresizing(view);
    setBackground(view, createColor("tertiarySystemBackgroundColor"));
    setLabelText(view, text);
    setAlignment(view, alignment);
    setTag(view, tag);
    ((void(*)(id,SEL,int))objc_msgSend)(view, sel_getUid("setBorderStyle:"), 3);
    ((void(*)(id,SEL,int))objc_msgSend)(view, sel_getUid("setKeyboardType:"), keyboard);
    ((void(*)(id,SEL,id))objc_msgSend)(view, sel_getUid("setDelegate:"), delegate);
    setHeight(view, h);
    return view;
}

void hideView(id view, bool hide) {
    ((void(*)(id,SEL,bool))objc_msgSend)(view, sel_getUid("setHidden:"), hide);
}

void removeView(id v) {
    ((void(*)(id,SEL))objc_msgSend)(v, sel_getUid("removeFromSuperview"));
}

void enableButton(id view, bool enabled) {
    ((void(*)(id,SEL,bool))objc_msgSend)(view, sel_getUid("setEnabled:"), enabled);
}

void setTag(id view, int tag) {
    ((void(*)(id,SEL,int))objc_msgSend)(view, sel_getUid("setTag:"), tag);
}

void setTextColor(id view, id color) {
    ((void(*)(id,SEL,id))objc_msgSend)(view, sel_getUid("setTextColor:"), color);
}

void setBackground(id view, id color) {
    ((void(*)(id,SEL,id))objc_msgSend)(view, sel_getUid("setBackgroundColor:"), color);
}

void setTintColor(id view, id color) {
    ((void(*)(id,SEL,id))objc_msgSend)(view, sel_getUid("setTintColor:"), color);
}

void setLabelText(id view, CFStringRef text) {
    ((void(*)(id,SEL,CFStringRef))objc_msgSend)(view, sel_getUid("setText:"), text);
}

void setLabelFontWithStyle(id view, int style) {
    if (!style) return;
    id fontStyle;
    switch (style) {
        case TextFootnote:
            fontStyle = UIFontTextStyleFootnote;
            break;
        case TextSubhead:
            fontStyle = UIFontTextStyleSubheadline;
            break;
        case TextBody:
            fontStyle = UIFontTextStyleBody;
            break;
        case TextHead:
            fontStyle = UIFontTextStyleHeadline;
            break;
        case TextTitle1:
            fontStyle = UIFontTextStyleTitle1;
            break;
        case TextTitle2:
            fontStyle = UIFontTextStyleTitle2;
            break;
        default:
            fontStyle = UIFontTextStyleTitle3;
    }
    id font = ((id(*)(Class,SEL,CFStringRef))objc_msgSend)
    (objc_getClass("UIFont"), sel_getUid("preferredFontForTextStyle:"), (CFStringRef)fontStyle);
    setFont(view, font);
}

void setLabelFontWithSize(id view, CGFloat size) {
    id font = ((id(*)(Class,SEL,CGFloat))objc_msgSend)
    (objc_getClass("UIFont"), sel_getUid("systemFontOfSize:"), size);
    setFont(view, font);
}

void setButtonTitle(id view, CFStringRef title, int state) {
    ((void(*)(id,SEL,CFStringRef,int))objc_msgSend)(view, sel_getUid("setTitle:forState:"),
                                                    title, state);
}

void setButtonColor(id view, id color, int state) {
    ((void(*)(id,SEL,id,int))objc_msgSend)(view, sel_getUid("setTitleColor:forState:"),
                                           color, state);
}
