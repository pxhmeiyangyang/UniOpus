//  InputView.m
//  VoiceCubeSample
//
//  Created by 刘俊 on 14-5-15.
//  Copyright (c) 2014年 yunzhisheng. All rights reserved.
//

#import "InputView.h"

#define DECREASE_ANIMATION if([[UIDevice currentDevice].systemVersion intValue]<7) {return;}

typedef NS_ENUM(NSInteger, InpuType)
{
    Voice_input,
    KeyBoard_input,
};

#define SwitchBtn_Width 40

@implementation InputView

@synthesize superView;
@synthesize delegate;
@synthesize inputField;
@synthesize noKeyboard;

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self)
    {
        // Initialization code
        

    }
    return self;
}

-(id)initWithDelegate:(id)dele
{
    if (self=[super init])
    {
        delegate=dele;
        noKeyboard = NO;
        controlType = CONTROL_TOUCH;
    }
    return self;
}

-(void)addWidget
{
    [self addTalkView];
    
    if (!noKeyboard)
    {
        [self addSwitchButton];
    }
}

-(void)setContolMode:(CONTROL_TYPE)mode
{
    controlType = mode;
}

-(void)setMicBtnEnable:(BOOL)condition
{
    [voiceView setEnable:condition];
}

-(void)micBtnReset
{
    [voiceView clipReset];
    [self disAppera];
}

#pragma BlurImage

-(void)appear
{
    [UIView animateWithDuration:0.3 animations:^{
        
        switchBtn.alpha = 0;
        
    } completion:^(BOOL finished) {
        
        if (isTalking)
        {
            [voiceView pointsAppear];
        }
    }];
}

-(void)disAppera
{
    [voiceView pointsDisAppear];
    
    [UIView animateWithDuration:0.3 animations:^{
        
        switchBtn.alpha = 1;
        
    } completion:^(BOOL finished) {
        
        [switchBtn removeFromSuperview];
        [self insertSubview:switchBtn atIndex:self.subviews.count];
    }];
}

-(void)addSwitchButton
{
    switchBtn=[UIButton buttonWithType:UIButtonTypeCustom];
    switchBtn.frame=CGRectMake(5,
                               (self.bounds.size.height-SwitchBtn_Width)/2,
                               SwitchBtn_Width,
                               SwitchBtn_Width);
    [switchBtn setImage:[UIImage imageNamed:@"keyBoard.png"] forState:UIControlStateNormal];
    [switchBtn setImage:[UIImage imageNamed:@"keyBoard_h.png"] forState:UIControlStateHighlighted];
    [switchBtn addTarget:self action:@selector(switchHandle:) forControlEvents:UIControlEventTouchUpInside];
    
    [self addSubview:switchBtn];
    
    //初始显示语音按钮
    switchBtn.tag=Voice_input;
}

-(void)addTalkView
{
    if (voiceView==nil)
    {
        voiceView=[[VoiceView alloc]initWithFrame:CGRectMake(0,
                                                             0,
                                                             self.bounds.size.width,
                                                             80)];
        //voiceView.backgroundColor=[UIColor grayColor];
        voiceView.delegate=self;
        voiceView.controlType = controlType;
        [self addSubview:voiceView];
        
        [voiceView setup];
    }
    else
    {
        [self addSubview:voiceView];
        voiceView.alpha=0;
    }
}

-(void)addTextField
{
    if (inputField==nil)
    {
        inputBac=[[UIView alloc]initWithFrame:CGRectMake(50,
                                                         0,
                                                         [UIScreen mainScreen].bounds.size.width-50,
                                                         self.frame.size.height)];
        
        //输入框
        inputField=[[UITextField alloc]initWithFrame:CGRectMake(0,
                                                                (inputBac.bounds.size.height-SwitchBtn_Width)/2,
                                                                220,
                                                                SwitchBtn_Width)];
        inputField.backgroundColor=[UIColor whiteColor];
        inputField.contentVerticalAlignment=UIControlContentVerticalAlignmentCenter;
        inputField.placeholder=@"  请输入您想说的话";
        inputField.delegate=self;
        
        UIImage *fieImage=[UIImage imageNamed:@"textBac.png"];
        inputField.background=[fieImage resizableImageWithCapInsets:UIEdgeInsetsMake(10, 10, 10, 10)];
        
        //完成按钮
        UIButton *finishBtn=[UIButton buttonWithType:UIButtonTypeCustom];
        finishBtn.frame=CGRectMake(230-5,
                                   (inputBac.bounds.size.height-SwitchBtn_Width)/2,
                                   SwitchBtn_Width,
                                   SwitchBtn_Width);
        [finishBtn setImage:[UIImage imageNamed:@"send.png"] forState:UIControlStateNormal];
        [finishBtn setImage:[UIImage imageNamed:@"send_h.png"] forState:UIControlStateHighlighted];
        [finishBtn addTarget:self
                      action:@selector(inputFinish:)
            forControlEvents:UIControlEventTouchUpInside];
        
        [inputBac addSubview:inputField];
        [inputBac addSubview:finishBtn];
    }

    inputBac.alpha=0;
    
    [self addSubview:inputBac];
    [inputField becomeFirstResponder];
}

-(void)switchHandle:(UIButton *)sender
{
    sender.userInteractionEnabled=NO;
    
    if (sender.tag==Voice_input)
    {
        sender.tag=KeyBoard_input;
        
        [UIView animateWithDuration:0.2 animations:^{
            voiceView.alpha=0;
            
            switchBtn.alpha=0;
            [switchBtn setImage:[UIImage imageNamed:@"voice.png"] forState:UIControlStateNormal];
            [switchBtn setImage:[UIImage imageNamed:@"voice_h.png"] forState:UIControlStateHighlighted];
        }
                         completion:^(BOOL finished)
         {
             [voiceView removeFromSuperview];
             [self addTextField];
             
             [UIView animateWithDuration:0.2 animations:^{
                 inputBac.alpha=1;
                 switchBtn.alpha=1;
             }
                              completion:^(BOOL finished)
              {
                  sender.userInteractionEnabled=YES;
              }];
         }];
    }
    else
    {
        sender.tag=Voice_input;
        
        [UIView animateWithDuration:0.2 animations:^{
            inputBac.alpha=0;
            
            switchBtn.alpha=0;
            [switchBtn setImage:[UIImage imageNamed:@"keyBoard.png"] forState:UIControlStateNormal];
            [switchBtn setImage:[UIImage imageNamed:@"keyBoard_h.png"] forState:UIControlStateHighlighted];
        }
                         completion:^(BOOL finished)
         {
             [inputBac removeFromSuperview];
             [self addTalkView];
             
             switchBtn.alpha=1;
             
             [UIView animateWithDuration:0.2 animations:^{
                 voiceView.alpha=1;
             }
                              completion:^(BOOL finished)
              {
                  sender.userInteractionEnabled=YES;
                  [switchBtn removeFromSuperview];
                  [self insertSubview:switchBtn atIndex:self.subviews.count];
              }];
         }];
    }
}

-(void)inputFinish:(UIButton *)sender
{
    [inputField resignFirstResponder];
}

-(void)updateVolume:(int)value
{
    [voiceView voiceChanging:value];
//    for (int i = 0; i<15; i++)
//    {
//        [voiceView voiceChanging:value];
//    }
}

#pragma mark --
#pragma mark InputViewDelegate

- (void)textFieldDidEndEditing:(UITextField *)textField
{
//    if (self.delegate&&[self.delegate respondsToSelector:@selector(fieldDidEndEditing:)])
//    {
//        [self.delegate fieldDidEndEditing:textField.text];
//    }
}

#pragma mark --
#pragma mark VoiceViewDelgate

-(void)voiceStart
{
    [self appear];
    
    isTalking = YES;
    startTime = [[NSDate date] timeIntervalSince1970];
    
    //NSLog(@"voiceWillStart");
    if (self.delegate&&[self.delegate respondsToSelector:@selector(inputStart)])
    {
        [self.delegate inputStart];
    }
}

-(void)voiceWillCancle
{
    //NSLog(@"voiceWillCancle");
    
    if (self.delegate&&[self.delegate respondsToSelector:@selector(inputWillCancle)])
    {
        [self.delegate inputWillCancle];
    }
}

-(void)voiceDidCancle
{
    [self disAppera];
    
    isTalking = NO;
    
    //NSLog(@"voiceDidCancle");
    if (self.delegate&&[self.delegate respondsToSelector:@selector(inputDidCancle)])
    {
        [self.delegate inputDidCancle];
    }
}

-(void)voiceDidStop
{
    [self disAppera];
    
    isTalking = NO;
    
    if (controlType == CONTROL_TOUCH)
    {
        NSTimeInterval stopTime = [[NSDate date] timeIntervalSince1970];
        double diff = stopTime - startTime;
        if (diff <= 0.2)
        {
            if (self.delegate&&[self.delegate respondsToSelector:@selector(inputDidCancle)])
            {
                [self.delegate inputDidCancle];
            }
        }
        else
        {
            if (self.delegate&&[self.delegate respondsToSelector:@selector(inputDidStop)])
            {
                [self.delegate inputDidStop];
            }
        }
    }
    else
    {
        if (self.delegate&&[self.delegate respondsToSelector:@selector(inputDidStop)])
        {
            [self.delegate inputDidStop];
        }
    }
}

-(void)voiceDidLoading
{
    //NSLog(@"voiceDidLoading");
    if (self.delegate&&[self.delegate respondsToSelector:@selector(inputDidLoading)])
    {
        [self.delegate inputDidLoading];
    }
}

/*
// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect
{
    // Drawing code
}
*/

@end
