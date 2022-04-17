#include "InputVC.h"
#include <CoreFoundation/CFNotificationCenter.h>
#include <CoreFoundation/CFCharacterSet.h>
#include <CoreFoundation/CFNumberFormatter.h>
#include <dispatch/queue.h>
#include <string.h>
#include "AppDelegate.h"
#include "Views.h"

extern CFStringRef UIKeyboardDidShowNotification;
extern CFStringRef UIKeyboardWillHideNotification;
extern CFStringRef UIKeyboardFrameEndUserInfoKey;
extern CFStringRef UIFontTextStyleFootnote;

Class InputVCClass;
Class InputViewClass;

struct InputCache {
    const SEL bnd, skbt, conv, cgrv, bfr, sse, sci, scrvs, lfn;
    bool (*becomeResponder)(id,SEL);
    void (*setScroll)(id,SEL,bool);
    void (*setInset)(id,SEL,HAInsets);
    void (*scrollRect)(id,SEL,CGRect,bool);
    void (*layout)(id,SEL);
};

static CFStringRef inputFieldError;
static CFCharacterSetRef charSet;
static CFNumberFormatterRef numFormatter;
static struct InputCache cache;

static void getBounds(id view, CGRect *result) {
#if defined(__arm64__)
    *result = msg0(CGRect, view, cache.bnd);
#else
    ((void(*)(CGRect*,id,SEL))objc_msgSend_stret)(result, view, cache.bnd);
#endif
}

static void checkScrollPos(id scroll, id child) {
    CGRect fieldRect, fieldInView;
    getBounds(child, &fieldRect);
#if defined(__arm64__)
    fieldInView = msg2(CGRect, CGRect, id, scroll, cache.conv, fieldRect, child);
#else
    (((void(*)(CGRect*,id,SEL,CGRect,id))objc_msgSend_stret)(&fieldInView, scroll, cache.conv,
                                                             fieldRect, child));
#endif
    fieldInView.size.height += 32;
    fieldInView.origin.y -= 16;
    cache.scrollRect(scroll, cache.scrvs, fieldInView, true);
}

static void keyboardShown(CFNotificationCenterRef ctr _U_, void *self,
                          CFNotificationName name _U_, const void *obj _U_, CFDictionaryRef info) {
    InputVC *d = (InputVC *)((char *)self + VCSize);
    int index = (int)d->tbl->view.getTag(d->activeField, d->tbl->view.gtg);
    if (index < 0) return;

    id value = (id)CFDictionaryGetValue(info, UIKeyboardFrameEndUserInfoKey);
    CGRect kbRect;
#if defined(__arm64__)
    kbRect = msg0(CGRect, value, cache.cgrv);
#else
    ((void(*)(CGRect*,id,SEL))objc_msgSend_stret)(&kbRect, value, cache.cgrv);
#endif

    cache.setScroll(d->scrollView, cache.sse, true);
    cache.setInset(d->scrollView, cache.sci, (HAInsets){d->topOffset, 0, kbRect.size.height, 0});
    checkScrollPos(d->scrollView, d->children[index].view);
}

static void keyboardWillHide(CFNotificationCenterRef ctr _U_, void *self,
                             CFNotificationName name _U_, const void *obj _U_, CFDictionaryRef info _U_) {
    InputVC *d = (InputVC *)((char *)self + VCSize);
    cache.setInset(d->scrollView, cache.sci, (HAInsets){d->topOffset, 0, d->bottomOffset, 0});
    CGRect bounds;
    getBounds(d->vStack, &bounds);
    cache.setScroll(d->scrollView, cache.sse, (int)bounds.size.height >= d->scrollHeight);
}

void initValidatorStrings(Class Field) {
    charSet = CFCharacterSetGetPredefined(kCFCharacterSetDecimalDigit);
    CFLocaleRef locale = CFLocaleCopyCurrent();
    numFormatter = CFNumberFormatterCreate(NULL, locale, kCFNumberFormatterDecimalStyle);
    CFRelease(locale);
    inputFieldError = localize(CFSTR("inputFieldError"));
    SEL sse = sel_getUid("setScrollEnabled:"), sci = sel_getUid("setContentInset:");
    SEL scrvs = sel_getUid("scrollRectToVisible:animated:"), bfr = sel_getUid("becomeFirstResponder");
    SEL lfn = sel_getUid("layoutIfNeeded");
    Class Scroll = objc_getClass("UIScrollView");
    memcpy(&cache, &(struct InputCache){sel_getUid("bounds"), sel_getUid("setKeyboardType:"),
        sel_getUid("convertRect:fromView:"), sel_getUid("CGRectValue"), bfr, sse, sci, scrvs, lfn,
        (bool(*)(id,SEL))getImpO(Field, bfr), (void(*)(id,SEL,bool))getImpO(Scroll, sse),
        (void(*)(id,SEL,HAInsets))getImpO(Scroll, sci),
        (void(*)(id,SEL,CGRect,bool))getImpO(Scroll, scrvs), (void(*)(id,SEL))getImpO(View, lfn)
    }, sizeof(struct InputCache));
}

static void inputView_reset(InputView *v, short value, VCacheRef tbl) {
    v->valid = true;
    v->result = value;
    tbl->view.hide(v->errorLabel, tbl->view.shd, true);
}

static void showInputError(IVPair *pair, InputVC *d) {
    pair->data->valid = false;
    d->tbl->view.hide(pair->data->errorLabel, d->tbl->view.shd, false);
    cache.layout(pair->view, cache.lfn);
    checkScrollPos(d->scrollView, pair->view);
    if (UIAccessibilityIsVoiceOverRunning()) {
        CFStringRef message = d->tbl->label.getText(pair->data->errorLabel, d->tbl->label.gtxt);
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1500000000), dispatch_get_main_queue(), ^{
            UIAccessibilityPostNotification(UIAccessibilityAnnouncementNotification, (id)message);
        });
    }
}

void inputVC_addChild(id self, CFStringRef hint, short min, short max) {
    InputVC *d = (InputVC *)((char *)self + VCSize);
    VCacheRef tbl = d->tbl;

    int index = d->count++;
    id view = Sels.new(InputViewClass, Sels.nw);
    d->children[index].view = view;
    d->children[index].data = (InputView *)((char *)view + ViewSize);
    InputView *v = d->children[index].data;
    v->minVal = min;
    v->maxVal = max;
    CFLocaleRef l = CFLocaleCopyCurrent();
    CFStringRef errorText = formatStr(l, inputFieldError, min, max);
    CFRelease(l);
    v->hintLabel = createLabel(tbl, d->clr, CFRetain(hint), UIFontTextStyleSubheadline, ColorLabel);
    tbl->label.setLines(v->hintLabel, tbl->label.snl, 0);
    tbl->view.setIsAcc(v->hintLabel, tbl->view.sace, false);
    v->field = createTextfield(tbl, d->clr, self, d->toolbar, hint, index);
    msg1(void, long, v->field, cache.skbt, 4);
    tbl->view.setHint(v->field, tbl->view.shn, errorText);
    if (d->setKB)
        msg1(void, long, v->field, sel_getUid("setKeyboardAppearance:"), 1);
    v->errorLabel = createLabel(tbl, d->clr, errorText, UIFontTextStyleFootnote, ColorRed);
    tbl->label.setLines(v->errorLabel, tbl->label.snl, 0);
    tbl->view.setIsAcc(v->errorLabel, tbl->view.sace, false);

    id vStack = createVStack((id []){v->hintLabel, v->field, v->errorLabel}, 3);
    tbl->stack.setSpace(vStack, tbl->stack.ssp, 4);
    tbl->stack.setMargins(vStack, tbl->stack.smr, (HAInsets){4, 0, 4, 0});
    msg1(void, bool, vStack, tbl->view.trans, false);
    tbl->view.addSub(view, tbl->view.asv, vStack);
    pin(&tbl->cc, vStack, view);
    tbl->stack.addSub(d->vStack, tbl->stack.asv, view);
    Sels.viewRel(vStack, Sels.rel);
    tbl->view.hide(v->errorLabel, tbl->view.shd, true);
}

void inputVC_updateFields(InputVC *self, const short *vals) {
    int count = self->count;
    CFLocaleRef l = CFLocaleCopyCurrent();
    for (int i = 0; i < count; ++i) {
        short value = vals[i];
        InputView *v = self->children[i].data;
        CFStringRef str = formatStr(l, CFSTR("%d"), value);
        self->tbl->field.setText(v->field, self->tbl->label.stxt, str);
        inputView_reset(v, value, self->tbl);
        CFRelease(str);
    }
    self->tbl->button.setEnabled(self->button, self->tbl->button.en, true);
    CFRelease(l);
}

void inputView_deinit(id self, SEL _cmd) {
    InputView *v = (InputView *)((char *)self + ViewSize);
    Sels.viewRel(v->hintLabel, Sels.rel);
    Sels.viewRel(v->field, Sels.rel);
    Sels.viewRel(v->errorLabel, Sels.rel);
    msgSup0(void, (&(struct objc_super){self, View}), _cmd);
}

id inputVC_init(id self, SEL _cmd _U_, VCacheRef tbl, CCacheRef clr) {
    self = msgSup0(id, (&(struct objc_super){self, VC}), sel_getUid("init"));
    InputVC *d = (InputVC *)((char *)self + VCSize);
    d->tbl = tbl;
    d->clr = clr;
    return self;
}

void inputVC_deinit(id self, SEL _cmd) {
    InputVC *d = (InputVC *)((char *)self + VCSize);
    for (int i = 0; i < 4; ++i) {
        if (d->children[i].view) Sels.viewRel(d->children[i].view, Sels.rel);
    }
    Sels.viewRel(d->toolbar, Sels.rel);
    Sels.viewRel(d->scrollView, Sels.rel);
    Sels.viewRel(d->vStack, Sels.rel);
    CFNotificationCenterRemoveEveryObserver(CFNotificationCenterGetLocalCenter(), self);
    msgSup0(void, (&(struct objc_super){self, VC}), _cmd);
}

void inputVC_viewDidLoad(id self, SEL _cmd) {
    msgSup0(void, (&(struct objc_super){self, VC}), _cmd);

    InputVC *d = (InputVC *)((char *)self + VCSize);
    d->scrollView = createScrollView();
    d->vStack = createVStack(nil, 0);
    d->tbl->stack.setMargins(d->vStack, d->tbl->stack.smr, (HAInsets){16, 8, 16, 8});
    d->toolbar = msg1(id, CGRect, Sels.alloc(objc_getClass("UIToolbar"), Sels.alo),
                      sel_getUid("initWithFrame:"), ((CGRect){{0}, {100, 100}}));
    unsigned char dark = getUserInfo()->darkMode;
    if (dark < 2) {
        msg1(void, id, d->toolbar, sel_getUid("setBarTintColor:"),
             clsF1(id, int, d->clr->cls, sel_getUid("getBarColorWithType:"), 1));
        d->setKB = dark == 1;
    }
    msg0(void, d->toolbar, sel_getUid("sizeToFit"));

    CFStringRef doneLabel = localize(CFSTR("done"));
    Class BarItem = objc_getClass("UIBarButtonItem"), Image = objc_getClass("UIImage");
    SEL btnInit = sel_getUid("initWithImage:style:target:action:"), imgInit = sel_getUid("imageNamed:");
    CFStringRef imgNames[] = {CFSTR("ico_chevron_up"), CFSTR("ico_chevron_down")};
    id arrows[2]; SEL arrowSels[] = {sel_getUid("jumpToPrev"), sel_getUid("jumpToNext")};
    for (int i = 0; i < 2; ++i) {
        arrows[i] = msg4(id, id, long, id, SEL, Sels.alloc(BarItem, Sels.alo), btnInit,
                         clsF1(id, CFStringRef, Image, imgInit, imgNames[i]), 0, self, arrowSels[i]);
    }
    id flexSpace = msg3(id, long, id, SEL, Sels.alloc(BarItem, Sels.alo),
                        sel_getUid("initWithBarButtonSystemItem:target:action:"), 5, nil, nil);
    id doneButton = msg4(id, CFStringRef, long, id, SEL, Sels.alloc(BarItem, Sels.alo),
                         sel_getUid("initWithTitle:style:target:action:"),
                         doneLabel, 2, self, sel_getUid("dismissKeyboard"));
    msg1(void, id, d->toolbar, sel_getUid("setTintColor:"),
         d->clr->getColor(d->clr->cls, d->clr->sc, ColorRed));
    CFRelease(doneLabel);

    CFArrayRef array = CFArrayCreate(
      NULL, (const void *[]){arrows[0], arrows[1], flexSpace, doneButton}, 4, &retainedArrCallbacks);
    msg2(void, CFArrayRef, bool, d->toolbar, sel_getUid("setItems:animated:"), array, false);

    addVStackToScrollView(d->tbl, msg0(id, self, sel_getUid("view")), d->vStack, d->scrollView);
    CFRelease(array);
    Sels.objRel(flexSpace, Sels.rel);
    Sels.objRel(arrows[0], Sels.rel);
    Sels.objRel(arrows[1], Sels.rel);
    Sels.objRel(doneButton, Sels.rel);

    CFNotificationCenterRef center = CFNotificationCenterGetLocalCenter();
    CFNotificationCenterAddObserver(center, self, keyboardShown, UIKeyboardDidShowNotification,
                                    NULL, CFNotificationSuspensionBehaviorDrop);
    CFNotificationCenterAddObserver(center, self, keyboardWillHide, UIKeyboardWillHideNotification,
                                    NULL, CFNotificationSuspensionBehaviorDrop);
}

void inputVC_viewDidAppear(id self, SEL _cmd, bool animated) {
    msgSup1(void, bool, (&(struct objc_super){self, VC}), _cmd, animated);

    InputVC *d = (InputVC *)((char *)self + VCSize);
    if (!d->scrollHeight) {
        CGRect bounds;
        HAInsets insets;
        getBounds(d->scrollView, &bounds);
        d->scrollHeight = (int)bounds.size.height;
#if defined(__arm64__)
        insets = msg0(HAInsets, d->scrollView, sel_getUid("contentInset"));
#else
        ((void(*)(HAInsets*,id,SEL))objc_msgSend_stret)(&insets, d->scrollView,
                                                        sel_getUid("contentInset"));
#endif
        d->topOffset = (short)insets.top;
        d->bottomOffset = (short)insets.bottom;
    }
}

void inputVC_dismissKeyboard(id self, SEL _cmd _U_) {
    msg1(bool, bool, msg0(id, self, sel_getUid("view")), sel_getUid("endEditing:"), true);
}

void inputVC_jumpToPrev(id self, SEL _cmd _U_) {
    InputVC *d = (InputVC *)((char *)self + VCSize);
    int tag = (int)d->tbl->view.getTag(d->activeField, d->tbl->view.gtg);
    if (tag) {
        if (tag > 0) {
            cache.becomeResponder(d->children[tag - 1].data->field, cache.bfr);
        } else {
            inputVC_dismissKeyboard(self, nil);
        }
        return;
    }
    CFArrayRef views = d->tbl->stack.getSub(d->vStack, d->tbl->stack.gsv);
    int count = (int)CFArrayGetCount(views);
    for (int i = 0; i < count; ++i) {
        id v = (id)CFArrayGetValueAtIndex(views, i);
        int t = (int)d->tbl->view.getTag(v, d->tbl->view.gtg);
        if (t < 0) {
            cache.becomeResponder(v, cache.bfr);
            return;
        }
    }
    inputVC_dismissKeyboard(self, nil);
}

void inputVC_jumpToNext(id self, SEL _cmd _U_) {
    InputVC *d = (InputVC *)((char *)self + VCSize);
    int tag = ((int)d->tbl->view.getTag(d->activeField, d->tbl->view.gtg));
    if (tag >= d->count - 1) {
        inputVC_dismissKeyboard(self, nil);
    } else {
        cache.becomeResponder(d->children[tag + 1].data->field, cache.bfr);
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
            if (!CFCharacterSetIsCharacterMember(
                   charSet, CFStringGetCharacterFromInlineBuffer(&buf, i))) return false;
        }
    }

    int i = (int)tbl->view.getTag(field, tbl->view.gtg);
    IVPair *pair = &d->children[i];

    CFStringRef text = msg0(CFStringRef, field, tbl->label.gtxt);
    if (range.location + range.length > CFStringGetLength(text)) return false;
    CFMutableStringRef newText = CFStringCreateMutableCopy(NULL, 0, text);
    CFStringReplace(newText, range, replacement);

    if (!CFStringGetLength(newText)) {
        CFRelease(newText);
        tbl->button.setEnabled(d->button, tbl->button.en, false);
        showInputError(pair, d);
        return true;
    }

    CFNumberRef num = CFNumberFormatterCreateNumberFromString(NULL, numFormatter, newText, NULL, 0);
    int newVal = -1;
    CFRelease(newText);
    if (num != NULL) {
        CFNumberGetValue(num, kCFNumberIntType, &newVal);
        CFRelease(num);
    }

    if (newVal < pair->data->minVal || newVal > pair->data->maxVal) {
        tbl->button.setEnabled(d->button, tbl->button.en, false);
        showInputError(pair, d);
        return true;
    }

    inputView_reset(pair->data, (short)newVal, tbl);
    for (i = 0; i < d->count; ++i) {
        if (!d->children[i].data->valid) return true;
    }

    tbl->button.setEnabled(d->button, tbl->button.en, true);
    return true;
}
