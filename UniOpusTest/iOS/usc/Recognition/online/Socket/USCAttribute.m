//
//  USCAttribute.m
//  Socket
//
//  Created by 刘俊 on 15/7/20.
//  Copyright (c) 2015年 yunzhisheng. All rights reserved.
//

#import "USCAttribute.h"

@implementation USCAttribute

@synthesize attribute;
@synthesize value;

-(id)initWithKey:(Attribute)key value:(NSString *)str
{
    if (self = [super init])
    {
        attribute = key;
        value = str;
    }
    return self;
}

-(NSInteger)bodyLength
{
    NSData *data = [self getBody];
    return data.length;
}

-(NSData *)getBody
{
    NSData *valueData = [value dataUsingEncoding:NSUTF8StringEncoding];
    char *valueByte = (char *)valueData.bytes;
    for (int i = 0; i < valueData.length; i++)
    {
        valueByte[i] ^= '@';
    }
    
    return [NSData dataWithBytes:valueByte length:valueData.length];
}

-(void)dealloc
{    
    value = nil;
}

@end
