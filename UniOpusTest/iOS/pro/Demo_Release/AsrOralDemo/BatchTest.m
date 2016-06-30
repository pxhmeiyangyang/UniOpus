//
//  BatchTest.m
//  AsrOralDemo
//
//  Created by 刘俊 on 15/7/17.
//  Copyright (c) 2015年 yunzhisheng. All rights reserved.
//

#import "BatchTest.h"

@implementation BatchTest

-(void)start
{
    number = 1;
    
    test = [[BatchPCM alloc]init];
    test.delegate = self;
    
    queue = [[NSOperationQueue alloc]init];
    [queue addOperation:test];
    
    enginer = [EngineManager sharedManager];
    enginer.delegate = self;
    [enginer setRecognizerMode:Recognize_PCM];

    [self performSelector:@selector(finish) withObject:nil afterDelay:3];
}

-(void)finish
{
    NSLog(@"----------- resart : %d-----------",(int)number);
    
    number ++;
    [enginer setOralText:@"Dinner"];
    [enginer startRecognize];
}

-(void)error
{
    
}

- (void)onBeginOral
{
    
}

- (void)onStopOral
{
    
}

- (void)onResult:(NSString *)result isLast:(BOOL)isLast
{
    
}

- (void)onEndOral:(NSError *)error
{
    if (error)
    {
        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString *documentsPath = [paths objectAtIndex:0];
        NSString *fileName = [NSString stringWithFormat:@"error_%@.txt",[NSDate date]];
        NSString *errorPath = [documentsPath stringByAppendingPathComponent:fileName];
        
        NSString *errorStr = [NSString stringWithFormat:@"%@ --- %ld --- %@",error.domain,(long)error.code,[NSDate date]];
        [errorStr writeToFile:errorPath atomically:YES encoding:NSUTF8StringEncoding error:nil];
    }
    [test setFinishValue];
}

- (void)onVADTimeout
{
    
}

- (void)onUpdateVolume:(int)volume
{
    
}

- (void)onRecordingBuffer:(NSData *)recordingData
{
    
}

- (void)oralEngineDidInit:(NSError *)error
{
    
}

- (void)audioFileDidRecord:(NSString *)url
{
    
}


@end
