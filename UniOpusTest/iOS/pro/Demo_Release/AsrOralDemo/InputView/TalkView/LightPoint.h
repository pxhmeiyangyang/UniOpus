//
//  LightPoint.h
//  Test
//
//  Created by 刘俊 on 14-5-22.
//  Copyright (c) 2014年 yunzhisheng. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>

typedef NS_ENUM(NSInteger, LightType)
{
    Left_point,
    Right_point,
};

@interface LightPoint : CALayer

@property int tag;
@property LightType type;

-(void)appear;
-(void)disappear;

@end
