//
//  ImageHelpers.m
//  HealthApp
//
//  Created by Christopher Ray on 3/28/21.
//

#import "ImageHelpers.h"

UIImage *resizeImage(UIImage *image, CGFloat newWidth) {
    CGFloat scale = newWidth / image.size.height;
    CGFloat newHeight = image.size.height * scale;

    UIGraphicsBeginImageContext(CGSizeMake(newWidth, newHeight));
    [image drawInRect:CGRectMake(0, 0, newWidth, newHeight)];
    UIImage *newImage = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    return newImage;
}
