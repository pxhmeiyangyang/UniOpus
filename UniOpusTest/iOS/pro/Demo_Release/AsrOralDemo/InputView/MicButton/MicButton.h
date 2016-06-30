//
//  MicButton.h
//  ButtonTest
//
//  Created by 刘俊 on 14-8-20.
//  Copyright (c) 2014年 yunzhisheng. All rights reserved.
//

#import <UIKit/UIKit.h>

typedef NS_ENUM(NSInteger, TalkingConditionType)
{
    TalkingStart = 0,
    TalkingLoading,
    TalkingCancle,
    TalkingWillCancle,
    TalkingEnd,
};

typedef NS_ENUM(NSInteger, CLIP_TYPE)
{
    CLIP_START,                         //开始
    CLIP_FINISH,                        //结束
};

@interface MicButton : UIButton
{
    TalkingConditionType currentType;
}

@property (nonatomic)id delegate;

@end

@protocol MicButtonDelegate <NSObject>

-(void)micBtnTouchStart;
-(void)micBtnToucnEnd;
-(void)micBtnTouchWillCancle;
-(void)micBtnTouchLoading;
-(void)micBtnTouchCancle;

@end
