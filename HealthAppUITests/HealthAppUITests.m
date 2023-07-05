#import <XCTest/XCTest.h>

enum {
    InputZero = 1,
    InputButtonEnable = 2,
    InputDecimal = 4
};

enum {
    DismissExercise,
    DismissBack,
    DismissEnd
};

static NSString *settingsTitles[] = {
    @"Max squat weight", @"Max pull-up weight", @"Max bench weight", @"Max deadlift weight"
};

#pragma mark - XCUIElement Helpers

@implementation XCUIElement(HAExt)
- (bool)valueEquals:(NSString *)other { return [self.value compare:other] == 0; }

- (bool)labelEquals:(NSString *)other { return [self.label compare:other] == 0; }

- (void)tapField { [[self coordinateWithNormalizedOffset:CGVectorMake(0.9, 0.9)] tap]; }

- (void)deleteAllText {
    [self typeText:[@"" stringByPaddingToLength:((NSString *)self.value).length
                                     withString:XCUIKeyboardKeyDelete
                                startingAtIndex:0]];
}

- (void)checkFieldsWithLabels:(NSArray<NSString *> *)labels
                       values:(NSArray<NSString *> *)values
                         mins:(NSArray<NSNumber *> *)mins
                        maxes:(NSArray<NSNumber *> *)maxes {
    int count = (int)labels.count;
    XCUIElementQuery *fields = self.textFields;
    for (int i = 0; i < count; ++i) {
        XCUIElement *field = fields[labels[i]];
        XCTAssert([field valueEquals:values ? values[i] : @""]);
        NSString *hint = [[NSString alloc] initWithFormat:@"Input must be between %d and %d.",
                          mins[i].intValue, maxes[i].intValue];
        XCTAssert([field.accessibilityHint compare:hint] == 0);
        [hint release];
    }
}

- (void)typeNumber:(NSString *)numStr
            button:(XCUIElement *)button
           enabled:(bool)enabled
               min:(int)min
               max:(int)max {
    int count = (int)numStr.length;
    for (int i = 0; i < count; ++i) {
        unichar chars[] = {[numStr characterAtIndex:i]};
        NSString *charStr = [[NSString alloc] initWithCharacters:chars length:1];
        [self typeText:charStr];
        [charStr release];

        NSString *substr = [numStr substringWithRange:(NSRange){0, i + 1}];
        XCTAssert([self valueEquals:substr]);
        float currNum = substr.floatValue;
        if (currNum < min || currNum > max) {
            XCTAssert(!button.isEnabled);
        } else {
            XCTAssert(button.isEnabled == enabled);
        }
    }
    XCTAssert([self valueEquals:numStr]);
}

- (void)insertDeleteNumber:(NSNumber *)number
                    button:(XCUIElement *)button
                   enabled:(bool)enabled
                       min:(int)min
                       max:(int)max {
    NSString *numStr = number.stringValue;
    [self typeNumber:numStr button:button enabled:enabled min:min max:max];

    int count = (int)numStr.length;
    for (int i = count - 1; i >= 0; --i) {
        [self typeText:XCUIKeyboardKeyDelete];
        if (i == 0) {
            XCTAssert([self valueEquals:@""]);
            XCTAssert(!button.isEnabled);
            break;
        }

        NSString *substr = [numStr substringWithRange:(NSRange){0, i}];
        XCTAssert([self valueEquals:substr]);
        float currNum = substr.floatValue;
        if (currNum < min || currNum > max) {
            XCTAssert(!button.isEnabled);
        } else {
            XCTAssert(button.isEnabled == enabled);
        }
    }
    [number release];
}

- (XCUIElement *)headerForSection:(int)section {
    NSString *headerId = [[NSString alloc] initWithFormat:@"header_%d", section];
    XCUIElement *label = self.staticTexts[headerId];
    [headerId release];
    return label;
}

- (XCUIElementQuery *)defaultCells { return [self cellsInSection:0]; }

- (XCUIElementQuery *)cellsInSection:(int)section {
    NSString *cellId = [[NSString alloc] initWithFormat:@"cell_%d_", section];
    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"identifier BEGINSWITH %@", cellId];
    [cellId release];
    return [self.cells matchingPredicate:predicate].buttons;
}

- (void)checkSectionWithHeading:(NSString *)heading titles:(NSArray<NSString *> *)titles {
    [self checkSectionAtIndex:0 heading:heading titles:titles labels:nil];
}

- (void)checkSectionWithHeading:(NSString *)heading
                         titles:(NSArray<NSString *> *)titles
                         labels:(NSArray<NSString *> *)labels {
    [self checkSectionAtIndex:0 heading:heading titles:titles labels:labels];
}

- (void)checkSectionAtIndex:(int)index
                    heading:(NSString *)heading
                     titles:(NSArray<NSString *> *)titles
                     labels:(NSArray<NSString *> *)labels {
    if (heading) {
        XCTAssert([[self headerForSection:index] labelEquals:heading]);
    } else {
        XCTAssert(![self headerForSection:index].exists);
    }

    int count = (int)titles.count;
    XCUIElementQuery *cells = [self cellsInSection:index];
    for (int i = 0; i < count; ++i) {
        XCUIElement *cell = [cells elementBoundByIndex:i];
        if (labels) {
            NSString *label = [[NSString alloc] initWithFormat:@"%@, %@", labels[i], titles[i]];
            XCTAssert([cell labelEquals:label]);
            [label release];
        } else {
            XCTAssert([cell labelEquals:titles[i]]);
        }
    }
}

- (void)checkExerciseWithSets:(int)sets rest:(int)rest checkAfter:(bool)checkAfter {
    [self checkExerciseWithSets:sets duration:0 rest:rest checkAfter:checkAfter];
}

- (void)checkExerciseWithSets:(int)sets
                     duration:(int)duration
                         rest:(int)rest
                   checkAfter:(bool)checkAfter {
    NSString *restStr = rest ? [[NSString alloc] initWithFormat:@"Rest: %d s", rest] : nil;

    for (int i = 0; i < sets; ++i) {
        if (sets > 1) {
            NSString *setStr = [[NSString alloc] initWithFormat:@"Set %d of %d", i + 1, sets];
            XCTAssert([self.label hasPrefix:setStr]);
            [setStr release];
        } else {
            XCTAssert(![self.label hasPrefix:@"Set "]);
        }

        if (duration) {
            sleep(duration + 1);
            if (duration == 60)
                XCTAssert([self labelEquals:@"Minimum exercise duration met, Run/row for 60 s"]);
        } else {
            [self tap];
        }

        if (restStr) {
            XCTAssert([self.label hasSuffix:restStr]);
            [self tap];
            if (i != sets - 1) XCTAssert(![self.label hasSuffix:restStr]);
        }
    }

    if (checkAfter) {
        if (restStr) XCTAssert(![self.label hasSuffix:restStr]);
        XCTAssert(!self.isEnabled);
    }

    if (restStr) [restStr release];
}
@end

#pragma mark - Base Class

@interface BaseUITests: XCTestCase {
@public
    XCUIApplication *app;
    XCUIElementQuery *tabs;
    XCUIElement *homeCollection;
    XCUIElement *settingsScroll;
    XCUIElementQuery *settingsSegments;
    XCUIElementQuery *settingsFields;
    XCUIElement *settingsSaveButton;
}
@end

@implementation BaseUITests
- (void)setUp {
    self.continueAfterFailure = NO;
    app = [[XCUIApplication alloc] init];
}

- (void)tearDown { [app release]; }

#pragma mark Helpers

- (void)setupProperties {
    [app launch];
    tabs = app.tabBars.firstMatch.buttons;
    homeCollection = app.collectionViews[@"collection_HomeVC"];
}

- (void)takePhotoWithName:(NSString *)name {
    XCUIScreenshot *shot = [XCUIScreen.mainScreen screenshot];
    XCTAttachment *attachment = [XCTAttachment attachmentWithScreenshot:shot];
    attachment.name = name;
    attachment.lifetime = XCTAttachmentLifetimeKeepAlways;
    [self addAttachment:attachment];
}

- (void)checkHistory {
    [XCTContext runActivityNamed:@"checkHistory" block:^(id<XCTActivity> _Nonnull activity) {
        [[tabs elementBoundByIndex:1] tap];
        NSString *segmentNames[] = {@"6 Months", @"1 Year", @"2 Years"};
        NSString *sections[] = {@"Total Workouts", @"Activity Time By Type", @"Lift Progress"};
        XCUIElement *historyScroll = app.scrollViews[@"scroll_HistoryVC"];
        XCUIElementQuery *headers = historyScroll.staticTexts;
        XCUIElementQuery *segments = app.navigationBars.segmentedControls.firstMatch.buttons;
        for (int i = 0; i < 3; ++i) {
            XCTAssert([[segments elementBoundByIndex:i] labelEquals:segmentNames[i]]);
            XCTAssert([[headers elementBoundByIndex:i] labelEquals:sections[i]]);
        }

        XCTAssert([segments elementBoundByIndex:0].isSelected);
        [self takePhotoWithName:@"history_atTop"];
        [historyScroll swipeUp];
        [[segments elementBoundByIndex:1] tap];
        sleep(3);
        [self takePhotoWithName:@"history_atMiddle"];
    }];
}

- (void)setupSettingsProperties {
    settingsScroll = app.scrollViews[@"scroll_SettingsVC"];
    settingsSegments = settingsScroll.segmentedControls.firstMatch.buttons;
    settingsFields = settingsScroll.textFields;
    settingsSaveButton = settingsScroll.buttons[@"Save settings"];
}

- (XCUIElement *)planHeader { return [homeCollection headerForSection:0]; }

- (XCUIElementQuery *)planCells { return [homeCollection cellsInSection:0]; }

- (XCUIElementQuery *)customWorkoutCells { return [homeCollection cellsInSection:1]; }

- (XCUIElement *)workoutCollection { return app.collectionViews[@"collection_WorkoutVC"]; }

- (void)tapDown { [[app.toolbars.buttons elementBoundByIndex:1] tap]; }

- (void)tapStart { [app.navigationBars.buttons[@"Start"] tap]; }

- (void)tapBack { [[app.navigationBars.buttons elementBoundByIndex:0] tap]; }

- (void)tapButtonForDismissalType:(int)dismissType {
    if (dismissType == DismissBack) {
        [self tapBack];
    } else {
        [app.navigationBars.buttons[@"End"] tap];
    }
}

- (void)checkNavBarWithTitles:(NSArray<NSString *> *)titles enabled:(bool)enabled {
    [XCTContext runActivityNamed:@"checkNavBar" block:^(id<XCTActivity> _Nonnull activity) {
        XCUIElementQuery *buttons = app.navigationBars[titles[1]].buttons;
        if (titles[0].length) XCTAssert(buttons[titles[0]].isEnabled);
        if (titles[2].length) {
            XCTAssert(buttons[titles[2]].isEnabled == enabled);
        } else {
            XCTAssert(buttons.count == 0);
        }
    }];
}

- (void)dismissAlertWithActionName:(NSString *)action {
    [XCTContext runActivityNamed:@"dismissAlert" block:^(id<XCTActivity> _Nonnull activity) {
        usleep(250000);
        [self takePhotoWithName:@"alert_before"];
        [app.alerts.firstMatch.buttons[action] tap];
        sleep(2);
        [self takePhotoWithName:@"alert_after"];
    }];
}

- (void)checkField:(XCUIElement *)field
            button:(XCUIElement *)button
           options:(int)options
               min:(int)min
             value:(float)value {
    [self checkField:field button:button options:options min:min max:999 value:value];
}

- (void)checkField:(XCUIElement *)field
            button:(XCUIElement *)button
           options:(int)options
               min:(int)min
               max:(int)max
             value:(float)value {
    [XCTContext runActivityNamed:@"checkField" block:^(id<XCTActivity> _Nonnull activity) {
        XCTAssert(button.isEnabled == (options & InputZero));
        if (options & InputZero) {
            XCTAssert([field.value compare:@""] != 0);
            [field deleteAllText];
            XCTAssert(!button.isEnabled);
        }

        XCTAssert([field valueEquals:@""]);
        [field typeText:@"Not digits"];
        XCTAssert([field valueEquals:@""]);
        [field typeText:@"100."];
        if (options & InputDecimal) {
            XCTAssert([field valueEquals:@"100."]);
            [field typeText:@"."];
            XCTAssert([field valueEquals:@"100."]);
            [field typeText:@"2."];
            XCTAssert([field valueEquals:@"100.2"]);
            [field typeText:@"5."];
            XCTAssert([field valueEquals:@"100.25"]);
            [field deleteAllText];
        } else {
            XCTAssert([field valueEquals:@"100"]);
            [field deleteAllText];
        }

        NSNumber *numbers[] = {
            [[NSNumber alloc] initWithInt:MAX(min - 1, 0)],
            [[NSNumber alloc] initWithInt:min],
            [[NSNumber alloc] initWithInt:max + 1],
            [[NSNumber alloc] initWithInt:max]
        };
        for (int i = 0; i < 4; ++i) {
            [field insertDeleteNumber:numbers[i]
                               button:button
                              enabled:options & InputButtonEnable
                                  min:min
                                  max:max];
        }

        NSNumber *valueNum = [[NSNumber alloc] initWithFloat:value];
        [field typeNumber:valueNum.stringValue
                   button:button
                  enabled:options & InputButtonEnable
                      min:min
                      max:max];
        [valueNum release];
        [self tapDown];
    }];
}

- (void)checkSettingsFieldsEqualValues:(NSArray<NSString *> *)values {
    [XCTContext runActivityNamed:@"checkSettings" block:^(id<XCTActivity> _Nonnull activity) {
        for (int i = 0; i < 4; ++i) {
            XCTAssert([settingsFields[settingsTitles[i]] valueEquals:values[i]]);
        }
    }];
}

- (void)checkSettingsFieldsEqualValues:(NSArray<NSString *> *)before
                        updateToValues:(NSArray<NSNumber *> *)after
                               options:(int)options {
    [XCTContext runActivityNamed:@"checkSettingsFull" block:^(id<XCTActivity> _Nonnull activity) {
        [settingsScroll checkFieldsWithLabels:[NSArray arrayWithObjects:settingsTitles count:4]
                                       values:before
                                         mins:@[@0, @0, @0, @0]
                                        maxes:@[@999, @999, @999, @999]];

        [settingsFields[settingsTitles[0]] tapField];
        for (int i = 0; i < 4; ++i) {
            [self checkField:settingsFields[settingsTitles[i]]
                      button:settingsSaveButton
                     options:options | InputZero | InputButtonEnable
                         min:0
                       value:after[i].floatValue];
        }
    }];
}

- (void)checkHICSetupWorkoutWithTitle:(NSString *)title {
    [self checkSetupWorkoutForType:4
                      defaultTitle:@"Sprint 10 to 1s"
                             title:title
                            labels:nil
                              mins:nil
                             maxes:nil
                      valuesToType:nil];
}

- (void)checkSetupWorkoutForType:(int)type
                    defaultTitle:(NSString *)defaultTitle
                           title:(NSString *)title
                          labels:(NSArray<NSString *> *)labels
                            mins:(NSArray<NSNumber *> *)mins
                           maxes:(NSArray<NSNumber *> *)maxes
                    valuesToType:(NSArray<NSNumber *> *)values {
    [XCTContext runActivityNamed:@"checkSetupWorkout" block:^(id<XCTActivity> _Nonnull activity) {
        [[[self customWorkoutCells] elementBoundByIndex:type] tap];
        sleep(2);

        int count = values ? (int)values.count : 0;
        [self checkNavBarWithTitles:@[@"Cancel", @"Set up Workout", @"Go"] enabled:!count];
        XCUIElement *scroll = app.scrollViews[@"scroll_SetupWorkoutVC"];
        XCUIElementQuery *fields = scroll.textFields;
        XCTAssert(fields.count == count + 1);

        XCUIElement *nameField = [fields elementBoundByIndex:0];
        XCTAssert([nameField valueEquals:defaultTitle]);
        [nameField tap];
        [app.pickerWheels.firstMatch adjustToPickerWheelValue:title];

        XCUIElement *goButton = app.navigationBars.buttons[@"Go"];
        if (count) {
            [scroll checkFieldsWithLabels:labels values:nil mins:mins maxes:maxes];
            [self tapDown];

            for (int i = 0; i < count; ++i) {
                [self checkField:fields[labels[i]]
                          button:goButton
                         options:i < count - 1 ? 0 : InputButtonEnable
                             min:mins[i].intValue
                             max:maxes[i].intValue
                           value:values[i].intValue];
            }
        }

        [goButton tap];
        sleep(2);
    }];
}

- (void)checkStrengthWithTitle:(NSString *)title
                  valuesToType:(NSArray<NSNumber *> *)values
                       weights:(NSArray<NSString *> *)weights
                      exercise:(NSString *)exercise
                          unit:(NSString *)unit {
    [XCTContext runActivityNamed:@"checkStrength" block:^(id<XCTActivity> _Nonnull activity) {
        int sets = values[0].intValue, reps = values[1].intValue;
        [self checkSetupWorkoutForType:1
                          defaultTitle:@"Main Strength"
                                 title:title
                                labels:@[@"Sets", @"Reps", @"Max weight percentage"]
                                  mins:@[@1, @1, @1]
                                 maxes:@[@5, @5, @100]
                          valuesToType:values];

        [self checkNavBarWithTitles:@[@"", title, @"Start"] enabled:true];
        XCUIElement *collection = [self workoutCollection];
        XCUIElementQuery *cells = [collection defaultCells];
        NSString *setsStr = [NSString stringWithFormat:@"Set 1 of %d", sets];
        [collection checkSectionWithHeading:nil
                                     titles:@[
            [NSString stringWithFormat:@"Squat x %d @ %@ %@", reps, weights[0], unit],
            [NSString stringWithFormat:@"Bench x %d @ %@ %@", reps, weights[1], unit],
            [NSString stringWithFormat:@"%@ x %d @ %@ %@", exercise, reps, weights[2], unit]]
                                     labels:@[setsStr, setsStr, setsStr]];
        [self tapStart];

        for (int i = 0; i < 3; ++i) {
            [[cells elementBoundByIndex:i] checkExerciseWithSets:sets rest:120 checkAfter:i < 2];
        }
        sleep(2);
    }];
}

- (void)checkSEWithTitle:(NSString *)title
                  labels:(NSArray<NSString *> *)labels
                  rounds:(int)rounds
             dismissType:(int)dismissType {
    [XCTContext runActivityNamed:@"checkSE" block:^(id<XCTActivity> _Nonnull activity) {
        int count = (int)labels.count;
        [self checkNavBarWithTitles:@[@"", title, @"Start"] enabled:true];
        XCUIElement *collection = [self workoutCollection];
        XCUIElementQuery *cells = [collection defaultCells];
        [self tapStart];

        for (int i = 0; i < rounds; ++i) {
            bool lastGroup = i == rounds - 1;
            NSString *header = nil;
            if (rounds > 1)
                header = [[NSString alloc] initWithFormat:@"Round %d of %d", i + 1, rounds];
            [collection checkSectionWithHeading:header titles:labels];
            if (header) [header release];

            for (int j = 0; j < count; ++j) {
                bool lastEx = j == count - 1;
                int rest = lastEx ? 180 : 90;
                XCUIElement *cell = [cells elementBoundByIndex:j];
                bool checkAfter = !(lastGroup && lastEx);
                if (!checkAfter && dismissType != DismissExercise) {
                    [cell tap];
                    NSString *restStr = [[NSString alloc] initWithFormat:@"Rest: %d s", rest];
                    XCTAssert([cell.label hasSuffix:restStr]);
                    [restStr release];
                    [self tapButtonForDismissalType:dismissType];
                    break;
                }
                [cell checkExerciseWithSets:1 rest:rest checkAfter:checkAfter];
            }
        }
        sleep(2);
    }];
}

- (void)checkEnduranceWithDismissalType:(int)dismissType {
    [XCTContext runActivityNamed:@"checkEndurance" block:^(id<XCTActivity> _Nonnull activity) {
        [self checkNavBarWithTitles:@[@"", @"Endurance", @"Start"] enabled:true];
        XCUIElement *collection = [self workoutCollection];
        XCUIElement *cell = [collection defaultCells].firstMatch;
        [collection checkSectionWithHeading:nil titles:@[@"Run/row for 60 s"] labels:nil];
        [self tapStart];

        [cell checkExerciseWithSets:1 duration:60 rest:0 checkAfter:false];
        if (dismissType == DismissExercise) {
            [cell tap];
        } else {
            [self tapButtonForDismissalType:dismissType];
        }
        sleep(2);
    }];
}

- (void)runKB2WithDismissalType:(int)dismissType {
    [XCTContext runActivityNamed:@"checkKB2" block:^(id<XCTActivity> _Nonnull activity) {
        XCUIElement *cell = [[self planCells] elementBoundByIndex:3];
        [cell tap];
        sleep(2);
        [self checkSEWithTitle:@"KB 2"
                        labels:@[@"KB swing x 20",
                                 @"Goblet squat x 20",
                                 @"Renegade row x 20",
                                 @"1-arm KB deadlift x 20",
                                 @"KB floor press x 20"]
                        rounds:2
                   dismissType:dismissType];
        XCTAssert(!cell.isEnabled);
    }];
}

- (void)checkUpdateMaxesWithOptions:(int)options weightsAfter:(NSArray<NSString *> *)values {
    [XCTContext runActivityNamed:@"checkUpdateMaxes" block:^(id<XCTActivity> _Nonnull activity) {
        [[[self customWorkoutCells] elementBoundByIndex:0] tap];
        sleep(2);

        [self checkNavBarWithTitles:@[@"", @"Test Day", @"Start"] enabled:true];
        [self tapStart];
        [[[[self workoutCollection] defaultCells] elementBoundByIndex:0] tap];
        sleep(2);

        [self checkNavBarWithTitles:@[@"", @"Set Weight & Reps", @"Finish"] enabled:false];
        XCUIElement *scroll = app.scrollViews[@"scroll_UpdateMaxesVC"];
        XCUIElementQuery *steps = scroll.steppers.firstMatch.buttons;
        XCUIElement *minusButton = [steps elementBoundByIndex:0];
        XCUIElement *plusButton = [steps elementBoundByIndex:1];
        XCUIElement *stepperLabel = [scroll.staticTexts elementBoundByIndex:1];
        XCTAssert([stepperLabel labelEquals:@"Reps: 1"]);
        XCTAssert(!minusButton.isEnabled);

        for (int i = 2; i < 11; ++i) {
            [plusButton tap];
        }
        XCTAssert([stepperLabel labelEquals:@"Reps: 10"]);
        XCTAssert(!plusButton.isEnabled);

        for (int i = 9; i > 0; --i) {
            [minusButton tap];
        }
        XCTAssert([stepperLabel labelEquals:@"Reps: 1"]);
        XCTAssert(!minusButton.isEnabled);

        XCUIElement *finish = app.navigationBars.buttons[@"Finish"];
        XCUIElement *field = scroll.textFields.firstMatch;
        [field tapField];
        [self takePhotoWithName:@"updateMaxes_beforeInput"];
        [self checkField:field button:finish options:options | InputButtonEnable min:1 value:800];
        [self takePhotoWithName:@"updateMaxes_afterInput"];
        [finish tap];
        sleep(2);

        [self tapBack];
        sleep(2);

        [[tabs elementBoundByIndex:2] tap];
        [self checkSettingsFieldsEqualValues:values];

        [[tabs elementBoundByIndex:0] tap];
    }];
}

- (void)checkUpdateMaxesWithInitialWeights:(NSArray<NSString *> *)before
                              valuesToType:(NSArray<NSString *> *)weights
                                      reps:(NSArray<NSNumber *> *)reps
                              weightsAfter:(NSArray<NSString *> *)after
                                      unit:(NSString *)unit {
    [XCTContext runActivityNamed:@"checkUpdateMaxesFull" block:^(id<XCTActivity> _Nonnull activity) {
        [[[self customWorkoutCells] elementBoundByIndex:0] tap];
        sleep(2);

        XCUIElement *collection = [self workoutCollection];
        XCUIElementQuery *cells = [collection defaultCells];
        NSString *hints[] = {
            @"Max squat weight", @"Max pull-up weight", @"Max bench weight", @"Max deadlift weight"
        };
        [collection checkSectionWithHeading:nil
                                     titles:@[
            [NSString stringWithFormat:@"Squat x 1 @ %@ %@", before[0], unit],
            [NSString stringWithFormat:@"Pull-up x 1 @ %@ %@", before[1], unit],
            [NSString stringWithFormat:@"Bench x 1 @ %@ %@", before[2], unit],
            [NSString stringWithFormat:@"Deadlift x 1 @ %@ %@", before[3], unit]]];
        [self tapStart];

        for (int i = 0; i < 4; ++i) {
            XCUIElement *cell = [cells elementBoundByIndex:i];
            XCTAssert(cell.isEnabled);
            [cell tap];
            sleep(2);

            XCUIElement *scroll = app.scrollViews[@"scroll_UpdateMaxesVC"];
            [scroll checkFieldsWithLabels:@[hints[i]] values:nil mins:@[@1] maxes:@[@999]];
            XCUIElement *field = scroll.textFields.firstMatch;
            [field tapField];
            [field typeText:weights[i]];
            [app.toolbars.buttons[@"Done"] tap];

            XCUIElement *plusButton = [scroll.steppers.firstMatch.buttons elementBoundByIndex:1];
            int currReps = reps[i].intValue;
            for (int j = 1; j < currReps; ++j) {
                [plusButton tap];
            }

            NSString *repsStr = [[NSString alloc] initWithFormat:@"Reps: %d", currReps];
            XCTAssert([[scroll.staticTexts elementBoundByIndex:1] labelEquals:repsStr]);
            [repsStr release];
            [app.navigationBars.buttons[@"Finish"] tap];
            sleep(2);

            if (i < 3) XCTAssert(!cell.isEnabled);
        }
        sleep(2);

        [[tabs elementBoundByIndex:2] tap];
        [self checkSettingsFieldsEqualValues:after];
    }];
}
@end

#pragma mark - First Launch

@interface HealthAppUITestsA: BaseUITests
@end

@implementation HealthAppUITestsA
- (void)testA {
    [self setupProperties];
    sleep(10);
    [self checkHistory];
}
@end

@interface HealthAppUITestsB: BaseUITests
@end

@implementation HealthAppUITestsB
#pragma mark - Main Tests
- (void)testA {
    [self setupProperties];

    [[tabs elementBoundByIndex:2] tap];
    [self setupSettingsProperties];
    [[settingsSegments elementBoundByIndex:0] tap];

    if (@available(iOS 13, *)) {
    } else {
        XCUIElement *darkModeSwitch = settingsScroll.switches.firstMatch;
        if ([darkModeSwitch valueEquals:@"1"]) [darkModeSwitch tap];
        XCTAssert([darkModeSwitch valueEquals:@"0"]);
    }

    [settingsFields[settingsTitles[0]] tapField];
    for (int i = 0; i < 4; ++i) {
        XCUIElement *field = settingsFields[settingsTitles[i]];
        [field deleteAllText];
        [field typeText:@"0"];
        [self tapDown];
    }

    [settingsSaveButton tap];
    [self dismissAlertWithActionName:@"Save"];
    [settingsScroll.buttons[@"Delete data"] tap];
    [self dismissAlertWithActionName:@"Delete"];
}

- (void)testB {
    [self setupProperties];
    [self checkNavBarWithTitles:@[@"", @"Home", @""] enabled:true];
    XCTAssert(![self planHeader].exists);
    XCTAssert([self planCells].count == 0);
    [homeCollection checkSectionAtIndex:1
                                heading:@"Add Custom Workout"
                                 titles:@[
        @"Test Max", @"Strength", @"Strength-Endurance", @"Endurance", @"Conditioning"]
                                 labels:nil];
    [self takePhotoWithName:@"home_customContainerOnly"];

    [[tabs elementBoundByIndex:2] tap];
    [self setupSettingsProperties];
    {
        [self checkNavBarWithTitles:@[@"", @"App Settings", @""] enabled:true];
        XCTAssert(settingsScroll.staticTexts[@"Change workout plan"].exists);
        NSString *segmentNames[] = {@"None", @"Foundation", @"Continuation"};
        for (int i = 0; i < 3; ++i) {
            XCTAssert([[settingsSegments elementBoundByIndex:i] labelEquals:segmentNames[i]]);
        }

        XCTAssert([settingsSegments elementBoundByIndex:0].isSelected);
        [[settingsSegments elementBoundByIndex:1] tap];

        if (@available(iOS 13, *)) {
        } else {
            XCTAssert([settingsScroll
                       descendantsMatchingType:XCUIElementTypeAny][@"Dark mode"].exists);
            XCUIElement *darkModeSwitch = settingsScroll.switches.firstMatch;
            XCTAssert([darkModeSwitch valueEquals:@"0"]);
            [darkModeSwitch tap];
            XCTAssert([darkModeSwitch valueEquals:@"1"]);
        }
    }

    [self checkSettingsFieldsEqualValues:@[@"0", @"0", @"0", @"0"]
                          updateToValues:@[@50, @0, @20, @0]
                                 options:0];
    [settingsSaveButton tap];
    [self dismissAlertWithActionName:@"Cancel"];

    [[tabs elementBoundByIndex:0] tap];
    XCTAssert(![self planHeader].exists);
    XCTAssert([self planCells].count == 0);

    [[tabs elementBoundByIndex:2] tap];
    [settingsSaveButton tap];
    [self dismissAlertWithActionName:@"Save"];

    [[tabs elementBoundByIndex:0] tap];
    [homeCollection checkSectionWithHeading:@"Workouts This Week"
                                     titles:@[
        @"KB 1", @"Endurance", @"Endurance", @"KB 2", @"Endurance"]
                                     labels:@[
        @"Monday", @"Tuesday", @"Wednesday", @"Thursday", @"Saturday"]];
    [self takePhotoWithName:@"home_bothContainers"];
    [self runKB2WithDismissalType:DismissExercise];
    XCTAssert(homeCollection.isHittable);
}

- (void)testC {
    [self setupProperties];
    XCUIElementQuery *planCells = [self planCells];
    XCTAssert(![planCells elementBoundByIndex:3].isEnabled);

    [[tabs elementBoundByIndex:2] tap];
    [self setupSettingsProperties];
    {
        if (@available(iOS 13, *)) {
        } else {
            XCTAssert([settingsScroll.switches.firstMatch valueEquals:@"1"]);
        }
        XCTAssert([settingsSegments elementBoundByIndex:1].isSelected);
        [self checkSettingsFieldsEqualValues:@[@"50", @"0", @"20", @"0"]];
        XCUIElement *deleteButton = settingsScroll.buttons[@"Delete data"];
        [deleteButton tap];
        [self dismissAlertWithActionName:@"Cancel"];

        [[tabs elementBoundByIndex:0] tap];
        XCTAssert(![planCells elementBoundByIndex:3].isEnabled);

        [[tabs elementBoundByIndex:2] tap];
        [deleteButton tap];
        [self dismissAlertWithActionName:@"Delete"];
    }

    [self checkHistory];

    [[tabs elementBoundByIndex:0] tap];
    XCTAssert([planCells elementBoundByIndex:3].isEnabled);
    [[planCells elementBoundByIndex:0] tap];
    sleep(2);
    [self checkSEWithTitle:@"KB 1"
                    labels:@[@"KB swing x 20",
                             @"Push-up x 20",
                             @"Goblet squat x 20",
                             @"Inverted row x 20",
                             @"1-arm KB deadlift x 20"]
                    rounds:3
               dismissType:DismissEnd];

    XCTAssert(![planCells elementBoundByIndex:0].isEnabled);
    [[planCells elementBoundByIndex:4] tap];
    sleep(2);
    [self checkEnduranceWithDismissalType:DismissExercise];

    XCTAssert(homeCollection.isHittable);
    XCTAssert(![planCells elementBoundByIndex:4].isEnabled);
}

- (void)testD {
    [self setupProperties];
    XCUIElementQuery *planCells = [self planCells];
    XCTAssert(![planCells elementBoundByIndex:0].isEnabled);
    XCTAssert(![planCells elementBoundByIndex:4].isEnabled);

    [self runKB2WithDismissalType:DismissBack];

    [[planCells elementBoundByIndex:2] tap];
    sleep(2);
    [self checkEnduranceWithDismissalType:DismissEnd];
    XCTAssert(![planCells elementBoundByIndex:2].isEnabled);

    [[planCells elementBoundByIndex:1] tap];
    sleep(2);
    [self checkEnduranceWithDismissalType:DismissBack];
    sleep(2);

    XCTAssert(app.otherElements[@"confetti"].isHittable);
    [self takePhotoWithName:@"homeConfetti"];
    sleep(5);
    [self dismissAlertWithActionName:@"OK"];
    XCTAssert(homeCollection.isHittable);
    XCTAssert(![planCells elementBoundByIndex:1].isEnabled);
}

- (void)testE {
    [self setupProperties];
    XCUIElementQuery *planCells = [self planCells];
    for (int i = 0; i < 5; ++i) {
        XCTAssert(![planCells elementBoundByIndex:i].isEnabled);
    }

    [[tabs elementBoundByIndex:2] tap];
    [self setupSettingsProperties];
    [[settingsSegments elementBoundByIndex:0] tap];
    [settingsSaveButton tap];
    [self dismissAlertWithActionName:@"Save"];

    [[tabs elementBoundByIndex:0] tap];
    XCTAssert(![self planHeader].exists);
    XCTAssert([self planCells].count == 0);
    [self takePhotoWithName:@"home_noPlan"];

    [[tabs elementBoundByIndex:2] tap];
    [[settingsSegments elementBoundByIndex:1] tap];
    [settingsSaveButton tap];
    [self dismissAlertWithActionName:@"Save"];

    [[tabs elementBoundByIndex:0] tap];
    planCells = [self planCells];
    for (int i = 0; i < 5; ++i) {
        XCTAssert(![planCells elementBoundByIndex:i].isEnabled);
    }

    [self takePhotoWithName:@"home_allCompleted"];
    [homeCollection swipeUp];
    [self checkUpdateMaxesWithOptions:0 weightsAfter:@[@"50", @"0", @"20", @"0"]];
    [self checkUpdateMaxesWithInitialWeights:@[@"50.00", @"0.00", @"20.00", @"0.00"]
                                valuesToType:@[@"40", @"50", @"135", @"225"]
                                        reps:@[@1, @4, @2, @2]
                                weightsAfter:@[@"50", @"67", @"139", @"231"]
                                        unit:@"pounds"];

    [[tabs elementBoundByIndex:0] tap];
    [self checkUpdateMaxesWithInitialWeights:@[@"50.00", @"67.00", @"139.00", @"231.00"]
                                valuesToType:@[@"255", @"25", @"165", @"175"]
                                        reps:@[@5, @1, @4, @2]
                                weightsAfter:@[@"287", @"67", @"180", @"231"]
                                        unit:@"pounds"];
}

- (void)testF {
    [self setupProperties];
    [homeCollection swipeUp];
    [self checkStrengthWithTitle:@"Aux Strength"
                    valuesToType:@[@4, @2, @85]
                         weights:@[@"243.95", @"153.00", @"196.35"]
                        exercise:@"Deadlift"
                            unit:@"pounds"];

    [[[self customWorkoutCells] elementBoundByIndex:2] tap];
    sleep(2);
    [app.navigationBars.buttons[@"Cancel"] tap];
    sleep(2);

    [self checkSetupWorkoutForType:2
                      defaultTitle:@"KB 1"
                             title:@"DB 1"
                            labels:@[@"Sets", @"Reps"]
                              mins:@[@1, @1]
                             maxes:@[@3, @50]
                      valuesToType:@[@2, @35]];
    [self checkSEWithTitle:@"DB 1"
                    labels:@[@"DB bench x 35",
                             @"DB lunge x 35",
                             @"DB row x 35",
                             @"DB push-press x 35",
                             @"Goblet squat x 35",
                             @"Lying leg raise x 35"]
                    rounds:2
               dismissType:DismissExercise];

    [self checkSetupWorkoutForType:3
                      defaultTitle:@"Endurance"
                             title:@"Endurance"
                            labels:@[@"Duration in minutes"]
                              mins:@[@15]
                             maxes:@[@180]
                      valuesToType:@[@110]];
    [self checkEnduranceWithDismissalType:DismissExercise];

    [self checkHICSetupWorkoutWithTitle:@"GC 8"];

    {
        [self checkNavBarWithTitles:@[@"", @"GC 8", @"Start"] enabled:true];
        XCUIElement *collection = [self workoutCollection];
        XCUIElementQuery *cells = [collection defaultCells];
        [self tapStart];
        [self takePhotoWithName:@"gc8_bothContainers"];
        for (int i = 0; i < 4; ++i) {
            bool lastGroup = i == 3;
            NSString *header = [[NSString alloc] initWithFormat:@"Circuit 1 of 2 - Round %d of 4",
                                i + 1];
            [collection checkSectionWithHeading:header
                                         titles:@[@"KB snatch x 20",
                                                  @"Box jump x 25",
                                                  @"Hanging knee-to-elbow x 25",
                                                  @"Dip x 25",
                                                  @"Burpee x 5"]];
            [collection checkSectionAtIndex:1
                                    heading:@"Circuit 2 of 2"
                                     titles:@[@"Handstand hold for 5 s"]
                                     labels:@[@"Set 1 of 3"]];
            [header release];
            for (int j = 0; j < 5; ++j) {
                XCUIElement *cell = [cells elementBoundByIndex:j];
                [cell checkExerciseWithSets:1 rest:0 checkAfter:!(lastGroup && j == 4)];
            }
        }

        [collection checkSectionAtIndex:1
                                heading:@"Circuit 2 of 2"
                                 titles:@[@"Handstand hold for 5 s"]
                                 labels:@[@"Set 1 of 3"]];
        [self takePhotoWithName:@"gc8_onlySecondContainer"];
        XCUIElement *cell = [collection cellsInSection:1].firstMatch;
        [cell checkExerciseWithSets:3 duration:5 rest:180 checkAfter:false];
        sleep(2);
    }

    [self checkHICSetupWorkoutWithTitle:@"Sprint 10 to 1s"];

    {
        [self checkNavBarWithTitles:@[@"", @"Sprint 10 to 1s", @"Start"] enabled:true];
        XCUIElement *collection = [self workoutCollection];
        XCUIElementQuery *cells = [collection defaultCells];
        [self tapStart];
        for (int i = 10; i > 0; --i) {
            bool lastGroup = i == 1;
            [collection checkSectionWithHeading:nil
                                         titles:@[[NSString stringWithFormat:@"Burpee x %d", i],
                                                  [NSString stringWithFormat:@"Squat x %d", i],
                                                  @"Sprint/row 100/125 m"]];
            for (int j = 0; j < 3; ++j) {
                XCUIElement *cell = [cells elementBoundByIndex:j];
                [cell checkExerciseWithSets:1 rest:0 checkAfter:!(lastGroup && j == 2)];
            }
        }
        sleep(2);
    }

    [self checkHICSetupWorkoutWithTitle:@"Track Intervals"];

    {
        [self checkNavBarWithTitles:@[@"", @"Track Intervals", @"Start"] enabled:true];
        XCUIElement *collection = [self workoutCollection];
        XCUIElementQuery *cells = [collection defaultCells];
        [collection checkSectionWithHeading:@"AMRAP 1 min"
                                     titles:@[
            @"Run/row for 5 s", @"KB swing x 20", @"Run/row for 5 s", @"KB snatch x 20"]];
        [self tapStart];
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 2; ++j) {
                int i1 = j << 1;
                XCUIElement *cell = [cells elementBoundByIndex:i1];
                [cell checkExerciseWithSets:1 duration:5 rest:0 checkAfter:true];
                [[cells elementBoundByIndex:i1 + 1] checkExerciseWithSets:1 rest:0 checkAfter:true];
            }
        }

        [[cells elementBoundByIndex:0] checkExerciseWithSets:1 duration:5 rest:0 checkAfter:true];
        sleep(31);
    }

    XCTAssert(homeCollection.isHittable);
}

#pragma mark - Locale Testing

- (void)testG {
    app.launchArguments = @[@"-inUITest", @"-AppleLanguages", @"(en)", @"-AppleLocale", @"en_CA"];
    [self setupProperties];

    [[tabs elementBoundByIndex:2] tap];
    [self setupSettingsProperties];
    [self checkSettingsFieldsEqualValues:@[@"130.18", @"30.39", @"81.65", @"104.78"]
                          updateToValues:@[@130.18, @30.39, @81.65, @104.78]
                                 options:InputDecimal];
    [settingsSaveButton tap];
    [self dismissAlertWithActionName:@"Save"];

    [[tabs elementBoundByIndex:0] tap];
    [homeCollection swipeUp];
    [self checkUpdateMaxesWithOptions:InputDecimal
                         weightsAfter:@[@"130.18", @"30.39", @"81.65", @"104.78"]];
    [self checkUpdateMaxesWithInitialWeights:@[@"130.18", @"30.39", @"81.65", @"104.78"]
                                valuesToType:@[@"115.76", @"30.39", @"81.69", @"106.6"]
                                        reps:@[@2, @1, @1, @1]
                                weightsAfter:@[@"130.18", @"30.39", @"81.65", @"106.59"]
                                        unit:@"kg"];

    [[tabs elementBoundByIndex:0] tap];
    [self checkStrengthWithTitle:@"Main Strength" valuesToType:@[@5, @3, @75]
                weights:@[@"97.64", @"61.23", @"7.37"] exercise:@"Pull-up" unit:@"kg"];
}

- (void)testH {
    [self setupProperties];

    [[tabs elementBoundByIndex:2] tap];
    [self setupSettingsProperties];
    [self checkSettingsFieldsEqualValues:@[@"287", @"67", @"180", @"235"]];
}
@end
