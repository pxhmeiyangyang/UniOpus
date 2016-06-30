//
//  UniSocketRecognition.m
//  usc
//
//  Created by 刘俊 on 15/11/2.
//  Copyright © 2015年 yunzhisheng. All rights reserved.
//

#import "UniSocketRecognition.h"
#import "USCMarcos.h"

#import "USCRequest.h"
#import "USCAttribute.h"
#import "USCResponse.h"

#import "Settings.h"

#import "UniPCMRecorder.h"

@interface UniSocketRecognition()
{
    int getResultTimeout;
    
    NSString *currentPrivateIp;
    int currentPrivatePort;
    
    UniPCMRecorder *testRecorder; //音频测试用
    
    BOOL isCanceled;
}

@end

static  NSMutableData *resultData = [NSMutableData dataWithCapacity:1];
static  NSMutableData *startData = [NSMutableData dataWithCapacity:1];

@implementation UniSocketRecognition

@synthesize appKey = _appKey;
@synthesize oralText = _oralText;
@synthesize delegate = _delegate;

- (id)init
{
    if (self = [super init])
    {
        [self reset];
        
//        dispatch_queue_attr_t highPriorityAttr = dispatch_queue_attr_make_with_qos_class (DISPATCH_QUEUE_SERIAL, QOS_CLASS_USER_INITIATED,-1);
//        socketQueue = dispatch_queue_create("cn.yunzhisheng.socketRecog", highPriorityAttr);
//        socketQueue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0);
        
        socketQueue = dispatch_queue_create("cn.yunzhisheng.socketRecog", 0);
        
        opusDataArray = [NSMutableArray array];
        dataIndex = 0;
        totalLength = NSIntegerMax;
        
        //testRecorder = [UniPCMRecorder defaultRecorder];
        NSString *name = [NSString stringWithFormat:@"%d",(int)[NSDate timeIntervalSinceReferenceDate]];
        [testRecorder start:name];
        testRecorder.totalLength = 0;
        
        USCLog(@"recognition -> init");
    }
    return self;
}

-(void)getBackupIP
{
    //随机选取私有协议备份IP
    NSRange ipRange = NSMakeRange(0, INT16_MAX);
    NSString *ipStr;
    if ([currentPrivateIp isEqualToString:oral_domain])
    {
        int index = arc4random()%3;
        ipStr = PirvateProtocal_IP[index];
    }
    else
    {
        while (ipRange.length != 0)
        {
            int index = arc4random()%3;
            ipStr = PirvateProtocal_IP[index];
            ipRange = [ipStr rangeOfString:currentPrivateIp];
        }
    }
    private_IPArray = @[ipStr];
    
    USCLog(@"getBackupIP | private_IPArray : %@",private_IPArray);
}

//第一次选取的ip出错后在选一次ip
-(void)updateIP
{
    NSRange ipRange = NSMakeRange(0, INT16_MAX);
    NSString *ipStr;
    while (ipRange.length != 0)
    {
        int index = arc4random()%3;
        ipStr = PirvateProtocal_IP[index];
        NSArray* strAry = [currentPrivateIp componentsSeparatedByString:@"."];
        ipRange = [ipStr rangeOfString:strAry[0]];
    }
    
    private_IPArray = @[ipStr];
}

-(void)setSocket:(USCAsyncSocket *)socket
{
    //mSocket = socket;
    mSocket = [[USCAsyncSocket alloc]init];
    [mSocket setDelegate:self delegateQueue:dispatch_get_main_queue()];
}

/**
 * 私有协议服务端错误码处理：只在socket回调函数socket:didReadData:withTag:中使用
 */
-(BOOL)privateErrorFromServer:(int)errorCode
{
    USCLog(@"recognition -> privateErrorFromServer : %d",errorCode);
    
    ErrorType error = [USCErrorCode onlineErrorTransform:errorCode];
    if (error == NO_Define)
    {
        [self setError:NetWork_Connect_Error];
        
        return NO;
    }
    else
    {
        [self setError:error];
        
        return YES;
    }
}

-(void)appendData:(NSData *)data
{
    [opusDataArray addObject:data];
}

+ (NSString*)stringWithUUID
{
    //create a new UUID
    CFUUIDRef    uuidObj = CFUUIDCreate(nil);
    //get the string representation of the UUID
    NSString *uuidString=  (__bridge_transfer NSString *)CFUUIDCreateString(nil, uuidObj);
    return uuidString;
}

-(void)start
{
    USCLog(@"recognition -> start | currentIp : %@ | port : %d",currentPrivateIp, currentPrivatePort);
    
    // 初始化，设置参数
    UIDevice *device = [UIDevice currentDevice];
    NSString *deviceInfo = [NSString stringWithFormat:@"iOS m:%@ n:%@ v:%@",device.model, device.systemName, device.systemVersion];
    NSString *vendor = device.identifierForVendor.UUIDString;
    if (_identifier == NULL || _identifier.length == 0)
    {
        _identifier =  vendor;
    }
    
    NSString *guid = [UniSocketRecognition stringWithUUID];
    
    USCAttribute *attribute1 = [[USCAttribute alloc]initWithKey:SSUP_USER_ID value:_identifier];
    USCAttribute *attribute2 = [[USCAttribute alloc]initWithKey:SSUP_AUDIO_ENC_METH value:@"opus"];
    USCAttribute *attribute3 = [[USCAttribute alloc]initWithKey:SSUP_COLLECTED_INFO value:deviceInfo];
    USCAttribute *attribute4 = [[USCAttribute alloc]initWithKey:SSUP_ORAL_TASK_TYPE value:_oralTask];
    USCAttribute *attribute5 = [[USCAttribute alloc]initWithKey:SSUP_IMEI_SET value:_identifier];
    USCAttribute *attribute6 = [[USCAttribute alloc]initWithKey:SSUP_AUTO_LONG_MODE value:@"1"];
    USCAttribute *attribute7 = [[USCAttribute alloc]initWithKey:SSUP_ORAL_EVAL_TEXT value:_oralText];
    USCAttribute *attribute8 = [[USCAttribute alloc]initWithKey:SSUP_SESSION_ID value:guid];
    USCAttribute *attribute9 = [[USCAttribute alloc]initWithKey:SSUP_APP_KEY value:_appKey];
    attriArray = nil;
    attriArray = @[attribute1,attribute2,attribute3,attribute4,attribute5,
                   attribute6,attribute7,attribute8,attribute9];
    
    if (_oralText != nil && _oralText.length > 0)
    {
        //每60个字符的超时时间是3s (不满60个为3s)
        int length = (int)_oralText.length;
        int timeout = length <= 60 ? 3 : length / 60 * 3;
        
        [self setResultTimeout:timeout];
        
        USCLog(@"Recognition -> setGetResultTimeout : %d",timeout);
    }
    
    NSError *error;
    BOOL result;
    result = [mSocket connectToHost:currentPrivateIp onPort:currentPrivatePort withTimeout:CONNECT_TIMEOUT error:&error];
    USCLog(@"Recognition -> connect : %d | error : %@", result, error);
    if (result == 1)
    {
        USCRequest *request = [[USCRequest alloc]init];
        [request setAttributeArray:attriArray];
        NSData *requestData = [request requestFormater:ORAL_REQ_START];
        USCLog(@"requestData length : %d",(int)[requestData length]);
        
        [mSocket writeData:requestData withTimeout:START_WRITE_TIMEOUT tag:SOC_START];
    }
    else
    {
        [self errorHandle:NetWork_Connect_Error];
    }
}

-(void)stop
{
    //获取音频的总长度即可确定resume结束的时机并启动stopConnecting
    totalLength = opusDataArray.count;
}

-(void)cancel
{
    isCanceled = YES;
    
    if ([mSocket isConnected])
    {
        [mSocket disconnect];
    }
}

-(void)stopConnecting
{
    dispatch_async(socketQueue, ^{
        
        BOOL isConnect = [mSocket isConnected];
        USCLog(@"recognition -> stop | isConnect : %d",isConnect);
        if (isConnect)
        {
            BOOL asyncRecognize = [[USCPreference sharePreference]getAsyncRecognize];

            USCRequest *request = [[USCRequest alloc]init];
            [request setAttributeArray:attriArray];
            
            NSData *requestData;
            if (asyncRecognize)
            {
                requestData = [request requestFormater:ORAL_REQ_STOP_ASYNC];
            }
            else
            {
                requestData = [request requestFormater:ORAL_REQ_STOP];
            }
            
            [mSocket writeData:requestData withTimeout:STOP_WRITE_TIMEOUT tag:SOC_STOP];
        }
        else
        {
            [self setError:NetWork_Connect_Error];
            
            USCLog(@"recognition -> stop | disconnect");
        }
    });
}

-(void)startResumeLoop
{
    USCLog(@"socketRecog -> resume start");
    
    dispatch_async(socketQueue, ^{
        
        while ([mSocket isConnected])
        {
            if (dataIndex < opusDataArray.count)
            {
                //NSData *encodeData = opusDataArray[dataIndex++];
                
                //将缓存的数据整合后一起上传（减少resume次数）
                NSData *encodeData = [self getCacheData];
                [self resume:encodeData];
            }
            else
            {
                if (dataIndex != totalLength)
                {
                    [NSThread sleepForTimeInterval:0.01];
                }
                else
                {
                    break;
                }
            }
        }
        
        [testRecorder finish];
        
        USCLog(@"socketRecog -> resume finish");
        
        //resume结束后stop
        [self stopConnecting];
    });
}

-(void)resume:(NSData *)pcmData
{
    BOOL isConnect = [mSocket isConnected];
    
    USCLog(@"recognition -> resume | isConnect : %d",isConnect);
    USCLog(@"recognition -> resume | pcmData : %lu",(unsigned long)pcmData.length);
    
    if (isConnect)
    {
        USCRequest *request = [[USCRequest alloc]init];
        
        [testRecorder appendData:pcmData];
        
        NSData *requestData = [request requestFormater_resume:pcmData];
        
        [mSocket writeData:requestData withTimeout:RESUME_WRITE_TIMEOUT tag:SOC_RESUME];
    }
    else
    {
        [self setError:NetWork_Connect_Error];
    }
}

-(void)reset
{
    preference = [USCPreference sharePreference];

    //重置数据池
    if (startData)
    {
        [startData resetBytesInRange:NSMakeRange(0, [resultData length])];
        [startData setLength:0];
    }
    if (resultData)
    {
        [resultData resetBytesInRange:NSMakeRange(0, [resultData length])];
        [resultData setLength:0];
    }
    
    getResultTimeout = STOP_READ_TIMEOUT;
    _backupEnable = YES;
    isCanceled = NO;
    
    currentPrivateIp = [preference getCurrentPrivateIp];
    currentPrivateIp = [preference getCurrentPrivateIp];
    currentPrivatePort = [preference getCurrentPrivatePort];
    
    recogError = No_Error;
    
    //获取缓存的IP及端口数据
    [self getBackupIP];
}

-(NSString *)getResult
{
    USCLog(@"recognition -> getResult : %@",resultStr);
    
    BOOL asyncRecognize = [[USCPreference sharePreference]getAsyncRecognize];
    if (asyncRecognize)
    {
        resultStr = ASYNC_RESULT;
    }
    
    return resultStr;
}

-(NSString *)getSessionID
{
    return sessionid;
}

-(void)errorHandle:(ErrorType)errorType
{
    if (_backupEnable)
    {
        //文本错误不启用备份
        if ([USCErrorCode isTextError:errorType])
        {
            [self setError:errorType];
            [self failCallBack];
        }
        else
        {
            BackupType backupType = [preference getBackupType];
            if (backupType == Backup_PrivateIP)
            {
                USCLog(@"socketRecognition -> error finish");
                [self setError:errorType];
                [self failCallBack];
            }
            else
            {
                USCLog(@"socketRecognition -> IPBackup");
                
                NSString *ipStr = private_IPArray[0];
                
                currentPrivateIp = ipStr;
                currentPrivatePort = 80;
                
                [preference setCurrentPrivateIp:currentPrivateIp];
                [preference setCurrentPrivatePort:currentPrivatePort];
                
                [preference setBackupType:Backup_PrivateIP];
                
                //重置数据指针
                dataIndex = 0;
                [self start];
            }
        }
    }
    else
    {
        [self setError:errorType];
        [self failCallBack];
    }
}

-(void)setError:(ErrorType)type
{
    USCLog(@"recognition -> setError : %d",type);
    recogError = type;
}

-(ErrorType)getError
{
    return recogError;
}

-(void)setResultTimeout:(int)time
{
    getResultTimeout = time <= 0 ? STOP_READ_TIMEOUT : time;
}

//将缓存的数据整合后一起上传（减少resume次数）
-(NSData *)getCacheData
{
    NSMutableData *cacheData = [NSMutableData data];
    NSInteger count = opusDataArray.count;
    
    while (dataIndex < count)
    {
        NSData *data = opusDataArray[dataIndex++];
        [cacheData appendData:data];
    }
    
    return cacheData;
}

- (void)dealloc
{
    USCLog(@"recognition -> dealloc");
    
    mSocket = nil;
    resultStr = nil;
    sessionid = nil;
    attriArray = nil;
    currentPrivateIp = nil;
    _appKey = nil;
    _oralText = nil;
    _oralTask = nil;
    _identifier = nil;
    private_IPArray = nil;
    opusDataArray = nil;
}

#pragma mark - CallBack

-(void)startCallback
{
    if (isCanceled)
    {
        return ;
    }
    
    if (_delegate && [_delegate respondsToSelector:@selector(uniSocketRecognitonDidStart)])
    {
        [_delegate uniSocketRecognitonDidStart];
    }
}

-(void)stopCallback
{
    if (isCanceled)
    {
        return ;
    }
    
    if (_delegate && [_delegate respondsToSelector:@selector(uniSocketRecognitonDidStop)])
    {
        [_delegate uniSocketRecognitonDidStop];
    }
}

-(void)failCallBack
{
    if (isCanceled)
    {
        return ;
    }
    
    BOOL isMainThread = [NSThread isMainThread];
    if (isMainThread)
    {
        if (_delegate && [_delegate respondsToSelector:@selector(uniSocketRecognitonDidFail:)])
        {
            [_delegate uniSocketRecognitonDidFail:recogError];
        }
    }
    else
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            
            if (_delegate && [_delegate respondsToSelector:@selector(uniSocketRecognitonDidFail:)])
            {
                [_delegate uniSocketRecognitonDidFail:recogError];
            }
        });
    }
}

#pragma mark - USCAsyncSocketDelegate

- (void)socket:(USCAsyncSocket *)sock didConnectToHost:(NSString *)host port:(uint16_t)port
{
    USCLog(@"didConnectToHost : %@",host);
    
    //根据当前连接重新选择备份IP，避免重复
    currentPrivateIp = host;
    [self updateIP];
}

- (void)socket:(USCAsyncSocket *)sock didReadData:(NSData *)data withTag:(long)tag
{
    USCLog(@"didReadData : %lu",tag);
    if (tag == SOC_START)
    {
        [startData appendData:data];
        
        USCResponse *response = [[USCResponse alloc]initWithData:startData];
        int resCode = [response getResponseCode];
        if (resCode == 0)
        {
            BOOL illegal = [response isIllegalPackage];
            if (!illegal)
            {
                BOOL isFull = [response packageIsFull];
                if (isFull)
                {
                    [response analysis];
                    sessionid = [response getSessionID];
                    
                    [self startCallback];
                    [self startResumeLoop];
                }
                else
                {
                    [mSocket readDataWithTimeout:getResultTimeout tag:SOC_START];
                }
            }
            else
            {
                [self setError:NetWork_Connect_Error];
            }
        }
        else
        {
            //如果确认是服务端错误，则立即断开链接
            BOOL isOnlineError = [self privateErrorFromServer:resCode];
            if (isOnlineError)
            {
                [mSocket disconnect];
            }
        }
    }
    else if (tag == SOC_STOP)
    {
        [resultData appendData:data];
        
        USCResponse *response = [[USCResponse alloc]initWithData:resultData];
        int resCode = [response getResponseCode];
        if (resCode == 0)
        {
            BOOL illegal = [response isIllegalPackage];
            if (!illegal)
            {
                BOOL isFull = [response packageIsFull];
                if (isFull)
                {
                    [response analysis];
                    resultStr = [response getResult];
                    
                    [self stopCallback];
                }
                else
                {
                    [mSocket readDataWithTimeout:getResultTimeout tag:SOC_STOP];
                }
            }
            else
            {
                [self setError:NetWork_Connect_Error];
            }
        }
        else
        {
            //如果确认是服务端错误，则立即断开链接
            BOOL isOnlineError = [self privateErrorFromServer:resCode];
            if (isOnlineError)
            {
                [mSocket disconnect];
            }
        }
    }
}

- (void)socket:(USCAsyncSocket *)sock didReadPartialDataOfLength:(NSUInteger)partialLength tag:(long)tag
{
    USCLog(@"didReadPartialDataOfLength");
}

- (void)socket:(USCAsyncSocket *)sock didWriteDataWithTag:(long)tag
{
    USCLog(@"didWriteDataWithTag : %lu",tag);
    if (tag == SOC_START)
    {
        [mSocket readDataWithTimeout:getResultTimeout tag:tag];
    }
    else if (tag == SOC_STOP)
    {
        [mSocket readDataWithTimeout:getResultTimeout tag:tag];
    }
}

- (void)socket:(USCAsyncSocket *)sock didWritePartialDataOfLength:(NSUInteger)partialLength tag:(long)tag
{
    USCLog(@"didWritePartialDataOfLength");
}

- (NSTimeInterval)socket:(USCAsyncSocket *)sock shouldTimeoutReadWithTag:(long)tag
                 elapsed:(NSTimeInterval)elapsed
               bytesDone:(NSUInteger)length
{
    USCLog(@"shouldTimeoutReadWithTag");
    return -1;
}


- (NSTimeInterval)socket:(USCAsyncSocket *)sock shouldTimeoutWriteWithTag:(long)tag
                 elapsed:(NSTimeInterval)elapsed
               bytesDone:(NSUInteger)length
{
    USCLog(@"shouldTimeoutWriteWithTag");
    return -1;
}

- (void)socketDidCloseReadStream:(USCAsyncSocket *)sock
{
    USCLog(@"socketDidCloseReadStream");
}

- (void)socketDidDisconnect:(USCAsyncSocket *)sock withError:(NSError *)err
{
    USCLog(@"socketDidDisconnect : %@",err);
    if (err != NULL && err.code == 7)
    {
        USCLog(@"socketDidDisconnect recogError : %d",recogError);
        if (recogError != No_Error)
        {
            [self errorHandle:recogError];
        }
    }
    else if (err.code == 61 || err.code == 3 || err.code == 51) //61 连接被拒绝 | 3 连接超时 | 51 DNS 解析错误
    {
        [self errorHandle:NetWork_Connect_Error];
    }
    else
    {
        if (recogError == No_Error)
        {
            [self setError:NetWork_Connect_Error];
        }
        [self failCallBack];
    }
}

- (void)socketDidSecure:(USCAsyncSocket *)sock
{
    USCLog(@"socketDidSecure");
}

- (void)socket:(USCAsyncSocket *)sock didReceiveTrust:(SecTrustRef)trust
completionHandler:(void (^)(BOOL shouldTrustPeer))completionHandler
{
    USCLog(@"didReceiveTrust");
}

@end
