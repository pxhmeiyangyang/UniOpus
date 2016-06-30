//
//  BatchPCM.m
//  AsrOralDemo
//
//  Created by 刘俊 on 15/7/17.
//  Copyright (c) 2015年 yunzhisheng. All rights reserved.
//

#import "BatchPCM.h"

@implementation BatchPCM

-(id)init
{
    if (self == [super init])
    {
        finish = NO;
    }
    return self;
}

-(void)main
{
    while (true)
    {
        NSLog(@"running : %d",finish);
        
        if (finish)
        {
            dispatch_async(dispatch_get_main_queue(), ^{
               
                if (_delegate && [_delegate respondsToSelector:@selector(finish)])
                {
                    [NSThread sleepForTimeInterval:1];
                    
                    [_delegate finish];
                }
            });
            
            finish = NO;
        }
        
        [NSThread sleepForTimeInterval:1];
    }
}

-(void)setBeginValue
{
    begin = YES;
}

-(void)setFinishValue
{
    finish = YES;
}

@end
