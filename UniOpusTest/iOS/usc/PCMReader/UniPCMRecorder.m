//
//  UniPCMRecorder.m
//  usc
//
//  Created by 刘俊 on 15/11/24.
//  Copyright © 2015年 yunzhisheng. All rights reserved.
//

#import "UniPCMRecorder.h"

@implementation UniPCMRecorder

+(UniPCMRecorder *)defaultRecorder
{
    static UniPCMRecorder *recorder;
    static dispatch_once_t once;
    dispatch_once(&once, ^{
       
        recorder = [[UniPCMRecorder alloc]init];
    });
    
    return recorder;
}

-(id)init
{
    if (self = [super init])
    {
        if (USC_BUG)
        {
            recordData = [[NSMutableData alloc] init];
            progressName = @"pcmData";
        }
    }
    return self;
}

-(void)start:(NSString *)name
{
    if (USC_BUG)
    {
        _totalLength = 0;
        progressName = name;
        [recordData resetBytesInRange:NSMakeRange(0, recordData.length)];
        [recordData setLength:0];
    }
}

-(void)appendData:(NSData *)data
{
    if (USC_BUG)
    {
        NSData *cacheData = [data mutableCopy];
        [recordData appendData:cacheData];
    }
}

-(void)finish
{
    if (USC_BUG)
    {
        if (recordData!=nil)
        {
            NSString *fileName = [NSString stringWithFormat:@"%@.pcm",progressName];
            NSString *path = [ NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) lastObject];
            NSString *pcmPath = [path stringByAppendingPathComponent:fileName];
            
            BOOL result = [recordData writeToFile:pcmPath atomically:YES];
            if (result)
            {
                NSLog(@"WriteToFile Sucessfully");
            }
        }
        else
        {
            NSLog(@"NULL");
        }
    }
}

@end
