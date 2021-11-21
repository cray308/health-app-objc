#include "InputVC.h"
#include "ViewControllerHelpers.h"

#define toggleScrolling(_v, _enable) setBool(_v, sel_getUid("setScrollEnabled:"), _enable)

gen_uset_source(char, unsigned short, ds_cmp_num_eq, DSDefault_addrOfVal, DSDefault_sizeOfVal,
                DSDefault_shallowCopy, DSDefault_shallowDelete)

extern id UIKeyboardDidShowNotification;
extern id UIKeyboardWillHideNotification;
extern id UIKeyboardFrameEndUserInfoKey;

Class InputVCClass;
Ivar InputVCDataRef;
Class InputViewClass;
Ivar InputViewDataRef;

static CFStringRef inputFieldError;

void initValidatorStrings(void) {
    inputFieldError = localize(CFSTR("inputFieldError"));
}

static inline void setScrollInsets(id sv, HAInsets margins) {
    ((void(*)(id,SEL,HAInsets))objc_msgSend)(sv, sel_getUid("setContentInset:"), margins);
    ((void(*)(id,SEL,HAInsets))objc_msgSend)(sv, sel_getUid("setScrollIndicatorInsets:"), margins);
}

static void showInputError(InputVCData *data, InputViewData *child) {
    enableButton(data->button, false);
    child->valid = false;
    hideView(child->errorLabel, false);
    CFStringRef text = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@. %@"),
                                                child->hintText, child->errorText);
    setAccessibilityLabel(child->field, text);
    CFRelease(text);
}

void inputVC_addChild(id self, CFStringRef hint, short min, short max) {
    InputVCData *d = (InputVCData *) object_getIvar(self, InputVCDataRef);

    int index = d->count++;
    d->children[index] = createObjectWithFrame(InputViewClass, CGRectZero);
    id view = d->children[index];
    InputViewData *ptr = calloc(1, sizeof(InputViewData));
    ptr->minVal = min;
    ptr->maxVal = max;
    ptr->hintText = hint;
    ptr->errorText = CFStringCreateWithFormat(NULL, NULL, inputFieldError, min, max);
    ptr->hintLabel = createLabel(hint, TextFootnote, 4, false);
    ptr->field = createTextfield(self, NULL, hint, 4, 4, index);
    ptr->errorLabel = createLabel(ptr->errorText, TextFootnote, 4, false);
    DMLabel *errorPtr = (DMLabel *) ptr->errorLabel;
    errorPtr->colorCode = ColorRed;
    setTextColor(ptr->errorLabel, createColor(ColorRed));

    id vStack = createStackView((id []){ptr->hintLabel, ptr->field, ptr->errorLabel},
                                3, 1, 4, (Padding){4, 8, 4, 8});
    addSubview(view, vStack);
    pin(vStack, view, (Padding){0}, 0);
    addArrangedSubview(d->vStack, view);
    releaseObj(vStack);
    hideView(ptr->errorLabel, true);
    setInputAccessory(ptr->field, d->toolbar);
    object_setIvar(view, InputViewDataRef, (id) ptr);
}

void inputView_deinit(id self, SEL _cmd) {
    InputViewData *ptr = (InputViewData *) object_getIvar(self, InputViewDataRef);
    struct objc_super super = {self, objc_getClass("UIView")};
    CFRelease(ptr->errorText);
    releaseObj(ptr->hintLabel);
    releaseObj(ptr->field);
    releaseObj(ptr->errorLabel);
    free(ptr);
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);
}

void inputView_reset(InputViewData *data, short value) {
    data->valid = true;
    data->result = value;
    hideView(data->errorLabel, true);
    setAccessibilityLabel(data->field, data->hintText);
}

id inputVC_init(id self, SEL _cmd, id nibName, id bundle) {
    struct objc_super super = {self, objc_getClass("UIViewController")};
    self = ((id(*)(struct objc_super *,SEL,id,id))objc_msgSendSuper)(&super, _cmd, nibName, bundle);
    if (!self) return nil;

    InputVCData *data = calloc(1, sizeof(InputVCData));
    object_setIvar(self, InputVCDataRef, (id) data);
    return self;
}

void inputVC_deinit(id self, SEL _cmd) {
    InputVCData *data = (InputVCData *) object_getIvar(self, InputVCDataRef);
    struct objc_super super = {self, objc_getClass("UIViewController")};

    uset_free(char, data->set);
    for (int i = 0; i < 4; ++i) {
        if (data->children[i])
            releaseObj(data->children[i]);
    }
    releaseObj(data->toolbar);
    releaseObj(data->scrollView);
    releaseObj(data->vStack);
    id center = getDeviceNotificationCenter();
    SEL sig = sel_getUid("removeObserver:name:object:");
    ((void(*)(id,SEL,id,id,id))objc_msgSend)(center, sig, self, UIKeyboardDidShowNotification, nil);
    ((void(*)(id,SEL,id,id,id))objc_msgSend)(center, sig,
                                             self, UIKeyboardWillHideNotification, nil);
    free(data);
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);
}

void inputVC_viewDidLoad(id self, SEL _cmd) {
    struct objc_super super = {self, objc_getClass("UIViewController")};
    ((void(*)(struct objc_super *,SEL))objc_msgSendSuper)(&super, _cmd);

    unsigned short nums[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    CGRect bounds;
    getScreenBounds(&bounds);
    CGFloat width = bounds.size.width;

    InputVCData *data = (InputVCData *) object_getIvar(self, InputVCDataRef);
    data->set = uset_new_fromArray(char, nums, 10);
    data->scrollView = createScrollView();
    data->vStack = createStackView(nil, 0, 1, 0, (Padding){0});
    data->toolbar = createObjectWithFrame(objc_getClass("UIToolbar"), ((CGRect){{0}, {width, 50}}));
    voidFunc(data->toolbar, sel_getUid("sizeToFit"));

    Class btnClass = objc_getClass("UIBarButtonItem");
    id flexSpace = ((id(*)(id,SEL,int,id,SEL))objc_msgSend)
    (allocClass(btnClass), sel_getUid("initWithBarButtonSystemItem:target:action:"), 5, nil, nil);
    id doneButton = ((id(*)(id,SEL,CFStringRef,int,id,SEL))objc_msgSend)
    (allocClass(btnClass), sel_getUid("initWithTitle:style:target:action:"),
     CFSTR("Done"), 0, self, sel_getUid("dismissKeyboard"));

    if (osVersion < Version14)
        setTintColor(doneButton, createColor(ColorRed));

    CFArrayRef array = CFArrayCreate(NULL, (const void *[]){flexSpace, doneButton},
                                     2, &retainedArrCallbacks);
    ((void(*)(id,SEL,CFArrayRef,bool))objc_msgSend)(data->toolbar,
                                                    sel_getUid("setItems:animated:"), array, false);
    toggleInteraction(data->toolbar, true);

    id view = getView(self);
    addSubview(view, data->scrollView);
    id guide = getLayoutGuide(view);
    pin(data->scrollView, guide, (Padding){0}, 0);
    addVStackToScrollView(data->vStack, data->scrollView);

    CFRelease(array);
    releaseObj(flexSpace);
    releaseObj(doneButton);

    id center = getDeviceNotificationCenter();
    SEL obsSig = sel_getUid("addObserver:selector:name:object:");
    ((void(*)(id,SEL,id,SEL,id,id))objc_msgSend)(center, obsSig, self, sel_getUid("keyboardShown:"),
                                                 UIKeyboardDidShowNotification, nil);
    ((void(*)(id,SEL,id,SEL,id,id))objc_msgSend)
    (center, obsSig, self, sel_getUid("keyboardWillHide:"), UIKeyboardWillHideNotification, nil);
}

void inputVC_viewDidAppear(id self, SEL _cmd, bool animated) {
    struct objc_super super = {self, objc_getClass("UIViewController")};
    ((void(*)(struct objc_super *,SEL,bool))objc_msgSendSuper)(&super, _cmd, animated);

    InputVCData *data = (InputVCData *) object_getIvar(self, InputVCDataRef);
    if (!data->scrollHeight) {
        CGRect bounds;
        getRect(data->scrollView, &bounds, 1);
        data->scrollHeight = bounds.size.height;
    }
}

void inputVC_dismissKeyboard(id self, SEL _cmd _U_) {
    id view = getView(self);
    setBool(view, sel_getUid("endEditing:"), true);
}

bool inputVC_fieldShouldReturn(id self _U_, SEL _cmd _U_, id field) {
    return getBool(field, sel_getUid("resignFirstResponder"));
}

void inputVC_keyboardShown(id self, SEL _cmd _U_, id notif) {
    InputVCData *data = (InputVCData *) object_getIvar(self, InputVCDataRef);
    toggleScrolling(data->scrollView, true);
    id userInfo = getObject(notif, sel_getUid("userInfo"));
    id info = getObjectWithObject(userInfo,
                                  sel_getUid("objectForKey:"), UIKeyboardFrameEndUserInfoKey);

    CGRect kbRect, viewRect, fieldRect, fieldInView;
    id view = getView(self);
    getRect(data->activeField, &fieldRect, 1);
    getRect(view, &viewRect, 0);

#if defined(__arm64__)
    kbRect = ((CGRect(*)(id,SEL))objc_msgSend)(info, sel_getUid("CGRectValue"));
    fieldInView = ((CGRect(*)(id,SEL,CGRect,id))objc_msgSend)
    (view, sel_getUid("convertRect:fromView:"), fieldRect, data->activeField);
#else
    ((void(*)(CGRect*,id,SEL))objc_msgSend_stret)(&kbRect, info, sel_getUid("CGRectValue"));
    ((void(*)(CGRect*,id,SEL,CGRect,id))objc_msgSend_stret)
    (&fieldInView, view, sel_getUid("convertRect:fromView:"), fieldRect, data->activeField);
#endif

    viewRect.size.height -= kbRect.size.height;
    setScrollInsets(data->scrollView, (HAInsets){0, 0, kbRect.size.height, 0});

    CGPoint upperY = fieldInView.origin;
    upperY.y += fieldInView.size.height;
    if (!(CGRectContainsPoint(viewRect, fieldInView.origin) &&
          CGRectContainsPoint(viewRect, upperY))) {
        ((void(*)(id,SEL,CGRect,bool))objc_msgSend)
        (data->scrollView, sel_getUid("scrollRectToVisible:animated:"), fieldRect, true);
    }
}

void inputVC_keyboardWillHide(id self, SEL _cmd _U_, id notif _U_) {
    InputVCData *data = (InputVCData *) object_getIvar(self, InputVCDataRef);
    setScrollInsets(data->scrollView, (HAInsets){0});
    CGRect bounds;
    getRect(data->vStack, &bounds, 1);
    toggleScrolling(data->scrollView, (int) bounds.size.height >= data->scrollHeight);
}

void inputVC_fieldBeganEditing(id self, SEL _cmd _U_, id field) {
    ((InputVCData *) object_getIvar(self, InputVCDataRef))->activeField = field;
}

void inputVC_fieldStoppedEditing(id self, SEL _cmd _U_, id field _U_) {
    ((InputVCData *) object_getIvar(self, InputVCDataRef))->activeField = nil;
}

bool inputVC_fieldChanged(id self, SEL _cmd _U_, id field, CFRange range, CFStringRef replacement) {
    InputVCData *d = (InputVCData *) object_getIvar(self, InputVCDataRef);

    int len = (int) CFStringGetLength(replacement);
    if (len) {
        CFStringInlineBuffer buf;
        CFStringInitInlineBuffer(replacement, &buf, CFRangeMake(0, len));
        for (int i = 0; i < len; ++i) {
            if (!uset_contains(char, d->set, CFStringGetCharacterFromInlineBuffer(&buf, i)))
                return false;
        }
    }

    int i = getTag(field);
    if (i == d->count) return true;
    InputViewData *child = (InputViewData *) object_getIvar(d->children[i], InputViewDataRef);

    CFStringRef text = getText(field);
    CFMutableStringRef newText = CFStringCreateMutableCopy(NULL, 0, text ? text : CFSTR(""));
    CFStringReplace(newText, range, replacement);

    if (!CFStringGetLength(newText)) {
        CFRelease(newText);
        showInputError(d, child);
        return true;
    }

    short newVal = CFStringGetIntValue(newText);
    CFRelease(newText);

    if (newVal < child->minVal || newVal > child->maxVal) {
        showInputError(d, child);
        return true;
    }

    inputView_reset(child, newVal);
    for (i = 0; i < d->count; ++i) {
        child = (InputViewData *) object_getIvar(d->children[i], InputViewDataRef);
        if (!child->valid) return true;
    }

    enableButton(d->button, true);
    return true;
}
