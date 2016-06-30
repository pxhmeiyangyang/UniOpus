//
//  MicButton.m
//  ButtonTest
//
//  Created by 刘俊 on 14-8-20.
//  Copyright (c) 2014年 yunzhisheng. All rights reserved.
//

#import "MicButton.h"

@implementation MicButton

@synthesize delegate;

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code
    }
    return self;
}

-(void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    [super touchesBegan:touches withEvent:event];
    
    if ([self.delegate respondsToSelector:@selector(micBtnTouchStart)])
    {
        [self.delegate micBtnTouchStart];
    }
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    CGPoint poi = [[touches anyObject]locationInView:self];
    if (poi.x>0
        &&poi.y>0
        &&poi.x<self.frame.size.width
        &&poi.y<self.frame.size.height)
    {
        if (currentType==TalkingLoading)
        {
            return;
        }
        currentType = TalkingLoading;
        
        if ([self.delegate respondsToSelector:@selector(micBtnTouchLoading)])
        {
            [self.delegate micBtnTouchLoading];
        }
    }
    else
    {
        if (poi.y<0)//上滑取消
        {
            if (currentType==TalkingWillCancle)
            {
                return;
            }
            currentType = TalkingWillCancle;
            
            if ([self.delegate respondsToSelector:@selector(micBtnTouchWillCancle)])
            {
                [self.delegate micBtnTouchWillCancle];
            }
        }
    }
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    [super touchesEnded:touches withEvent:event];
    
    CGPoint poi = [[touches anyObject]locationInView:self];
    //NSLog(@"%@",NSStringFromCGPoint(poi));
    
    if (poi.y<0)
    {
        if ([self.delegate respondsToSelector:@selector(micBtnTouchCancle)])
        {
            [self.delegate micBtnTouchCancle];
        }
    }
    else
    {
        if ([self.delegate respondsToSelector:@selector(micBtnToucnEnd)])
        {
            [self.delegate micBtnToucnEnd];
        }
    }
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
    [super touchesCancelled:touches withEvent:event];
    
    if ([self.delegate respondsToSelector:@selector(micBtnTouchCancle)])
    {
        [self.delegate micBtnTouchCancle];
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
