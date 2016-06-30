//
//  InputView_Plugin.m
//  AsrOralDemo
//
//  Created by 刘俊 on 15/1/7.
//  Copyright (c) 2015年 yunzhisheng. All rights reserved.
//

#import "InputView_Plugin.h"

@implementation InputView_Plugin

@synthesize delegate = _delegate;

-(id)initWithInputView:(UIView *)view
{
    if (self = [super init])
    {
        superView = view;
        [self appear];
    }
    
    return self;
}

-(void)appear
{
    CGRect superRect = superView.frame;

    if (nextButton==nil)
    {
        UIImage *nextImage = [UIImage imageNamed:@"btn_next_normal.png"];
        UIImage *nextImagePressed = [UIImage imageNamed:@"btn_next_pressed.png"];
        nextButton = [UIButton buttonWithType:UIButtonTypeCustom];
        [nextButton setImage:nextImage forState:UIControlStateNormal];
        [nextButton setImage:nextImagePressed forState:UIControlStateHighlighted];
        nextButton.frame = CGRectMake(superRect.size.width -60 -20,
                                      (superRect.size.height-60)/2, 60, 60);
        [nextButton addTarget:self
                       action:@selector(nextBtnPressed)
             forControlEvents:UIControlEventTouchUpInside];
    }
    
    if (preButton==nil)
    {
        UIImage *preImage = [UIImage imageNamed:@"btn_pre_normal.png"];
        UIImage *preImagePressed = [UIImage imageNamed:@"btn_pre_pressed.png"];
        preButton = [UIButton buttonWithType:UIButtonTypeCustom];
        [preButton setImage:preImage forState:UIControlStateNormal];
        [preButton setImage:preImagePressed forState:UIControlStateHighlighted];
        preButton.frame = CGRectMake(20, (superRect.size.height-60)/2, 60, 60);
        [preButton addTarget:self
                      action:@selector(preBtnPressed)
            forControlEvents:UIControlEventTouchUpInside];
    }

    [superView addSubview:nextButton];
    [superView addSubview:preButton];
}

-(void)disappear
{
    [nextButton removeFromSuperview];
    [preButton removeFromSuperview];
}

- (void)preBtnPressed
{
    if (_delegate!=nil&&[_delegate respondsToSelector:@selector(previousItem)])
    {
        [self.delegate previousItem];
    }
}

- (void)nextBtnPressed
{
    if (_delegate!=nil&&[_delegate respondsToSelector:@selector(nextItem)])
    {
        [self.delegate nextItem];
    }
}

@end
