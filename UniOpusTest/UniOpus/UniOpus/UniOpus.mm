//
//  UniOpus.m
//  UniOpus
//
//  Created by pxh on 16/6/29.
//  Copyright © 2016年 裴新华. All rights reserved.
//

#import "UniOpus.h"
#include "opus.h"

@interface UniOpus()
{
    NSMutableArray *audioDataAry;
    dispatch_queue_t encodeQue;
    BOOL isCanceled;
    BOOL isStoped;
    BOOL finishCallBack;
    NSInteger encodeNum;
    OpusEncoder *enc;
}
@end

@implementation UniOpus
-(id)init
{
    if (self = [super init])
    {
        [self opusCreat];
        
        encodeQue = dispatch_queue_create("cn.unisound.encode", nil);
        
        audioDataAry = [[NSMutableArray alloc]init];
        isStoped = NO;
        isCanceled = NO;
        finishCallBack = NO;
        
        encodeNum = 0;
    }
    return self;
}

-(void)opusCreat
{
    if (enc == NULL)
    {
        int err = 0;
        opus_int32 skip = 0;
        
        int sample_rate = 16000;
        enc = opus_encoder_create(sample_rate, 1, OPUS_APPLICATION_VOIP, &err);
        opus_encoder_ctl(enc, OPUS_RESET_STATE);
        opus_encoder_ctl(enc, OPUS_SET_BANDWIDTH(OPUS_BANDWIDTH_WIDEBAND));
        opus_encoder_ctl(enc, OPUS_SET_BITRATE(sample_rate));
        opus_encoder_ctl(enc, OPUS_SET_VBR(1));
        opus_encoder_ctl(enc, OPUS_SET_COMPLEXITY(10));
        opus_encoder_ctl(enc, OPUS_SET_INBAND_FEC(0));
        opus_encoder_ctl(enc, OPUS_SET_FORCE_CHANNELS(OPUS_AUTO));
        opus_encoder_ctl(enc, OPUS_SET_DTX(0));
        opus_encoder_ctl(enc, OPUS_SET_PACKET_LOSS_PERC(0));
        opus_encoder_ctl(enc, OPUS_GET_LOOKAHEAD(&skip));
        opus_encoder_ctl(enc, OPUS_SET_LSB_DEPTH(16));
    }
}

-(void)opusDestroy
{
    if (enc)
    {
        opus_encoder_destroy(enc);
        enc = nil;
    }
}

-(NSData *)encodeInFrame:(NSData *)pcmData
{
    NSMutableData *encodeData = [NSMutableData data];
    
    int frame_size = 320;
    NSRange range = NSMakeRange(0, 0);
    while (range.location < pcmData.length)
    {
        NSMutableData *blockData = [NSMutableData data];
        
        if ((pcmData.length - range.location) >= frame_size * 2)
        {
            range.length = frame_size * 2;
        }
        else
        {
            range.length = pcmData.length - range.location;
        }
        
        [blockData appendData:[[NSData alloc]initWithData:[pcmData subdataWithRange:range]]];
        //文件末端不满一帧的部分用0补齐
        if (blockData.length < frame_size * 2)
        {
            while (blockData.length < frame_size * 2)
            {
                short i = 0;
                [blockData appendBytes:(void *)&i length:sizeof(short)];
            }
        }
        
        opus_int16 *pcm = (opus_int16 *)blockData.bytes;
        unsigned char outData[(frame_size + 1) * 2];
        
        opus_int32 length = opus_encode(enc, pcm, frame_size, outData + sizeof(short), frame_size * 2);
        outData[0] = length & 0xFF;
        outData[1] = (length & 0xFF00) >> 8;
        
        NSData *data = [[NSData alloc]initWithBytes:outData length:length + sizeof(short)];
        
        [encodeData appendData:data];
        
        range.location += range.length;
    }
    
    return encodeData;
}

-(void)appendAudioData:(NSData *)data
{
    NSLog(@"appendAudioData");
    
    NSData *originalData = [[NSData alloc]initWithData:data];
    [audioDataAry addObject:originalData];
    
    [self encode];
}

-(void)encode
{
    dispatch_async(encodeQue, ^{
        
        if (!isCanceled)
        {
            NSData *pcmData = audioDataAry[encodeNum];
            
            NSData *encode_data = [self encodeInFrame:pcmData];
            
            encodeNum ++; //为保证线程的信息同步，该条语句必须要编码完成后执行
            
            //将编码后的数据在主线程回调
            dispatch_async(dispatch_get_main_queue(), ^{
                
                NSLog(@"UniOpus -> encode : %ld |encodeNum : %ld| isStop : %d | isCanceled : %d",(long)audioDataAry.count, (long)encodeNum, isStoped, isCanceled);
                
                if (isCanceled)
                {
                    return ;
                }
                
                if (encodeNum < audioDataAry.count)
                {
                    [self callBackEncodeData:encode_data];
                }
                else if (encodeNum == audioDataAry.count)
                {
                    if (isStoped)
                    {
                        [self callBackEncodeData:encode_data];
                        [self callBackFinish];
                    }
                    else
                    {
                        [self callBackEncodeData:encode_data];
                    }
                }
            });
        }
        else
        {
            //[self callBackFinish];
        }
    });
}

-(void)stopEncode
{
    NSLog(@"UniOpus -> stopEncode");
    isStoped = YES;
    
    //如果编码已完成，则直接返回，若还没完成，则在完成时返回
    if (encodeNum == audioDataAry.count)
    {
        [self callBackFinish];
    }
}

-(void)cancelEncode
{
    NSLog(@"UniOpus -> cancelEncode");
    
    isCanceled = YES;
}

-(void)callBackFinish
{
    if (!finishCallBack)
    {
        finishCallBack = YES;
        if (_delegate && [_delegate respondsToSelector:@selector(opusDataDidFinished)])
        {
            [_delegate opusDataDidFinished];
        }
    }
}

-(void)callBackEncodeData:(NSData *)data
{
    if (_delegate && [_delegate respondsToSelector:@selector(opusDataDidEncode:)])
    {
        [_delegate opusDataDidEncode:data];
    }
}

-(void)dealloc
{
    NSLog(@"UniOpus -> dealloc");
    
    [audioDataAry removeAllObjects];
    audioDataAry = nil;
    
    isCanceled = NO;
    
    [self opusDestroy];
    
    encodeQue = nil;
}

@end

