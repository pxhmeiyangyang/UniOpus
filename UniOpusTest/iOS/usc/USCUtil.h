//
//  USCUtil.h
//  usc
//
//  Created by hejinlai on 12-11-6.
//  Copyright (c) 2012年 yunzhisheng. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIApplication.h>
#import <sys/socket.h>
#import <sys/sysctl.h>
#import <net/if.h>
#import <net/if_dl.h>

@interface USCUtil : NSObject

+ (void)escapePartialTag:(NSMutableString *)sentence;
+ (void)escapeSentenceTag:(NSMutableString *)sentence isRemoveBeginToken:(BOOL)flag;
+ (NSString *)escapeBrackets:(NSString *)str;

+ (NSString *)getPackageName;
//+ (NSString *)getCarrier;

+ (NSString *)getLocalMacAddress;

//+ (int)dataNetworkTypeFromStatusBar;


@end
