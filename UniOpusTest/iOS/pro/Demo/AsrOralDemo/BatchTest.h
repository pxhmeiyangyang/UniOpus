//
//  BatchTest.h
//  AsrOralDemo
//
//  Created by 刘俊 on 15/7/17.
//  Copyright (c) 2015年 yunzhisheng. All rights reserved.
//

/*
 生词
 长文本
 
 正常文本
 |模式
 |
 */

#import <Foundation/Foundation.h>
#import "EngineManager.h"
#import "BatchPCM.h"

@interface BatchTest : NSObject <EngineManagerDelegate>
{
    EngineManager *enginer;
    BatchPCM *test;
    NSOperationQueue *queue;
    NSInteger number;
}

-(void)start;

@end
