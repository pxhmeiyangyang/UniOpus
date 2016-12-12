//
//  libOpus.h
//  libOpus
//
//  Created by pxh on 2016/12/8.
//  Copyright © 2016年 pxh. All rights reserved.
//

#import <Foundation/Foundation.h>


//@protocol OpusDelegate <NSObject>
//
///**
// *  opus编码完成后编码数据回调
// *
// *  @param encodeData 编码数据
// */
//-(void)opusDataDidEncode:(NSData *)encodeData;
//
///**
// *  编码完成回调
// */
//-(void)opusDataDidFinished;
//
//@end

typedef void(^OpusDataDidFinished) ();

typedef void(^OpusDataDidEncode) (NSData* data);

@interface libOpus : NSObject


//@property (nonatomic,assign)id <OpusDelegate>delegate;

@property (nonatomic,copy)OpusDataDidFinished opusDataDidFinished;

@property (nonatomic,copy)OpusDataDidEncode opusDataDidEncode;
/**
 *  opus编码函数
 *  按照buffer进行编码
 *  @param data 传入需要编码的音频数据
 */
-(void)appendAudioData:(NSData *)data isEncode:(BOOL)isEncode;

/**
 *  停止编码
 */
-(void)stopEncode;

/**
 *  退出编码
 */
-(void)cancelEncode;


@end
