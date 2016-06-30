//
//  USCPCMReader.m
//  usc
//
//  Created by 刘俊 on 15/7/7.
//  Copyright (c) 2015年 yunzhisheng. All rights reserved.
//

#import "USCPCMReader.h"
#import "USCMarcos.h"

static const int bufferByteSize = 3200;     //每次从文件读取的长度

@implementation USCPCMReader
@synthesize delegate = _delegate;

- (id) initWithPath:(NSString *)path
{
    if (self = [super init])
    {
        filePath = [NSString stringWithString:path];
        
        pcmAnalysis = [[UniPCMAnalysis alloc]init];
        pcmAnalysis.delegate = self;
        
        //testRecorder = [UniPCMRecorder defaultRecorder];
        
        USCLog(@"ReadPCMData -> init");
    }
    
    return self;
}

-(void)reset
{
    analysisFinishing = NO;
    isReading = NO;
    [pcmAnalysis analysisReset];
    
    NSString *name = [NSString stringWithFormat:@"%d",(int)[NSDate timeIntervalSinceReferenceDate]];
    [testRecorder start:name];
}

-(void)main
{
    USCLog(@"Recognizing -> start");
    USCLog(@"Recognizing -> path : %@",[filePath class]);
    
    // reset
    [self reset];
    
    if([[NSFileManager defaultManager] fileExistsAtPath:filePath])
    {
        [self readingStart:No_Error];
        
        //读取音频文件
        isReading = YES;
        NSData *wholeData = [[NSData alloc]initWithContentsOfFile:filePath];
        int wholeLength = (int)wholeData.length;
        int i = 0;

        while (isReading)
        {
            if (self.isCancelled)
            {
                USCLog(@"PCM Read canceled!");
                
                [pcmAnalysis cancelAnalysis];
                
                return ;
            }
            
            NSRange range;
            int length = wholeLength - i * bufferByteSize;
            if (length >= bufferByteSize)
            {
                range = NSMakeRange(i * bufferByteSize, bufferByteSize);
            }
            else if (length > 0 && length < bufferByteSize)
            {
                range = NSMakeRange(i * bufferByteSize, length);
            }
            else if (length <= 0)
            {
                USCLog(@"PCM Read Finished!");
                break;
            }

            NSData *blockData = [wholeData subdataWithRange:range];
            [testRecorder appendData:blockData];
            
            [pcmAnalysis analysisWithPCMData:blockData];
            
            i++;
            
            [NSThread sleepForTimeInterval:0.1];
        }
        
        //停止对pcm数据的处理
        [pcmAnalysis stopAnalysis];
        
        [testRecorder finish];
        
        while(!analysisFinishing)
        {
            usleep(50 * 1000);
        }
        
        USCLog(@"Finish");
    }
    else
    {
        USCLog(@"ReadPCMData -> PCMReading error");
        
        [self readingStart:Device_Audio_File_Error];
    }
}

// 停止读取文件
- (void) readingStop
{
    USCLog(@"ReadPCMData -> finish");
    
    if (isReading)
    {
        isReading = NO;
    }
}

-(void)readingStart:(ErrorType)code
{
    USCLog(@"ReadPCMData -> start : %d",code);
    if (_delegate != nil && [_delegate respondsToSelector:@selector(onReadPCMFileStart:)])
    {
        [_delegate onReadPCMFileStart:code];
    }
}

- (void) dealloc
{
    USCLog(@"ReadPCMData -> dealloc");
    
    isReading = NO;
    
    pcmAnalysis = nil;
}

#pragma mark -
#pragma mark UniVADDelegate

- (void)uniPCMAnalysisDidUpdataVolume:(int)volume
{
    if (_delegate && [_delegate respondsToSelector:@selector(onReadPCMUpdateVolume:)])
    {
        [_delegate onReadPCMUpdateVolume:volume];
    }
}

- (void)uniPCMAnalysisDidCheckBuffer:(NSData *)buffer
{
    if (_delegate && [_delegate respondsToSelector:@selector(onReadPCMData:)])
    {
        [_delegate onReadPCMData:buffer];
    }
}

-(void)uniPCMAnalysisDisStop
{
    USCLog(@"uniPCMAnalysisDisStop");
    
    if (_delegate && [_delegate respondsToSelector:@selector(onReadPCMFileStop:)])
    {
        [_delegate onReadPCMFileStop:No_Error];
    }
    
    analysisFinishing = YES;
}

-(void)uniPCMAnalysisIsTimeout
{
    
}

@end
