//
//  EngineStateMachine.h
//  AsrOralDemo
//
//  Created by 刘俊 on 16/1/14.
//  Copyright © 2016年 yunzhisheng. All rights reserved.
//

#import <Foundation/Foundation.h>

#define START   0
#define STOP    1
#define CANCEL  2

@interface EngineStateMachine : NSObject
{
    NSMutableArray *stack;
}

-(void)reset;

-(BOOL)couldStart;
-(BOOL)couldStop;
-(BOOL)couldCancel;

@end
