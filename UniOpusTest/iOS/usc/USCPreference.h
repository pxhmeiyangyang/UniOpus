//
//  USCPreference.h
//  usc
//
//  Created by 刘俊 on 15/7/10.
//  Copyright (c) 2015年 yunzhisheng. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "UniBackupInfo.h"

typedef enum : NSUInteger {
    Backup_PrivateDomain = 1,
    Backup_PrivateIP = 2,
    Backup_HTTP = 3,
} BackupType;

#define COEFFICIENT_SCORE 1.0

#define ASYNC_RESULT @"ASYNC_RESULT" //为保证原有流程不被干扰，启用延时评测时，在线识别统一返回该result

@interface USCPreference : NSObject
{
    NSString *currentPrivateIP;
    int currentPrivatePort;
    
    NSString *currentHTTPIP;
    
    float coefficientScore;
    
    BackupType backType;
    
    UniBackupInfo *backupInfo;
    
    BOOL asyncRecognize; //是否启用延时评测
}

@property (nonatomic, readonly)NSTimeInterval recogntionTimeout;

+(USCPreference *)sharePreference;

/**
 *设置识别超时时间
 */
-(void)setRecognitionTimeout:(NSTimeInterval)interval;

-(void)setCoefficientScore:(float)score;
-(float)getCoefficientScore;

-(NSString *)getCurrentPrivateIp;
-(void)setCurrentPrivateIp:(NSString *)ipStr;

-(void)setCurrentPrivatePort:(int)port;
-(int)getCurrentPrivatePort;

-(void)setCurrentHTTPIP:(NSString *)ipStr;
-(NSString *)getCurrentHTTPIP;

-(void)setBackupType:(BackupType)type;
-(BackupType)getBackupType;

-(void)setBackupInfo:(UniBackupInfo *)info;
-(UniBackupInfo *)getBackupInfo;

//延时评测
-(void)setAsyncRecognize:(BOOL)isAsync;
-(BOOL)getAsyncRecognize;

-(void)setOfflineResultWaitingTime:(NSTimeInterval)time;

@end
