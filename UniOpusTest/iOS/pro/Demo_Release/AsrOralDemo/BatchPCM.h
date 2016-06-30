//
//  BatchPCM.h
//  AsrOralDemo
//
//  Created by 刘俊 on 15/7/17.
//  Copyright (c) 2015年 yunzhisheng. All rights reserved.
//

#import <Foundation/Foundation.h>

@protocol BatchPCMDelegate <NSObject>

-(void)finish;
-(void)error;

@end

@interface BatchPCM : NSOperation
{
    BOOL begin;
    BOOL finish;
}

@property (nonatomic,assign)id delegate;

-(void)setBeginValue;
-(void)setFinishValue;

@end
