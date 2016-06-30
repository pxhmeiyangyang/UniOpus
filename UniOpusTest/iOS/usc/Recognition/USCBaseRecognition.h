//
//  USCBaseRecognition.h
//  usc
//
//  Created by 刘俊 on 15/5/6.
//  Copyright (c) 2015年 yunzhisheng. All rights reserved.
//

#import <Foundation/Foundation.h>

typedef enum : NSUInteger {
    RecognitionStart,
    RecognitionStop,
    RecognitionGetResult,
    RecognitionError,
    RecognitionSessionID,
} RecognitionLifeCycle;

@interface USCBaseRecognition : NSOperation
{
    RecognitionLifeCycle recognitionLifeCycle;
    
    //在识别结果回调或者出错回调中置为YES
    BOOL isFinished;
}

-(void)setFinished:(BOOL)finish;
-(BOOL)getFinished;

@end
