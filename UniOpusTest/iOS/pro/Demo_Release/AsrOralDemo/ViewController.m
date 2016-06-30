//
//  ViewController.m
//  AsrOralDemo
//
//  Created by 刘俊 on 15/1/7.
//  Copyright (c) 2015年 yunzhisheng. All rights reserved.
//

#import "ViewController.h"
#import "SettingController.h"

#import "InputView.h"
#import "InputView_Plugin.h"

#import "PCMPlayer.h"
#import "MP3Player.h"

#define VALIDCOLOR          [UIColor colorWithRed:(CGFloat)32/255 green:(CGFloat)153/255 blue:(CGFloat)249/255 alpha:1]
#define VALIDCOLOR_H        [UIColor colorWithRed:(CGFloat)246/255 green:(CGFloat)136/255 blue:(CGFloat)38/255 alpha:1]
#define INVALIDCOLOR        [UIColor grayColor]
#define GREEN               [UIColor colorWithRed:(CGFloat)36/255 green:(CGFloat)163/255 blue:(CGFloat)146/255 alpha:1];

@interface ViewController ()<UIAlertViewDelegate,InputViewDelegate>
{
    InputView *_inputView;
    InputView_Plugin *inputView_Plugin;
    UIButton *playback;
    UIActivityIndicatorView *waitingView;
    IBOutlet UITextView *textView;
    IBOutlet UITextView *protocalView;
    IBOutlet UILabel *noticeLabel;
    
    NSArray *sample;
    int index;
    
    MP3Player *mp3;
    BOOL isPlaying;
    NSError *engineError;
    
    NSString* audioFileUrl;
    NSMutableData* bufferData;
}

@end

@implementation ViewController

-(void)addInputView
{
    if (_inputView == nil)
    {
        CGRect frame = CGRectMake(0, self.view.frame.size.height - 90, self.view.frame.size.width, 80);
        _inputView = [[InputView alloc]initWithFrame:frame];
        
        [self.view addSubview:_inputView];
    }
    
    _inputView.delegate           = self;
    _inputView.superView          = self.view;
    _inputView.noKeyboard         = YES;
    [_inputView setContolMode:CONTROL_CLIP];
    _inputView.backgroundColor=[UIColor colorWithRed:(CGFloat)242/255 green:(CGFloat)242/255 blue:(CGFloat)242/255 alpha:1];
    _inputView.layer.shadowColor=[UIColor grayColor].CGColor;
    _inputView.layer.shadowOffset = CGSizeMake(0, 0.6);
    _inputView.layer.shadowOpacity=.2;
    [_inputView addWidget];

    inputView_Plugin              = [[InputView_Plugin alloc]initWithInputView:_inputView];
    inputView_Plugin.delegate     = self;
}

-(void)addWaitingView
{
    // 初始化等待指示器
    waitingView = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleWhiteLarge];
    [self.view addSubview:waitingView];
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    [self addInputView];
    
    [self addWaitingView];
    
    [self initData];
    
    [self loadData];
    
    [self initEngineManager];
}

-(void)initData{
    isPlaying = NO;
    audioFileUrl = @"";
}

-(void)viewWillLayoutSubviews
{
    [self setFrame];
}

-(void)initEngineManager
{
    enginer = [EngineManager sharedManager];
    enginer.delegate = self;
}

- (void)setFrame
{
    CGRect textViewFrame = textView.frame;
    CGRect protocalFrame =  protocalView.frame;
    CGRect screenSize = [UIScreen mainScreen].bounds;
    
    //
    textViewFrame.origin.y = screenSize.size.height - _inputView.frame.size.height - textView.frame.size.height - 20;
    textViewFrame.size.width = screenSize.size.width - 20;
    textView.frame = textViewFrame;
    
    //
    protocalFrame.size.width  = screenSize.size.width - 20;
    protocalFrame.size.height = screenSize.size.height
                                - protocalFrame.origin.y
                                - (10 + _inputView.frame.size.height)
                                - (15 + textView.frame.size.height);
    
    protocalView.frame = protocalFrame;
    
    //
    waitingView.center = CGPointMake(screenSize.size.width/2, screenSize.size.height/2 - textView.frame.size.height/2);
}

- (void)playBackBtnVisible
{
    [playback setTitleColor:VALIDCOLOR forState:UIControlStateNormal];
    playback.alpha = 1;
    playback.userInteractionEnabled = YES;
}

-(void)playBackBtnInvisible
{
    [playback setTitleColor:INVALIDCOLOR forState:UIControlStateNormal];
    playback.alpha = 0.4;
    playback.userInteractionEnabled = NO;
}

- (void)currentCondition:(NSString *)title error:(BOOL)isError finish:(BOOL)willFinish
{
    //protocalView.text = nil;
    noticeLabel.text = title;
    if (isError)
    {
        noticeLabel.textColor = [UIColor redColor];
    }
    else
    {
        noticeLabel.textColor = [UIColor blackColor];
    }
    
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(recognizerEnd) object:nil];
    if (willFinish)
    {
        [self performSelector:@selector(recognizerEnd) withObject:nil afterDelay:1];
    }
}

-(void)recognizerEnd
{
    noticeLabel.text = @"请点击麦克风开始录音";
    noticeLabel.textColor = [UIColor blackColor];
    [waitingView stopAnimating];
}

-(void)resetMicBtnCondition:(NSError *)error
{
    [self currentCondition:error.domain error:YES finish:YES];
    
    [_inputView setMicBtnEnable:YES];
    
    [_inputView micBtnReset];
    
    [waitingView stopAnimating];
    
    [inputView_Plugin appear];
}

-(IBAction)playRecordingFile
{
    if (!isPlaying)
    {
        isPlaying = YES;

        [self play];
    }
}

-(NSString *)jsonPrint:(NSString *)originalResult
{
    NSData *originalData = [originalResult dataUsingEncoding:NSUTF8StringEncoding];
    id objc = [NSJSONSerialization JSONObjectWithData:originalData options:NSJSONReadingAllowFragments error:nil];
    if (objc != nil)
    {
        NSData *formatData = [NSJSONSerialization dataWithJSONObject:objc options:NSJSONWritingPrettyPrinted error:nil];
        NSString *jsonFormatString = [[NSString alloc]initWithData:formatData encoding:NSUTF8StringEncoding];
        
        return jsonFormatString;
    }
    
    return nil;
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark -
#pragma mark MP3Player

-(void)play
{
//    NSArray *paths = NSSearchPathForDirectoriesInDomains( NSDocumentDirectory, NSUserDomainMask, YES);
//    NSString *documentsDirectory = [paths objectAtIndex:0];
//    NSString *recordingPath = [documentsDirectory stringByAppendingPathComponent:FILE_NAME_RECORDING];
//    if ([[NSFileManager defaultManager] fileExistsAtPath:recordingPath])
//    {
//        NSLog(@"Play Start");
//        if (mp3 == nil)
//        {
//            mp3 = [[MP3Player alloc]init];
//            mp3.delegate = self;
//        }
//        [mp3 playWithFile:recordingPath];
//        [mp3 play];
//    }
    if (audioFileUrl.length > 0) {
        if (mp3 == nil) {
            mp3 = [[MP3Player alloc]init];
            mp3.delegate = self;
        }
        [mp3 playWithUrl:audioFileUrl];
        [mp3 play];
    }
}

#pragma mark
#pragma mark UIAlertViewDelegate

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex
{
    [self resetMicBtnCondition:engineError];
}

#pragma mark
#pragma mark PCMPlayerDelegate

-(void)playFinished:(NSError *)error
{
    NSLog(@"play Finished : %@",error);
    
    if (error==nil)
    {
        [self currentCondition:@"回放结束" error:NO finish:YES];
    }
    
    isPlaying = NO;
}

#pragma mark - InputView_PluginDelegate

-(void)nextItem
{
    if (index + 1 < sample.count)
    {
        textView.text = sample[++index];
        protocalView.text = @"";
        
        [self playBackBtnInvisible];
    }
}

-(void)previousItem
{
    if (index - 1 >= 0)
    {
        textView.text = sample[--index];
        protocalView.text = @"";
        
        [self playBackBtnInvisible];
    }
}

#pragma mark - InputViewDelegate

-(void)inputStart
{
    [inputView_Plugin disappear];
    [self startRecognize];
}

-(void)inputWillCancle
{
    
}

-(void)inputDidCancle
{
    [self cancelRecognize];
}

-(void)inputDidLoading
{
    
}

-(void)inputDidStop
{
    [_inputView setMicBtnEnable:NO];

    [inputView_Plugin appear];
    [self stopRecognize];
}

#pragma mark - AsrOral

-(NSArray *)loadTestSentence
{
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *document = paths[0];
    NSString *txtPath = [document stringByAppendingPathComponent:@"sample.txt"];
    if ([[NSFileManager defaultManager]fileExistsAtPath:txtPath])
    {
        NSString *string = [[NSString  alloc] initWithContentsOfFile:txtPath encoding:NSUTF8StringEncoding error:nil];
        return [string componentsSeparatedByString:@"\n"];
    }
    else
    {
        NSString *resPath = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"sample.bundle"];
        NSString *txtPath = [resPath stringByAppendingPathComponent:@"sample.txt"];
        NSString *string = [[NSString  alloc] initWithContentsOfFile:txtPath encoding:NSUTF8StringEncoding error:nil];
        return [string componentsSeparatedByString:@"\n"];
    }
}

-(void)loadData
{
    sample = [self loadTestSentence];
    resultText = [[NSMutableString alloc]init];
    
    index = 0;
    textView.text = sample[index];
}

-(void)startRecognize
{
    [self playBackBtnInvisible];
    
    protocalView.text = @"";
    
    [enginer setOralText:textView.text];
    
    //调用start方法后，播放初始化的等待动画
    [waitingView startAnimating];
    
    [resultText deleteCharactersInRange:NSMakeRange(0, resultText.length)];
    
    [self currentCondition:@"开始初始化" error:NO finish:NO];
    
    engineError = nil;
    
    [enginer startRecognize];
}

-(void)stopRecognize
{
    [enginer stopRecognize];
    [waitingView startAnimating];
    
    [self currentCondition:@"结束录音" error:NO finish:NO];
}

-(void)cancelRecognize
{
    [enginer cancelRecognize];
    [waitingView startAnimating];
    
    [self currentCondition:@"取消录音" error:NO finish:NO];
}

#pragma mark - EngineManagerDelegate

- (void)onBeginOral{
    //录音初始化完成，关闭初始化动画
    [waitingView stopAnimating];

    //buffer的长度清空
    bufferData = nil;
    
    [self currentCondition:@"开始录音" error:NO finish:NO];
}

- (void)onStopOral{
    [_inputView setMicBtnEnable:NO];

    [_inputView micBtnReset];
    [inputView_Plugin appear];

    [self currentCondition:@"正在识别..." error:NO finish:NO];
}

- (void)onResult:(NSString *)result isLast:(BOOL)isLast{
    [resultText appendString:result];
    NSString *jsonResult = [self jsonPrint:resultText];
    protocalView.text = jsonResult;
}

- (void)onEndOral:(NSError *)error{
    if (error)
    {
        NSLog(@"onEndOral | error = %@", error);
        NSString *errorStr = [NSString stringWithFormat:@"%@ \n %ld",error.domain,(long)error.code];

        UIAlertView *alert = [[UIAlertView alloc]initWithTitle:@"提示"
                                                       message:errorStr
                                                      delegate:self
                                             cancelButtonTitle:@"确定"
                                             otherButtonTitles:nil];
        [alert show];

        engineError = error;
    }

    
    //显示 buffer的长度
//    NSString* showStr = [NSString stringWithFormat:@"buffer的长度：%ld bytes",bufferData.length];
//    protocalView.text = showStr;
    
    
    [_inputView setMicBtnEnable:YES];

    [self currentCondition:@"打分结束" error:NO finish:YES];
    
    [waitingView stopAnimating];
}

- (void)onVADTimeout{
    [waitingView startAnimating];
    [_inputView micBtnReset];
    [inputView_Plugin appear];
    [_inputView setMicBtnEnable:NO];

    [self currentCondition:@"VAD超时" error:NO finish:NO];
}

- (void)onUpdateVolume:(int)volume{
    [_inputView updateVolume:volume];
}

- (void)onRecordingBuffer:(NSData *)recordingData{
    if (bufferData == nil)
    {
        bufferData = [[NSMutableData alloc]init];
    }
    
    if (recordingData!=nil)
    {
        [bufferData appendData:recordingData];
    }
    NSLog(@"recordingDatas len=%ld", recordingData.length);
}

- (void)oralEngineDidInit:(NSError *)error{
    NSLog(@"oralEngineDidInit : %@ ",error);
}

- (void)audioFileDidRecord:(NSString *)url{
    audioFileUrl = url;
    NSLog(@"--audioFileDidRecord : %@",url);
}

- (void)onAsyncResult:(NSString *)url{
//    protocalView.text = url;
    NSLog(@"--onAsyncResult:%@",url);
}

@end
