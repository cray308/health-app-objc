#include "InputVC.h"
#include <CoreFoundation/CFNotificationCenter.h>
#include <string.h>
#include "AppDelegate.h"
#include "Views.h"

extern CFStringRef UIKeyboardDidShowNotification;
extern CFStringRef UIKeyboardWillHideNotification;
extern CFStringRef UIKeyboardFrameEndUserInfoKey;

Class InputVCClass;
Class InputViewClass;

struct InputCache {
    const SEL sse, sci;
    void (*setScroll)(id,SEL,bool);
    void (*setInset)(id,SEL,HAInsets);
};

static CFStringRef inputFieldError;
static struct InputCache cache;

static void keyboardShown(CFNotificationCenterRef ctr _U_, void *self,
                          CFNotificationName name _U_, const void *obj _U_, CFDictionaryRef userinfo) {
    InputVC *data = (InputVC *)((char *)self + VCSize);
    cache.setScroll(data->scrollView, cache.sse, true);
    id value = (id)CFDictionaryGetValue(userinfo, UIKeyboardFrameEndUserInfoKey);
    CGRect kbRect, viewRect, fieldRect, fieldInView;
    id view = msg0(id, (id)self, sel_getUid("view"));
    getRect(data->activeField, &fieldRect, RectBounds);
    getRect(view, &viewRect, RectFrame);

#if defined(__arm64__)
    kbRect = msg0(CGRect, value, sel_getUid("CGRectValue"));
    fieldInView = msg2(CGRect, CGRect, id, view,
                       sel_getUid("convertRect:fromView:"), fieldRect, data->activeField);
#else
    ((void(*)(CGRect*,id,SEL))objc_msgSend_stret)(&kbRect, value, sel_getUid("CGRectValue"));
    (((void(*)(CGRect*,id,SEL,CGRect,id))objc_msgSend_stret)
     (&fieldInView, view, sel_getUid("convertRect:fromView:"), fieldRect, data->activeField));
#endif

    viewRect.size.height -= kbRect.size.height;
    cache.setInset(data->scrollView, cache.sci, (HAInsets){data->topOffset, 0, kbRect.size.height, 0});

    CGPoint upperY = {fieldInView.origin.x, fieldInView.origin.y + fieldInView.size.height};
    if (!(CGRectContainsPoint(viewRect, fieldInView.origin) && CGRectContainsPoint(viewRect, upperY)))
        msg2(void, CGRect, bool, data->scrollView,
             sel_getUid("scrollRectToVisible:animated:"), fieldRect, true);
}

static void keyboardWillHide(CFNotificationCenterRef ctr _U_, void *self,
                             CFNotificationName name _U_, const void *obj _U_, CFDictionaryRef info _U_) {
    InputVC *data = (InputVC *)((char *)self + VCSize);
    cache.setInset(data->scrollView, cache.sci, (HAInsets){data->topOffset, 0, data->bottomOffset, 0});
    CGRect bounds;
    getRect(data->vStack, &bounds, RectBounds);
    cache.setScroll(data->scrollView, cache.sse, (int)bounds.size.height >= data->scrollHeight);
}

void initValidatorStrings(CFBundleRef bundle) {
    inputFieldError = localize(bundle, CFSTR("inputFieldError"));
    SEL sse = sel_getUid("setScrollEnabled:"), sci = sel_getUid("setContentInset:");
    Class Scroll = objc_getClass("UIScrollView");
    memcpy(&cache, &(struct InputCache){sse, sci, (void(*)(id,SEL,bool))getImpO(Scroll, sse),
        (void(*)(id,SEL,HAInsets))getImpO(Scroll, sci)
    }, sizeof(struct InputCache));
}

static void inputView_reset(InputView *data, short value, VCacheRef tbl) {
    data->valid = true;
    data->result = value;
    tbl->view.hide(data->errorLabel, tbl->view.shd, true);
    tbl->view.setAcc(data->field, tbl->view.sacl, tbl->label.getText(data->hintLabel, tbl->label.gtxt));
}

static void showInputError(InputView *child, VCacheRef tbl) {
    child->valid = false;
    tbl->view.hide(child->errorLabel, tbl->view.shd, false);
    CFStringRef hintText = tbl->label.getText(child->hintLabel, tbl->label.gtxt);
    CFStringRef errorText = tbl->label.getText(child->errorLabel, tbl->label.gtxt);
    CFStringRef text = formatStr(CFSTR("%@. %@"), hintText, errorText);
    tbl->view.setAcc(child->field, tbl->view.sacl, text);
    CFRelease(text);
}

void inputVC_addChild(id self, CFStringRef hint, short min, short max) {
    InputVC *d = (InputVC *)((char *)self + VCSize);
    VCacheRef tbl = d->tbl;

    int index = d->count++;
    id view = Sels.new(InputViewClass, Sels.nw);
    d->children[index] = (InputView *)((char *)view + ViewSize);
    InputView *ptr = d->children[index];
    ptr->minVal = min;
    ptr->maxVal = max;
    CFStringRef errorText = formatStr(inputFieldError, min, max);
    CFRetain(hint);
    ptr->hintLabel = createLabel(tbl, d->clr, hint, UIFontTextStyleFootnote, false);
    ptr->field = createTextfield(tbl, d->clr, self, CFSTR(""), hint, 4, 4, index);
    if (d->setKB)
        msg1(void, long, ptr->field, sel_getUid("setKeyboardAppearance:"), 1);
    ptr->errorLabel = createLabel(tbl, d->clr, errorText, UIFontTextStyleFootnote, false);
    tbl->label.setColor(ptr->errorLabel, tbl->label.stc, d->clr->getColor(d->clr->cls, d->clr->sc, ColorRed));

    id vStack = createStackView(tbl, (id []){ptr->hintLabel, ptr->field, ptr->errorLabel},
                                3, 1, 0, 4, (Padding){4, 8, 4, 8});
    tbl->view.setTrans(vStack, tbl->view.trans, false);
    tbl->view.addSub(view, tbl->view.asv, vStack);
    pin(&tbl->cc, vStack, view);
    tbl->stack.addSub(d->vStack, tbl->stack.asv, view);
    Sels.viewRel(vStack, Sels.rel);
    tbl->view.hide(ptr->errorLabel, tbl->view.shd, true);
    tbl->field.setInput(ptr->field, tbl->field.siac, d->toolbar);
}

void inputVC_updateFields(InputVC *self, const short *vals) {
    int count = self->count;
    for (int i = 0; i < count; ++i) {
        short value = vals[i];
        InputView *child = self->children[i];
        CFStringRef str = formatStr(CFSTR("%d"), value);
        self->tbl->field.setText(child->field, self->tbl->label.stxt, str);
        inputView_reset(child, value, self->tbl);
        CFRelease(str);
    }
    self->tbl->button.setEnabled(self->button, self->tbl->button.en, true);
}

void inputView_deinit(id self, SEL _cmd) {
    InputView *ptr = (InputView *)((char *)self + ViewSize);
    Sels.viewRel(ptr->hintLabel, Sels.rel);
    Sels.viewRel(ptr->field, Sels.rel);
    Sels.viewRel(ptr->errorLabel, Sels.rel);
    msgSup0(void, (&(struct objc_super){self, View}), _cmd);
}

id inputVC_init(id self, SEL _cmd _U_, VCacheRef tbl, CCacheRef clr) {
    self = msgSup0(id, (&(struct objc_super){self, VC}), sel_getUid("init"));
    InputVC *iv = (InputVC *)((char *)self + VCSize);
    iv->tbl = tbl;
    iv->clr = clr;
    return self;
}

void inputVC_deinit(id self, SEL _cmd) {
    InputVC *data = (InputVC *)((char *)self + VCSize);
    for (int i = 0; i < 4; ++i) {
        InputView *child = data->children[i];
        if (child)
            Sels.viewRel((id)((char *)child - ViewSize), Sels.rel);
    }
    Sels.viewRel(data->toolbar, Sels.rel);
    Sels.viewRel(data->scrollView, Sels.rel);
    Sels.viewRel(data->vStack, Sels.rel);
    CFNotificationCenterRemoveEveryObserver(CFNotificationCenterGetLocalCenter(), self);
    msgSup0(void, (&(struct objc_super){self, VC}), _cmd);
}

void inputVC_viewDidLoad(id self, SEL _cmd) {
    msgSup0(void, (&(struct objc_super){self, VC}), _cmd);

    CGRect bounds;
    getScreenBounds(&bounds);
    unsigned char dark = getUserInfo()->darkMode;

    InputVC *data = (InputVC *)((char *)self + VCSize);
    data->scrollView = createScrollView();
    data->vStack = createStackView(data->tbl, nil, 0, 1, 0, 0, (Padding){0});
    data->toolbar = msg1(id, CGRect, Sels.alloc(objc_getClass("UIToolbar"), Sels.alo),
                         sel_getUid("initWithFrame:"), ((CGRect){{0}, {bounds.size.width, 50}}));
    if (dark < 2) {
        msg1(void, id, data->toolbar, sel_getUid("setBarTintColor:"),
             clsF1(id, int, data->clr->cls, sel_getUid("getBarColorWithType:"), ColorBarModal));
        data->setKB = dark == 1;
    }
    msg0(void, data->toolbar, sel_getUid("sizeToFit"));

    CFStringRef doneLabel = localize(CFBundleGetMainBundle(), CFSTR("done"));
    Class BarItem = objc_getClass("UIBarButtonItem");
    id flexSpace = msg3(id, long, id, SEL, Sels.alloc(BarItem, Sels.alo),
                        sel_getUid("initWithBarButtonSystemItem:target:action:"), 5, nil, nil);
    id doneButton = msg4(id, CFStringRef, long, id, SEL, Sels.alloc(BarItem, Sels.alo),
                         sel_getUid("initWithTitle:style:target:action:"),
                         doneLabel, 0, self, sel_getUid("dismissKeyboard"));
    msg1(void, id, data->toolbar, sel_getUid("setTintColor:"),
         data->clr->getColor(data->clr->cls, data->clr->sc, ColorRed));
    CFRelease(doneLabel);

    CFArrayRef array = CFArrayCreate(NULL, (const void *[]){flexSpace, doneButton}, 2, &retainedArrCallbacks);
    msg2(void, CFArrayRef, bool, data->toolbar, sel_getUid("setItems:animated:"), array, false);

    addVStackToScrollView(data->tbl, msg0(id, self, sel_getUid("view")), data->vStack, data->scrollView);
    CFRelease(array);
    Sels.objRel(flexSpace, Sels.rel);
    Sels.objRel(doneButton, Sels.rel);

    CFNotificationCenterRef center = CFNotificationCenterGetLocalCenter();
    CFNotificationCenterAddObserver(center, self, keyboardShown, UIKeyboardDidShowNotification,
                                    NULL, CFNotificationSuspensionBehaviorDrop);
    CFNotificationCenterAddObserver(center, self, keyboardWillHide, UIKeyboardWillHideNotification,
                                    NULL, CFNotificationSuspensionBehaviorDrop);
}

void inputVC_viewDidAppear(id self, SEL _cmd, bool animated) {
    msgSup1(void, bool, (&(struct objc_super){self, VC}), _cmd, animated);

    InputVC *data = (InputVC *)((char *)self + VCSize);
    if (!data->scrollHeight) {
        CGRect bounds;
        HAInsets insets;
        getRect(data->scrollView, &bounds, RectBounds);
        data->scrollHeight = (int)bounds.size.height;
#if defined(__arm64__)
        insets = msg0(HAInsets, data->scrollView, sel_getUid("contentInset"));
#else
        ((void(*)(HAInsets*,id,SEL))objc_msgSend_stret)(&insets, data->scrollView,
                                                        sel_getUid("contentInset"));
#endif
        data->topOffset = (short)insets.top;
        data->bottomOffset = (short)insets.bottom;
    }
}

void inputVC_dismissKeyboard(id self, SEL _cmd _U_) {
    InputVC *d = (InputVC *)((char *)self + VCSize);
    int tag = d->activeField ? ((int)d->tbl->view.getTag(d->activeField, d->tbl->view.gtg)) : 255;
    if (tag >= d->count - 1) {
        msg1(bool, bool, msg0(id, self, sel_getUid("view")), sel_getUid("endEditing:"), true);
    } else {
        msg0(bool, d->children[tag + 1]->field, sel_getUid("becomeFirstResponder"));
    }
}

void inputVC_fieldBeganEditing(id self, SEL _cmd _U_, id field) {
    ((InputVC *)((char *)self + VCSize))->activeField = field;
}

void inputVC_fieldStoppedEditing(id self, SEL _cmd _U_, id field _U_) {
    ((InputVC *)((char *)self + VCSize))->activeField = nil;
}

bool inputVC_fieldChanged(id self, SEL _cmd _U_, id field, CFRange range, CFStringRef replacement) {
    InputVC *d = (InputVC *)((char *)self + VCSize);
    VCacheRef tbl = d->tbl;

    int len = (int)CFStringGetLength(replacement);
    if (len) {
        CFStringInlineBuffer buf;
        CFStringInitInlineBuffer(replacement, &buf, CFRangeMake(0, len));
        for (int i = 0; i < len; ++i) {
            UniChar val = CFStringGetCharacterFromInlineBuffer(&buf, i);
            if (val < 48 || val > 57) return false;
        }
    }

    int i = (int)tbl->view.getTag(field, tbl->view.gtg);
    InputView *child = d->children[i];

    CFStringRef text = msg0(CFStringRef, field, tbl->label.gtxt);
    if (range.location + range.length > CFStringGetLength(text)) return false;
    CFMutableStringRef newText = CFStringCreateMutableCopy(NULL, 0, text);
    CFStringReplace(newText, range, replacement);

    if (!CFStringGetLength(newText)) {
        CFRelease(newText);
        tbl->button.setEnabled(d->button, tbl->button.en, false);
        showInputError(child, tbl);
        return true;
    }

    int newVal = (int)CFStringGetIntValue(newText);
    CFRelease(newText);

    if (newVal < child->minVal || newVal > child->maxVal) {
        tbl->button.setEnabled(d->button, tbl->button.en, false);
        showInputError(child, tbl);
        return true;
    }

    inputView_reset(child, (short)newVal, tbl);
    for (i = 0; i < d->count; ++i) {
        if (!d->children[i]->valid) return true;
    }

    tbl->button.setEnabled(d->button, tbl->button.en, true);
    return true;
}
