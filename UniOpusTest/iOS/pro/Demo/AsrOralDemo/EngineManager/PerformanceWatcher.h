//
//  PerformanceWatcher.h
//  AsrOralDemo
//
//  Created by 刘俊 on 15/10/23.
//  Copyright © 2015年 yunzhisheng. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface PerformanceWatcher : NSObject
{
    NSMutableArray *watcherAry;
}

-(void)startWatcher;
-(void)addTimeWatcherWithName:(NSString *)name;
-(void)finishDiscription;

@end
