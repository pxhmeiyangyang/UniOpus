//
//  USCPreference.m
//  usc
//
//  Created by 刘俊 on 15/7/10.
//  Copyright (c) 2015年 yunzhisheng. All rights reserved.
//

#import "USCPreference.h"
#import "Settings.h"

#define RECOGNITION_TIMEOUT 1.0

static USCPreference *mPreference;

@implementation USCPreference

@synthesize recogntionTimeout;

+(USCPreference *)sharePreference
{
    if (mPreference == nil)
    {
        mPreference = [[USCPreference alloc]init];
    }
    
    return mPreference;
}

-(id)init
{
    if (self == [super init])
    {
        [self initPreference];
    }
    return self;
}

-(void)initPreference
{
    recogntionTimeout = RECOGNITION_TIMEOUT;
    coefficientScore = COEFFICIENT_SCORE;
    currentPrivateIP = oral_domain;
    currentPrivatePort = oral_port;
    backType = Backup_PrivateDomain;
    currentHTTPIP = nil;//不设初始值
    asyncRecognize = NO;
}

-(void)setRecognitionTimeout:(NSTimeInterval)interval
{
    recogntionTimeout = interval;
}

-(void)setCoefficientScore:(float)score;
{
    coefficientScore = score;
}

-(float)getCoefficientScore
{
    return coefficientScore;
}

-(NSString *)getCurrentPrivateIp
{
    return currentPrivateIP;
}

-(void)setCurrentPrivateIp:(NSString *)ipStr
{
    currentPrivateIP = nil;
    currentPrivateIP = [[NSString alloc]initWithString:ipStr];
}

-(void)setCurrentPrivatePort:(int)port
{
    currentPrivatePort = port;
}

-(int)getCurrentPrivatePort
{
    return currentPrivatePort;
}

-(void)setCurrentHTTPIP:(NSString *)ipStr
{
    currentHTTPIP = ipStr;
}

-(NSString *)getCurrentHTTPIP
{
    return currentHTTPIP;
}

-(void)setBackupType:(BackupType)type
{
    backType = type;
}

-(BackupType)getBackupType
{
    return backType;
}

-(void)setBackupInfo:(UniBackupInfo *)info
{
    backupInfo = info;
}

-(UniBackupInfo *)getBackupInfo
{
    return backupInfo;
}

-(void)setAsyncRecognize:(BOOL)isAsync
{
    asyncRecognize = isAsync;
}

-(BOOL)getAsyncRecognize
{
    return asyncRecognize;
}

-(void)setOfflineResultWaitingTime:(NSTimeInterval)time{
    if (time != RECOGNITION_TIMEOUT && time > 0) {
        recogntionTimeout = time;
    }
}

@end
