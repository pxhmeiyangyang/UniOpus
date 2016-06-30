//
//  USCHTTPRecognition.h
//  Socket
//
//  Created by 刘俊 on 15/9/28.
//  Copyright © 2015年 yunzhisheng. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "USCErrorCode.h"
#import "USCMarcos.h"
#import "USCPreference.h"

@protocol HTTPRecognitionDelegate <NSObject>

-(void)recognitionSessionIdDidGet:(NSString *)sessionID;
-(void)recognitionResultDidGet:(NSString *)result;
-(void)recognitionConnectDidFail:(ErrorType)error;

@end

@interface USCHTTPRecognition : NSObject<NSURLConnectionDataDelegate, NSURLConnectionDelegate>
{
    NSString *boundaryStr;
    NSMutableData *receiveData;
    
    NSMutableURLRequest *urlReq;
    NSURLConnection *connect;
    
    USCPreference *preference;
}

@property (nonatomic, assign) id delegate;
@property (nonatomic, strong) NSString *appKey;
@property (nonatomic, strong) NSString *oralText;
@property (nonatomic, strong) NSString *urlStr;
@property (nonatomic, assign) NSString *oralTask;
@property (nonatomic, strong) NSData *audioData;
@property (nonatomic, strong) NSString* identifier;
@property (nonatomic, assign) NSTimeInterval requestTimeout;

-(void)startRecognition;
-(void)cancelRecognition;

@end
