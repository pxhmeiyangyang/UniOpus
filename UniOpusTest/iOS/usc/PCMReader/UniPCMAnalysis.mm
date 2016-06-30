//
//  UniPCMAnalysis.m
//  usc
//
//  Created by 刘俊 on 15/10/29.
//  Copyright © 2015年 yunzhisheng. All rights reserved.
//

#import "UniPCMAnalysis.h"

@implementation UniPCMAnalysis

-(id)init
{
    if (self == [super init])
    {
        audioDataArray = [[NSMutableArray alloc]init];
        analysisQueue = dispatch_queue_create("cn.Unisound.PCMAnalysis", nil);
        
        asrVad = new ASRVadWapper();
        asrVad->init();
        
        _VADEnable = NO;
        VADHasTimeout = NO;
    }
    return self;
}

-(void)stopAnalysis
{
    dispatch_async(analysisQueue, ^{
        
        USCLog(@"analysis -> stopAnalysis");
        
        if (_delegate && [_delegate respondsToSelector:@selector(uniPCMAnalysisDisStop)])
        {
            [_delegate uniPCMAnalysisDisStop];
        }
    });
}

-(void)cancelAnalysis
{
    dispatch_suspend(analysisQueue);
}

-(void)analysisReset
{
    asrVad->reset();
    VADHasTimeout = NO;
}

-(void)setVadFront:(int)frontTime Back:(int)backTime;
{
    _VADEnable = YES;
    asrVad->setVadTimeout(frontTime, backTime);
}

static int getEng(const short* psDataIn, int len, int* energy)
{
    int i;
    float fEng = 0, C = 0;
    *energy = 0;
    for (i = 0; i<len; i++)
    {
        C += psDataIn[i];
        fEng += psDataIn[i] * psDataIn[i];
    }
    fEng = fEng / len - (C / len) * (C / len);
    *energy = (int)(powf(fEng, 0.2f) * 2);
    *energy = MAX(*energy, 0);
    *energy = MIN(*energy, 100);
    
    return 0;
}

-(void)analysisWithPCMData:(NSData *)data
{
    USCLog(@"analysis -> _VADEnable : %d | VADHasTimeout : %d",_VADEnable, VADHasTimeout);

    //如果开启了VAD同时触发了VAD超时，则不再接受音频输入
    if (_VADEnable && VADHasTimeout)
    {
        return ;
    }
    
    NSData *audioData = [[NSData alloc]initWithData:data];
    dispatch_async(analysisQueue, ^{
        
        int _pcmSize = (int)audioData.length;
        char *_pcmData = (char *)audioData.bytes;
        
        //获取音量
        int energy = 0;
        getEng((const short *)audioData.bytes, (_pcmSize)/2, &energy);
        
        if (_VADEnable)
        {
            int volume = 0;
            int isTimeout = asrVad->isVadTimeout(_pcmData, _pcmSize, &volume);
            
            USCLog(@"analysis -> isTimeout : %d | vadHasTimeout : %d",isTimeout, VADHasTimeout);
            
            if (_delegate
                && [_delegate respondsToSelector:@selector(uniPCMAnalysisIsTimeout)]
                && isTimeout == VADTimeout
                && !VADHasTimeout)
            {
                USCLog(@"analysis -> uniPCMAnalysisIsTimeout");
                
                [_delegate uniPCMAnalysisIsTimeout];
                
                VADHasTimeout = YES;
            }
        }
        
        if (_delegate
            &&[_delegate respondsToSelector:@selector(uniPCMAnalysisDidUpdataVolume:)]
            &&[_delegate respondsToSelector:@selector(uniPCMAnalysisDidCheckBuffer:)])
        {
            [_delegate uniPCMAnalysisDidUpdataVolume:energy];
            [_delegate uniPCMAnalysisDidCheckBuffer:audioData];
        }
    });
}

-(void)dealloc
{
    USCLog(@"PCMAnalysis -> dealloc");
    //dispatch_suspend(analysisQueue);
    analysisQueue = nil;
    
    delete asrVad;
    asrVad = NULL;
}

@end
