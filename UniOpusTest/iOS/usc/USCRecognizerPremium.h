//
//  USCRecognizerPremium.h
//  usc
//
//  Created by hejinlai on 13-1-24.
//  Copyright (c) 2013年 yunzhisheng. All rights reserved.
//

#import <Foundation/Foundation.h>

@protocol USCRecognizerPremiumDelegate <NSObject>

/*
     录音和识别成功启动后，回调该方法.
     为了防止前半部分语音被截断的现象，建议
     在此方法回调后，再提醒用户开始说话
 */
- (void)onStart;


/*
     部分识别结果回调，isLast表示是否是最后一次
 */
- (void)onResult:(NSString *)result isLast:(BOOL)isLast;


/*
     识别结束回调，error为nil表示成功，否则表示出现了错误
 */
- (void)onEnd:(NSError *)error;


/*
     说话停顿超时回调
 */
- (void)onVADTimeout;


/*
     说话时的音量大小，范围0~100
 */
- (void)onUpdateVolumn:(int)volumn;

@end


@interface USCRecognizerPremium : NSObject

@property (nonatomic, assign) id<USCRecognizerPremiumDelegate> delegate;

/*
     初始化，appKey请在http://dev.hivoice.cn申请
 */
- (id)initWithAppKey:(NSString *)appKey;

/*
     开始识别
 */
- (void)start;

/*
     停止识别
 */
- (void)stop;

/*
     取消识别
 */
- (void)cancel;

/*
     设置说话停顿的超时时间，单位ms
     frontTime：说话之前的停顿超时时间，默认3000ms
     backTime： 说话之后的停顿超时时间，默认1000ms
 */
- (void)setVadFrontTimeout:(int)frontTime BackTimeout:(int)backTime;

/*
 设置录音采样率，支持8000和16000，默认为16000
 */
- (void)setSampleRate:(int)rate;

/*
     获取当前版本号
 */
+ (NSString *)getVersion;

@end

