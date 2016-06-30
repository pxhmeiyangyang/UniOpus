//
//  USCPCMReader.h
//  usc
//
//  Created by 刘俊 on 15/7/7.
//  Copyright (c) 2015年 yunzhisheng. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "USCErrorCode.h"
#import "UniPCMAnalysis.h"
#import "UniPCMRecorder.h"

@protocol USCPCMReaderDelegate <NSObject>

- (void) onReadPCMUpdateVolume:(int)volume;

- (void) onReadPCMFileStart:(ErrorType)errorCode;
- (void) onReadPCMFileStop:(ErrorType)errorCode;

- (void) onReadPCMData:(NSData *)data;

@end

@interface USCPCMReader : NSOperation <UniPCMAnalysisDelegate>
{
    NSString *filePath;         //pcm源文件路径
    BOOL isReading;
    UniPCMAnalysis *pcmAnalysis;
    
    BOOL analysisFinishing;
    
    UniPCMRecorder *testRecorder;
}

@property (nonatomic, assign) id<USCPCMReaderDelegate> delegate;

- (id) initWithPath:(NSString *)path;

@end
