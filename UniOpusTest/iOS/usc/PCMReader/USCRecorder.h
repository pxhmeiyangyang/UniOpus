//
//  USCRecorder.h
//  usc
//
//  Created by hejinlai on 12-11-16.
//  Copyright (c) 2012年 yunzhisheng. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AudioToolbox/AudioToolbox.h>
#import "USCErrorCode.h"
#import "UniPCMAnalysis.h"

#define kNumberAudioQueueBuffers 3
#define kBufferDurationSeconds 0.1f


@protocol USCRecorderDelegate <NSObject>

- (void) onUpdataVolume:(int)volume;
- (void) onVADTimeout;

- (void) onRecordingStart:(ErrorType)errorCode;
- (void) onRecordingBuffer:(NSData *)recordingData;
- (void) onRecordingStop;

@end

@interface USCRecorder : NSObject<UniPCMAnalysisDelegate>
{
    AudioQueueRef				_audioQueue;
    AudioQueueBufferRef			_audioBuffers[kNumberAudioQueueBuffers];
    AudioStreamBasicDescription	_recordFormat;
    
    UniPCMAnalysis *pcmAnalysis;
}

@property (nonatomic, assign) id<USCRecorderDelegate> delegate;
@property (nonatomic, assign) int netType;
@property (nonatomic, assign) int sampleRate;
@property (nonatomic, assign) NSMutableData *recordingDatas;
@property (nonatomic, assign) BOOL readyToStop;//准备执行停止操作，此时间段内不再触发超时

- (void)setVadFrontTimeout:(int)frontTime BackTimeout:(int)backTime;

- (void) startRecording;
- (void) stopRecording;
- (void) cancelRecording;

@end
