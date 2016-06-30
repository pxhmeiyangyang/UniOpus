//
//  USCRequest.m
//  Socket
//
//  Created by 刘俊 on 15/7/20.
//  Copyright (c) 2015年 yunzhisheng. All rights reserved.
//

#import "USCRequest.h"
#import "USCAttribute.h"
#import "RFIWriter.h"

#import "USCMarcos.h"

@implementation USCRequest

-(id)init
{
    if (self = [super init])
    {
        attriArray = [NSArray array];
    }
    return self;
}

-(void)setAttributeArray:(NSArray *)attriAry
{
    attriArray = attriAry;
}

-(NSData *)requestFormater:(ResuqetCode)code
{
    NSMutableData *requestData = [[NSMutableData alloc]init];
    
    RFIWriter *writer = [RFIWriter writerWithData:requestData];
    
    int magic1 = 0x4d;
    int magic2 = 0x40;
    int version1 = 0x02;
    int version2 = 0x01;
    int reqCode = code;
    int compress = 0x01;
    int empty = 0;
    [requestData appendBytes:&magic1 length:1];
    [requestData appendBytes:&magic2 length:1];
    [requestData appendBytes:&version1 length:1];
    [requestData appendBytes:&version2 length:1];
    [requestData appendBytes:&reqCode length:1];
    [requestData appendBytes:&compress length:1];
    [requestData appendBytes:&empty length:1];
    [requestData appendBytes:&empty length:1];
    
    int totalLen = 0;
    
    totalLen += 4;
    
    for(USCAttribute *attri in attriArray)
    {
        totalLen += 8;
        totalLen += [attri bodyLength];
    }
    
    totalLen += 2;
    
    int attriCount = (int)attriArray.count;
    //attribute总长度(4字节)
    [writer writeInt32:ntohl(totalLen)];
    //attribute个数（4字节）
    [writer writeInt32:ntohl(attriCount)];
    
    //attribute key内容以及body长度（均为4字节）
    for(USCAttribute *attri in attriArray)
    {
        int attribute = attri.attribute;
        int bodyLen = (int)[attri bodyLength];
        
        //code是小端
        [requestData appendBytes:&attribute length:1];
        [requestData appendBytes:&empty length:1];
        [requestData appendBytes:&empty length:1];
        [requestData appendBytes:&empty length:1];
        
        //body长度是大端
        [writer writeInt32:htonl(bodyLen)];
    }
    
    //attribute
    for(USCAttribute *attri in attriArray)
    {
        NSData *bodyData = [attri getBody];
        
        [requestData appendData:bodyData];
    }
    
    int end1 = 0x21;
    int end2 = 0x40;
    [requestData appendBytes:&end1 length:1];
    [requestData appendBytes:&end2 length:1];
    
    NSString *condition;
    if (code == ORAL_REQ_START)
    {
        condition = @"Start";
    }
    else
    {
        condition = @"Stop";
    }
    USCLog(@"requestData | %@",condition);
    
    return requestData;
}

-(NSData *)requestFormater_resume:(NSData *)audioData
{
    NSMutableData *requestData = [[NSMutableData alloc]init];
    
    RFIWriter *writer = [RFIWriter writerWithData:requestData];
    
    int magic1 = 0x4d;
    int magic2 = 0x40;
    int version1 = 0x02;
    int version2 = 0x01;
    int reqCode = 0x11;
    int compress = 0x01;
    int empty = 0;
    [requestData appendBytes:&magic1 length:1];
    [requestData appendBytes:&magic2 length:1];
    [requestData appendBytes:&version1 length:1];
    [requestData appendBytes:&version2 length:1];
    [requestData appendBytes:&reqCode length:1];
    [requestData appendBytes:&compress length:1];
    [requestData appendBytes:&empty length:1];
    [requestData appendBytes:&empty length:1];
    
    int totalLen = 0;
    
    totalLen = (int)audioData.length;
    
    totalLen += 4;
    totalLen += 2;
    
    //resume请求不包含attribute
    int attriCount = 0;
    //attribute总长度(4字节)
    [writer writeInt32:ntohl(totalLen)];
    //attribute个数（4字节）
    [writer writeInt32:ntohl(attriCount)];
    
    //attribute
    [requestData appendData:audioData];
    
    int end1 = 0x21;
    int end2 = 0x40;
    [requestData appendBytes:&end1 length:1];
    [requestData appendBytes:&end2 length:1];
    
    return requestData;
}

-(void)dealloc
{
    attriArray = nil;
}

@end
