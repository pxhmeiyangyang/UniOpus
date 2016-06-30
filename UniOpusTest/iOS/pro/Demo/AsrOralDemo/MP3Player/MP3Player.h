//
//  MP3Player.h
//  AsrOralDemo
//
//  Created by 刘俊 on 15/6/26.
//  Copyright (c) 2015年 yunzhisheng. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

@interface MP3Player : NSObject<AVAudioPlayerDelegate>
{
    NSString *filePath;
    AVAudioPlayer *player;
}

@property (nonatomic, assign) id delegate;

- (id)initWithPath:(NSString *)path;
- (BOOL) play;
- (void)stop;

-(void)playWithFile:(NSString *)path;

/**
 *  使用音频文件的链接地址播放音频
 *
 *  @param url 音频的链接地址
 */
-(void)playWithUrl:(NSString* )url;

@end

@protocol MP3PlayerDelegate <NSObject>

-(void)playFinished:(NSError *)error;

@end
