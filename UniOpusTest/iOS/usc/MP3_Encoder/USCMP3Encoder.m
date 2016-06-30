//
//  USCMP3Encoder.m
//  usc
//
//  Created by 刘俊 on 15/6/3.
//  Copyright (c) 2015年 yunzhisheng. All rights reserved.
//

#import "USCMP3Encoder.h"
#import "USCMarcos.h"

#include "lame.h"

// 全局指针
lame_t lame;

@implementation USCMP3Encoder
@synthesize sampleRate = _sampleRate;
@synthesize delegate = _delegate;
@synthesize setToStopped = _setToStopped;
@synthesize isEncodeCanceled = _isEncodeCanceled;

-(id)init
{
    if (self = [super init])
    {
        recordingBuffer = [[NSMutableArray alloc]init];
        hasFinished = NO;
        _isEncodeCanceled = NO;
        
        encodeNum = 0;
    }
    return self;
}

-(void)appendAudioData:(NSData *)data
{
    if (_setToStopped)
    {
        return ;
    }
    
    USCLog(@"Encode -> appendAudioData");
    
    [recordingBuffer addObject:data];
}

-(BOOL)hasFinished
{
    return hasFinished;
}

-(void)encodeCancel
{
    USCLog(@"Encode -> cancel");
    
    _setToStopped = YES;
    _isEncodeCanceled = YES;
}

-(void)main
{
    [self lameInit];
    
    while (true)
    {
        USCLog(@"Encode -> encodeNum : %lu | recordingBuffer : %lu",(long)encodeNum,(unsigned long)recordingBuffer.count);
        
        if (encodeNum < recordingBuffer.count && !_isEncodeCanceled)
        {
            NSData *audioData = recordingBuffer[encodeNum ++];

            if (audioData != nil)
            {
                //我们得到的录音数据是byte类型的,首先要将它转换为short类型,由于一个short是两个byte,长度要减半
                short *data = (short *)audioData.bytes;
                int pcmLen = (int)audioData.length;
                int mp3Len = pcmLen / 2 ;
                
                unsigned char buffer[pcmLen];
                
                int bufferLen = lame_encode_buffer(lame, data, data, mp3Len, buffer, pcmLen);
                
                if (bufferLen > 0)
                {
                    NSData *encodeData = [NSData dataWithBytes:buffer length:bufferLen];
                    
                    [self dataCallBack:encodeData];
                }
            }
        }

        if (_isEncodeCanceled)
        {
            USCLog(@"Encode -> isCancel");
            
            [self lameClose];
            
            break ;
        }
        else if (_setToStopped && encodeNum >= recordingBuffer.count)
        {
            USCLog(@"Encode -> break");
            
            //先关闭lame在执行结束回调
            [self lameClose];
            
            [self finishCallBack];
            
            break;
        }
        else
        {
            [NSThread sleepForTimeInterval:0.05];
            //USCLog(@"Encode -> sleep");
        }
    }
}

-(void)lameInit
{
    lame = lame_init();
    lame_set_num_channels(lame, 1);
    lame_set_in_samplerate(lame, _sampleRate);
    lame_set_brate(lame, 32);
    lame_set_mode(lame, 1);
    lame_set_quality(lame, 9);
    lame_init_params(lame);
}

-(int)lameClose
{
    return lame_close(lame);
}

-(void)dataCallBack:(NSData *)data
{
    dispatch_sync(dispatch_get_main_queue(), ^{
        
        if (_delegate && [_delegate respondsToSelector:@selector(audioDataDidEncode:)])
        {
            [_delegate audioDataDidEncode:data];
        }        
    });
}

-(void)finishCallBack
{
    USCLog(@"Encode -> finishCallBack");
    
    hasFinished = YES;

    dispatch_sync(dispatch_get_main_queue(), ^{
        
        if (_delegate && [_delegate respondsToSelector:@selector(encodeDidFinished)])
        {
            [_delegate encodeDidFinished];
        }
    });
}

- (void)dealloc
{
    USCLog(@"Encode -> release");
    
    [recordingBuffer removeAllObjects];
    recordingBuffer = nil;
}

@end
