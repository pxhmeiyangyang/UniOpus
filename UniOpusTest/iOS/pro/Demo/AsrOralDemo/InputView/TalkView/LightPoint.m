//
//  LightPoint.m
//  Test
//
//  Created by 刘俊 on 14-5-22.
//  Copyright (c) 2014年 yunzhisheng. All rights reserved.
//

#import "LightPoint.h"

@implementation LightPoint

@synthesize type,tag;

-(void)appear
{
    self.cornerRadius=self.frame.size.width/2;
    self.backgroundColor=[UIColor colorWithRed:(CGFloat)32/255 green:(CGFloat)187/255 blue:(CGFloat)252/255 alpha:1].CGColor;
}

-(void)disappear
{
    self.backgroundColor=[UIColor colorWithRed:(CGFloat)217/255 green:(CGFloat)217/255 blue:(CGFloat)217/255 alpha:1].CGColor;
}


@end
