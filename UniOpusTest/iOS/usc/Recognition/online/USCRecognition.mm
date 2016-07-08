//
//  USCRecognition.m
//  usc
//
//  Created by hejinlai on 12-11-9.
//  Copyright (c) 2012年 yunzhisheng. All rights reserved.
//

#import "USCRecognition.h"
#import "USCMarcos.h"
#import "UniPCMRecorder.h"
#import "UniBackupInfo.h"

#import "Settings.h"

@interface USCRecognition()
{
    int readTimeout;
    BOOL opusFinish;//编码是否完成，必须等到完成后才可以停止私有协议识别或者启动HTTP识别
    
    UniPCMRecorder *testRecorder;
}

@end

static  NSMutableData *resultData = [NSMutableData dataWithCapacity:1];
static  NSMutableData *startData = [NSMutableData dataWithCapacity:1];

@implementation USCRecognition

@synthesize recordingQueue = _recordingQueue;
@synthesize delegate = _delegate;
@synthesize appKey = _appKey;
@synthesize oralText = _oralText;
@synthesize oralTask = _oralTask;
@synthesize identifier = _identifier;

- (id)init
{
    if (self = [super init])
    {
        _recordingQueue = [[NSMutableArray alloc]init];
        httpData = [[NSMutableData alloc]init];
        
        preference = [USCPreference sharePreference];
        
        //testRecorder = [UniPCMRecorder defaultRecorder];
        NSString *name = [NSString stringWithFormat:@"%d",(int)[NSDate timeIntervalSinceReferenceDate]];
        [testRecorder start:name];
        testRecorder.totalLength = 0;
        
        opusFinish = NO;
        _backupEnable = YES;
        
        USCLog(@"recognition -> init");
    }
    return self;
}

-(void)setSocket:(USCAsyncSocket *)socket
{
    mSocket = socket;
}

-(void)setOralTask:(NSString *)oralTask
{
    _oralTask = oralTask;
}

-(void)setOralText:(NSString *)oralText
{
    _oralText = oralText;
    
    //每60个字符的超时时间是3s (不满60个为3s)
    int length = (int)_oralText.length;
    int timeout = length <= 60 ? 3 : length / 60 * 3;
    
    readTimeout = timeout;
    
    USCLog(@"Recognition -> setGetResultTimeout : %d",timeout);
}
//在调用startOpus之后，调用该函数进行编码
- (void)appendAudioData:(NSData *)audioData
{
    [uniOpus appendAudioData:audioData];
}

-(void)start
{
    //启动opus编码
    [self startOpus];
    
    NSString *name = [NSString stringWithFormat:@"%d",(int)[NSDate timeIntervalSinceReferenceDate]];
    [testRecorder start:name];
    
    if(_backupEnable)
    {
        //检测备份超时并更新（在启动识别前进行）
        [self updateBackupCondition];
        
        //继承从上一次可用的评测方式
        BackupType type = [preference getBackupType];
        USCLog(@"recognition -> start | BackupTpe : %lu",(unsigned long)type);
        if (type == Backup_PrivateDomain || type == Backup_PrivateIP)
        {
            [self startSocketRecognition];
        }
    }
    else
    {
        [self startSocketRecognition];
    }
}

-(void)stop
{
    USCLog(@"recognition -> stop");
    [testRecorder finish];

    [self stopOpus];
}

-(void)cancel
{
    USCLog(@"recognition -> cancel");
    [testRecorder finish];
    
    [self cancelOpus];
    
    if(socketRecognition)
    {
        [socketRecognition cancel];
    }
    
    if(httpRecog)
    {
        [httpRecog cancelRecognition];
    }
}

-(void)finishRecognition
{
    if (opusFinish)
    {
        BackupType type = [preference getBackupType];
        USCLog(@"recognition -> stop | BackupTpe : %lu",(unsigned long)type);
        
        if (type == Backup_PrivateDomain || type == Backup_PrivateIP)
        {
            [socketRecognition stop];
        }
        else
        {
            [self startHttpRecognition];
        }
    }
}

#pragma mark - IPBackup

-(void)updateBackupCondition
{
    //如果最后一次是域名，则不处在备份中，将布尔值设为NO
    //如果最后一次是私有IP或者HTTP，则计算超时时间,若超时，则重置备份状态
    BackupType currentType = [preference getBackupType];
    if (currentType == Backup_PrivateDomain)
    {
        [self backupToPreference:Backup_PrivateDomain];
    }
    else
    {
        if ([self isBackupTimeout])
        {
            [preference setBackupType:Backup_PrivateDomain];
            [preference setCurrentPrivateIp:oral_domain];
            [preference setCurrentPrivatePort:oral_port];
            [preference setCurrentHTTPIP:nil];
            
            [self backupToPreference:Backup_PrivateDomain];
        }
    }
}

-(void)backupToPreference:(BackupType)type
{
    
    //如果已处在备份状态（Backup_PrivateIP || Backup_HTTP），则不再刷新
    UniBackupInfo * backupInfo = [preference getBackupInfo];
    if (backupInfo != nil)
    {
        NSNumber *backup = backupInfo.isBackup;
        if ([backup boolValue] && (type == Backup_PrivateIP||type == Backup_HTTP))
        {
            return ;
        }
    }
    
    NSNumber *isBackup;
    if (type == Backup_PrivateDomain)
    {
        isBackup = [NSNumber numberWithBool:NO];
    }
    else
    {
        isBackup = [NSNumber numberWithBool:YES];
    }
    UniBackupInfo *info = [UniBackupInfo new];
    info.isBackup = isBackup;
    info.updateTime = [NSDate date];

    [preference setBackupInfo:info];
    USCLog(@"recogniton -> backupToPreference");
}

-(BOOL)isBackupTimeout
{
    BOOL isTimeout = NO;

    UniBackupInfo *info = [preference getBackupInfo];
    NSNumber *isBackup = info.isBackup;
    NSDate *updateTime = info.updateTime;
    if (isBackup != nil && updateTime != nil)
    {
        if ([isBackup boolValue])
        {
            NSTimeInterval time = [[NSDate date] timeIntervalSinceDate:updateTime];
            if (time > BACKUP_TIMEOUT)
            {
                isTimeout = YES;
                USCLog(@"recogniton -> backupTimeout");
            }
        }
    }
    
    return isTimeout;
}

- (void)dealloc
{
    USCLog(@"recognition -> dealloc");
    
    _recordingQueue = nil;
    _delegate = nil;
    _appKey = nil;
    _oralText = nil;
    _oralTask = nil;
    _identifier = nil;
    
    [httpData resetBytesInRange:NSMakeRange(0, httpData.length)];
    httpData = nil;
    
    //socketRecognition = nil;
    httpRecog = nil;
    
    uniOpus = nil;
}

#pragma mark - CallBack

- (void)doRecognitionStart
{
    if (_delegate != nil)
    {
        [_delegate onRecognitionStart];
    }
}

- (void)doRecognitionResult:(NSString *)result isLast:(BOOL)isLast
{
    if (_delegate != nil)
    {
        [_delegate onRecognitionResult:result isLast:isLast];
    }
}

- (void)doRecognitionStop
{
    if (_delegate != nil)
    {
        [_delegate onRecognitionStop];
    }
}

- (void)doRecognitionError:(ErrorType)error;
{
    if (_delegate != nil)
    {
        [_delegate onRecognitionError:error];
    }
}

- (void)doMaxSpeechTimeout;
{
    if (_delegate != nil)
    {
        [_delegate onMaxSpeechTimeout];
    }
}

- (void)doRecognitionSessionID:(NSString *)sessionId
{
    if (_delegate != nil)
    {
        [_delegate onSessionId:sessionId];
    }
}


#pragma mark - UniOpus

-(void)startOpus
{
    //启动opus编码模块
    if (uniOpus)
    {
        uniOpus = nil;
    }
    uniOpus = [[UnisoundOpus alloc]init];
    uniOpus.delegate = self;
}

-(void)stopOpus
{
    if (uniOpus)
    {
        [uniOpus stopEncode];
    }
}

-(void)cancelOpus
{
    if (uniOpus)
    {
        [uniOpus cancelEncode];
    }
}

#pragma mark - UniOpusDelegate

-(void)opusDataDidEncode:(NSData *)encodeData
{
    [testRecorder appendData:encodeData];

    //backup for recognition
    if (socketRecognition)
    {
        [socketRecognition appendData:encodeData];
    }
    
    //backup for HttpFront
    [httpData appendData:encodeData];
}

-(void)opusDataDidFinished
{
    USCLog(@"recognition -> opusDataDidFinished");
    
    opusFinish = YES;
    
    [self finishRecognition];
}

#pragma mark -
#pragma mark SocketRecognition

-(void)startSocketRecognition
{
    USCLog(@"recognition - > startSocketRecognition");
    
    // 分配新的识别线程
    socketRecognition = [[UniSocketRecognition alloc] init];
    socketRecognition.delegate = self;
    socketRecognition.appKey = _appKey;
    socketRecognition.oralText = _oralText;
    socketRecognition.oralTask = _oralTask;
    socketRecognition.identifier = _identifier;
    socketRecognition.backupEnable = _backupEnable;
    [socketRecognition setSocket:mSocket];
    
    [socketRecognition start];
}

#pragma mark UniSocketDelegate

-(void)uniSocketRecognitonDidStart
{
    [self doRecognitionStart];
}

-(void)uniSocketRecognitonDidStop
{
    if (_backupEnable)
    {
        //刷新备份超时时间
        [self backupToPreference:[preference getBackupType]];
    }
    
    NSString *result = [socketRecognition getResult];
    NSString *sessionID = [socketRecognition getSessionID];
    
    [self doRecognitionResult:result isLast:YES];
    [self doRecognitionSessionID:sessionID];
    [self doRecognitionStop];
}

-(void)uniSocketRecognitonDidFail:(ErrorType)type
{
    USCLog(@"uniSocketRecognitonDidFail : %d",type);
    
    if (_backupEnable)
    {
        //文本错误不启用备份
        if (![USCErrorCode isTextError:type])
        {
            //在私有协议失败后启动http评测
            [preference setBackupType:Backup_HTTP];
            //刷新备份超时时间
            [self backupToPreference:[preference getBackupType]];
            
            [self finishRecognition];
        }
        else
        {
            [self doRecognitionError:type];
        }
    }
    else
    {
        [self doRecognitionError:type];
    }
}

#pragma mark -
#pragma mark HTTPRecognition

-(void)startHttpRecognition
{
    USCLog(@"Recognition -> httpClientStart");
    
    if (httpRecog == nil)
    {
        httpRecog = [[USCHTTPRecognition alloc]init];
        httpRecog.delegate = self;
        httpRecog.oralTask = _oralTask;
        httpRecog.appKey = _appKey;
        httpRecog.oralText = _oralText;
        httpRecog.audioData = httpData;
        httpRecog.identifier = _identifier;
        httpRecog.requestTimeout = readTimeout;
        [httpRecog startRecognition];
    }
}

-(void)checkoutOpusEncode
{
    
}

#pragma mark HTTPRecognitionDelegate

-(void)recognitionSessionIdDidGet:(NSString *)senID
{
    USCLog(@"recognitionSessionIdDidGet : %@",senID);
    BOOL asyncRecognize = [[USCPreference sharePreference]getAsyncRecognize];
    if (!asyncRecognize) {
        [self doRecognitionSessionID:senID];
    }
}

-(void)recognitionResultDidGet:(NSString *)result
{
    USCLog(@"recognitionResultDidGet");
    
    //刷新备份超时时间
    [self backupToPreference:[preference getBackupType]];
    
    [self doRecognitionResult:result isLast:YES];
    
    [self doRecognitionStop];
}

-(void)recognitionConnectDidFail:(ErrorType)error
{
    USCLog(@"recognitionConnectDidFail : %d",error);
    
    //刷新备份超时时间
    [self backupToPreference:[preference getBackupType]];
    
    [self doRecognitionError:error];
}

@end
