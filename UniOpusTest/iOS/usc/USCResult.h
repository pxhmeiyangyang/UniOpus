//
//  USCResult.h
//  usc
//
//  Created by 刘俊 on 15/5/6.
//  Copyright (c) 2015年 yunzhisheng. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "USCErrorCode.h"

/*
 识别类型
 */
typedef enum
{
    // 在线结果
    uOnLineResult = 1,
    
    // 离线结果
    uOfflineFixResult = 2,
} ResultType;

@interface USCResult : NSObject

@property (nonatomic,assign) ResultType resultType;
@property (nonatomic,retain)NSMutableString *content;
@property (nonatomic,assign)ErrorType resultError;
@property (nonatomic,assign)BOOL isLast;
@property (nonatomic,retain)NSString *appKey;


-(id)initWithType:(ResultType )type;
-(void)reset;
-(BOOL)getResult;
-(void)setReslut:(NSString *)result;

//根据客户需求对分数做定向校准
-(void)adjustResultWithKey;

@end
