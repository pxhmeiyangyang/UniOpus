//
//  USCRecognizer.m
//  usc
//
//  Created by hejinlai on 12-11-3.
//  Copyright (c) 2012年 yunzhisheng. All rights reserved.
//

#import "USCRecognizer.h"
#import "USCUtil.h"
#import "USCRecognitionManager.h"
#import "USCPCMReaderManager.h"
#import "USCPreference.h"

#import "USCMarcos.h"
#import "USCErrorCode.h"
#import <dispatch/dispatch.h>
#import "Settings.h"
#import "USCOralEduWrapper.h"
#import "USCMP3Encoder.h"

#define kBgQueue dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0)
#define kMainQueue dispatch_get_main_queue()

//单次识别过程中的状态转变，待完善
typedef enum : int
{
    BeginOral = 0,
    OralHasBegan = 1,
    StopOral = 2,
    OralHasStoped = 3,
    VADTimeOut = 3,
    RecordingStop = 4,
    RecognitionStart = 5,
    RecognitionWithResult = 6,
    RecognitionWithError = 6,
    RecognitionStop = 7,
    OralHasEnded = 8
}RecognizerLife;

@interface USCRecognizer() <USCRecognitionManagerDelegate, USCMP3EncoderDelegate, USCPCMReaderManagerDelegate>
{
    USCMP3Encoder *_mp3Encoder;
    NSOperationQueue *encoderQueue;
    
    dispatch_semaphore_t semaphore;
    
    // 识别是否取消
    BOOL isRecognizerCancelled;
    BOOL recognitionFinished;
    
    BOOL _isOnlineWhenMix;
    
    int _sampleRate;

    // 超时定时器
    NSTimer *oralTimer;
    
    USCRecognitionManager *recognitionManager;
    USCPCMReaderManager *pcmReaderManager;
    
    RecognizerLife recognizerLife;
    
    NSError *recognizerError;
    NSError *initError;
    
    NSString *_identifier;
}

@end

@implementation USCRecognizer

@synthesize delegate = _delegate;

+(NSString*)version
{
    NSString *keyStr = AppKey;
    //NSLog(@"keyStr : %@",keyStr);
    keyStr = [NSString stringWithFormat:@"%@***%@",[keyStr substringWithRange:NSMakeRange(0, 6)],[keyStr substringWithRange:NSMakeRange(keyStr.length - 6, 6)]];
    NSString *version = [NSString stringWithFormat:@"\n- - Unisound - -\nv%@ | m%d | %@",OralSdkVersion,MIX_RECOGNIZE,keyStr];
    return version;
}

- (id)init
{
    if (self = [super init])
    {
        //initData方法必须第一个执行
        [self initData];
        [self initReaderManager];
        [self initRecognition:nil];
        
        initError = nil;
        
        USCLog(@"SDKVersion : %@",OralSdkVersion);
        USCLog(@"recognizer -> init");
    }
    
    return self;
}

- (id)initWithSource:(NSString *)sourcePath
{
    if (self = [super init])
    {
        //initData方法必须第一个执行
        [self initData];
        [self initReaderManager];
        ErrorType type = [self initRecognition:sourcePath];
        initError = [USCErrorCode getError:type];
        
        USCLog(@"SDKVersion : %@",OralSdkVersion);
        USCLog(@"recognizer -> init");
    }
    
    return self;
}

-(void)initData
{
    encoderQueue = [[NSOperationQueue alloc]init];
    
    [encoderQueue setMaxConcurrentOperationCount:1];
    
    isRecognizerCancelled = NO;
    
    _sampleRate = 16000;
    
    recognizerLife = OralHasEnded;
    
    semaphore = dispatch_semaphore_create(1);
    
    _audioType = AudioType_PCM;
    
    //默认不启用
    _asyncRecognize = NO;

}

-(void)resetData
{
    USCLog(@"recognizer -> resetData");
    recognizerError = nil;
    isRecognizerCancelled = NO;
    recognitionFinished = NO;
}

-(void)initReaderManager
{
    USCLog(@"recognizer -> initReaderManager");
    pcmReaderManager = [[USCPCMReaderManager alloc]init];
    pcmReaderManager.delegate = self;
}

-(ErrorType)initRecognition:(NSString *)sourcePath
{
    ErrorType type = No_Error;
    
    recognitionManager = [[USCRecognitionManager alloc]init];
    recognitionManager.delegate = self;
    if (MIX_RECOGNIZE)
    {
        type = [recognitionManager initOfflineEngine:sourcePath];
    }
    return type;
}

// 设置vad前置端点和后置端点的静音时间
- (void)setVadFrontTimeout:(int)frontTime backTimeout:(int)backTime
{
    [pcmReaderManager setVadFrontTimeout:frontTime backTimeout:backTime];
}

/**
 * 设置识别超时，默认为3S（从停止录音后开始计时）
 */
-(void)setRecognitionTimeout:(NSTimeInterval)interval
{
    USCPreference *preference = [USCPreference sharePreference];
    [preference setRecognitionTimeout:interval];
}

-(void)setAudioType:(AudioType)audioType
{
    _audioType = audioType;
}

-(void)setDelegate:(id<USCRecognizerDelegate>)delegate
{
    USCLog(@"recognizer -> set delegate");
    
    _delegate = delegate;
    
    if (_delegate && [_delegate respondsToSelector:@selector(oralEngineDidInit:)])
    {
        [_delegate oralEngineDidInit:initError];
    }
}

/*
     停止录音和识别，清空录音数据
 */
- (void)clear
{
    USCLog(@"recognizer -> clear");
    
    // 停止录音
    [pcmReaderManager cancelReading];
    
    // 取消识别线程
    [recognitionManager cancel];
    
    //取消MP3转码线程
    [self cancelMP3Encoder];
    
    isRecognizerCancelled = YES;
}

#pragma mark - setting

// 开始识别
- (void)start
{
    USCLog(@"                  ");
    USCLog(@"                  ");
    USCLog(@"                  ");
    USCLog(@"                  ");
    USCLog(@"                  ");
    USCLog(@"                  ");
    USCLog(@"                  ");
    USCLog(@"recognizer -> start | recognizerLife : %d", recognizerLife);
    
    dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);
    
    [self resetData];
    
    [self setCondition:BeginOral];
    
    //参数检测（必须在状态重置后执行）
    BOOL result = [self parameterChecking];
    if (result)
    {
        // 开始录音
        pcmReaderManager.readingMode = READ_FROM_MIC;
        [pcmReaderManager startReading];
    }
}

- (void)startWithPCM:(NSString *)path
{
    USCLog(@"                  ");
    USCLog(@"                  ");
    USCLog(@"                  ");
    USCLog(@"                  ");
    USCLog(@"                  ");
    USCLog(@"                  ");
    USCLog(@"                  ");
    USCLog(@"recognizer -> startWithPCM : %@ | recognizerLife : %d", path, recognizerLife);
    
    dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);
    
    [self resetData];
    
    [self setCondition:BeginOral];
    
    //参数检测（必须在状态重置后执行）
    BOOL result = [self parameterChecking];
    if (result)
    {
        //启动pcm文件读取线程
        [self startPCMReader:path];
    }
}

// 停止识别
- (void)stop
{
    USCLog(@"recognizer -> stop | recognizerLife : %d",recognizerLife);
    
    dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);
    
    if (recognizerLife < OralHasBegan)
    {
//        [self clear];
//
//        [self oralEnd:[USCErrorCode getError:Device_Record_Error]];
        
        return ;
    }
    
    [self setCondition:StopOral];
    
    [pcmReaderManager stopReading];
}

// 取消识别
- (void)cancel
{
    USCLog(@"recognizer -> cancel");
    
    [self clear];
    
    [self setCondition:OralHasEnded];
    
    if (_delegate && [_delegate respondsToSelector:@selector(onEndOral:)])
    {
        [_delegate onEndOral:nil];
    }
}

- (void)setIdentifier:(NSString *)identifier
{
    USCLog(@"recognizer -> setIdentifier : %@",identifier);
    _identifier = [[NSString alloc]initWithString:identifier];
}

- (void)setOutScoreCoefficient:(float)score
{
    USCLog(@"recognizer -> setOutScoreCoefficient : %f",score);
    
    USCPreference *preference = [USCPreference sharePreference];
    [preference setCoefficientScore:score];
}

-(void)setAsyncRecognize:(BOOL)asyncRecognize
{
    USCLog(@"recognizer -> setAsyncRecognize : %d",asyncRecognize);
    
    //只在纯在线模式中生效
    if (!MIX_RECOGNIZE)
    {
        _asyncRecognize = asyncRecognize;
        
        USCPreference *preference = [USCPreference sharePreference];
        [preference setAsyncRecognize:_asyncRecognize];
    }
    else
    {
        NSLog(@"AsyncRecognize just be useful in online mode");
    }
}



-(void)startRecognition
{
    USCLog(@"recognizer -> startRecognition");
    [recognitionManager setIdentifier:_identifier];
    [recognitionManager setOralTask:_oralTask];
    
    BOOL isJsonStr = [self stringIsJson:_oralText];
    
    if (MIX_RECOGNIZE && !isJsonStr && !_isOnlineWhenMix)
    {
        [recognitionManager startWithMode:MIX withOralText:_oralText];
    }
    else
    {
        [recognitionManager startWithMode:ONLINE_ONLY withOralText:_oralText];
    }
}

-(void)stopRecognition
{
    USCLog(@"recognizer -> stopRecognition");
    [recognitionManager stop];
}

-(void)startMP3Encoder
{
    USCLog(@"recognizer -> startMp3Encoder");
    
    if (_audioType == AudioType_MP3)
    {
        if (_mp3Encoder)
        {
            _mp3Encoder = nil;
        }
        _mp3Encoder = [[USCMP3Encoder alloc]init];
        _mp3Encoder.delegate = self;
        _mp3Encoder.sampleRate = _sampleRate;
        
        [encoderQueue addOperation:_mp3Encoder];
    }
}

-(void)stopMP3Encoder
{
    USCLog(@"recognizer -> stopMP3Encoder");
    
    if (encoderQueue)
    {
        _mp3Encoder.setToStopped = YES;
    }
}

- (void)cancelMP3Encoder
{
    USCLog(@"recognizer -> cancelMP3Encoder");
    
    if (_mp3Encoder)
    {
        [_mp3Encoder encodeCancel];
    }
}

-(void)setCondition:(RecognizerLife)condition
{
    USCLog(@"recognizer -> setCondition : %d",condition);
    if (condition == OralHasEnded) {
        _isOnlineWhenMix = NO;
    }
    recognizerLife = condition;
}

- (void)setIsOnlineWhenMix:(BOOL)isOnline{
    if (MIX_RECOGNIZE) {
        _isOnlineWhenMix = isOnline;
    }
}

/**
 *  设置离线结果等待时长（离在线混合模式下得到离线评测结果后等待在线评测结果的时长）
 *  此函数仅在离在线混合模式下生效
 *  默认等待时长为1.0秒
 *  @param time 等待时长
 */
- (void)setOfflineResultWaitingTime:(NSTimeInterval)time{
     USCLog(@"recognizer -> setOfflineResultWaitingTime : %f",time);
    if (MIX_RECOGNIZE) {
        USCPreference *preference = [USCPreference sharePreference];
        [preference setOfflineResultWaitingTime:time];
    }
}

-(BOOL)parameterChecking
{
    USCLog(@"recognizer -> parameterChecking");
    
    BOOL result = YES;
    ErrorType type = No_Error;

    if (_oralText == nil || _oralText.length == 0)
    {
        result = NO;
        type = Ser_Empty_Text;
    }
    
    if (_oralText.length >= 5000)
    {
        result = NO;
        type = Ser_Text_Is_Too_Long;
    }
    
    //判断文本是否只包含空格
    NSString *tmpStr = [[NSString alloc]initWithString:_oralText];
    tmpStr = [tmpStr stringByReplacingOccurrencesOfString:@" " withString:@""];
    if (tmpStr == nil || tmpStr.length == 0)
    {
        result = NO;
        type = Ser_Empty_Text;
    }
    
    if (!result)
    {
        [self clear];
        
        [self oralEnd:[USCErrorCode getError:type]];
    }

    return result;
}

-(void)loadFinalEnd
{
    USCLog(@"recognizer -> loadFinalEnd | recognitionFinished : %d | hasFinished : %d",recognitionFinished,[_mp3Encoder hasFinished]);
    USCLog(@"recognizer -> loadFinalEnd | recognizerLife : %d",recognizerLife);
    
    if (recognizerLife == OralHasEnded)
    {
        return;
    }
    
    if (_audioType == AudioType_PCM)
    {
        if (recognitionFinished)
        {
            // 通知外面接口本次识别成功结束
            [self oralEnd:recognizerError];
        }
    }
    else
    {
        if ((_mp3Encoder && [_mp3Encoder hasFinished] && recognitionFinished)
            ||(_mp3Encoder && _mp3Encoder.isEncodeCanceled && recognitionFinished))
        {
            // 通知外面接口本次识别成功结束
            [self oralEnd:recognizerError];
        }
    }
}

-(void)startPCMReader:(NSString *)path
{
    pcmReaderManager.readingMode = READ_FROM_PCM_FILE;
    [pcmReaderManager setPCMFilePath:path];
    [pcmReaderManager startReading];
}

#pragma mark -
#pragma mark - Out CallBack

-(void)oralBegin
{
    [self setCondition:OralHasBegan];
    
    dispatch_semaphore_signal(semaphore);
    
    BOOL isMainThread = [NSThread isMainThread];
    if (isMainThread)
    {
        if (_delegate && [_delegate respondsToSelector:@selector(onBeginOral)])
        {
            [_delegate onBeginOral];
        }
    }
    else
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            
            USCLog(@"Reader -> Jump to mainThread");
            
            if (_delegate && [_delegate respondsToSelector:@selector(onBeginOral)])
            {
                [_delegate onBeginOral];
            }
        });
    }
}

-(void)oralEnd:(NSError *)error
{
#if CUSTOM_LOG
    //在每次评测结束后将log写入文件
    LJWriteLogToFile();
    
//    NSString *oralLog = getLogString();
//    if (_delegate && [_delegate respondsToSelector:@selector(oralLog:)]) {
//        [_delegate oralLog:oralLog];
//    }

#endif
    
    dispatch_semaphore_signal(semaphore);

    [self setCondition:OralHasEnded];
    
    // 通知录音识别结束
    if (_delegate && [_delegate respondsToSelector:@selector(onEndOral:)])
    {
        [_delegate onEndOral:error];
    }
}

#pragma mark -
#pragma mark USCPCMReaderManager Callback

// 音量大小
- (void) onUpdateVolume:(int)volume
{
    if (_delegate && [_delegate respondsToSelector:@selector(onUpdateVolume:)])
    {
        [_delegate onUpdateVolume:volume];
    }
}

// vad超时
- (void) onVADTimeout
{
    USCLog(@"recognizer -> onVADTimeout");
    
    if (_delegate && [_delegate respondsToSelector:@selector(onVADTimeout)])
    {
        [_delegate onVADTimeout];
    }
}

// 录音启动是否成功
- (void) onRecordingStart:(ErrorType)errorCode
{
    USCLog(@"recognizer -> onRecordingStart : %d",errorCode);
    
    if (errorCode == No_Error)
    {
        // 通知录音启动成功
        [self oralBegin];
        USCLog(@"recognizer -> onBeginOral");
        
        dispatch_async(dispatch_get_main_queue(), ^{
            
            //启动MP3编码线程
            [self startMP3Encoder];
            
            // 启动识别线程
            [self startRecognition];
        });
    }
    else
    {
        BOOL isMainThread = [NSThread isMainThread];
        USCLog(@"isMainThread : %d",isMainThread);
        if (isMainThread)
        {
            [self clear];
            
            [self oralEnd:[USCErrorCode getError:errorCode]];
        }
        else
        {
            USCLog(@"recognizer -> recording start | Jump to mainThread");

            dispatch_async(dispatch_get_main_queue(), ^{
                
                [self clear];
                
                [self oralEnd:[USCErrorCode getError:errorCode]];
                
                USCLog(@"recognizer -> onEndOral error = %i", errorCode);
            });
        }
    }
}

- (void)onRecordingBuffer:(NSData *)recordingData
{
    USCLog(@"recognizer -> onRecordingBuffer");
    
    [recognitionManager appendAudioData:recordingData];
    
    if(_audioType == AudioType_MP3)
    {
        [_mp3Encoder appendAudioData:recordingData];
    }
    else
    {
        if (_delegate && [_delegate respondsToSelector:@selector(onRecordingBuffer:)])
        {
            [_delegate onRecordingBuffer:recordingData];
        }
    }
}

- (void)onRecordingStop
{
    USCLog(@"recognizer -> onStopOral | isRecognizerCancelled : %d | recognizerLife : %d", isRecognizerCancelled, recognizerLife);
    
    if (recognizerLife == OralHasEnded)
    {
        return ;
    }
    
    // 识别取消，不再启动识别线程
    if (isRecognizerCancelled)
    {
        [self oralEnd:[USCErrorCode getError:Device_Record_Error]];
        
        return;
    }
    
    if (_delegate && [_delegate respondsToSelector:@selector(onStopOral)])
    {
        [_delegate onStopOral];
    }
    
    [self performSelector:@selector(stopRecognition) withObject:nil];
    
    if (_audioType == AudioType_MP3)
    {
        [self performSelector:@selector(stopMP3Encoder) withObject:nil];
    }
}

#pragma mark -
#pragma mark USCMP3Encoder Callback

-(void)audioDataDidEncode:(NSData *)encodingData
{
    //USCLog(@"recognizer -> audioDataDidEncode : %d",(int)encodingData.length);
    
    if (_delegate && [_delegate respondsToSelector:@selector(onRecordingBuffer:)])
    {
        [_delegate onRecordingBuffer:encodingData];
    }
}

-(void)encodeDidFinished
{
    USCLog(@"recognizer -> encodeDidFinished");
    
    [self loadFinalEnd];
}


#pragma mark - USCRecognitionManagerDelegate Callback

- (void)recognizerDidStart
{
    
}

- (void)recognizerDidGetResult:(NSString *)result
{
    USCLog(@"recognizer -> recognizerDidGetResult | isRecognizerCancelled : %d",isRecognizerCancelled);

    if (!isRecognizerCancelled)
    {
        //如果启用延时评测，则不回调
        if (!_asyncRecognize)
        {
            if (_delegate && [_delegate respondsToSelector:@selector(onResult: isLast:)])
            {
                [_delegate onResult:result isLast:YES];
            }
        }
    }
}

- (void)recognizerDidStop:(ErrorType )error
{
    USCLog(@"recognizer -> recognizerDidStop : %d",error);
    
    if (error != No_Error)
    {
        recognizerError = nil;
        recognizerError = [USCErrorCode getError:error];
        
        //如果识别出错，则取消本次操作
        // 停止录音
        [pcmReaderManager cancelReading];
        
        //取消MP3转码线程
        [self cancelMP3Encoder];
        
        isRecognizerCancelled = YES;
    }

    recognitionFinished = YES;
    
    [self loadFinalEnd];
}

- (void)engineDidInit:(ErrorType)error
{
    if (_delegate && [_delegate respondsToSelector:@selector(oralEngineDidInit:)])
    {
        [_delegate oralEngineDidInit:[USCErrorCode getError:error]];
    }
}

- (void)audioFileDidRecord:(NSString *)url
{
    USCLog(@"recognizer -> audioFileDidRecord | url : %@",url);
    
    if(url != NULL && url.length > 0)
    {
        if (_delegate && [_delegate respondsToSelector:@selector(audioFileDidRecord:)])
        {
            [_delegate audioFileDidRecord:url];
        }
    }
}

- (void)recognizerDidGetResultUrl:(NSString *)url
{
    USCLog(@"recognizer -> recognizerDidGetResultUrl | url : %@",url);
    
    if(url != NULL && url.length > 0)
    {
        if (_delegate && [_delegate respondsToSelector:@selector(onAsyncResult:)])
        {
            [_delegate onAsyncResult:url];
        }
    }
}

- (void)dealloc
{
    USCLog(@"recognizer -> dealloc");
    
    [self destory];
}

-(void)destory
{
    USCLog(@"destory in");
    
    USCLog(@"destory in delegate");
    
    // 代理对象
    if (_delegate != nil)
    {
        _delegate = nil;
    }
    
    USCLog(@"--- destory finish ---");
}

#pragma mark - tools
-(BOOL)stringIsJson:(NSString* )string{
    if ([string hasPrefix:@"{"] && [string hasSuffix:@"}"]) {
        return YES;
    }
    return NO;
}

@end
