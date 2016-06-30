//
//  UniSocketRecognition.h
//  usc
//
//  Created by 刘俊 on 15/11/2.
//  Copyright © 2015年 yunzhisheng. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "USCAsyncSocket.h"
#import "USCErrorCode.h"
#import "USCPreference.h"

#import <UIKit/UIKit.h>

#define CONNECT_TIMEOUT           3
#define START_WRITE_TIMEOUT       10
#define STOP_WRITE_TIMEOUT        3
#define RESUME_WRITE_TIMEOUT      3
#define START_READ_TIMEOUT        60
#define STOP_READ_TIMEOUT         30
#define RESUME_READ_TIMEOUT       3

typedef NS_ENUM(long, SocketTag){
    SOC_START = 2,
    SOC_STOP = 3,
    SOC_RESUME = 4,
};

@protocol UniSocketDelegate <NSObject>

-(void)uniSocketRecognitonDidStart;
-(void)uniSocketRecognitonDidStop;
-(void)uniSocketRecognitonDidFail:(ErrorType)type;

@end

@interface UniSocketRecognition : NSObject<USCAsyncSocketDelegate>
{
    USCAsyncSocket *mSocket;
    NSArray *attriArray;
    NSArray *private_IPArray;
    USCPreference *preference;
    
    NSString *resultStr;
    NSString *sessionid;
    ErrorType recogError;
    
    dispatch_queue_t socketQueue;
    NSMutableArray *opusDataArray;
    NSInteger dataIndex;
    NSInteger totalLength;//音频数据的总长度
}

@property (nonatomic, assign) NSString *appKey;
@property (nonatomic, assign) NSString *oralText;
@property (nonatomic, assign) NSString *oralTask;
@property (nonatomic, assign) NSString *identifier;
@property (nonatomic, assign) id <UniSocketDelegate>delegate;
@property (nonatomic, assign) BOOL backupEnable;//是否启用备份机制，混合模式不开启

-(void)setSocket:(USCAsyncSocket *)socket;

-(void)appendData:(NSData *)data;

-(void)start;
-(void)stop;
-(void)cancel;

-(NSString *)getResult;
-(NSString *)getSessionID;

-(ErrorType)getError;

@end
