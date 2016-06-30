//
//  USCRecognitionManager.m
//  usc
//
//  Created by 刘俊 on 15/5/5.
//  Copyright (c) 2015年 yunzhisheng. All rights reserved.
//


/*
 MIX模式执行规则
 
 ---onResult(-R-)和onError(-E-)方法在单次识别中只会执行一个
 
 onlineResult   OR
 onlineError    OE
 offlineResult  FR
 offlineResult  FE
 
 可能产生的时序是：
 OR_FR      -R-
 OR_FE      -R- -E-
 OE_FR      -E- -R-
 OE_FE      -E-
 FR_OR      -R-
 FR_OE      -R- -E-
 FE_OR      -E- -R-
 FE_OE      -E-
 
 */

#import "USCRecognitionManager.h"
#import "USCRecognition.h"
#import "USCRecognition_offline.h"
#import "USCMarcos.h"
#import <dispatch/dispatch.h>
#import "Settings.h"
#import "USCOralEduWrapper.h"

#import "USCAsyncSocket.h"

#import "USCPreference.h"

#import "UniPCMRecorder.h"

#define kBgQueue dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0)
#define kMainQueue dispatch_get_main_queue()

@interface USCRecognitionManager()<USCRecognitionDelegate, USCOfflineRecognitionDelegate>
{
    NSOperationQueue *_operationQueue;
    USCRecognition *_recognition;
    USCRecognition_offline *_recognitionOffline;
    
    NSString *_appKey;
    // 识别是否取消
    BOOL isRecognizerCancelled;
    // 识别是否结束(在线或离线识别线程执行了stop回调,每回合开始时重置)
    BOOL isRecognizerStopped;
    // 已经回调识别结果
    BOOL isResultCallback;
    
    int _sampleRate;
    
    RecognitionMode recognitionMode;
    
    //离线识别引擎
    USCOralEduWrapper *eduOfflineEngine;
    
    //Socket引擎
    USCAsyncSocket *socketEngine;
    
    //识别超时
    NSTimer *recognitionTimer;
    
    UniPCMRecorder *testRecorder;
}

@end

@implementation USCRecognitionManager

@synthesize delegate = _delegate;

- (id)init
{
    if (self = [super init])
    {
        [self initData];
        
        USCLog(@"Manager -> init");
    }
    
    return self;
}

- (id)initWithSource:(NSString *)sourcePath
{
    if (self = [super init])
    {
        [self initData];
        
        USCLog(@"Manager -> init");
    }
    
    return self;
}

-(void)initData
{
    _appKey = AppKey;
    
    _operationQueue = [[NSOperationQueue alloc] init];
    
    [_operationQueue setMaxConcurrentOperationCount:1];
    
    socketEngine = [[USCAsyncSocket alloc]init];
    
    isRecognizerCancelled = NO;
    
    _sampleRate = 16000;
    
    _oralTask = @"B"; //默认为B模式
    
    _onlineResult = [[USCResult alloc]initWithType:uOnLineResult];
    _onlineResult.appKey = AppKey;
    _offlineResult = [[USCResult alloc]initWithType:uOfflineFixResult];
    _offlineResult.appKey = AppKey;
    
    //testRecorder = [UniPCMRecorder defaultRecorder];
    
    USCLog(@"Manager -> initData");
}

-(void)resetData
{
    USCLog(@"Manager -> resetData");
    USCLog(@"_operationQueue.operations : %@",_operationQueue.operations);
    
    [_onlineResult reset];
    [_offlineResult reset];
    
    // 识别是够结束(在线或离线识别线程执行了stop回调,每回合开始时重置)
    isRecognizerStopped = NO;
    
    isRecognizerCancelled = NO;
    
    isResultCallback = NO;
    
    //根据用户输入格式化评测模式
    [self updateOralTask];
    
    [testRecorder start:@"USCRecognitionManager"];
}

//初始化离线引擎
-(ErrorType)initOfflineEngine:(NSString *)sourcePath
{
    _path = sourcePath;
    return [self initEngine_offline];
}

// 停止识别
- (void)stop
{
    USCLog(@"Manager -> stop | recognitionMode : %d",recognitionMode);
    
    [testRecorder finish];
    
    // 停止离线识别线程，在线等待编码完成后停止
    if (recognitionMode == MIX)
    {
        if (_recognitionOffline)
        {
            _recognitionOffline.setToStopped = YES;
        }
        
        if (_recognition)
        {
            [_recognition stop];
        }
    }
    else
    {
        if (_recognition)
        {
            [_recognition stop];
        }
    }
}

// 取消识别
- (void)cancel
{
    USCLog(@"Manager -> cancel");
    
    _recognitionOffline.setToStopped = YES;
    [_recognitionOffline cancel];
    
    [_recognition cancel];
    //_recognition = nil;
    _recognitionOffline = nil;
    
    // 取消识别线程
    [_operationQueue cancelAllOperations];
    
    isRecognizerCancelled = YES;
}

//立即结束本次识别流程
/*
 执行本函数时，说明还未得到在线结果
 若此时离线有结果，则返回离线结果
 若此时离线无结果，则返回超时
 */
-(void)finish
{
    USCLog(@"Manager -> finish | isResultCallback : %d | isRecognizerCancelled : %d",isResultCallback, isRecognizerCancelled);
    
    if (isResultCallback)
    {
        return ;
    }
    
    if (!isRecognizerCancelled)
    {
        if (recognitionMode == MIX)
        {
            if (_offlineResult.content != NULL
                && _offlineResult.content.length > 0)
            {
                USCLog(@"Manager -> finish | callbackOfflineReslut");
                
                //先给出结果，再停止识别
                [self callbackReslut:_offlineResult];
                
                isResultCallback = YES;
                
                [self loadFinalStop];
            }
        }
    }
}

/*
 如果得到离线结果时在线还没有结果，则启动在线识别超时定时器
 */
-(void)startTimer
{
    if (_offlineResult.content != NULL
        && _offlineResult.content.length > 0
        && _offlineResult.resultError == No_Error
        && _onlineResult.content == NULL
        && _onlineResult.resultError == No_Error)
    {
        NSTimeInterval recognitionTimeout = [USCPreference sharePreference].recogntionTimeout;
        
        USCLog(@"Manager -> startTimer | timeout : %.f",recognitionTimeout);
        
        [recognitionTimer invalidate];
        recognitionTimer = [NSTimer scheduledTimerWithTimeInterval:recognitionTimeout
                                                            target:self
                                                          selector:@selector(finish)
                                                          userInfo:nil
                                                           repeats:NO];
    }
}

-(void)cancelTimer
{
    USCLog(@"Manager -> cancelTimer | isMainThread : %d",[NSThread isMainThread]);
    
    if ([NSThread isMainThread])
    {
        if (recognitionTimer)
        {
            USCLog(@"Manager -> cancelTimer | in Main");
            [recognitionTimer invalidate];
            recognitionTimer = nil;
        }
    }
    else
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            
            USCLog(@"Manager -> cancelTimer | Jump to Main");
            if (recognitionTimer)
            {
               [recognitionTimer invalidate];
                recognitionTimer = nil;
            }
        });
    }
}

- (void)setIdentifier:(NSString *)identifier
{
    USCLog(@"Manager -> setIdentifier : %@",identifier);
    
    _identifier = identifier;
}

- (void)setOralTask:(NSString *)oralTask
{
    USCLog(@"Manager -> setOralTask : %@",oralTask);
    
    if (oralTask != nil && oralTask.length > 0)
    {
        _oralTask = oralTask;
    }
}

// 开始识别
- (void)startWithMode:(RecognitionMode)mode withOralText:(NSString *)text
{
    USCLog(@"Manager -> startWithMode | mode : %d |result : %@", mode, text);
    
    recognitionMode = mode;
    
    [self resetData];
    
    _oralText = text;
    
    if (mode == ONLINE_ONLY)
    {
        [self startOnlineRecognition];

        USCLog(@"Only online Recognition");
    }
    else if (mode == MIX)
    {
        //在线
        [self startOnlineRecognition];
        //离线
        [self startOfflineRecognition];
        
        USCLog(@"Mix Recognition");
    }
}

- (void)appendAudioData:(NSData *)audioData
{
    if (audioData == NULL)
    {
        USCLog(@"Manager -> appendAudioData | no audioData");
        return;
    }
    
    if (_recognition)
    {
        [_recognition appendAudioData:audioData];
    }
    
    if (_recognitionOffline)
    {
        [_recognitionOffline appendAudioData:audioData];
    }
    
    [testRecorder appendData:audioData];
}

- (void)startOnlineRecognition
{
    USCLog(@"Manager - > startOnlineRecognition");
    
    if (_recognition)
    {
        _recognition = nil;
    }
    
    BOOL backupEnable = YES;
    BOOL asyncRecognize = [[USCPreference sharePreference]getAsyncRecognize];
    if (recognitionMode == MIX || asyncRecognize)
    {
        backupEnable = NO;
    }
    _recognition = [[USCRecognition alloc] init];
    _recognition.appKey = _appKey;
    _recognition.delegate = self;
    _recognition.oralText = _oralText;
    _recognition.oralTask = _oralTask;
    _recognition.identifier = _identifier;
    _recognition.backupEnable = backupEnable;
    
    [_recognition start];
}

-(void)recognitionRelease
{
    USCLog(@"Manager - > recognitionRelease");
    
    if (_recognitionOffline!=nil)
    {
        _recognitionOffline.setToStopped = YES;
    }
    
    if (_recognitionOffline!=nil)
    {
        [_recognitionOffline cancel];
    }
    
    _recognition = nil;
    _recognitionOffline = nil;
}

-(void)updateOralTask
{
    USCLog(@"Manager - > updateOralTask ： %@",_oralTask);
    
    if (_oralTask != nil && _oralTask.length > 0)
    {
        float score = [[USCPreference sharePreference]getCoefficientScore];
        
        //如果score为1，则输入什么模式就使用什么模式
        if(score != 1)
        {
            if(_oralTask.length > 30)
            {
                //if (![_oralTask containsString:@"OUT_SCORE_COEFFICIENT"])
                NSRange range = [_oralTask rangeOfString:@"OUT_SCORE_COEFFICIENT"];
                if (range.length == 0)
                {
                    _oralTask = [NSString stringWithFormat:@"%@#OUT_SCORE_COEFFICIENT=%.2f", _oralTask, score];
                }
            }
            else
            {
                NSString *otherSettings = [self getServiceParametersByMode:_oralTask];
                _oralTask = [NSString stringWithFormat:@"%@#OUT_SCORE_COEFFICIENT=%.2f", otherSettings, score];
            }
        }
    }
    
    USCLog(@"Manager - > updateOralTask : %@",_oralTask);
}

-(NSString *)getServiceParametersByMode:(NSString *)mode
{
    USCLog(@"Manager - > getServiceParametersByMode : %@",mode);
    
    if(mode == NULL)
    {
        mode = @"A";
    }
    
    if([mode isEqualToString:@"enstar"])
    {
        return @"enstar,1,IN_ACCENT#OUT_SENT_REF_TEXT#OUT_SENT_ASR_TEXT#OUT_SENT_TIMESTAMP#OUT_SENT_SCORE#OUT_WORD_TEXT#OUT_WORD_CLASS#OUT_WORD_TIMESTAMP#OUT_WORD_SCORE#OUT_WORD_VOLUME#OUT_PHONE_TEXT#OUT_PHONE_TIMESTAMP#OUT_PHONE_VOLUME#OUT_PHONE_ACCENT#OUT_SENT_FLUENCY#OUT_SENT_INTEGRITY#OUT_SENT_PRONUNCIATION#OUT_WORD_STRESS#OUT_SENT_SYNTACTICAL_TEXT#OUT_SENT_SYNTACTICAL_ALL#OUT_SENT_SYNTACTICAL_PRO#OUT_SENT_SYNTACTICAL_FLU#OUT_SENT_SYNTACTICAL_INT#OUT_SENT_KEYWORDS_TEXT#OUT_SENT_KEYWORDS_ALL#OUT_SENT_KEYWORDS_SCORE#OUT_SENT_KEYWORDS_INT#OUT_SENT_KEYWORDS_FLU";
    }
    else if([mode isEqualToString:@"B"])
    {
        return @"B,1,OUT_SENT_REF_TEXT#OUT_SENT_ASR_TEXT#OUT_SENT_TIMESTAMP#OUT_SENT_SCORE#OUT_WORD_TEXT#OUT_WORD_CLASS#OUT_WORD_TIMESTAMP#OUT_WORD_SCORE#OUT_WORD_VOLUME#OUT_PHONE_TEXT#OUT_PHONE_TIMESTAMP#OUT_PHONE_VOLUME#OUT_SENT_FLUENCY#OUT_SENT_INTEGRITY#OUT_SENT_PRONUNCIATION#OUT_WORD_STRESS#OUT_SENT_SYNTACTICAL_TEXT#OUT_SENT_SYNTACTICAL_ALL#OUT_SENT_SYNTACTICAL_PRO#OUT_SENT_SYNTACTICAL_FLU#OUT_SENT_SYNTACTICAL_INT#OUT_SENT_KEYWORDS_TEXT#OUT_SENT_KEYWORDS_ALL#OUT_SENT_KEYWORDS_SCORE#OUT_SENT_KEYWORDS_INT#OUT_SENT_KEYWORDS_FLU";
    }
    else if([mode isEqualToString:@"E"])
    {
        return @"E,1,OUT_SENT_REF_TEXT#OUT_SENT_ASR_TEXT#OUT_SENT_TIMESTAMP#OUT_SENT_SCORE#OUT_WORD_TEXT#OUT_WORD_CLASS#OUT_WORD_TIMESTAMP#OUT_WORD_SCORE#OUT_WORD_VOLUME#OUT_PHONE_TEXT#OUT_PHONE_TIMESTAMP#OUT_PHONE_VOLUME#OUT_SENT_FLUENCY#OUT_SENT_INTEGRITY#OUT_SENT_PRONUNCIATION#OUT_WORD_STRESS#OUT_SENT_SYNTACTICAL_TEXT#OUT_SENT_SYNTACTICAL_ALL#OUT_SENT_SYNTACTICAL_PRO#OUT_SENT_SYNTACTICAL_FLU#OUT_SENT_SYNTACTICAL_INT#OUT_SENT_KEYWORDS_TEXT#OUT_SENT_KEYWORDS_ALL#OUT_SENT_KEYWORDS_SCORE#OUT_SENT_KEYWORDS_INT#OUT_SENT_KEYWORDS_FLU";
    }
    else if([mode isEqualToString:@"C"])
    {
        return @"C,1,OUT_SENT_REF_TEXT#OUT_SENT_ASR_TEXT#OUT_SENT_TIMESTAMP#OUT_SENT_SCORE#OUT_WORD_TEXT#OUT_WORD_CLASS#OUT_WORD_TIMESTAMP#OUT_WORD_SCORE#OUT_WORD_VOLUME#OUT_SENT_FLUENCY#OUT_SENT_INTEGRITY#OUT_SENT_PRONUNCIATION#OUT_WORD_STRESS#OUT_SENT_SYNTACTICAL_TEXT#OUT_SENT_SYNTACTICAL_ALL#OUT_SENT_SYNTACTICAL_PRO#OUT_SENT_SYNTACTICAL_FLU#OUT_SENT_SYNTACTICAL_INT#OUT_SENT_KEYWORDS_TEXT#OUT_SENT_KEYWORDS_ALL#OUT_SENT_KEYWORDS_SCORE#OUT_SENT_KEYWORDS_INT#OUT_SENT_KEYWORDS_FLU";
    }
    else if([mode isEqualToString:@"D"])
    {
        return @"D,0,OUT_SENT_REF_TEXT#OUT_SENT_ASR_TEXT#OUT_SENT_TIMESTAMP#OUT_SENT_SCORE#OUT_WORD_TEXT#OUT_WORD_CLASS#OUT_WORD_TIMESTAMP#OUT_WORD_SCORE#OUT_WORD_VOLUME#OUT_PHONE_TEXT#OUT_PHONE_TIMESTAMP#OUT_PHONE_VOLUME#OUT_SENT_FLUENCY#OUT_SENT_INTEGRITY#OUT_SENT_PRONUNCIATION#OUT_WORD_STRESS#OUT_SENT_SYNTACTICAL_TEXT#OUT_SENT_SYNTACTICAL_ALL#OUT_SENT_SYNTACTICAL_PRO#OUT_SENT_SYNTACTICAL_FLU#OUT_SENT_SYNTACTICAL_INT#OUT_SENT_KEYWORDS_TEXT#OUT_SENT_KEYWORDS_ALL#OUT_SENT_KEYWORDS_SCORE#OUT_SENT_KEYWORDS_INT#OUT_SENT_KEYWORDS_FLU";
    }
    else if([mode isEqualToString:@"gzedunet"] || [mode isEqualToString:@"G"])
    {
        return @"G,1,OUT_SENT_REF_TEXT#OUT_SENT_ASR_TEXT#OUT_SENT_TIMESTAMP#OUT_SENT_SCORE#OUT_WORD_TEXT#OUT_WORD_CLASS#OUT_WORD_TIMESTAMP#OUT_WORD_SCORE#OUT_WORD_VOLUME#OUT_PHONE_TEXT#OUT_PHONE_TIMESTAMP#OUT_PHONE_SCORE#OUT_PHONE_VOLUME#OUT_SENT_FLUENCY#OUT_SENT_INTEGRITY#OUT_SENT_PRONUNCIATION#OUT_WORD_STRESS#OUT_SENT_SYNTACTICAL_TEXT#OUT_SENT_SYNTACTICAL_ALL#OUT_SENT_SYNTACTICAL_PRO#OUT_SENT_SYNTACTICAL_FLU#OUT_SENT_SYNTACTICAL_INT#OUT_SENT_KEYWORDS_TEXT#OUT_SENT_KEYWORDS_ALL#OUT_SENT_KEYWORDS_SCORE#OUT_SENT_KEYWORDS_INT#OUT_SENT_KEYWORDS_FLU";
    }
    else if([mode isEqualToString:@"gzedunet_answer"] || [mode isEqualToString:@"H"])
    {
        return @"H,2,OUT_SENT_REF_TEXT#OUT_SENT_ASR_TEXT#OUT_SENT_TIMESTAMP#OUT_SENT_SCORE#OUT_WORD_TEXT#OUT_WORD_CLASS#OUT_WORD_TIMESTAMP#OUT_WORD_SCORE#OUT_WORD_VOLUME#OUT_PHONE_TEXT#OUT_PHONE_TIMESTAMP#OUT_PHONE_SCORE#OUT_PHONE_VOLUME#OUT_SENT_FLUENCY#OUT_SENT_INTEGRITY#OUT_SENT_PRONUNCIATION#OUT_WORD_STRESS";
    }
    else
    {
        //default "A"
        return @"A,0,OUT_SENT_REF_TEXT#OUT_SENT_ASR_TEXT#OUT_SENT_TIMESTAMP#OUT_SENT_SCORE#OUT_WORD_TEXT#OUT_WORD_CLASS#OUT_WORD_TIMESTAMP#OUT_WORD_SCORE#OUT_WORD_VOLUME#OUT_SENT_FLUENCY#OUT_SENT_INTEGRITY#OUT_SENT_PRONUNCIATION#OUT_WORD_STRESS#OUT_SENT_SYNTACTICAL_TEXT#OUT_SENT_SYNTACTICAL_ALL#OUT_SENT_SYNTACTICAL_PRO#OUT_SENT_SYNTACTICAL_FLU#OUT_SENT_SYNTACTICAL_INT#OUT_SENT_KEYWORDS_TEXT#OUT_SENT_KEYWORDS_ALL#OUT_SENT_KEYWORDS_SCORE#OUT_SENT_KEYWORDS_INT#OUT_SENT_KEYWORDS_FLU";
    }
}

#pragma mark
#pragma mark - Load Final Callback

-(void)callbackReslut:(USCResult *)result
{
    USCLog(@"Manager -> callbackReslut | isMainThread : %d",[NSThread isMainThread]);

    if (result.content != NULL && result.content.length > 0)
    {
        if ([NSThread isMainThread])
        {
            if (_delegate && [_delegate respondsToSelector:@selector(recognizerDidGetResult:)])
            {
                USCLog(@"Manager -> callbackReslut mainThread");
                
                [_delegate recognizerDidGetResult:result.content];
            }
        }
        else
        {
            dispatch_async(dispatch_get_main_queue(), ^{
                if (_delegate && [_delegate respondsToSelector:@selector(recognizerDidGetResult:)])
                {
                    USCLog(@"Manager -> callbackReslut jump mainThread");
                    
                    [_delegate recognizerDidGetResult:result.content];
                }
            });
        }
    }
}

-(void)callbackStop:(ErrorType )error
{
    USCLog(@"Manager -> callbackStop | isMainThread : %d | error : %d",[NSThread isMainThread],error);

    //先结束本次识别,再处理回调
    [self cancel];
    
    if ([NSThread isMainThread])
    {
        if (_delegate && [_delegate respondsToSelector:@selector(recognizerDidStop:)])
        {
            [_delegate recognizerDidStop:error];
        }
    }
    else
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            
            if (_delegate && [_delegate respondsToSelector:@selector(recognizerDidStop:)])
            {
                [_delegate recognizerDidStop:error];
            }
        });
    }
}

-(void)loadFinalResult
{
    USCLog(@"Manager -> loadFinalResult | isRecognizerCancelled : %d",isRecognizerCancelled);
    USCLog(@"Manager -> loadFinalResult | isResultCallback : %d",isResultCallback);
    USCLog(@"Manager -> loadFinalResult | _onlineResult.resultError : %d",_onlineResult.resultError);
    USCLog(@"Manager -> loadFinalResult | _offlineResult.resultError : %d",_offlineResult.resultError);
    USCLog(@"Manager -> loadFinalResult | _onlineResult.content : %@",_onlineResult.content);
    USCLog(@"Manager -> loadFinalResult | _offlineResult.content : %@",_offlineResult.content);
    
    if (isResultCallback)
    {
        return ;
    }
    
    if (!isRecognizerCancelled)
    {
        /*
         -纯在线模式-
         调用该方法则直接返回在线结果
         
         -混合模式-
         1.如果在线先执行且结果正常，则直接返回在线
         2.如果离线先执行，则等待在线结果
         3.如果离线先执行，且发现在线识别出错，则直接取离线结果
         */
        
        if(recognitionMode == ONLINE_ONLY)
        {
            if (_onlineResult.content != NULL
                && _onlineResult.content.length > 0)
            {
                USCLog(@"Manager OnlineOnly-> onLineResult");

                [self callbackReslut:_onlineResult];
                
                isResultCallback = YES;
            }
        }
        else if (recognitionMode == MIX)
        {
            if (_onlineResult.content != NULL
                && _onlineResult.content.length > 0)
            {
                USCLog(@"Manager Mix -> onLineResult");

                [self callbackReslut:_onlineResult];
                
                isResultCallback = YES;
            }
            else
            {
                if (_offlineResult.content != NULL
                    && _offlineResult.content.length > 0)
                {
                    if(_onlineResult.resultError != No_Error)
                    {
                        USCLog(@"Manager Mix -> onOfflineResult");
                        
                        [self callbackReslut:_offlineResult];
                        
                        isResultCallback = YES;
                    }
                }
            }
        }
    }
}

-(void)loadFinalStop
{
    USCLog(@"Manager -> loadFinalEnd | isRecognizerStopped : %d",isRecognizerStopped);
    USCLog(@"Manager -> loadFinalEnd | isRecognizerCancelled : %d",isRecognizerCancelled);
    USCLog(@"Manager -> loadFinalEnd | isResultCallback : %d",isResultCallback);
    
    if (isRecognizerStopped)
    {
        return ;
    }
    
    //先执行result再执行end回调
    if (!isRecognizerCancelled && isResultCallback)
    {
        USCLog(@"Manager -> onStop success");
        
        //取消超时定时器
        [self cancelTimer];
        
        isRecognizerStopped = YES;

        [self callbackStop:No_Error];
    }
}

-(void)loadFinalError
{
    BOOL isFinished = [self recognitionFinished];
    USCLog(@"Manager -> loadFinalError | isFinished : %d",isFinished);
    if (!isFinished)
    {
        USCLog(@"loadFinalError return");
        return ;
    }
    
    if (isRecognizerCancelled)
    {
        return ;
    }
    
    USCLog(@"Manager -> loadFinalError | onlineError = %d", _onlineResult.resultError);
    USCLog(@"Manager -> loadFinalError | offlineError = %d", _offlineResult.resultError);
    
    //混合模式中，先判断离线是否出错，再判断在线是否出错(若同时出错，输出离线错误)
    if (recognitionMode == MIX)
    {
        //在线出错，离线有结果，则返回离线结果，否则等待离线结束
        //离线出错，在线有结果，则返回在线结果，否则等待在线结束
        //在线、离线都出错，取离线错误
        
        if(_onlineResult.resultError != No_Error)
        {
            if (_offlineResult.resultError != No_Error)
            {
                //取消本次识别
                [self cancel];
                
                [self callbackStop:_offlineResult.resultError];
            }
            else if(_offlineResult.content != NULL && _offlineResult.content.length > 0)
            {
                //给出离线结果
                [self loadFinalResult];
                
                [self loadFinalStop];
            }
        }
        else if(_offlineResult.resultError != No_Error)
        {
            if (_onlineResult.content != NULL && _onlineResult.content.length > 0)
            {
                //给出在线结果
                [self loadFinalResult];
                
                [self loadFinalStop];
            }
        }
    }
    else if (recognitionMode == ONLINE_ONLY)
    {
        if(_onlineResult.resultError != No_Error)
        {
            //取消本次识别
            [self cancel];
            
            [self callbackStop:_onlineResult.resultError];
        }
    }
}

//在、离线识别完成度
-(BOOL)recognitionFinished
{
    if (recognitionMode == ONLINE_ONLY)
    {
        USCLog(@"recognitionFinished | online : %d",_recognition.getFinished);

        //由于recognition已启用getFinished属性，所以此处直接返回YES
        if (_recognition != NULL && _recognition.getFinished)
        {
            return YES;
        }
        else
        {
            return NO;
        }
    }
    else if (recognitionMode == MIX)
    {
        USCLog(@"recognitionFinished | online : %d | offline : %d ",_recognition.getFinished, _recognitionOffline.getFinished);

        if (_recognition != NULL && _recognitionOffline != NULL
            && _recognition.getFinished && _recognitionOffline.getFinished)
        {
            return YES;
        }
        else
        {
            return NO;
        }
    }
    else
    {
        return YES;
    }
}

#pragma mark -
#pragma mark - Online

-(NSString *)getAudioFileUrl:(NSString *)sessionId
{
    if (sessionId != NULL && sessionId.length > 0)
    {
        NSArray *sessionAry = [sessionId componentsSeparatedByString:@":"];
        if (sessionAry!=NULL&&sessionAry.count==3)
        {
            NSString *url = [NSString stringWithFormat:@"http://%s:%hd/WebAudio-1.0-SNAPSHOT/audio/play/%@/%@/%@",
                             asrAudioFileServer_domain,
                             asrAudioFileServer_port,
                             sessionAry[2],
                             sessionAry[1],
                             sessionAry[0]];
            return url;
        }
        else
        {
            USCLog(@"recorder -> getAudioFileUrl : no url");
            return NULL;
        }
    }
    else
    {
        USCLog(@"recorder -> getAudioFileUrl : no url");
        return NULL;
    }
}

-(NSString *)getRecogResultUrl:(NSString *)sessionId
{
    if (sessionId != NULL && sessionId.length > 0)
    {
        NSArray *sessionAry = [sessionId componentsSeparatedByString:@":"];
        if (sessionAry!=NULL&&sessionAry.count==3)
        {
            NSString *url = [NSString stringWithFormat:@"http://%s:%hd/WebAudio-1.0-SNAPSHOT/result/%@/%@/%@",
                             asrAudioFileServer_domain,
                             asrAudioFileServer_port,
                             sessionAry[2],
                             sessionAry[1],
                             sessionAry[0]];
            return url;
        }
        else
        {
            USCLog(@"recorder -> getRecogResultUrl : no url");
            return NULL;
        }
    }
    else
    {
        USCLog(@"recorder -> getRecogResultUrl : no url");
        return NULL;
    }
}

#pragma mark - USCRecognitionDelegate CallBack

- (void)onRecognitionStart
{
    USCLog(@"Manager -> onRecognitionStart");
}

// 部分结果回调
- (void)onRecognitionResult:(NSString *)result isLast:(BOOL)isLast
{
    USCLog(@"Manager -> onRecognitionResult | isLast : %d",isLast);
    if(result.length > 0)
    {
        [_onlineResult setReslut:result];
        _onlineResult.isLast = isLast;
    }
    
    [_recognition setFinished:YES];
    
    [self performSelector:@selector(loadFinalResult) withObject:nil];
}

// 识别停止通知事件
- (void)onRecognitionStop
{
    USCLog(@"Manager -> onRecognitionStop");
    
    [self loadFinalStop];
}

// 识别服务器发生了错误
- (void)onRecognitionError:(ErrorType)error
{
    USCLog(@"Manager -> onRecognitionError : %d",error);
    
    _onlineResult.resultError = error;
    
    [_recognition setFinished:YES];
    
    [self loadFinalError];
} 

// 说话超过一分钟，超时
- (void)onMaxSpeechTimeout
{
    USCLog(@"Manager -> onMaxSpeechTimeout");
    //[self cancel];
}

- (void)onSessionId:(NSString *)sessionId
{
    USCLog(@"Manager -> onSessionId : %@",sessionId);
    
    NSString *url = [self getAudioFileUrl:sessionId];
    if(url != NULL && url.length > 0)
    {
        if (_delegate && [_delegate respondsToSelector:@selector(audioFileDidRecord:)])
        {
            USCLog(@"Manager -> onSessionId = %@", url);
            [_delegate audioFileDidRecord:url];
        }
    }
    
    BOOL asyncRecognize = [[USCPreference sharePreference]getAsyncRecognize];
    if (asyncRecognize)
    {
        NSString *resultUrl = [self getRecogResultUrl:sessionId];
        if(resultUrl != NULL && resultUrl.length > 0)
        {
            if (_delegate && [_delegate respondsToSelector:@selector(recognizerDidGetResultUrl:)])
            {
                USCLog(@"Manager -> onSessionId | recognizeUrl = %@", resultUrl);
                [_delegate recognizerDidGetResultUrl:resultUrl];
            }
        }
    }
}

#pragma mark -
#pragma mark - Offline

-(ErrorType)initEngine_offline
{
    USCLog(@"initEngine");
    
    eduOfflineEngine = [[USCOralEduWrapper alloc]init];
    
    int initCode = [eduOfflineEngine initEngine:_path];
    
    ErrorType error = No_Error;
    if (initCode != OK_Engine)
    {
        error = [USCErrorCode offlineErrorTransform:initCode];
    }
    return error;
}

- (void)startOfflineRecognition
{
    USCLog(@"Manager -> startOfflineRecognition");
    
    // 先释放上一次分配的资源
    if (_recognitionOffline)
        
    {
        _recognitionOffline = nil;
    }

    // 分配新的识别线程
    
    _recognitionOffline = [[USCRecognition_offline alloc] init];
    _recognitionOffline.queuePriority = NSOperationQueuePriorityLow; //将离线识别线程的优先级调低
    _recognitionOffline.appKey = _appKey;
    _recognitionOffline.delegate = self;
    _recognitionOffline.sampleRate = _sampleRate;
    _recognitionOffline.oralText = _oralText;
    _recognitionOffline.oralTask = _oralTask;
    [_recognitionOffline setFinished:NO];
    [_recognitionOffline setEngine:eduOfflineEngine];
    
    USCLog(@"Manager -> _oralText : %@",_oralText);
    
    [_operationQueue addOperation:_recognitionOffline];
    _operationQueue.suspended = NO;
}

#pragma mark USCOfflineRecognitionDelegate CallBack

- (void)onOfflineRecognitionStart
{
    USCLog(@"Manager -> onOfflineRecognitionStart");
}

- (void)onOfflineRecognitionResult:(NSString *)result isLast:(BOOL)isLast
{
    USCLog(@"Manager -> onOfflineRecognitionResult | isLast : %d",isLast);
    if(result.length > 0)
    {
        [_offlineResult setReslut:result];
        _offlineResult.isLast = isLast;
    }
    
    [_recognitionOffline setFinished:YES];
    
//    //启动识别超时定时器
//    [self startTimer];
//    
//    [self performSelector:@selector(loadFinalResult) withObject:nil];
    
    if ([NSThread isMainThread])
    {
        //启动识别超时定时器
        [self startTimer];
        [self performSelector:@selector(loadFinalResult) withObject:nil];
    }
    else
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            
            USCLog(@"Manager -> onOfflineRecognitionResult jump mainThread");
            
            //启动识别超时定时器
            [self startTimer];
            [self performSelector:@selector(loadFinalResult) withObject:nil];
        });
    }    
}

- (void)onOfflineRecognitionStop
{
    USCLog(@"Manager -> onOfflineRecognitionStop");
    
    //[self loadFinalStop];
    
    if ([NSThread isMainThread])
    {
        [self performSelector:@selector(loadFinalStop) withObject:nil];
    }
    else
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            
            USCLog(@"Manager -> onOfflineRecognitionStop jump mainThread");
            [self performSelector:@selector(loadFinalStop) withObject:nil];
        });
    }
}

- (void)onOfflineRecognitionError:(ErrorType)error
{
    USCLog(@"Manager -> onOfflineRecognitionError : %d",error);
    
    _offlineResult.resultError = error;
    
    [_recognitionOffline setFinished:YES];
    
    //[self loadFinalError];
    
    if ([NSThread isMainThread])
    {
        [self performSelector:@selector(loadFinalError) withObject:nil];
    }
    else
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            
            USCLog(@"Manager -> onOfflineRecognitionError jump mainThread");
            [self performSelector:@selector(loadFinalError) withObject:nil];
        });
    }
}

- (void)onOfflineMaxSpeechTimeout
{
    USCLog(@"Manager -> onOfflineMaxSpeechTimeout");
}

#pragma mark - Release

- (void)dealloc
{
    USCLog(@"Manager -> dealloc");
    
    [self destory];
}

-(void)destory
{
    USCLog(@"destory in");
    
    USCLog(@"destory in delegate");
    
    if(_delegate != nil)
    {
        _delegate = nil;
    }
    
    USCLog(@"destory in _recognition");
    //识别线程
    [self recognitionRelease];
    
    //离线识别引擎
    if(eduOfflineEngine!=nil)
    {
        [eduOfflineEngine engineRelease];
    }
    
    USCLog(@"destory in _operationQueue");
    
    // 线程队列
    if (_operationQueue!=nil)
    {
        [_operationQueue cancelAllOperations];
    }
    
    USCLog(@"--- destory finish ---");
}

@end

