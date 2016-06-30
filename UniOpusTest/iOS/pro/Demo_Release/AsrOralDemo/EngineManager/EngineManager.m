//
//  EngineManager.m
//  AsrOralDemo
//
//  Created by 刘俊 on 15/6/25.
//  Copyright (c) 2015年 yunzhisheng. All rights reserved.
//

#import "EngineManager.h"


#define kTEST @"A,0,#OUT_SENT_REF_TEXT#OUT_SENT_ASR_TEXT#OUT_SENT_TIMESTAMP#OUT_SENT_SCORE#OUT_WORD_TEXT#OUT_WORD_CLASS#OUT_WORD_TIMESTAMP#OUT_WORD_SCORE#OUT_WORD_VOLUME#OUT_SENT_FLUENCY#OUT_SENT_INTEGRITY#OUT_SENT_PRONUNCIATION#OUT_WORD_STRESS#OUT_SENT_SYNTACTICAL_TEXT#OUT_SENT_SYNTACTICAL_ALL#OUT_SENT_SYNTACTICAL_PRO#OUT_SENT_SYNTACTICAL_FLU#OUT_SENT_SYNTACTICAL_INT#OUT_SENT_KEYWORDS_TEXT#OUT_SENT_KEYWORDS_ALL#OUT_SENT_KEYWORDS_SCORE#OUT_SENT_KEYWORDS_INT#OUT_SENT_KEYWORDS_FLU#OUT_SCORE_COEFFICIENT=1.0"

@implementation EngineManager

@synthesize delegate;

static EngineManager *mEngineManager;

+ (EngineManager *)sharedManager
{
//    if(mEngineManager == nil)
//    {
//        mEngineManager = [[EngineManager alloc]init];
//    }
//    
//    return mEngineManager;
    static dispatch_once_t onceTocken;
    dispatch_once(&onceTocken, ^{
        mEngineManager = [[EngineManager alloc]init];
    });
    return mEngineManager;
}

-(id)init
{
    if (self = [super init])
    {
        [self recognizerInit];
        
        recogMode = Recognize_REC;  //默认使用录音
        
        stateMachine = [[EngineStateMachine alloc]init];
        [stateMachine reset];
        
    }
    
    return self;
}

- (NSString *)version
{
    return [USCRecognizer version];
}

-(void)recognizerInit
{
    NSString *sourcePath = [[NSBundle mainBundle].resourcePath stringByAppendingPathComponent:@"tmp_oral_offline.bundle"];
    recognizer = [[USCRecognizer alloc] initWithSource:sourcePath];
    recognizer.delegate = self;
    //recognizer.oralTask = @"B";         //默认B模式
//    recognizer.audioType = AudioType_MP3;
    recognizer.asyncRecognize = NO;
    [recognizer setOutScoreCoefficient:1];
    [recognizer setVadFrontTimeout:2000 backTimeout:2000];

    NSLog(@"SDK Version : %@",[USCRecognizer version]);
    
    //默认使用麦克风
    
    [self setRecognizerMode:Recognize_REC];
}

-(void)startRecognize
{
    if (![stateMachine couldStart])
    {
        return;
    }
    
    printf("\n\n\n\n\n\n");
    NSLog(@" - startRecognize - ");
    
    
    [recognizer setIsOnlineWhenMix:self.isOnlineWhenMix];
    
    recordData = nil;
    
    //读取设置
    [self loadRecognizerSettings:recognizer];
    
    if (recogMode == Recognize_REC)
    {
        [recognizer start];
    }
    else if (recogMode == Recognize_PCM)
    {
        NSString *pcmFilePath = [self pcmFilePath];
        if (pcmFilePath != NULL)
        {
            [recognizer startWithPCM:pcmFilePath];
        }
        else
        {
            NSError *error = [NSError errorWithDomain:@"没有PCM文件" code:NO_PCM_FILE userInfo:nil];
//            if (delegate != NULL && [delegate respondsToSelector:@selector(onEndOral:)])
//            {
//                [delegate onEndOral:error];
            [self onEndOral:error];
//            }
        }
    }
}

-(void)stopRecognize
{
    if (![stateMachine couldStop])
    {
        return;
    }
    
    NSLog(@" - stopRecognize - ");
    
    
    
    [recognizer stop];
}

-(void)cancelRecognize
{
    if (![stateMachine couldCancel])
    {
        return;
    }
    
    NSLog(@" - cancelRecognize - ");
    
    [recognizer cancel];
}

-(void)setRecognizerMode:(RecognizeMode)mode
{
    NSLog(@"setRecognizerMode : %d",(int)mode);
    recogMode = mode;
}

-(RecognizeMode)getRecognizerMode
{
    return recogMode;
}

-(NSString *)getOralTask
{
    return recognizer.oralTask;
}

-(void)setOralText:(NSString *)text
{
    NSLog(@"setOralText : %@",text);
    
    text = [text stringByReplacingOccurrencesOfString:@"\n" withString:@""];
    text = [text stringByReplacingOccurrencesOfString:@"\r" withString:@""];
    
    recognizer.oralText = text;
}

-(void)setOralTask:(NSString *)task
{
    NSLog(@"setOralTask : %@",task);
    recognizer.oralTask = task;
}

- (void)setOfflineResultWaitingTime:(NSTimeInterval)time{
    NSLog(@"setOfflineResultWaitingTime : %f",time);
    [recognizer setOfflineResultWaitingTime:time];
}

-(NSString *)pcmFilePath
{
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsPath = [paths objectAtIndex:0];
    NSString *filePath = [documentsPath stringByAppendingPathComponent:FILE_INPUT_TEST_PCM];
    
    if ([[NSFileManager defaultManager] fileExistsAtPath:filePath])
    {
        return filePath;
    }
    
    return NULL;
}

-(void)writeFile
{
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsPath = [paths objectAtIndex:0];
    NSString *testPath;
    if (recognizer.audioType == AudioType_MP3) {
        testPath = [documentsPath stringByAppendingPathComponent:FILE_NAME_RECORDING];
    }else if(recognizer.audioType == AudioType_PCM){
        testPath = [documentsPath stringByAppendingPathComponent:FILE_INPUT_PCM];
    }
    
    NSError *error;
    BOOL res=[recordData writeToFile:testPath options:NSDataWritingFileProtectionComplete error:&error];
    if (res)
    {
        NSLog(@"WriteToFile Successfully!");
    }
    else
    {
        NSLog(@"WriteToFile Fail");
    }
}

-(void)loadRecognizerSettings:(USCRecognizer *)recog
{
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *document = paths[0];
    NSString *settingPath = [document stringByAppendingPathComponent:@"Setting.plist"];
    if ([[NSFileManager defaultManager]fileExistsAtPath:settingPath])
    {
        NSDictionary *settingDic = [NSDictionary dictionaryWithContentsOfFile:settingPath];
        if (settingDic != NULL && settingDic.count > 0)
        {
            NSLog(@"loadRecognizerSettings : %@",settingDic);
            
            NSString *identifier = settingDic[@"identifier"];
            NSString *oralTask = settingDic[@"oraltask"];
            NSString *coefficientScore = settingDic[@"coefficientScore"];
            NSString *frontVAD = settingDic[@"frontVAD"];
            NSString *backVAD = settingDic[@"backVAD"];
            NSString *recordType = settingDic[@"recordtype"];
            
            if (identifier != NULL && identifier.length > 0)
            {
                [recog setIdentifier:identifier];
            }
            
            if (oralTask != NULL && oralTask.length > 0)
            {
                recog.oralTask = oralTask;
            }
            
            if (coefficientScore != NULL && coefficientScore.length > 0)
            {
                [recog setOutScoreCoefficient:[coefficientScore floatValue]];
            }
            
            if (recordType != NULL && recordType.length > 0)
            {
                if ([recordType isEqualToString:@"mp3"])
                {
                    recog.audioType = AudioType_MP3;
                }
                else
                {
                    recog.audioType = AudioType_PCM;
                }
            }
            
            if (frontVAD != NULL && frontVAD.length > 0
                &&backVAD != NULL && backVAD.length > 0)
            {
                int front = [frontVAD intValue];
                int back = [backVAD intValue];
                if (front != 0 && back != 0)
                {
                    [recog setVadFrontTimeout:front backTimeout:back];
                }
            }
            else
            {
                [recog setVadFrontTimeout:2000 backTimeout:2000];
            }
        }
    }
}

/**
 *  根据评测文本预估阅读时长
 *
 *  @param text 评测文本
 */
-(NSInteger)getTimeByOralText:(NSString* )text{
    if (text.length > 0) {
        NSArray* array = [text componentsSeparatedByString:@" "];
        float y = array.count * 0.436 + 0.2;
        return (int)ceil(y);
    }
    return 0;
}
#pragma mark - 延时评测相关参数

-(BOOL)getAsyncRecognize{
    return recognizer.asyncRecognize;
}

-(void)setAsyncRecognize:(BOOL)asyncRecognize{
    NSLog(@"set asyncRecognize:%d",asyncRecognize);
    recognizer.asyncRecognize = asyncRecognize;
}

#pragma mark - USCRecognizerDelegate

- (void)onBeginOral
{
    NSLog(@"1.onBeginOral");
    
    
    
    if (delegate != NULL && [delegate respondsToSelector:@selector(onBeginOral)])
    {
        [delegate onBeginOral];
    }
}

- (void)onStopOral
{
    NSLog(@"2.onStopOral");
    
    
    
    if (delegate != NULL && [delegate respondsToSelector:@selector(onStopOral)])
    {
        [delegate onStopOral];
    }
}

- (void)onResult:(NSString *)result isLast:(BOOL)isLast
{
    NSLog(@"3.onResult : %@",result);
    
    
    
    if (delegate != NULL && [delegate respondsToSelector:@selector(onResult:isLast:)])
    {
        [delegate onResult:result isLast:isLast];
    }
}

- (void)onEndOral:(NSError *)error
{

    
    [stateMachine reset];
    
    if (delegate != NULL && [delegate respondsToSelector:@selector(onEndOral:)])
    {
        [delegate onEndOral:error];
    }
    
    NSLog(@"return data lenth%ld",[recordData length]);
    
    //保存录音文件(为确保音频文件完整，请在此回调中完成MP3拼接)
//    if (recognizer.audioType == AudioType_MP3) {
//        
//    }else if(recognizer.audioType == AudioType_PCM){
//        [self pcmFilePath];
//    }
    [self writeFile];
    
    NSLog(@"4-1.onEndOral");
    NSLog(@"------- -------");
    NSLog(@"------- -------");
    NSLog(@"------- -------");
}

- (void)onUpdateVolume:(int)volume
{
    //NSLog(@"volumn = %i", volume);
    if (delegate != NULL && [delegate respondsToSelector:@selector(onUpdateVolume:)])
    {
        [delegate onUpdateVolume:volume];
    }
}

- (void)onVADTimeout
{
    NSLog(@"2.onVADTimeout");
    
    
    
    [recognizer stop];
    
    if (delegate != NULL && [delegate respondsToSelector:@selector(onVADTimeout)])
    {
        [delegate onVADTimeout];
    }
}

- (void)onRecordingBuffer:(NSData *)recordingData
{
    if (recordData==nil)
    {
        recordData = [[NSMutableData alloc]init];
    }
    
    if (recordingData!=nil)
    {
        [recordData appendData:recordingData];
    }
    
    if (delegate != NULL && [delegate respondsToSelector:@selector(onRecordingBuffer:)])
    {
        [delegate onRecordingBuffer:recordingData];
    }
}

- (void)oralEngineDidInit:(NSError *)error
{
    NSLog(@"oralEngineDidInit : %@ ",error);
    
    
    
    if (delegate != NULL && [delegate respondsToSelector:@selector(oralEngineDidInit:)])
    {
        [delegate oralEngineDidInit:error];
    }
}

- (void)audioFileDidRecord:(NSString *)url
{
    NSLog(@"audioFileDidRecord : %@",url);
    
    
    
    if (delegate != NULL && [delegate respondsToSelector:@selector(audioFileDidRecord:)])
    {
        [delegate audioFileDidRecord:url];
    }
}

- (void)onAsyncResult:(NSString *)url
{
    NSLog(@"onAsyncResult : %@",url);

    
    if (delegate != NULL && [delegate respondsToSelector:@selector(onAsyncResult:)])
    {
        [delegate onAsyncResult:url];
    }
}



@end
