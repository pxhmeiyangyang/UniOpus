//
//  USCRecognition.h
//  usc
//
//  Created by hejinlai on 12-11-9.
//  Copyright (c) 2012年 yunzhisheng. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#import "USCBaseRecognition.h"
#import "UniSocketRecognition.h"
#import "USCHTTPRecognition.h"
#import "USCErrorCode.h"
#import "USCAsyncSocket.h"
#import "UniOpus.h"

#import "USCPreference.h"

#define BACKUP_TIMEOUT      15*60

@protocol USCRecognitionDelegate <NSObject>

- (void)onRecognitionStart;
- (void)onRecognitionResult:(NSString *)result isLast:(BOOL)isLast;
- (void)onRecognitionStop;
- (void)onRecognitionError:(ErrorType)error;
- (void)onMaxSpeechTimeout;
- (void)onSessionId:(NSString *)sessionId;

@end

@interface USCRecognition : USCBaseRecognition<UniSocketDelegate, HTTPRecognitionDelegate, UniOpusDelegate>
{
    NSMutableData *httpData;
    USCAsyncSocket *mSocket;
    USCPreference *preference;
    UniOpus *uniOpus;
    
    USCHTTPRecognition *httpRecog;
    UniSocketRecognition *socketRecognition;
}

@property (nonatomic, strong) NSMutableArray *recordingQueue;
@property (nonatomic, assign) id<USCRecognitionDelegate> delegate;
@property (nonatomic, assign) NSString *appKey;
@property (nonatomic, assign) NSString *oralText;
@property (nonatomic, assign) NSString *oralTask;
@property (nonatomic, assign) NSString *identifier;
@property (nonatomic, assign) BOOL backupEnable; //是否启用备份机制，混合模式不开启

-(void)setSocket:(USCAsyncSocket *)socket;
-(void)appendAudioData:(NSData *)audioData;

-(void)start;
-(void)stop;
-(void)cancel;

@end
