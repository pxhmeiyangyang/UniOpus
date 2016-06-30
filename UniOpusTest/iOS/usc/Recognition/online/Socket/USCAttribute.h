//
//  USCAttribute.h
//  Socket
//
//  Created by 刘俊 on 15/7/20.
//  Copyright (c) 2015年 yunzhisheng. All rights reserved.
//

#import <Foundation/Foundation.h>

typedef NS_ENUM(int, Attribute)
{
    SSUP_ENCRYPT = 0x00,
    SSUP_DECODE = 0x01,
    SSUP_AUDIO_ENC_METH = 0x02,  //support user and server point out which audio encode method used^M
    SSUP_AUDIO_FMT_SET = 0x03,   //support define format of audio^M
    SSUP_RSP_FMT_SET = 0x04,     //support define format of text returned to user^M
    SSUP_RSP_ENC_METH = 0x05,    //support define coding format of result, for example, gb2312, utf8 or unicode^M
    SSUP_RST_NUM = 0x06,         //support define how many result return to user^M
    SSUP_GRAM_FMT_SET = 0x07,    //support define grammar method, url-list, abnf, or grxml^M
    SSUP_CFD_SET = 0x08,         //support define limit of zhixindu^M
    SSUP_SUB_CAGA_SET = 0x09,    //iat or asr catogery^M
    SSUP_ENT_SET = 0x0a,         //support define different engine, sms16k, sms8k, vedio16k, video8k^M
    SSUP_WAIT_TIMEOUT = 0x0b,    //support define waiting timeout duration^M
    SSUP_IMEI_SET = 0x0c,        //support user set IMEI^M
    SSUP_APP_KEY = 0x0d,         //support user sending APP_KEY^M
    //public static static final  byte SSUP_USER_CARRIED=0x0e,
    //support user defined information^M
    SSUP_ASR_OPT_PACKAGE_NAME = 0x0e,
    SSUP_ASR_OPT_CARRIER = 0x0f,
    SSUP_ASR_OPT_NETWORK_TYPE = 0x10,
    SSUP_ASR_OPT_DEVICE_OS = 0x11,
    SSUP_USER_ID = 0x12,
    SSUP_COLLECTED_INFO = 0x13,
    SSUP_REQ_RSP_ENTITY = 0x14,
    SSUP_RSP_AUDIO_URL = 0x15,
    SSUP_MODEL_TYPE = 0x16,      //choose model type of ASR-Server^M
    SSUP_USRDATA_FLAG = 0x17,
    SSUP_ORAL_EVAL_TEXT = 0x18,
    SSUP_ORAL_TASK_TYPE = 0x19,
    SSUP_ORAL_EX1 = 0x1A,
    SSUP_ORAL_EX2 = 0x1B,
    SSUP_SESSION_ID = 0x1c,
    SSUP_AUTO_LONG_MODE = 0x1d,
    CURRSUPATTRNUM = SSUP_APP_KEY + 1,
    MAXSUPATTRNUM = 0xFF,        //max attribute number (using long to store attributes code)0~254, 255 is presenting no attribute set ^M
};

@interface USCAttribute : NSObject
{
    
}

@property (nonatomic, assign)Attribute attribute;
@property (nonatomic, copy)NSString *value;

-(id)initWithKey:(Attribute)key value:(NSString *)str;
-(NSData *)getBody;
-(NSInteger)bodyLength;

@end
