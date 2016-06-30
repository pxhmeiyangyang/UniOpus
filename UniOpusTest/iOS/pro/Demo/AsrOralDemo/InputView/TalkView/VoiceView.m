//
//  VoiceView.m
//  Test
//
//  Created by 刘俊 on 14-5-21.
//  Copyright (c) 2014年 yunzhisheng. All rights reserved.
//
/*
#define HoleRadius 30
#define MicRadius 40
#define ProcessLineWith 8
#define PointRadius 1
#define Colum_number 28
#define Row_number 15
 */

/*
    #define HoleRadius 30
    #define MicRadius 40
    #define ProcessLineWith 8
    #define PointRadius 2
    #define Colum_number 15
    #define Row_number 4
*/

#define DECREASE_ANIMATION [[UIDevice currentDevice].systemVersion intValue]<7
#define micImage [UIImage imageNamed:@"mic"]

#import "VoiceView.h"
#import "LightPoint.h"

@implementation VoiceView

@synthesize controlType;

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self)
    {
        controlType = CONTROL_TOUCH;
        clip_type = CLIP_FINISH;
        callBack = YES;
    }
    return self;
}

-(void)setup
{
    if (DECREASE_ANIMATION)
    {
        HoleRadius = 30;
        MicRadius = 40;
        ProcessLineWith = 8;
        PointRadius = 2;
        Colum_number = 15;
        Row_number = 4;
    }
    else
    {
        HoleRadius = 30;
        MicRadius = 40;
        ProcessLineWith = 8;
        PointRadius = 1;
        Colum_number = 36;
        Row_number = 13;
    }
    
    [self addMicView];
}

-(void)addMicView
{
    if (micBtn==nil)
    {
        micBtn=[[MicButton alloc]initWithFrame:CGRectMake((self.frame.size.width - MicRadius*2)/2,
                                                         0,
                                                         MicRadius*2,
                                                         MicRadius*2)];
        [micBtn setBackgroundImage:micImage forState:UIControlStateNormal];
        [micBtn setBackgroundImage:micImage forState:UIControlStateHighlighted];
        micBtn.delegate = self;
        [self addSubview:micBtn];
    }
    
    //添加遮罩层
    CAShapeLayer *maskLayer=[CAShapeLayer layer];
    CGRect rect=CGRectMake(micBtn.frame.size.width/2-HoleRadius,
                           micBtn.frame.size.height/2-HoleRadius,
                           HoleRadius*2, HoleRadius*2);
    
    UIBezierPath *circlePath=[UIBezierPath bezierPathWithRoundedRect:rect cornerRadius:HoleRadius];
    maskLayer.path = circlePath.CGPath;
    
    CGRect pathRect = CGRectMake(CGPointZero.x,
                                 CGPointZero.y,
                                 CGRectGetWidth(micBtn.bounds),
                                 CGRectGetHeight(micBtn.bounds));
    maskLayer.bounds = pathRect;
    maskLayer.position = CGPointMake(CGRectGetMidX(pathRect), CGRectGetMidY(pathRect));
    
    micBtn.layer.mask=nil;
    micBtn.layer.mask=maskLayer;
}

-(void)loadingProcessing
{
    [self addProcessLayer];
    
    CABasicAnimation *animation = [CABasicAnimation animationWithKeyPath:@"strokeEnd"];
    animation.fromValue = [NSNumber numberWithFloat:0.0f];
    animation.toValue = [NSNumber numberWithFloat:1.0f];
    animation.duration = 1.0f;
    animation.repeatCount=3;
    [processLayer addAnimation:animation forKey:@"myStroke"];
}

-(void)addProcessLayer
{
    if (processLayer==nil)
    {
        //背景
        CAShapeLayer *processBacLayer=[CAShapeLayer layer];
        CGPoint centerPoi=CGPointMake(self.frame.size.width/2, self.frame.size.height/2);
        UIBezierPath *processPath=[UIBezierPath bezierPathWithArcCenter:centerPoi
                                                                 radius:MicRadius-ProcessLineWith+4
                                                             startAngle:-M_PI_2
                                                               endAngle:M_PI*2
                                                              clockwise:YES];
        processBacLayer.path=processPath.CGPath;
        processBacLayer.fillColor=nil;
        processBacLayer.strokeColor=[UIColor whiteColor].CGColor;
        processBacLayer.lineCap =kCALineCapButt;
        processBacLayer.lineWidth=ProcessLineWith;
        [self.layer addSublayer:processBacLayer];
        
        //旋转部分
        processLayer=[CAShapeLayer layer];
        processLayer.path=processPath.CGPath;
        processLayer.fillColor=nil;
        processLayer.strokeColor=[UIColor colorWithRed:(CGFloat)169/255
                                                 green:(CGFloat)240/255
                                                  blue:(CGFloat)227/255
                                                 alpha:.8].CGColor;
        processLayer.lineWidth=ProcessLineWith;
        [self.layer addSublayer:processLayer];
    }
}

-(void)addCircle
{
    if (circleImage==nil)
    {
        circleImage = [[UIImageView alloc]initWithFrame:CGRectMake(0, 0, micBtn.frame.size.height, micBtn.frame.size.width)];
        [circleImage setImage:[UIImage imageNamed:@"circle"]];
    }
    
    [micBtn addSubview:circleImage];
    
    CAShapeLayer *processBacLayer=[CAShapeLayer layer];
    CGPoint centerPoi=CGPointMake(circleImage.frame.size.width/2,
                                  circleImage.frame.size.height/2);
    UIBezierPath *processPath=[UIBezierPath
                               bezierPathWithArcCenter:centerPoi
                               radius:40-8+4
                               startAngle:-M_PI_2
                               endAngle:M_PI*2
                               clockwise:YES];
    
    processBacLayer.path=processPath.CGPath;
    processBacLayer.fillColor = nil;
    processBacLayer.strokeColor = [UIColor whiteColor].CGColor;
    processBacLayer.lineCap = kCALineCapSquare;
    processBacLayer.lineWidth = 14;
    circleImage.layer.mask = processBacLayer;
}

-(void)startHandle
{
    [self talkWillStart];
    [self addCircle];
    [self circleRotate];
}

-(void)stopHandle
{
    [self talkDidEnd];
    [circleImage removeFromSuperview];
}

#pragma mark ---
#pragma mark micBtn Delegate

-(void)micBtnTouchStart
{
    //NSLog(@"micBtnTouchStart");
    
    if (controlType == CONTROL_TOUCH)
    {
        [self startHandle];
        
        if (self.delegate&&[self.delegate respondsToSelector:@selector(voiceStart)])
        {
            [self.delegate voiceStart];
        }
    }
    else
    {
        if (callBack)
        {
            if (clip_type == CLIP_FINISH)
            {
                [self startHandle];
                
                clip_type = CLIP_START;
                
                if (self.delegate&&[self.delegate respondsToSelector:@selector(voiceStart)])
                {
                    [self.delegate voiceStart];
                }
            }
            else
            {
                [self stopHandle];
                
                clip_type = CLIP_FINISH;
                
                if (self.delegate&&[self.delegate respondsToSelector:@selector(voiceDidStop)])
                {
                    [self.delegate voiceDidStop];
                }
            }
        }
    }
}

-(void)micBtnToucnEnd
{
    //NSLog(@"micBtnToucnEnd");
    
    if (controlType == CONTROL_TOUCH)
    {
        [self talkDidEnd];
        [circleImage removeFromSuperview];
        
        if (self.delegate&&[self.delegate respondsToSelector:@selector(voiceDidStop)])
        {
            [self.delegate voiceDidStop];
        }
    }
}

-(void)micBtnTouchWillCancle
{
    //NSLog(@"micBtnTouchWillCancle");
    
    if (controlType == CONTROL_TOUCH)
    {
        if (self.delegate&&[self.delegate respondsToSelector:@selector(voiceWillCancle)])
        {
            [self.delegate voiceWillCancle];
        }
    }
}

-(void)micBtnTouchLoading
{
    //NSLog(@"micBtnTouchLoading");
    
    if (controlType == CONTROL_TOUCH)
    {
        if (self.delegate&&[self.delegate respondsToSelector:@selector(voiceDidLoading)])
        {
            [self.delegate voiceDidLoading];
        }
    }
}

-(void)micBtnTouchCancle
{
    //NSLog(@"micBtnTouchCancle");
    
    if (controlType == CONTROL_TOUCH)
    {
        [self talkDidEnd];
        
        [circleImage removeFromSuperview];
        
        if (self.delegate&&[self.delegate respondsToSelector:@selector(voiceDidCancle)])
        {
            [self.delegate voiceDidCancle];
        }
    }
}

#pragma mark -
#pragma mark PointLayer

-(void)addPoints
{
    if (pointArray==nil)
    {
        pointArray=[[NSMutableArray alloc]init];
    }
    
    //左侧
    int poi_space = PointRadius * 2 + 1;
    int all_poi_length = poi_space * Colum_number;
    float location_up = (self.bounds.size.height - poi_space * Row_number)/2;
    float location_left = self.bounds.size.width/2 - all_poi_length - MicRadius - 4;
    for (int i=0; i<Colum_number; i++)
    {
        for (int j=0; j<Row_number; j++)
        {
            LightPoint *poi=[LightPoint layer];
            poi.frame=CGRectMake(location_left+poi_space*i,
                                 location_up+poi_space*j,
                                 PointRadius*2,
                                 PointRadius*2);
            poi.tag=i;
            poi.type=Left_point;
            [poi disappear];
            [self.layer addSublayer:poi];
            
            [pointArray addObject:poi];
        }
    }
    
    //右侧
    float location_right=self.bounds.size.width/2+MicRadius + 4;
    for (int i=0; i<Colum_number; i++)
    {
        for (int j=0; j<Row_number; j++)
        {
            LightPoint *poi=[LightPoint layer];
            poi.frame=CGRectMake(location_right+poi_space*i,
                                 location_up+poi_space*j,
                                 PointRadius*2,
                                 PointRadius*2);
            poi.tag=i;
            poi.type=Right_point;
            [poi disappear];
            [self.layer addSublayer:poi];
            
            [pointArray addObject:poi];
        }
    }
}

-(void)pointsAppear
{
    [self addPoints];
}

-(void)pointsDisAppear
{
    [self removeAllPoint];
}

-(void)removeAllPoint
{
    if (pointArray.count!=0)
    {
        for (LightPoint *poi in pointArray)
        {
            [poi removeFromSuperlayer];
        }
    }
    
    [pointArray removeAllObjects];
}

-(void)voiceChanging:(int)process
{
    float formatVlaue=(float)process/100;
    
    [self pointLoading:formatVlaue];
}

//禁用
-(void)setEnable:(BOOL)condition
{
    callBack = condition;
}

-(void)clipReset
{
    [self stopHandle];
    
    clip_type = CLIP_FINISH;
}

-(void)pointLoading:(float)process
{
    int scale= (int) Colum_number * process;
    
    if (scale>Colum_number)
    {
        return;
    }
        
    for (LightPoint *poi in pointArray)
    {
        if ((poi.tag>Colum_number-1-scale&&poi.type == Left_point)
            ||(poi.tag<scale&&poi.type==Right_point))
        {
            [poi appear];
        }
        else
        {
            [poi disappear];
        }
    }
}

#pragma mark -
#pragma mark Animation

-(void)circleRotate
{
    CABasicAnimation* rotationAnimation;
    rotationAnimation = [CABasicAnimation animationWithKeyPath:@"transform.rotation.z"];
    rotationAnimation.toValue = [NSNumber numberWithFloat: - M_PI * 2.0 ];
    rotationAnimation.duration = 1.5;
    rotationAnimation.cumulative = NO;
    rotationAnimation.repeatCount = 9999;
    //rotationAnimation.removedOnCompletion = YES;
    [circleImage.layer addAnimation:rotationAnimation forKey:@"rotationAnimation"];
}

-(void)talkWillStart
{
    CABasicAnimation *scaleAnimation = [CABasicAnimation animationWithKeyPath:@"transform.scale"];
    scaleAnimation.delegate=self;
    scaleAnimation.toValue = @(1.3);
    scaleAnimation.duration = 0.15f;
    scaleAnimation.removedOnCompletion = NO;
    scaleAnimation.fillMode = kCAFillModeForwards;
    scaleAnimation.timingFunction=[CAMediaTimingFunction
                                   functionWithName:kCAMediaTimingFunctionEaseInEaseOut];
    [micBtn.layer.mask addAnimation:scaleAnimation forKey:@"maskIncrease"];
}

//未能确定遮罩层变大后影响的范围，暂时不用
//http://wonderffee.github.io/blog/2013/10/13/understand-anchorpoint-and-position/
-(void)talkDidEnd
{
    CABasicAnimation *scaleAnimation = [CABasicAnimation animationWithKeyPath:@"transform.scale"];
    scaleAnimation.fromValue = @(1.3);
    scaleAnimation.toValue = @(1);
    scaleAnimation.duration = 0.15f;
    scaleAnimation.removedOnCompletion = NO;
    scaleAnimation.fillMode = kCAFillModeForwards;
    scaleAnimation.timingFunction=[CAMediaTimingFunction
                                   functionWithName:kCAMediaTimingFunctionEaseInEaseOut];
    [micBtn.layer.mask addAnimation:scaleAnimation forKey:@"maskReduce"];
}

#pragma mark CABasicAnimation

- (void)animationDidStop:(CAAnimation *)anim finished:(BOOL)flag
{
    //[maskLayer removeFromSuperlayer];
    //[self loadingProcessing];
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
