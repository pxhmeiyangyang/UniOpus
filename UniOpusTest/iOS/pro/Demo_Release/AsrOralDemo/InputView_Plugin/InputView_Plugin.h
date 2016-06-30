//
//  InputView_Plugin.h
//  AsrOralDemo
//
//  Created by 刘俊 on 15/1/7.
//  Copyright (c) 2015年 yunzhisheng. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

@interface InputView_Plugin : NSObject
{
    UIView *superView;
    
    UIButton *nextButton;
    UIButton *preButton;
}

@property (nonatomic,assign)id delegate;

-(id)initWithInputView:(UIView *)view;

-(void)appear;
-(void)disappear;

@end

@protocol InputView_PluginDelegate <NSObject>

-(void)nextItem;
-(void)previousItem;

@end
