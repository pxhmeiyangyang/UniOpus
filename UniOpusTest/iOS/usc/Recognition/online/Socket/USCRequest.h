//
//  USCRequest.h
//  Socket
//
//  Created by 刘俊 on 15/7/20.
//  Copyright (c) 2015年 yunzhisheng. All rights reserved.
//

#import <Foundation/Foundation.h>

typedef NS_ENUM(int, ResuqetCode) {
    ORAL_REQ_START = 0x1,
    ORAL_REQ_RESUME = 0x11,
    ORAL_REQ_STOP = 0x10,
    ORAL_REQ_STOP_ASYNC = 0xe0,
    ORAL_REQ_CANCEL = 0x12,
    ORAL_REQ_GET_RESULT = 0x13,
};

@interface USCRequest : NSObject
{
    NSArray *attriArray;
}

-(void)setAttributeArray:(NSArray *)attriAry;
-(NSData *)requestFormater:(ResuqetCode)code;
-(NSData *)requestFormater_resume:(NSData *)audioData;

@end
