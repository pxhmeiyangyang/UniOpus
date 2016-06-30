//
//  USCPCMReaderManager.h
//  usc
//
//  Created by 刘俊 on 15/7/8.
//  Copyright (c) 2015年 yunzhisheng. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "USCErrorCode.h"

typedef enum : int
{
    READ_FROM_MIC,
    READ_FROM_PCM_FILE,
}ReadingMode;

@protocol USCPCMReaderManagerDelegate <NSObject>

- (void) onUpdateVolume:(int)volume;
- (void) onVADTimeout;

- (void) onRecordingStart:(ErrorType)errorCode;
- (void) onRecordingBuffer:(NSData *)recordingData;
- (void) onRecordingStop;

@end

@interface USCPCMReaderManager : NSObject

@property (nonatomic, assign) id<USCPCMReaderManagerDelegate> delegate;
@property (nonatomic, strong) NSString *filePath;
@property (nonatomic, assign) ReadingMode readingMode;

-(void)startReading;

-(void)stopReading;

-(void)cancelReading;

-(void)setPCMFilePath:(NSString *)path;

-(void)setVadFrontTimeout:(int)frontTime backTimeout:(int)backTime;

@end
