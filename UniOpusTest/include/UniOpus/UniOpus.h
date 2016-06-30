//
//  UniOpus.h
//  UniOpus
//
//  Created by pxh on 16/6/29.
//  Copyright © 2016年 裴新华. All rights reserved.
//

#import <Foundation/Foundation.h>
#include "opus.h"

@protocol UniOpusDelegate <NSObject>

-(void)opusDataDidEncode:(NSData *)encodeData;
-(void)opusDataDidFinished;

@end

@interface UniOpus : NSObject

{
    NSMutableArray *audioDataAry;
    dispatch_queue_t encodeQue;
    BOOL isCanceled;
    BOOL isStoped;
    BOOL finishCallBack;
    NSInteger encodeNum;
    OpusEncoder *enc;
}

@property (nonatomic,assign)id <UniOpusDelegate>delegate;

-(void)appendAudioData:(NSData *)data;

-(void)stopEncode;
-(void)cancelEncode;

@end
