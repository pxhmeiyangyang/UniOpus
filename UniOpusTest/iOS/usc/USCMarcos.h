//
//  USCMarcos.h
//  UILabel
//
//  Created by 刘俊 on 15/9/23.
//  Copyright © 2015年 yunzhisheng. All rights reserved.
//

#import "Settings.h"
#include <pthread.h>

#ifndef USCMarcos_h
#define USCMarcos_h

static NSMutableString *logString;
static NSDateFormatter *formatter;

#if USC_BUG

    #if CUSTOM_LOG

        #define __THREAD__ pthread_mach_thread_np(pthread_self())
        #define USCLog(args...) _Log(__FILE__,__LINE__,__PRETTY_FUNCTION__,__THREAD__,args);

        void _Log(const char *file, int lineNumber, const char *funcName, int threadNumber, NSString *format,...)
        {
            if (logString == nil)
            {
                logString = [[NSMutableString alloc]init];
            }
            
            @synchronized(logString)
            {
                va_list ap;
                va_start (ap, format);
                NSString *msg = [[NSString alloc] initWithFormat:[NSString stringWithFormat:@"%@",format] arguments:ap];
                va_end (ap);
                
                if (formatter == nil)
                {
                    NSTimeZone * zone = [NSTimeZone timeZoneWithName:@"Asia/Shanghai"];
                    formatter = [[NSDateFormatter alloc]init];
                    [formatter setDateFormat:@"yyyy-MM-dd hh:mm:ss.SSS"];
                    formatter.timeZone = zone;
                }
                
                NSString *dateString = [formatter stringFromDate:[NSDate date]];
                NSString *logStr = [NSString stringWithFormat:@"%@ [Thread %x] %s [L%d] %@\n",dateString,threadNumber, funcName, lineNumber, msg];
                [logString appendString:logStr];
                
                fprintf(stderr,"%s",[logStr UTF8String]);
            }
        }

        void LJWriteLogToFile()
        {
            //    mach_port_t machTID = pthread_mach_thread_np(pthread_self());
            //    NSLog(@"current thread: %x", machTID);
            
            if (logString != nil && logString.length > 0)
            {
                NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
                NSString *documentsDirectory = [paths objectAtIndex:0];
                NSString *path = [documentsDirectory stringByAppendingPathComponent:@"logfile.txt"];
                
                // create if needed
                NSMutableString *existLogStr;
                if ([[NSFileManager defaultManager] fileExistsAtPath:path])
                {
                    existLogStr = [NSMutableString stringWithContentsOfFile:path encoding:NSUTF8StringEncoding error:nil];
                }
                // append
                if (existLogStr !=nil)
                {
                    [existLogStr appendString:logString];
                }
                else
                {
                    existLogStr = logString;
                }
                
                [existLogStr writeToFile:path atomically:YES encoding:NSUTF8StringEncoding error:nil];
                
                //清空string
                existLogStr = nil;
                [logString deleteCharactersInRange: NSMakeRange(0, logString.length)];
                [logString setString:@""];
            }
        }

        NSString * getLogString()
        {
            return logString;
        }

    #else

        #define USCLog(fmt, ...) NSLog((@"%s [Line %d] " fmt), __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__);

    #endif

#else

    #define USCLog(...);

#endif

#endif /* USCMarcos_h */
