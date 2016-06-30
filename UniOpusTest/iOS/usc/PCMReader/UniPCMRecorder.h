//
//  UniPCMRecorder.h
//  usc
//
//  Created by 刘俊 on 15/11/24.
//  Copyright © 2015年 yunzhisheng. All rights reserved.
//




/*-----------------
 * !!!!!!本类用于调试
 *-----------------
 */



#import <Foundation/Foundation.h>
#import "Settings.h"

@interface UniPCMRecorder : NSObject
{
    NSMutableData *recordData;
    NSString *progressName;
}

@property (nonatomic, assign)int totalLength;

+(UniPCMRecorder *)defaultRecorder;
-(void)start:(NSString *)progressName;
-(void)appendData:(NSData *)data;
-(void)finish;

@end
