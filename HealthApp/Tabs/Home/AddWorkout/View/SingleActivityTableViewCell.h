//
//  SingleActivityTableViewCell.h
//  HealthApp
//
//  Created by Christopher Ray on 3/27/21.
//

#ifndef SingleActivityTableViewCell_h
#define SingleActivityTableViewCell_h

#import <UIKit/UIKit.h>

@interface SingleActivityTableViewCell: UITableViewCell

- (void) configureWithIntensity: (NSString *)intensity duration: (int)duration;

@end

#endif /* SingleActivityTableViewCell_h */
