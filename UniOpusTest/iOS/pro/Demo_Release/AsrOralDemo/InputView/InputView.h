//
//  InputView.h
//  VoiceCubeSample
//
//  Created by 刘俊 on 14-5-15.
//  Copyright (c) 2014年 yunzhisheng. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "VoiceView.h"

@interface InputView : UIView<UITextFieldDelegate>
{
    UIButton *talkBtn;
    UIButton *switchBtn;
    UIView *inputBac;
    VoiceView *voiceView;
    
    BOOL isTalking;
    NSTimeInterval startTime;
    CONTROL_TYPE controlType;
}

@property (nonatomic)id delegate;
@property (nonatomic,strong)UIView *superView;
@property (nonatomic,strong)UITextField *inputField;
@property (nonatomic,assign)BOOL noKeyboard;//不显示键盘切换按钮

-(id)initWithDelegate:(id)dele;

-(void)addWidget;

-(void)updateVolume:(int)value;

-(void)setContolMode:(CONTROL_TYPE)mode;

-(void)setMicBtnEnable:(BOOL)condition;

-(void)micBtnReset;

@end

@protocol InputViewDelegate <NSObject>

-(void)inputStart;

-(void)inputWillCancle;

-(void)inputDidCancle;

-(void)inputDidLoading;

-(void)inputDidStop;

@end
