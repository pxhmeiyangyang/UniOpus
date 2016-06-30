//
//  USCResponse.m
//  Socket
//
//  Created by 刘俊 on 15/7/21.
//  Copyright (c) 2015年 yunzhisheng. All rights reserved.
//

#import "USCResponse.h"
#import "RFIReader.h"
#import "USCAttribute.h"

#import "USCMarcos.h"


//http://192.168.5.120/wiki/index.php/Private_front_service_errcode
/*
 
 错误码      错误码含义                                                     错误码说明
 0x0000FFFD	用户传过来的appkey                                             错误
 0x0000FFFE	连接引擎服务错误                                                连不上引擎服务，或者用户的文本全是生词
 0x0000FFFC	引擎服务stop返回错误
 0x0000FFF4	最开始从网络上读取私有协议8个字节的frame出错
 0x0000FFF6	不是私有协议的客户端
 0x0000FFF5	当私有协议单个包 大于32MB(2 << 25)时报错
 0x0000FFF7	65527 用户传过来的文本是空                                             用户传过来的文本是空，或者用户文本全部由空字符组成
 0x0000F000	私有协议内部未知错误
 0x0000F001	连接转码服务失败                                                暂未启用
 0x0000E001	连接引擎服务失败                                                暂未启用
 0x0000E006	传给引擎服务的用户文本全是生词                                     暂未启用
 0x0000E007	传给引擎服务的用户文本过长                                        暂未启用
 0x0000E008	引擎非用户引起的出错，此错误非用户传入的文本和音频引起                 暂未启用 当出现此错误码，日志等级设置为error
 
 0x00002001	连接n2t服务出错
 0x00002003	n2t解析错误
 0x00002006	n2t组装结果出错,只有enstar和E模式会组装结果
 0x0000C001	连接转码服务出错
 0x0000C003	转码服务转码过程中出错

 */

@implementation USCResponse

-(id)initWithData:(NSData *)data
{
    if (self = [super init])
    {
        responseData = [NSData dataWithData:data];
        attriDic = [[NSMutableDictionary alloc]init];
        
        responeCode = [self getResponseCode];
        
        totalLen = [self getTotalength];
    }
    return self;
}

-(void)analysis
{
    if (responseData == NULL || responseData.length <=0)
    {
        return ;
    }
    if (responeCode == 0)
    {
        NSRange totalRange = NSMakeRange(4, 4);
        int totalLength = 0;
        totalLength = [self getIntValueNtohl:totalRange];
        
        NSRange attriCountRange = NSMakeRange(totalRange.location + totalRange.length, 4);
        int attriCount = 0;
        attriCount = [self getIntValueNtohl:attriCountRange];
        
        USCLog(@"totalLength : %d | attriCount : %d",totalLength, attriCount);
        
        totalLength -= 4;
        
        //attribute count之后的格式为4字节code，4字节value长度，循环次数取决于attribute count
        for (int i = 0; i < attriCount; i++)
        {
            //attribute
            NSRange attriRange = NSMakeRange(12 + i * 8, 4);
            int attriCode= 0;
            attriCode = [self getIntValue:attriRange];
            
            //value长度
            //NSRange attriValueRange = NSMakeRange(16 + i * 8 + 1 + 1 + 1, 1);
            NSRange attriValueRange = NSMakeRange(12  + 4 + i * 8, 4);
            int attriLength = 0;
            attriLength = [self getIntValueNtohl:attriValueRange];
            
            //value的值
            NSRange valueRange = NSMakeRange(responseData.length - (totalLength - 8 * (attriCount - i)), attriLength);
            NSString *value = [self getStringValue:valueRange withStringKey:'@'];
            //USCLog(@"USCResponse -> analysis : %@",[responseData subdataWithRange:valueRange]);
            USCLog(@"USCResponse -> analysis : %@ | attriLength : %d",value, attriLength);
            
            NSNumber *code = [NSNumber numberWithInt:attriCode];
            if (value != nil)
            {
                [attriDic setObject:value forKey:code];
            }
            else
            {
                [attriDic setObject:[NSNull null] forKey:code];
            }
            
            totalLength -= 8;
            totalLength -= attriLength;
        }
        
        USCLog(@"attriDic : %@",attriDic);
        USCLog(@"totalLength : %d",totalLength);
        if (totalLength > 0)
        {
            NSRange resultRange = NSMakeRange(responseData.length - totalLength, totalLength);
            
            NSString *value = [self getStringValue:resultRange withIntKey:0x01];
            if(value != nil)
            {
                result = [[NSString alloc]initWithString:value];
            }
        }
    }
    else
    {
        USCLog(@"error");
        return ;
    }
}

//返回数据包总长度
-(BOOL)packageIsFull
{
    if (responseData == NULL || responseData.length <=0)
    {
        return 0;
    }
    
    int totalLength = 0;
    if (responeCode == 0)
    {
        NSRange totalRange = NSMakeRange(4, 4);
        
        totalLength = [self getIntValueNtohl:totalRange];
        
        totalLength += 8;
        
        USCLog(@"totalLength ： %d | responseData.length : %d",totalLength, (int)responseData.length);
        
        if (totalLength == responseData.length)
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
        return NO;
    }
}

-(long)getTotalength
{
    long length = 0;

    if (responseData == NULL || responseData.length <=0)
    {
        USCLog(@"getTotalength no data");
        return length;
    }
    
    if (responeCode == 0)
    {
        NSRange totalRange = NSMakeRange(4, 4);
        
        length = [self getIntValueNtohl:totalRange];
        
        length += 8;
    }
    
    USCLog(@"getTotalength : %ld",length);
    
    return length;
}

/**
 *包长度超过10M时定性为非法数据
 */
-(BOOL)isIllegalPackage
{
    if (totalLen > 10 * 1024 * 1024)
    {
        return YES;
    }
    else
    {
        return NO;
    }
}

-(NSString *)getResult
{
    return result;
}

-(int)getResponseCode
{
    int code = -1;
    if (responseData == NULL || responseData.length <=0)
    {
        return code;
    }
    NSRange responeseRange = NSMakeRange(0, 4);
    code = [self getIntValueNtohl:responeseRange];
    USCLog(@"responeCode ： %d",code);
    return code;
}

-(NSString *)getSessionID
{
    NSNumber *sessionid = [NSNumber numberWithInt:SSUP_SESSION_ID];
    return [attriDic objectForKey:sessionid];
}

-(int)getIntValueNtohl:(NSRange)range
{
    uint32_t content = 0;
    NSData *subData = [responseData subdataWithRange:range];
    [subData getBytes:&content length:range.length];
    
    return ntohl(content);
}

-(int)getIntValue:(NSRange)range
{
    int content = 0;
    NSData *subData = [responseData subdataWithRange:range];
    [subData getBytes:&content length:range.length];
    
    return content;
}

-(NSString *)getStringValue:(NSRange)range withStringKey:(char)key
{
    NSData *subData = [responseData subdataWithRange:range];
    Byte *valueByte = (Byte *)subData.bytes;
    for (int i = 0; i < subData.length; i++)
    {
        valueByte[i] ^= (Byte)key;
    }
    
    NSString *value = [[NSString alloc]initWithBytes:valueByte length:subData.length encoding:NSUTF8StringEncoding];
    
    return value;
}

-(NSString *)getStringValue:(NSRange)range withIntKey:(int)key
{
    NSData *subData = [responseData subdataWithRange:range];
    Byte *valueByte = (Byte *)subData.bytes;
    for (int i = 0; i < subData.length; i++)
    {
        if (valueByte[i] != 0 && valueByte[i] != 1)
        {
            valueByte[i] ^= key;
        }
    }
    
    NSString *value = [[NSString alloc]initWithBytes:valueByte length:subData.length encoding:NSUTF8StringEncoding];
    
    return value;
}

-(void)dealloc
{
    USCLog(@"USCResponse -> dealloc");
    
    responseData = nil;
    
    result = nil;
    
    audioUrl = nil;
    
    attriDic = nil;
}

@end
