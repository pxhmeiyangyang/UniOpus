//
//  USCRecognition_offline.m
//  usc
//
//  Created by 刘俊 on 15/1/5.
//  Copyright (c) 2015年 yunzhisheng. All rights reserved.
//

#import "USCRecognition_offline.h"

#import "USCRecognition.h"
#import "USCMarcos.h"
#import <arpa/inet.h>
#import <netdb.h>

#import "USCOralEduWrapper.h"

@interface USCRecognition_offline()
{
    
}

@end

@implementation USCRecognition_offline

@synthesize recordingQueue = _recordingQueue;
@synthesize setToStopped = _setToStopped;
@synthesize delegate = _delegate;
@synthesize appKey = _appKey;
@synthesize netType = _netType;
@synthesize sampleRate = _sampleRate;
@synthesize oralText = _oralText;

- (id)init
{
    if (self = [super init]) {
        
        _setToStopped = NO;
        
        _recordingQueue = [[NSMutableArray alloc]init];
        
        recognitionLifeCycle = RecognitionStart;
        
        USCLog(@"recognitionOffline -> init");
    }
    return self;
}

- (void)setEngine:(USCOralEduWrapper *)wrapper
{
    eduOfflineEngine = wrapper ;
}

- (void)appendAudioData:(NSData *)audioData
{
    USCLog(@"recognition -> inputBufferHandler");

    @synchronized(_recordingQueue)
    {
        [_recordingQueue addObject:audioData];
    }
}

- (void)doRecognitionStart
{
    if (_delegate != nil && ![self isCancelled])
    {
        [_delegate onOfflineRecognitionStart];
    }
}

- (void)doRecognitionResult:(NSString *)result isLast:(BOOL)isLast
{
    if (_delegate != nil && ![self isCancelled])
    {
        [_delegate onOfflineRecognitionResult:result isLast:isLast];
    }
    
    recognitionLifeCycle = RecognitionGetResult;
}

- (void)doRecognitionStop
{
    if (_delegate != nil && ![self isCancelled])
    {
        [_delegate onOfflineRecognitionStop];
    }
    
    recognitionLifeCycle = RecognitionStop;
}


- (void)doRecognitionError:(ErrorType)error;
{
    if (_delegate != nil && ![self isCancelled])
    {
        [_delegate onOfflineRecognitionError:error];
    }
    
    recognitionLifeCycle = RecognitionError;
}

- (void)doMaxSpeechTimeout;
{
    if (_delegate != nil && ![self isCancelled])
    {
        [_delegate onOfflineMaxSpeechTimeout];
    }
}


- (void)main
{
    USCLog(@"recognitionOffline -> main");
    
    // 判断是否取消
    if ([self isCancelled])
    {
        return;
    }
    
    //启动引擎
    int startCode = [eduOfflineEngine engineStart:_oralText oralTask:_oralTask];
    
    if (startCode != OK_Engine)
    {
        USCLog(@"recognitionOffline -> start fail: %i", startCode);
        [self doRecognitionError:[USCErrorCode offlineErrorTransform:startCode]];
        
        return;
    }
    else
    {
        [self doRecognitionStart];
    }
    
    while (true)
    {
        USCLog(@"recognitionOffline -> main : %d",_setToStopped);
        
        [NSThread sleepForTimeInterval:0.1];
        
        NSData *audioData = nil;
        @synchronized(_recordingQueue)
        {
            // begin @synchronized
            if (_recordingQueue.count > 0)
            {
                // 获取队头数据
                audioData = [_recordingQueue objectAtIndex:0];
                [_recordingQueue removeObjectAtIndex:0];
            }
        }// end @synchronized
        
        if (audioData != nil)
        {
            char *pcmData = (char *)audioData.bytes;
            int pcmSize = (int)audioData.length;
            
            int recognizeCode = [eduOfflineEngine recognize:pcmData withLen:pcmSize];
            
            USCLog(@"recognitionOffline -> recognizeCode=%i", recognizeCode);
            
            if (recognizeCode == OK_Engine)
            {
                NSString *partail = [eduOfflineEngine getResult];
                if (partail != nil && partail.length > 0)
                {
                    [self doRecognitionResult:partail isLast:NO];
                }
                else
                {
                    [self doRecognitionError:Offline_Recog_Error];
                    return;
                }
            }
            else if (recognizeCode < 0)
            {
                [self doRecognitionError:[USCErrorCode offlineErrorTransform:recognizeCode]];
                USCLog(@"recognitionOffline -> recognize error=%i", recognizeCode);
                return;
            }
        }
        else
        {
            if (_setToStopped)
            {
                USCLog(@"recognitionOffline -> break");
                break;
            }
            else
            {
                [NSThread sleepForTimeInterval:0.1];
                USCLog(@"recognitionOffline -> sleep");
            }
        }
        
        // 判断是否取消
        if ([self isCancelled])
        {
            return;
        }
    }
    
    // 停止识别
    int stopCode = [eduOfflineEngine engineStop];
    USCLog(@"recognitionOffline -> engine stop code : %d",stopCode);
    if (stopCode < 0)
    {
        [self doRecognitionError:[USCErrorCode offlineErrorTransform:stopCode]];
        return;
    }
    else
    {
        // 获取最后一次识别结果
        NSString *result = [eduOfflineEngine getResult];
        if (result != NULL)
        {
            NSMutableString *jsonResult = [[NSMutableString alloc] initWithString:result];
            [self doRecognitionResult:jsonResult isLast:YES];
        }
        else
        {
            [self doRecognitionError:Offline_Recog_Error];
            return;
        }
    }
    
    USCLog(@"recognitionOffline -> stop");
    
    // 判断是否取消
    if ([self isCancelled])
    {
        return;
    }
    
    // 通知识别结束
    [self doRecognitionStop];
}

- (void)dealloc
{
    USCLog(@"recognitionOffline -> dealloc");
    
    _recordingQueue = nil;
    _delegate = nil;
    _appKey = nil;
    _oralText = nil;
    _domain = nil;
    _oralTask = nil;
    _oralExt1 = nil;
    _oralExt2 = nil;
}


@end
