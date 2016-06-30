//
//  USCRecognition_offline.h
//  usc
//
//  Created by 刘俊 on 15/1/5.
//  Copyright (c) 2015年 yunzhisheng. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "USCBaseRecognition.h"
#import "USCErrorCode.h"

@class USCOralEduWrapper;

@protocol USCOfflineRecognitionDelegate <NSObject>

- (void)onOfflineRecognitionStart;
- (void)onOfflineRecognitionResult:(NSString *)result isLast:(BOOL)isLast;
- (void)onOfflineRecognitionStop;
- (void)onOfflineRecognitionError:(ErrorType)error;
- (void)onOfflineMaxSpeechTimeout;

@end

@interface USCRecognition_offline : USCBaseRecognition
{
    USCOralEduWrapper *eduOfflineEngine;
}

@property (nonatomic, strong) NSMutableArray *recordingQueue;
@property (nonatomic, assign) BOOL setToStopped;
@property (nonatomic, assign) id <USCOfflineRecognitionDelegate> delegate;
@property (nonatomic, assign) NSString *appKey;
@property (nonatomic, assign) int sampleRate;
@property (nonatomic, assign) int netType;
@property (nonatomic, assign) NSString *oralText;
@property (nonatomic, assign) NSString *domain;
@property (nonatomic, assign) short port;
@property (nonatomic, assign) NSString *oralTask;
@property (nonatomic, assign) NSString *oralExt1;
@property (nonatomic, assign) NSString *oralExt2;

- (void)setEngine:(USCOralEduWrapper *)wrapper;
- (void)appendAudioData:(NSData *)audioData;

@end
