//
//  USCMP3Encoder.h
//  usc
//
//  Created by 刘俊 on 15/6/3.
//  Copyright (c) 2015年 yunzhisheng. All rights reserved.
//

#import <Foundation/Foundation.h>
@protocol USCMP3EncoderDelegate <NSObject>

-(void)audioDataDidEncode:(NSData *)encodingData;
-(void)encodeDidFinished;

@end

@interface USCMP3Encoder : NSOperation
{
    NSMutableArray *recordingBuffer;
    BOOL hasFinished;
    
    NSInteger encodeNum;
}

@property (nonatomic, assign)int sampleRate;
@property (nonatomic, assign) BOOL setToStopped;//值为YES是，不再接受外部输入
@property (nonatomic, assign) BOOL isEncodeCanceled;//值为YES是，直接取消本次转码流程
@property (nonatomic, assign)id <USCMP3EncoderDelegate> delegate;

-(void)appendAudioData:(NSData *)data;
-(BOOL)hasFinished;
-(void)encodeCancel;

@end
