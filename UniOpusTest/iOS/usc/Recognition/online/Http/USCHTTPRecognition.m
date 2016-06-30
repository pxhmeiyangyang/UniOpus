//
//  USCHTTPRecognition.m
//  Socket
//
//  Created by 刘俊 on 15/9/28.
//  Copyright © 2015年 yunzhisheng. All rights reserved.
//

#define HTTPRequestTimeout 1

#import "USCHTTPRecognition.h"

@implementation USCHTTPRecognition

-(id)init
{
    if (self = [super init])
    {
        USCLog(@"http -> init");
        
        boundaryStr = getMultipartFormBoundary();
        receiveData = [[NSMutableData alloc]init];
        
        preference = [USCPreference sharePreference];
    }
    return self;
}

-(void)startRecognition
{
    USCLog(@"http -> startRecognition");
    
    _urlStr = [self getURLString];
    
    urlReq = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:_urlStr]];
    urlReq.timeoutInterval = _requestTimeout;
    
    NSData *body = [self getHttpBody];
    
    NSString *contentTypeValue = [NSString stringWithFormat:@"multipart/form-data; boundary=%@",boundaryStr];
    NSString *length = [NSString stringWithFormat:@"%lu", (unsigned long)[body length]];
    [urlReq addValue:length                                         forHTTPHeaderField:@"Content-length"];
    [urlReq addValue:_appKey                                        forHTTPHeaderField:@"appkey"];
    [urlReq addValue:[self UUIDstring]                              forHTTPHeaderField:@"session-id"];
    [urlReq addValue:_identifier                                    forHTTPHeaderField:@"device-id"];
    [urlReq addValue:contentTypeValue                               forHTTPHeaderField:@"Content-Type"];
    
    [urlReq setHTTPBody:body];
    [urlReq setHTTPMethod:@"POST"];
    
    connect = [[NSURLConnection alloc]initWithRequest:urlReq delegate:self startImmediately:YES];
}

-(void)cancelRecognition
{
    USCLog(@"http -> cancelRecognition");
    
    if (connect)
    {
        [connect cancel];
    }
}

-(NSString *)getURLString
{
    NSString *httpIP = [preference getCurrentHTTPIP];
    //随机选取获取http备份IP
    if (httpIP == nil)
    {
        int index = arc4random()%3;
        httpIP = HTTP_IP[index];
    }
    
    return [NSString stringWithFormat:@"http://%@/eval/opus",httpIP];
}
- (NSString*)UUIDstring
{
    //create a new UUID
    CFUUIDRef    uuidObj = CFUUIDCreate(nil);
    //get the string representation of the UUID
    NSString *uuidString=  (__bridge_transfer NSString *)CFUUIDCreateString(nil, uuidObj);
    return uuidString;
}

-(NSData *)getHttpBody
{
    NSMutableData *body = [NSMutableData data];
    
    NSString *bodyHeader = [NSString stringWithFormat:@"--%@\r\n",boundaryStr];
    [body appendData:[bodyHeader dataUsingEncoding:NSUTF8StringEncoding]];

    //
    NSDictionary *parameters = @{@"text":_oralText,@"mode":_oralTask};
    NSString *patameterStr = [self getContentDisposionSimpleData:parameters];
    
    [body appendData:[patameterStr dataUsingEncoding:NSUTF8StringEncoding]];
    
    //audioData
    NSMutableString *audioStatementstr = [NSMutableString string];
    //[audioStatementstr appendString:[NSString stringWithFormat:@"--%@\r\n",boundaryStr]];
    [audioStatementstr appendString:[NSString stringWithFormat:@"Content-Disposition: form-data; name=\"%@\"; filename=\"%@\"\r\n",@"voice",@"record.pcm"]];
    [audioStatementstr appendString:@"Content-Type: application/octet-stream\r\n\r\n"];
    
    [body appendData:[audioStatementstr dataUsingEncoding:NSUTF8StringEncoding]];
    
    [body appendData:_audioData];
    
    NSString *bodyFooter = [NSString stringWithFormat:@"\r\n--%@--\r\n",boundaryStr];
    
    [body appendData:[bodyFooter dataUsingEncoding:NSUTF8StringEncoding]];
    
    return body;
}

static NSString * getMultipartFormBoundary()
{
    return [NSString stringWithFormat:@"Boundary+%08X%08X", arc4random(), arc4random()];
}

-(NSString *)getContentDisposionSimpleData:(NSDictionary *)parameters
{
    NSMutableString *string = [NSMutableString string];
    
    for (NSString *key in parameters.allKeys)
    {
        NSString *value = [parameters objectForKey:key];
        [string appendString:[NSString stringWithFormat:@"Content-Disposition: form-data; name=\"%@\"\r\n\r\n%@\r\n",key,value]];
        [string appendString:[NSString stringWithFormat:@"--%@\r\n",boundaryStr]];
    }
    return string;
}

-(NSString *)getContentDisposionAudioDataHeader
{
    NSMutableString *string = [NSMutableString string];
    
    [string appendString:[NSString stringWithFormat:@"--%@\r\n",boundaryStr]];
    [string appendString:[NSString stringWithFormat:@"Content-Disposition: form-data; name=\"%@\"; filename=\"%@\"\r\n",@"voice",@"record.pcm"]];
    [string appendString:@"Content-Type: application/octet-stream"];
    [string appendString:[NSString stringWithFormat:@"--%@\r\n",boundaryStr]];
    
    return string;
}

#pragma mark --
#pragma mark NSURLConnectionDelegate

- (void)connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response
{
    USCLog(@"http -> didReceiveResponse : %@",response);
    NSHTTPURLResponse *rsp = (NSHTTPURLResponse *)response;
    if (rsp.statusCode != 200)
    {
        [connect cancel];
        if (_delegate != nil && [_delegate respondsToSelector:@selector(recognitionConnectDidFail:)])
        {
            USCLog(@"didReceiveResponse : %ld",(unsigned long)rsp.statusCode);
            ErrorType type = [USCErrorCode onlineErrorTransform:(int)rsp.statusCode];
            if (type == NO_Define)
            {
                [self.delegate recognitionConnectDidFail:NetWork_Http_Connect_Error];
            }
            else
            {
                [self.delegate recognitionConnectDidFail:type];
            }
        }
    }
    else
    {
        NSDictionary *headers = rsp.allHeaderFields;
        if (headers != nil)
        {
            NSString *sessionId = headers[@"Session-Id"];
            USCLog(@"http -> sessionId : %@",sessionId);
            if (sessionId != nil && [sessionId length]!= 0)
            {
                if (_delegate != nil && [_delegate respondsToSelector:@selector(recognitionSessionIdDidGet:)])
                {
                    [self.delegate recognitionSessionIdDidGet:sessionId];
                }
            }
        }
    }
}

- (void)connection:(NSURLConnection *)connection didReceiveData:(NSData *)data
{
    USCLog(@"didReceiveData");
    if (data != nil)
    {
        [receiveData appendData:data];
    }
}

- (void)connectionDidFinishLoading:(NSURLConnection *)connection
{
    USCLog(@"http -> connectionDidFinishLoading : %@",connection);
    NSString *result = [[NSString alloc]initWithData:receiveData encoding:NSUTF8StringEncoding];
    USCLog(@"http -> result : %@",result);
    if (result != nil)
    {
        if (_delegate != nil && [_delegate respondsToSelector:@selector(recognitionResultDidGet:)])
        {
            [self.delegate recognitionResultDidGet:result];
        }
    }
    else
    {
        [connect cancel];
        if (_delegate != nil && [_delegate respondsToSelector:@selector(recognitionConnectDidFail:)])
        {
            [self.delegate recognitionConnectDidFail:NetWork_Http_Read_Error];
        }
    }
}

- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error
{
    USCLog(@"http -> didFailWithError : %@",error);
    [connect cancel];
    if (_delegate != nil && [_delegate respondsToSelector:@selector(recognitionConnectDidFail:)])
    {
        [self.delegate recognitionConnectDidFail:NetWork_Http_Connect_Error];
    }
    
    //HTTP作为最后一种备份方案，如果出错则重置所有方案，同时将缓存的http ip置空，同时将端口改回8085
    [preference setBackupType:Backup_PrivateDomain];
    [preference setCurrentPrivateIp:oral_domain];
    [preference setCurrentPrivatePort:oral_port];
    [preference setCurrentHTTPIP:nil];
}

-(void)dealloc
{
    USCLog(@"http -> dealloc");
    
    urlReq = nil;
    [connect cancel];
    connect = nil;
    _delegate = nil;
    boundaryStr = nil;
    [receiveData resetBytesInRange:NSMakeRange(0, receiveData.length)];
    receiveData = nil;
}

@end
