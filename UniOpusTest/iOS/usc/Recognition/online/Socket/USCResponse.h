//
//  USCResponse.h
//  Socket
//
//  Created by 刘俊 on 15/7/21.
//  Copyright (c) 2015年 yunzhisheng. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface USCResponse : NSObject
{
    NSData *responseData;
    int responeCode;
    NSString *result;
    NSString *audioUrl;
    NSMutableDictionary *attriDic;
    long totalLen;
}

-(id)initWithData:(NSData *)data;
-(void)analysis;
-(int)getResponseCode;
-(NSString *)getResult;
-(NSString *)getSessionID;

//返回数据包总长度
-(BOOL)packageIsFull;

/**
 *包长度超过10M时定性为非法数据
 */
-(BOOL)isIllegalPackage;

@end
