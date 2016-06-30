//
//  USCRecorder.m
//  usc
//
//  Created by hejinlai on 12-11-16.
//  Copyright (c) 2012年 yunzhisheng. All rights reserved.
//
#import "USCRecorder.h"
#import "USCMarcos.h"
#include "math.h"
#import <UIKit/UIKit.h>
#import <AVFoundation/AVFoundation.h>

static NSMutableData *recordData = [[NSMutableData alloc] init];//LJ——测试音频时使用

#define debug NO

static const int bufferByteSize = 3200;
static const int sampeleRate = 16000;
static const int bitsPerChannel = 16;
static BOOL isRecording = NO;
static BOOL isVADTimeOut = YES;

@implementation USCRecorder

@synthesize delegate = _delegate;
@synthesize netType = _netType;
@synthesize sampleRate = _sampleRate;
@synthesize recordingDatas = _recordingDatas;
@synthesize readyToStop = _readyToStop;

- (id)init
{
    if (self = [super init]) {
        
        AudioSessionInitialize(NULL, NULL, NULL,  (__bridge void*)self);
        
        _readyToStop = NO;
        
        pcmAnalysis = [[UniPCMAnalysis alloc]init];
        pcmAnalysis.delegate = self;
        
        USCLog(@"recording -> init");
    }
    
    return self;
}

//LJ——测试音频时使用
-(void)appendData:(NSData *)data
{
    [recordData appendData:data];
}
//LJ——测试音频时使用
-(void)writeToFile
{
    if (recordData!=nil)
    {
        NSString *filePath = [ NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) lastObject];
        NSString *pcmPath = [filePath stringByAppendingPathComponent:@"pcmData.pcm"];
        
        BOOL result = [recordData writeToFile:pcmPath atomically:YES];
        if (result)
        {
            USCLog(@"WriteToFile Sucessfully");
        }
    }
    else
    {
        USCLog(@"NULL");
    }
}

// 设置vad超时时间
- (void)setVadFrontTimeout:(int)frontTime BackTimeout:(int)backTime
{
    USCLog(@"Recorder -> vadTime | frontTime : %d | backTime : %d",frontTime,backTime);
    [pcmAnalysis setVadFront:frontTime Back:backTime];
}

// 设置录音格式
- (void) setupAudioFormat:(UInt32) inFormatID SampleRate:(int) sampeleRate
{
    memset(&_recordFormat, 0, sizeof(_recordFormat));
    _recordFormat.mSampleRate = sampeleRate;
    
	UInt32 size = sizeof(_recordFormat.mChannelsPerFrame);
    AudioSessionGetProperty(kAudioSessionProperty_CurrentHardwareInputNumberChannels, &size, &_recordFormat.mChannelsPerFrame);
	_recordFormat.mFormatID = inFormatID;
	if (inFormatID == kAudioFormatLinearPCM)
    {
		// if we want pcm, default to signed 16-bit little-endian
		_recordFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
		_recordFormat.mBitsPerChannel = bitsPerChannel;
		_recordFormat.mBytesPerPacket = _recordFormat.mBytesPerFrame = (_recordFormat.mBitsPerChannel / 8) * _recordFormat.mChannelsPerFrame;
		_recordFormat.mFramesPerPacket = 1;
	}
}

-(void)addAudioBuffer:(AudioQueueBufferRef )buffer
{
    USCLog(@"Recorder -> addAudioBuffer");
    
    int _pcmSize = buffer->mAudioDataByteSize;
    char *_pcmData = (char *)buffer->mAudioData;
    
    NSData *audioData = [[NSData alloc] initWithBytes:_pcmData length:_pcmSize];
    
    if (debug)
    {
        [self appendData:audioData];
    }
    
    [pcmAnalysis analysisWithPCMData:audioData];
}

// 回调函数
void inputBufferHandler(void *inUserData,
                        AudioQueueRef inAQ,
                        AudioQueueBufferRef inBuffer,
                        const AudioTimeStamp *inStartTime,
                        UInt32 inNumPackets,
                        const AudioStreamPacketDescription *inPacketDesc)
{
    USCLog(@"recording -> inputBufferHandler | isRecrding : %d",isRecording);
    
    USCRecorder *recorder = (__bridge USCRecorder *)inUserData;
    if (recorder == nil)
    {
        return;
    }
        
    if (inNumPackets > 0 && isRecording)
    {
        // 回调录音数据
        [recorder addAudioBuffer:inBuffer];
        
        AudioQueueEnqueueBuffer(inAQ, inBuffer, 0, NULL);
    }
}

// 开始录音
- (void) startRecording
{
    if ([[[UIDevice currentDevice] systemVersion] compare:@"7.0"] != NSOrderedAscending)
    {
        AVAuthorizationStatus status = [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeAudio];
        USCLog(@"recording -> start status : %d",(int)status);
        
        if (status != AVAuthorizationStatusAuthorized
            && status != AVAuthorizationStatusNotDetermined)
        {
//            [_delegate onRecordingStart:Device_Audio_Authorize_failed];
            [_delegate onRecordingStart:Device_Record_Error];
            return;
        }
    }
    
    USCLog(@"recording -> start");

    //analysis reset
    [pcmAnalysis analysisReset];
    
    OSStatus error;    
    // set sesison 
    error = AudioSessionSetActive(true);
    if (error)
    {
        [_delegate onRecordingStart:RECORDING_AUDIO_SESSION_ERROR];
        return;
    }
  
    // category
    UInt32 category = kAudioSessionCategory_PlayAndRecord;
    error = AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(category), &category);
    if (error)
    {
        [_delegate onRecordingStart:RECORDING_CATEGORY_ERROR];
        return;
    }

    // 录音格式
    [self setupAudioFormat:kAudioFormatLinearPCM SampleRate:sampeleRate];
    
    // 设置回调函数
    error = AudioQueueNewInput(&_recordFormat, inputBufferHandler, (__bridge void*)self , NULL , NULL, 0 ,&_audioQueue);
    if (error)
    {
        [_delegate onRecordingStart:RECORDING_AUDIO_INPUT_ERROR];
        return;
    }
    // 创建缓冲器
    for (int i = 0; i < kNumberAudioQueueBuffers; ++i)
    {
        error = AudioQueueAllocateBuffer(_audioQueue, bufferByteSize, &_audioBuffers[i]);
        if (error)
        {
            [_delegate onRecordingStart:RECORDING_ALLOC_BUFFER_ERROR];
            return;
        }
        error = AudioQueueEnqueueBuffer(_audioQueue, _audioBuffers[i], 0, NULL);
        if (error)
        {
            [_delegate onRecordingStart:RECORDING_ENQUEUE_BUFFER_ERROR];
            return;
        }
    }
    
    // 开始录音
    isRecording = YES;
    isVADTimeOut = NO;
    _readyToStop = NO;
    
    error = AudioQueueStart(_audioQueue, NULL);
    if (error)
    {
        [_delegate onRecordingStart:RECORDING_AUDIO_START_ERROR];
        return;
    }
     
    // 通知录音启动成功
    [_delegate onRecordingStart:No_Error];
}

// 停止录音
- (void) stopRecording
{
    USCLog(@"recording -> stop | isRecording : %d",isRecording);
    
    if (isRecording)
    {
        isRecording = NO;
        
        //停止录音
        [self stopRecorder];
        
        //停止对pcm数据的处理
        [pcmAnalysis stopAnalysis];
        
        if (debug)
        {
            [self writeToFile];
        }
    }
}

-(void)stopRecorder
{
    //为解决录音完成后音量变小的问题 2014-9-23 By LiuJun
    // category
    UInt32 category = kAudioSessionCategory_MediaPlayback;
    AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(category), &category);
    
    AudioQueueStop(_audioQueue, true);
    AudioQueueDispose(_audioQueue, true);
}

// 取消录音
- (void) cancelRecording
{
    USCLog(@"recording -> cancel | isRecording : %d",isRecording);
    
    if (isRecording)
    {
        isRecording = NO;
        
        [self stopRecorder];
    }
}

// 获取系统音量
- (void)getSystemVolume
{
    AudioQueueLevelMeterState levelMeter;
    UInt32 levelMeterSize = sizeof(AudioQueueLevelMeterState);
    AudioQueueGetProperty(_audioQueue, kAudioQueueProperty_CurrentLevelMeterDB, &levelMeter, &levelMeterSize);
    if (_delegate) {
        [_delegate onUpdataVolume:levelMeter.mAveragePower];
    }
}

- (void) dealloc
{
    USCLog(@"recording -> dealloc");
    
    if(_delegate != nil)
    {
        _delegate = nil;
    }
    
    isRecording = NO;
    
    pcmAnalysis = nil;
    
    AudioQueueStop(_audioQueue, true);
    AudioQueueDispose(_audioQueue, true);
}

#pragma mark -
#pragma mark UniVADDelegate

- (void)uniPCMAnalysisDidUpdataVolume:(int)volume
{
    if (_delegate && [_delegate respondsToSelector:@selector(onUpdataVolume:)])
    {
        [_delegate onUpdataVolume:volume];
    }
}

- (void)uniPCMAnalysisIsTimeout
{
    //如果外部触发了停止，则不再向上触发vad超时回调
    if (_delegate
        && [_delegate respondsToSelector:@selector(onVADTimeout)]
        && !_readyToStop)
    {
        [_delegate onVADTimeout];
    }
}

- (void)uniPCMAnalysisDidCheckBuffer:(NSData *)buffer
{
    if (_delegate && [_delegate respondsToSelector:@selector(onRecordingBuffer:)])
    {
        [_delegate onRecordingBuffer:buffer];
    }
}

-(void)uniPCMAnalysisDisStop
{
    if (_delegate && [_delegate respondsToSelector:@selector(onRecordingStop)])
    {
        [_delegate onRecordingStop];
    }
}

@end
