//
//  VoiceView.h
//  Test
//
//  Created by 刘俊 on 14-5-21.
//  Copyright (c) 2014年 yunzhisheng. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "LightPoint.h"
#import "MicButton.h"

typedef NS_ENUM(NSInteger, CONTROL_TYPE)
{
    CONTROL_TOUCH,                         //长按(默认)
    CONTROL_CLIP,                          //点击
};

@interface VoiceView : UIView
{
    //UIImageView *micBtn;
    //CAShapeLayer *maskLayer;
    CAShapeLayer *processLayer;
    
    NSMutableArray *pointArray;
    
    MicButton *micBtn;
    CLIP_TYPE clip_type;
    
    UIImageView *circleImage;
    
    BOOL callBack;
    
    //音量条配置参数
    int HoleRadius;
    int MicRadius;
    int ProcessLineWith;
    int PointRadius;
    int Colum_number;
    int Row_number;
}

@property (nonatomic,assign)id delegate;
@property (nonatomic,assign)CONTROL_TYPE controlType;

-(void)setup;
-(void)talkWillStart;
//-(void)talkDidEnd;
-(void)loadingProcessing;

-(void)pointsAppear;
-(void)pointsDisAppear;

//音量控制
-(void)voiceChanging:(int)process;

//禁用
-(void)setEnable:(BOOL)condition;
-(void)clipReset;

@end

@protocol VoiceViewDelegate <NSObject>

-(void)voiceStart;

-(void)voiceWillCancle;

-(void)voiceDidCancle;

-(void)voiceDidLoading;

-(void)voiceDidStop;

-(void)voiceDidChanged;

@end
