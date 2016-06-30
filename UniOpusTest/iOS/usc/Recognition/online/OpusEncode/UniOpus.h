//
//  UniOpus.h
//  UniEncode
//
//  Created by 刘 俊 on 15/10/23.
//  Copyright © 2015年 Alixe. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "USCMarcos.h"
#include "opus.h"

#import "UniPCMRecorder.h"

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
    
    UniPCMRecorder *testRecorder;
    
    OpusEncoder *enc;
}

@property (nonatomic,assign)id <UniOpusDelegate>delegate;

-(void)appendAudioData:(NSData *)data;

-(void)stopEncode;
-(void)cancelEncode;

@end
