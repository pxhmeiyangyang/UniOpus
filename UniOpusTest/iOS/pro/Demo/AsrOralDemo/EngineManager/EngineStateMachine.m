//
//  EngineStateMachine.m
//  AsrOralDemo
//
//  Created by 刘俊 on 16/1/14.
//  Copyright © 2016年 yunzhisheng. All rights reserved.
//

#import "EngineStateMachine.h"

@implementation EngineStateMachine

-(void)reset
{
    if (stack == nil)
    {
        stack = [[NSMutableArray alloc]init];
    }
    [stack removeAllObjects];
}

-(BOOL)couldStart
{
    if (![stack containsObject:@START])
    {
        [stack addObject:@START];
        
        return YES;
    }
    else
    {
        return NO;
    }
}

-(BOOL)couldStop
{
    if ([stack containsObject:@START] && stack.count == 1)
    {
        [stack addObject:@STOP];
        
        return YES;
    }
    else
    {
        return NO;
    }
}

-(BOOL)couldCancel
{
    if ([stack containsObject:@START] && stack.count == 1)
    {
        [stack addObject:@CANCEL];
        
        return YES;
    }
    else
    {
        return NO;
    }
}

@end
