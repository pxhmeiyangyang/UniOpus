//
//  USCOralEduWrapper.h
//  usc
//
//  Created by 刘俊 on 14-9-5.
//  Copyright (c) 2014年 yunzhisheng. All rights reserved.
//

#define OK_Engine                  0
#define SEPERATOR                  @"#"
#define SEPERATOR_DETAIL           @" "
#define SCORE                      @"score"
#define BEGIN                      @"begin"
#define END                        @"end"
#define WORD                       @"word"
#define WORDS                      @"words"
#define TOTALSCORE                 @"totalscore"

#import <Foundation/Foundation.h>
#include "AsrFix.h"
#include "Settings.h"
#import "USCMarcos.h"

@interface USCOralEduWrapper : NSObject
{
    //void *asrFix;
}

-(int)initEngine:(NSString *)path;
-(int)engineStart:(NSString *)sentenceStr oralTask:(NSString *)oralTask;
-(int)recognize:(char* )pcm withLen:(int)len;
-(NSString *)getResult;
-(int)engineStop;
-(void)engineRelease;

@end
