//
//  PerformanceWatcher.m
//  AsrOralDemo
//
//  Created by 刘俊 on 15/10/23.
//  Copyright © 2015年 yunzhisheng. All rights reserved.
//

#import "PerformanceWatcher.h"

@interface TimerWatcher : NSObject
{
    NSMutableDictionary *timerDic;
}

@property (nonatomic, copy)NSDate *timer;
@property (nonatomic, copy)NSString *key;

@end

@implementation TimerWatcher

@end;

@implementation PerformanceWatcher

-(void)startWatcher
{
    watcherAry = nil;
    watcherAry = [[NSMutableArray alloc]init];
}

-(void)addTimeWatcherWithName:(NSString *)name
{
    NSDate *date = [NSDate date];
    
    TimerWatcher *watch = [[TimerWatcher alloc]init];
    watch.timer = date;
    watch.key = name;
    
    [watcherAry addObject:watch];
}

-(void)finishDiscription
{
    if (watcherAry.count <=1)
    {
        NSLog(@"至少需要两组观测数据");
        return ;
    }
    
    NSMutableString *logStr = [NSMutableString string];
    [logStr appendString:[NSString stringWithFormat:@"0           "]];
    
    TimerWatcher *watch = watcherAry[0];
//    for (int i = 0; i < watcherAry.count - 1; i++)
//    {
//        TimerWatcher *watch0 = watcherAry[i];
//        TimerWatcher *watch1 = watcherAry[i+1];
//        
//        NSDate *date0 = watch0.timer;
//        NSDate *date1 = watch1.timer;
//        
//        NSTimeInterval intervel0 = [date1 timeIntervalSinceDate:watch.timer];
//        [logStr appendString:[NSString stringWithFormat:@"%.3f",intervel0]];
//        
//        NSTimeInterval intervel = [date1 timeIntervalSinceDate:date0];
//        [logStr appendString:[NSString stringWithFormat:@"(%.3f)           ",intervel]];
//    }
//    
//    NSLog(@"%@",logStr);

    NSLog(@"0              0             | Key : %@",watch.key);
    for (int i = 0; i < watcherAry.count - 1; i++)
    {
        TimerWatcher *watch0 = watcherAry[i];
        TimerWatcher *watch1 = watcherAry[i+1];
        
        NSDate *date0 = watch0.timer;
        NSDate *date1 = watch1.timer;
        
        NSTimeInterval intervel0 = [date1 timeIntervalSinceDate:watch.timer];
        [logStr appendString:[NSString stringWithFormat:@"%.3f",intervel0]];
        
        NSTimeInterval intervel = [date1 timeIntervalSinceDate:date0];
        [logStr appendString:[NSString stringWithFormat:@"(%.3f)           ",intervel]];
        
        NSLog(@"↑ %.3f        ↖ %.3f       | Key : %@",intervel0,intervel,watch1.key);
    }
}

@end
