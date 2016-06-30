//
//  USCResult.m
//  usc
//
//  Created by 刘俊 on 15/5/6.
//  Copyright (c) 2015年 yunzhisheng. All rights reserved.
//

#import "USCResult.h"
#import "USCMarcos.h"

#define KEYS [NSArray arrayWithObjects:@"tzs4ezcmy5lvgo6rf7ancyqewirnxkbepb74j5yz", nil]
#define LINES @"lines"
#define SCORE @"score"
#define BEGIN @"begin"
#define END @"end"
#define VOLUME @"volume"
#define WORDS  @"words"

@implementation USCResult

-(id)initWithType:(ResultType )type
{
    self = [super init];
    if (self)
    {
        _resultType = type;
        [self reset];
    }
    return self;
}

-(void)reset
{
    if(_content != nil)
    {
        [_content deleteCharactersInRange:NSMakeRange(0, _content.length)];
        _content = nil;
    }
    _resultError = No_Error;
}

-(BOOL)getResult
{
    if (_content.length > 0||_resultError != No_Error)
    {
        return YES;
    }
    
    return NO;
}

-(void)setReslut:(NSString *)result
{
    if (result != NULL && result.length > 0)
    {
        if(_content == nil)
        {
            _content = [[NSMutableString alloc]init];
        }
        
        [_content appendString:result];
        
        [self adjustResultWithKey];
    }
}

-(void)adjustResultWithKey
{
    USCLog(@"adjustResultWithKey | _appkey : %@",_appKey);
    
    if (_appKey == NULL || _appKey.length == 0)
    {
        return ;
    }
    
    if ([KEYS containsObject:_appKey])
    {
        //[self improveScore];
    }
}

//学而思
/*
     评测曲线的映射公式：
     [10,20) ->[10,40) : y = 3x - 20
     [20,40) ->[40,60) : y = x + 20
     [40,60) ->[60,70) : y = x/2 + 40
     [60,100) ->[70,100) : y = 3x/4 + 25
 */
-(void)improveScore
{
    USCLog(@"improveScore");
    
    if (_content == NULL || _content.length == 0)
    {
        return ;
    }
    
    NSData *jsonData = [_content dataUsingEncoding:NSUTF8StringEncoding];
    
    id objc = [NSJSONSerialization JSONObjectWithData:jsonData options:NSJSONReadingMutableContainers error:nil];
    
    if (objc == NULL)
    {
        return;
    }
    
    NSMutableDictionary *resultTopDic = [NSMutableDictionary dictionaryWithDictionary:(NSDictionary *)objc];
    if (resultTopDic != NULL && resultTopDic.count > 0)
    {
        NSMutableArray *lineObj = [[NSMutableArray alloc]initWithArray:resultTopDic[LINES]];
        if (lineObj != NULL && lineObj.count > 0)
        {
            NSMutableDictionary *resultDic = [NSMutableDictionary dictionaryWithDictionary:lineObj[0]];
            if (resultDic != NULL && resultDic.count > 0)
            {
                float score = [[resultDic objectForKey:SCORE] floatValue];
                float newScore = 0;
                
                if (score >= 10 && score < 20)
                {
                    newScore = score * 3 - 20;
                }
                else if (score >= 20 && score < 40)
                {
                    newScore = score + 20;
                }
                else if (score >= 40 && score < 60)
                {
                    newScore = score / 2 + 40;
                }
                else if (score >= 60 && score < 100)
                {
                    newScore = 3 * score / 4 + 25;
                }
                
                USCLog(@"improveScore | oldscore : %f | newScore :%f",score,newScore);
                
                [resultDic setValue:[NSString stringWithFormat:@"%.2f",newScore] forKey:SCORE];
                
                //重新赋值
                if (_content != nil)
                {
                    _content = nil ;
                }
                
                NSData *newJsonData = [NSJSONSerialization dataWithJSONObject:resultDic options:2 error:nil];
                _content = [[NSMutableString alloc]initWithData:newJsonData encoding:NSUTF8StringEncoding];
            }
        }
    }
}

@end
