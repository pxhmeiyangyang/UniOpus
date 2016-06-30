//
//  USCPCMReaderManager.m
//  usc
//
//  Created by 刘俊 on 15/7/8.
//  Copyright (c) 2015年 yunzhisheng. All rights reserved.
//

#import "USCPCMReaderManager.h"
#import "USCRecorder.h"
#import "USCPCMReader.h"

#import "USCMarcos.h"


@interface USCPCMReaderManager()<USCRecorderDelegate, USCPCMReaderDelegate>
{
    USCRecorder *_recorder;
    USCPCMReader *_pcmReader;
    
    //录音模块运行所在的线程
    dispatch_queue_t recorder_queue;
    
    NSOperationQueue *pcmReaderQueue;
}

@end

@implementation USCPCMReaderManager

@synthesize filePath;
@synthesize readingMode;

-(id)init
{
    if (self == [super init])
    {
        pcmReaderQueue = [[NSOperationQueue alloc]init];
        [pcmReaderQueue setMaxConcurrentOperationCount:1];
        [self initRecorder];
    }
    return self;
}

-(void)initRecorder
{
    USCLog(@"Reader -> initRecorder");
    
    //同时初始化运行录音模块所需的线程
    recorder_queue = dispatch_queue_create("cn.yunzhisheng.usc", NULL);
    
    _recorder = nil;
    _recorder = [[USCRecorder alloc] init];
    _recorder.delegate = self;
}

-(void)setPCMFilePath:(NSString *)path
{
    filePath = nil;
    filePath = [[NSString alloc]initWithString:path];
}

// 设置vad前置端点和后置端点的静音时间
- (void)setVadFrontTimeout:(int)frontTime backTimeout:(int)backTime
{
    int frontFrame = frontTime /10;
    int backFrame = backTime /10;
    [_recorder setVadFrontTimeout:frontFrame BackTimeout:backFrame];
}

-(void)startReading
{
    USCLog(@"Reader -> startReading");
    
    if (readingMode == READ_FROM_MIC)
    {
        [self startRecording];
    }
    else if(readingMode == READ_FROM_PCM_FILE)
    {
        [self startPCMReader];
    }
}

-(void)stopReading
{
    USCLog(@"Reader -> stopReading : %d",readingMode);
    
    if (readingMode == READ_FROM_MIC)
    {
        [self stopRecording];
    }
    else if(readingMode == READ_FROM_PCM_FILE)
    {
        
    }
}

-(void)cancelReading
{
    USCLog(@"Reader -> cancelReading : %d",readingMode);
    
    if (readingMode == READ_FROM_MIC)
    {
        [self cancelRecording];
    }
    else if(readingMode == READ_FROM_PCM_FILE)
    {
        if (_pcmReader)
        {
//            [_pcmReader cancel];
            _pcmReader = nil;
        }
    }
}

-(void)startRecording
{
    // 开始录音
    dispatch_async(recorder_queue, ^{
        
        [_recorder startRecording];
    });
}

-(void)stopRecording
{
    //为防止录音被截断，在执行停止前加一秒的延迟
    int64_t delayInSeconds = 500;
    dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, delayInSeconds * NSEC_PER_MSEC);
    
    /*
     *@parameter 1,时间参照，从此刻开始计时
     *@parameter 2,延时多久，此处为秒级，还有纳秒等
     */
    
    //触发stop操作后将不再执行超时判断
    _recorder.readyToStop = YES;
    
    dispatch_after(popTime, recorder_queue, ^{
        
        //USCLog(@"Reader -> stopRecording | Do stopRecording");
        
        // 停止录音
        [_recorder stopRecording];
    });
}

-(void)cancelRecording
{
    //取消录音
    dispatch_async(recorder_queue, ^{
        
        [_recorder cancelRecording];
    });
}

-(void)startPCMReader
{
    USCLog(@"Reader -> startPCMReader");
    
    if (_pcmReader)
    {
        _pcmReader = nil;
    }
    _pcmReader = [[USCPCMReader alloc]initWithPath:filePath];
    _pcmReader.delegate = self;
    [pcmReaderQueue addOperation:_pcmReader];
}

-(void)destroy
{
    USCLog(@"Reader -> destroy");
}

-(void)dealloc
{
    USCLog(@"Reader -> dealloc");
    
    [self destroy];
}

#pragma mark -
#pragma mark Callback

-(void)volumeUpdate:(int)volume
{
    dispatch_sync(dispatch_get_main_queue(), ^{
        if (_delegate && [_delegate respondsToSelector:@selector(onUpdateVolume:)])
        {
            [_delegate onUpdateVolume:volume];
        }
    });
}

-(void)vadTimeout
{
    dispatch_sync(dispatch_get_main_queue(), ^{
        if (_delegate && [_delegate respondsToSelector:@selector(onVADTimeout)])
        {
            [_delegate onVADTimeout];
        }
    });
}

-(void)recordingStart:(ErrorType)errorCode
{
    USCLog(@"recordingStart : %d | isMainThread : %d",errorCode, [NSThread isMainThread]);

    if (_delegate && [_delegate respondsToSelector:@selector(onRecordingStart:)])
    {
        [_delegate onRecordingStart:errorCode];
    }
    
//    BOOL isMainThread = [NSThread isMainThread];
//    if (isMainThread)
//    {
//        if (_delegate && [_delegate respondsToSelector:@selector(onRecordingStart:)])
//        {
//            [_delegate onRecordingStart:errorCode];
//        }
//    }
//    else
//    {
//        dispatch_async(dispatch_get_main_queue(), ^{
//            
//            USCLog(@"Reader -> Jump to mainThread");
//            
//            if (_delegate && [_delegate respondsToSelector:@selector(onRecordingStart:)])
//            {
//                [_delegate onRecordingStart:errorCode];
//            }
//        });
//    }
}

-(void)recordingStop
{
    dispatch_sync(dispatch_get_main_queue(), ^{
        if (_delegate && [_delegate respondsToSelector:@selector(onRecordingStop)])
        {
            [_delegate onRecordingStop];
        }
    });
}


-(void)getRecordingBuffer:(NSData *)recordingData
{
    dispatch_sync(dispatch_get_main_queue(), ^{
        if (_delegate && [_delegate respondsToSelector:@selector(onRecordingBuffer:)])
        {
            [_delegate onRecordingBuffer:recordingData];
        }
    });
}

#pragma mark -
#pragma mark USCRecorder Callback

// 音量大小
- (void) onUpdataVolume:(int)volume
{
    [self volumeUpdate:volume];
}

// vad超时
- (void) onVADTimeout
{
    USCLog(@"Reader -> onVADTimeout");
    
    [self vadTimeout];
}

// 录音启动是否成功
- (void) onRecordingStart:(ErrorType)errorCode
{
    USCLog(@"Reader -> recording | onRecordingStart : %d",errorCode);
    
    [self recordingStart:errorCode];
}

- (void)onRecordingBuffer:(NSData *)recordingData
{
    //USCLog(@"Reader -> recording | onRecordingBuffer");
    
    [self getRecordingBuffer:recordingData];
}

- (void)onRecordingStop
{
    USCLog(@"Reader -> recording | onRecordingStop");
           
    [self recordingStop];
}

#pragma mark -
#pragma mark USCPCMReader Callback

- (void) onReadPCMUpdateVolume:(int)volume
{
    //USCLog(@"Reader -> reading | onReadPCMUpdateVolume : %d",volume);
    [self volumeUpdate:volume];
}

- (void) onReadPCMFileStart:(ErrorType)errorCode
{
    USCLog(@"Reader -> reading | onReadPCMFileStart : %d",errorCode);
    [self recordingStart:errorCode];
}

- (void) onReadPCMFileStop:(ErrorType)errorCode
{
    USCLog(@"Reader -> reading | onReadPCMFileStop : %d",errorCode);
    
    if (errorCode == No_Error)
    {
        [self recordingStop];
    }
}

- (void) onReadPCMData:(NSData *)data
{
    //USCLog(@"Reader -> reading | onReadPCMData");
    [self getRecordingBuffer:data];
}

@end
