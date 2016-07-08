//
//  UniOpus.h
//  UniOpus
//
//  Created by pxh on 16/6/29.
//  Copyright © 2016年 裴新华. All rights reserved.
//

#import <Foundation/Foundation.h>


@protocol UnisoundOpusDelegate <NSObject>

/**
 *  opus编码完成后编码数据回调
 *
 *  @param encodeData 编码数据
 */
-(void)opusDataDidEncode:(NSData *)encodeData;

/**
 *  编码完成回调
 */
-(void)opusDataDidFinished;

@end

@interface UnisoundOpus : NSObject

@property (nonatomic,assign)id <UnisoundOpusDelegate>delegate;

/**
 *  opus编码函数
 *  按照buffer进行编码
 *  @param data 传入需要编码的音频数据
 */
-(void)appendAudioData:(NSData *)data;

/**
 *  停止编码
 */
-(void)stopEncode;

/**
 *  退出编码
 */
-(void)cancelEncode;

@end
