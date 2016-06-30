//
//  USCOralEduWrapper.m
//  usc
//
//  Created by 刘俊 on 14-9-5.
//  Copyright (c) 2014年 yunzhisheng. All rights reserved.
//

#import "USCOralEduWrapper.h"

@implementation USCOralEduWrapper

void* asrFix = NULL;

-(id)init
{
    if (self = [super init])
    {
        //asrFix = NULL;
    }
    return self;
}

-(int)initEngine:(NSString *)path
{
#if MIX_RECOGNIZE
    NSString *bundlePath = path;
    USCLog(@"initEngine_bundlePath : %@",bundlePath);
    
    const char *domain = [bundlePath cStringUsingEncoding:NSUTF8StringEncoding];
    int resultCode = init(domain, 0, &asrFix);
    USCLog(@"initEngine : %d",resultCode);
    
    return resultCode;
#else
    return -1;
#endif
}

-(int)engineStart:(NSString *)sentenceStr oralTask:(NSString *)oralTask
{
#if MIX_RECOGNIZE
    const char *sentence = [sentenceStr UTF8String];
    const char *task = [oralTask UTF8String];
    int resultCode = start( task, sentence, "", asrFix);
    
    USCLog(@"engineStart : %d",resultCode);
    
    return resultCode;
#else
    return -1;
#endif
}

-(int)recognize:(char* )pcm withLen:(int)len
{
#if MIX_RECOGNIZE
    int result = recognize(pcm, len, asrFix);
    return result;
#else
    return -1;
#endif
}

-(NSString *)getResult
{
#if MIX_RECOGNIZE
    USCLog(@"getResult");
    
//    char cStr[800000];
//    strcpy(cStr, getResult(asrFix));
    
    char * cStr = getResult(asrFix);
    
    if (cStr != NULL )
    {
        //USCLog(@"getResult : %s",cStr);
        NSString *orginalResult= [NSString stringWithCString:cStr encoding:NSUTF8StringEncoding];
        USCLog(@"getResult : %@",orginalResult);
        
        
        //NSString *jsonResult = [self formatResult:orginalResult];
        return orginalResult;
    }
    else
    {
        USCLog(@"result is empty");
        return NULL;
    }
#else
    return NULL;
#endif
}

-(int)engineStop
{
#if MIX_RECOGNIZE
    return stop(asrFix);
#else
    return -1;
#endif
}

-(void)cancel
{
#if MIX_RECOGNIZE
    stop(asrFix);
#endif
}

-(void)engineRelease
{
#if MIX_RECOGNIZE
    release(&asrFix);
#endif
}

#pragma mark StringFormat

-(NSString *)formatResult:(NSString *)result
{
    NSString *jsonStr=@"";
    
    NSArray *infoArray = [result componentsSeparatedByString:SEPERATOR];
    if (infoArray.count!=0)
    {
        /*
         返回的格式为：
         (
         "",
         totalScore,
         "word starttime endtime score"
         ...
         ""
         )
         
         */
        
        /*
         输出格式为：
         {
            "words":
                     [
                         {"score":10,"end":780,"word":"I","begin":670},
                         {"score":9,"end":1040,"word":"was","begin":780},
                     ],
            "totalscore":94
         }
         */
        
        NSString *totalScore = infoArray[1];
        
        NSMutableArray *scoreArray = [[NSMutableArray alloc]init];
        for(int i =2;i<infoArray.count - 1 ;i++)
        {
            NSString *oneStr = infoArray[i];
            NSArray *resultArray = [oneStr componentsSeparatedByString:SEPERATOR_DETAIL];
            NSString *word = resultArray[0];
            NSString *startTime = resultArray[1];
            NSString *endTime = resultArray[2];
            NSString *score = resultArray[3];
            
            NSDictionary *resultForDic = @{SCORE:score,BEGIN:startTime,END:endTime,WORD:word};
            [scoreArray addObject:resultForDic];
        }
        
        NSDictionary *allInfo = @{TOTALSCORE:totalScore,WORDS:scoreArray};
        NSData *jsonData = [NSJSONSerialization dataWithJSONObject:allInfo options:NSJSONWritingPrettyPrinted error:nil];
        jsonStr = [[NSString alloc]initWithData:jsonData encoding:NSUTF8StringEncoding];
        USCLog(@"formatResult : %@",jsonStr);
    }
    return jsonStr;
}

-(void)dealloc
{

}

@end
