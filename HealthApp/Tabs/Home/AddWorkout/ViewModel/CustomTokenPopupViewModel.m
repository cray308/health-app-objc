//
//  CustomTokenPopupViewModel.m
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#import "CustomTokenPopupViewModel.h"
#import "AddWorkoutCoordinator.h"

CGFloat getImageDimension(int foregroundWidth);
void setTokensAndSpacingForRow(CustomTokenPopupViewModel *model);

CustomTokenPopupViewModel *customTokenPopupViewModel_init(AddWorkoutCoordinator *delegate, CGFloat width, int tokens) {
    CustomTokenPopupViewModel *model = malloc(sizeof(CustomTokenPopupViewModel));
    if (!model) return NULL;

    model->delegate = delegate;
    model->foregroundWidth = width;
    model->tokens = tokens;
    model->message = [[NSString alloc] initWithFormat:@"You earned %d %@!", tokens, (tokens > 1 ? @"tokens" : @"token")];
    model->imageDimension = getImageDimension(width);
    setTokensAndSpacingForRow(model);
    return model;
}

void customTokenPopupViewModel_tappedOkButton(CustomTokenPopupViewModel *model, UIView *popup) {
    [popup removeFromSuperview];
    addWorkoutCoordinator_didDismissAlert(model->delegate);
}

#pragma mark - Helpers

CGFloat getImageDimension(int foregroundWidth) {
    // most compact the images will be is 5 per row
    int width = 40;
    int spacing = (foregroundWidth - (5 * width)) / 6;
    while (!spacing) {
        width -= 5;
        spacing = (foregroundWidth - (5 * width)) / 6;
    }
    return width;
}

void setTokensAndSpacingForRow(CustomTokenPopupViewModel *model) {
    int numToDraw[10][2] = {{1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {3, 3}, {4, 3}, {4, 4}, {5, 4}, {5, 5}};
    int tokens = min(10, model->tokens);

    for (int i = 0; i < 2; ++i) {
        int tokensForRow = numToDraw[tokens - 1][i];
        CGFloat unusedSize = model->foregroundWidth - model->imageDimension * tokensForRow;
        model->spacingForRow[i] = unusedSize / (tokensForRow + 1);
        model->tokensForRow[i] = tokensForRow;
    }
}
