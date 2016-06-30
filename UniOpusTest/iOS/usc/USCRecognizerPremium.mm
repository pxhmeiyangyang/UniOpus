//
//  USCRecognizerPremium.m
//  usc
//
//  Created by hejinlai on 13-1-24.
//  Copyright (c) 2013年 yunzhisheng. All rights reserved.
//

#import "USCRecognizerPremium.h"
#import "USCRecognizer.h"
#import "USCErrorCode.h"

@interface USCRecognizerPremium() <USCRecognizerDelegate>
{
    USCRecognizer *recognizer;
}

@end


@implementation USCRecognizerPremium

@synthesize delegate = _delegate;

-(id)initWithAppKey:(NSString *)appKey
{
    if (self = [super init]) {
        recognizer = [[USCRecognizer alloc] initWithAppKey:appKey];
        recognizer.delegate = self;
    }
    
    return self;
}

-(void)start
{
    [recognizer start];
}

-(void)stop
{
    [recognizer stop];
}

-(void)cancel
{
    [recognizer cancel];
}

- (void)onStart
{
    if (_delegate && [_delegate respondsToSelector:@selector(onStart)]) {
        [_delegate onStart];
    }
}

-(void)onResult:(NSString *)result isLast:(BOOL)isLast
{
    if (_delegate && [_delegate respondsToSelector:@selector(onResult: isLast:)]) {
        [_delegate onResult:result isLast:isLast];
    }
}

-(void)onEnd:(NSError *)error
{
    if (_delegate && [_delegate respondsToSelector:@selector(onEnd:)]) {
        [_delegate onEnd:error];
    }
}

- (void)onCancel
{
    
}

-(void)onUpdateVolumn:(int)volumn
{
    if (_delegate && [_delegate respondsToSelector:@selector(onUpdateVolumn:)]) {
        [_delegate onUpdateVolumn:volumn];
    }
}

-(void)onVADTimeout
{
    if (_delegate && [_delegate respondsToSelector:@selector(onVADTimeout)]) {
        [_delegate onVADTimeout];
    }
}

-(void)onRecordingStop:(NSMutableData *)recordingData
{

}

-(void)onUploadUserData:(NSError *)error
{

}

- (void)setVadFrontTimeout:(int)frontTime BackTimeout:(int)backTime
{
    [recognizer setVadFrontTimeout:frontTime BackTimeout:backTime];
}

/*
 设置采样率，支持8k和16k，默认为16k
 */
- (void)setSampleRate:(int)rate
{
    [recognizer setSampleRate:rate];
}

+ (NSString *)getVersion
{
    return sdk_version;
}

-(void)dealloc
{
    [super dealloc];
    [recognizer release];
}


@end
