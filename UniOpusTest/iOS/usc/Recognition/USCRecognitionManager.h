//
//  USCRecognitionManager.h
//  usc
//
//  Created by 刘俊 on 15/5/5.
//  Copyright (c) 2015年 yunzhisheng. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "USCResult.h"
#import "USCErrorCode.h"

typedef enum : int
{
    ONLINE_ONLY,
    MIX,
}RecognitionMode;

@protocol USCRecognitionManagerDelegate <NSObject>

- (void)recognizerDidStart;

- (void)recognizerDidGetResult:(NSString *)result;

- (void)recognizerDidStop:(ErrorType)error;

- (void)audioFileDidRecord:(NSString *)url;

- (void)recognizerDidGetResultUrl:(NSString *)url;

@end

@interface USCRecognitionManager : NSObject
{
    
}

@property (nonatomic, assign) id <USCRecognitionManagerDelegate> delegate;
// 口语测评文本
@property (nonatomic, retain) NSString *oralText;

// 企业客户名称，需要找商务申请，默认为空
@property (nonatomic, retain) NSString *oralTask;

// 企业客户配置参数，需要找商务申请，默认为空
@property (nonatomic, retain) NSString *oralExt1;
@property (nonatomic, retain) NSString *oralExt2;

@property (nonatomic, retain) USCResult *onlineResult;
@property (nonatomic, retain) USCResult *offlineResult;

// 离线引擎的资源目录（初始化必备）
@property (nonatomic, retain) NSString *path;

//设备唯一标识
@property (nonatomic, retain) NSString *identifier;

- (id)initWithSource:(NSString *)sourcePath;

//初始化离线引擎
- (ErrorType)initOfflineEngine:(NSString *)sourcePath;

- (void)startWithMode:(RecognitionMode)mode withOralText:(NSString *)text;

- (void)stop;

- (void)cancel;

- (void)appendAudioData:(NSData *)audioData;

- (void)setIdentifier:(NSString *)identifier;

- (void)setOralTask:(NSString *)oralTask;

@end
