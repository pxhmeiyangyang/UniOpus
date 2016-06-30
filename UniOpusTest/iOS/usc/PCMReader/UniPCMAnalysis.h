//
//  UniPCMAnalysis.h
//  usc
//
//  Created by 刘俊 on 15/10/29.
//  Copyright © 2015年 yunzhisheng. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "ASRVadWapper.h"
#import "USCMarcos.h"

@protocol UniPCMAnalysisDelegate <NSObject>

- (void)uniPCMAnalysisDisStop;
- (void)uniPCMAnalysisDidUpdataVolume:(int)volume;
- (void)uniPCMAnalysisIsTimeout;
- (void)uniPCMAnalysisDidCheckBuffer:(NSData *)buffer;

@end;

@interface UniPCMAnalysis : NSObject
{
    NSMutableArray *audioDataArray;
    dispatch_queue_t analysisQueue;

    ASRVadWapper *asrVad;
    BOOL VADHasTimeout;
}

@property (nonatomic, assign)id<UniPCMAnalysisDelegate> delegate;
@property (nonatomic, assign)BOOL VADEnable;

-(void)setVadFront:(int)frontTime Back:(int)backTime;

-(void)stopAnalysis;
-(void)cancelAnalysis;

-(void)analysisReset;
-(void)analysisWithPCMData:(NSData *)data;

@end
