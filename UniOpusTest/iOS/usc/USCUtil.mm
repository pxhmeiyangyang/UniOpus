//
//  USCUtil.m
//  usc
//
//  Created by hejinlai on 12-11-6.
//  Copyright (c) 2012年 yunzhisheng. All rights reserved.
//

#import "USCUtil.h"
//#import <CoreTelephony/CTTelephonyNetworkInfo.h>
//#import <CoreTelephony/CTCarrier.h>

@implementation USCUtil


+ (void)escapePartialTag:(NSMutableString *)sentence
{
    NSRegularExpression *regex = [[NSRegularExpression alloc] initWithPattern:@"<s>|<unk>|\\s+" options:NSRegularExpressionCaseInsensitive error:nil];
    [regex replaceMatchesInString:sentence options:0 range:NSMakeRange(0, [sentence length]) withTemplate:@""];
    
    //NSLog(@"partial1 sentence=%@", sentence);
    
    regex = [[NSRegularExpression alloc] initWithPattern:@"(</s>)+|<SIL>" options:NSRegularExpressionCaseInsensitive error:nil];
    [regex replaceMatchesInString:sentence options:0 range:NSMakeRange(0, [sentence length]) withTemplate:@"，"];
    
    //NSLog(@"partial2 sentence=%@", sentence);
}


+ (void)escapeSentenceTag:(NSMutableString *)sentence isRemoveBeginToken:(BOOL)flag
{
    [self escapePartialTag:sentence];
    
    NSRegularExpression *regex;
    if (flag) {
        regex = [[NSRegularExpression alloc] initWithPattern:@"(^，)|(，$)" options:NSRegularExpressionCaseInsensitive error:nil];
        [regex replaceMatchesInString:sentence options:0 range:NSMakeRange(0, [sentence length]) withTemplate:@""];
    }else{
        regex = [[NSRegularExpression alloc] initWithPattern:@"(，$)" options:NSRegularExpressionCaseInsensitive error:nil];
        [regex replaceMatchesInString:sentence options:0 range:NSMakeRange(0, [sentence length]) withTemplate:@""];
    }
    
    //NSLog(@"partial3 sentence=%@", sentence);
    
    
    if ([sentence length] > 0) {
        regex = [[NSRegularExpression alloc] initWithPattern:@"([吗呢]$)|(^(请问)|(为(什么|啥))|(怎么))" options:NSRegularExpressionCaseInsensitive error:nil];
        NSRange range = [regex rangeOfFirstMatchInString:sentence options:0 range:NSMakeRange(0, [sentence length])];
        
        if (!NSEqualRanges(range, NSMakeRange(NSNotFound, 0)))
        {
            [sentence appendString:@"？"];
        }
        else
        {
            [sentence appendString:@"。"];
        }
        
        //NSLog(@"partial4 sentence=%@", sentence);
    }
}

//去掉尖括号
+ (NSString *)escapeBrackets:(NSString *)str
{
    if (str == nil || [str length] == 0) {
        return nil;
    }
    
    NSMutableString *result = [NSMutableString stringWithString:str];
    
    NSRegularExpression *regexLeft = [[NSRegularExpression alloc] initWithPattern:@"<" options:NSRegularExpressionCaseInsensitive error:nil];
    [regexLeft replaceMatchesInString:result options:0 range:NSMakeRange(0, [result length]) withTemplate:@""];
    
    NSRegularExpression *regexLeft2 = [[NSRegularExpression alloc] initWithPattern:@"＜" options:NSRegularExpressionCaseInsensitive error:nil];
    [regexLeft2 replaceMatchesInString:result options:0 range:NSMakeRange(0, [result length]) withTemplate:@""];
    
    NSRegularExpression *regexRight = [[NSRegularExpression alloc] initWithPattern:@">" options:NSRegularExpressionCaseInsensitive error:nil];
    [regexRight replaceMatchesInString:result options:0 range:NSMakeRange(0, [result length]) withTemplate:@""];
    
    NSRegularExpression *regexRight2 = [[NSRegularExpression alloc] initWithPattern:@"＞" options:NSRegularExpressionCaseInsensitive error:nil];
    [regexRight2 replaceMatchesInString:result options:0 range:NSMakeRange(0, [result length]) withTemplate:@""];
    
    return result;
}

+(NSString *)getLocalMacAddress
{    
    int                 mib[6];
    size_t              len;
    char                *buf;
    unsigned char       *ptr;
    struct if_msghdr    *ifm;
    struct sockaddr_dl  *sdl;
    
    mib[0] = CTL_NET;
    mib[1] = AF_ROUTE;
    mib[2] = 0;
    mib[3] = AF_LINK;
    mib[4] = NET_RT_IFLIST;
    
    if ((mib[5] = if_nametoindex("en0")) == 0) {
        //printf("Error: if_nametoindex error/n");
        return nil;
    }
    
    if (sysctl(mib, 6, NULL, &len, NULL, 0) < 0) {
        //printf("Error: sysctl, take 1/n");
        return nil;
    }
    
    if ((buf = (char *)malloc(len)) == NULL) {
        //printf("Could not allocate memory. error!/n");
        return nil;
    }
    
    if (sysctl(mib, 6, buf, &len, NULL, 0) < 0) {
        //printf("Error: sysctl, take 2");
        return nil;
    }
    
    ifm = (struct if_msghdr *)buf;
    sdl = (struct sockaddr_dl *)(ifm + 1);
    ptr = (unsigned char *)LLADDR(sdl);
    NSString *outstring = [NSString stringWithFormat:@"%02x:%02x:%02x:%02x:%02x:%02x", *ptr, *(ptr+1), *(ptr+2), *(ptr+3), *(ptr+4), *(ptr+5)];
    //NSString *outstring = [NSString stringWithFormat:@"%02x%02x%02x%02x%02x%02x", *ptr, *(ptr+1), *(ptr+2), *(ptr+3), *(ptr+4), *(ptr+5)];
    free(buf);
    
    if (outstring && outstring.length > 0) {
        NSString *macAddress = [outstring uppercaseString];
        //iOS7 -> 02:00:00:00:00:00
        if (![macAddress isEqualToString:@"02:00:00:00:00:00"]) {
            return macAddress;
        }
    }
    return outstring;
}


+ (NSString *)getPackageName
{
    NSBundle *bundle =[NSBundle mainBundle];
    NSDictionary *info =[bundle infoDictionary];
    return [info objectForKey:@"CFBundleIdentifier"];
}

#if 0
+ (NSString *)getCarrier
{
    CTTelephonyNetworkInfo *ctNetInfo = [[CTTelephonyNetworkInfo alloc] init];
    CTCarrier *carrier = ctNetInfo.subscriberCellularProvider;
    NSMutableString *carrierCode = [[[NSMutableString alloc] init] autorelease];
    if (carrier) {
        [carrierCode appendString:carrier.mobileCountryCode];
        [carrierCode appendString:carrier.mobileNetworkCode];
    }else{
        [carrierCode appendString:@"0"];
    }
    [ctNetInfo release];
    return carrierCode;
}


/*
     检测网络类型，2G，3G，wifi
 */
+ (int)dataNetworkTypeFromStatusBar {
    
    UIApplication *app = [UIApplication sharedApplication];
    NSArray *subviews = [[[app valueForKey:@"statusBar"] valueForKey:@"foregroundView"] subviews];
    NSNumber *dataNetworkItemView = nil;
    
    for (id subview in subviews) {
        if([subview isKindOfClass:[NSClassFromString(@"UIStatusBarDataNetworkItemView") class]]) {
            dataNetworkItemView = subview;
            break;
        }
    }
    
    int netType = NETWORK_TYPE_NONE;
    NSNumber * num = [dataNetworkItemView valueForKey:@"dataNetworkType"];
    if (num == nil) {
        
        netType = NETWORK_TYPE_NONE;
        
    }else{
        
        int n = [num intValue];
        if (n == 0) {
            netType = NETWORK_TYPE_NONE;
        }else if (n == 1){
            netType = NETWORK_TYPE_2G;
        }else if (n == 2){
            netType = NETWORK_TYPE_3G;
        }else{
            netType = NETWORK_TYPE_WIFI;
        }
        
    }
    
    return netType;
}

#endif

@end
