//
//  USCBaseRecognition.m
//  usc
//
//  Created by 刘俊 on 15/5/6.
//  Copyright (c) 2015年 yunzhisheng. All rights reserved.
//

#import "USCBaseRecognition.h"
#import "USCMarcos.h"

@implementation USCBaseRecognition

-(id)init
{
    if (self = [super init])
    {
        isFinished = NO;
    }
    return self;
}

-(void)setFinished:(BOOL)finish
{
    USCLog(@"USCBaseRecognition | setFinished :%d",finish);
    isFinished = finish;
}

-(BOOL)getFinished
{
    return isFinished;
}

@end
