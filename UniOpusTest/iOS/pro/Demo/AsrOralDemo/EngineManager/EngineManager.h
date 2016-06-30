//
//  EngineManager.h
//  AsrOralDemo
//
//  Created by 刘俊 on 15/6/25.
//  Copyright (c) 2015年 yunzhisheng. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "USCRecognizer.h"
#import "EngineStateMachine.h"
#import "PerformanceWatcher.h"

#define FILE_NAME_RECORDING            @"record.mp3" //音频输出已转码为MP3
#define FILE_INPUT_PCM                 @"record.pcm" //用于本地保存的pcm文件名
#define FILE_INPUT_TEST_PCM            @"test.pcm"   //用于识别的pcm文件名

typedef NS_ENUM(NSUInteger, RecognizeMode) {
    Recognize_REC,
    Recognize_PCM,
};

typedef NS_ENUM(NSInteger, EngineErrorCode) {
    NO_PCM_FILE = 100,
};

typedef enum : int
{
    OralBegan = 0,
    OralStoped = 1,
    OralEnd = 2,
}RecognizerStatus;

@protocol EngineManagerDelegate <NSObject>

- (void)onBeginOral;

- (void)onStopOral;

- (void)onResult:(NSString *)result isLast:(BOOL)isLast;

- (void)onEndOral:(NSError *)error;

- (void)onVADTimeout;

- (void)onUpdateVolume:(int)volume;

- (void)onRecordingBuffer:(NSData *)recordingData;

- (void)oralEngineDidInit:(NSError *)error;

- (void)audioFileDidRecord:(NSString *)url;

- (void)onAsyncResult:(NSString *)url;

@end

@interface EngineManager : NSObject<USCRecognizerDelegate>
{
    USCRecognizer *recognizer;
    NSMutableData *recordData;
    
    RecognizeMode recogMode;
    RecognizerStatus recogStatus;
    
    EngineStateMachine *stateMachine;
    PerformanceWatcher *watcher;
}

@property (nonatomic,assign)id <EngineManagerDelegate> delegate;

@property (nonatomic,assign)BOOL isOnlineWhenMix;

/*
 *初始化
 */
+ (EngineManager *)sharedManager;

/*
 *版本号
 */
- (NSString *)version;

/**
 *设置录音来源（麦克风或者PCM文件）
 */
-(void)setRecognizerMode:(RecognizeMode)mode;

/**
 *获取当前录音来源（麦克风或者PCM文件）
 */
-(RecognizeMode)getRecognizerMode;

/**
 *获取当前识别模式（A、B、C、D、E、enstar、gzedunet、gzedunet_answer）
 */
-(NSString *)getOralTask;

/**
 *设置评测文本
 */
-(void)setOralText:(NSString *)text;

/**
 *设置引擎运行模式,默认为B模式
 */
-(void)setOralTask:(NSString *)task;

/**
 *启动识别
 */
-(void)startRecognize;

/**
 *停止识别
 */
-(void)stopRecognize;

/**
 *取消识别
 */
-(void)cancelRecognize;

/**
 *  获取延时评测状态
 *
 *  @return 返回状态值
 */
-(BOOL)getAsyncRecognize;

/**
 *  设置延时评测状态
 *
 *  @param asyncRecognize 开启（yes）或者关闭（no）
 */
-(void)setAsyncRecognize:(BOOL)asyncRecognize;

/**
 *  根据评测文本预估阅读时长
 *
 *  @param text 评测文本
 */
-(NSInteger)getTimeByOralText:(NSString* )text;


/**
 *  设置离线结果等待时长（离在线混合模式下得到离线评测结果后等待在线评测结果的时长）
 *  此函数仅在离在线混合模式下生效
 *  默认等待时长为1.0秒
 *  @param time 等待时长
 */
- (void)setOfflineResultWaitingTime:(NSTimeInterval)time;
@end
