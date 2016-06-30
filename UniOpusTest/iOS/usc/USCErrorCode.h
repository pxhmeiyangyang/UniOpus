//
//  USCErrorCode.h
//  usc
//
//  Created by 刘俊 on 15/10/8.
//  Copyright © 2015年 yunzhisheng. All rights reserved.
//

#import <Foundation/Foundation.h>

/*
 错误类型包括
 ​网络错误/network——连接、收发网络包
 设备错误/device——麦克风录音、文件系统保存等
 服务器错误/server——服务器问题，或者服务器检查到客户端发送的数据有问题
 全生词错误/Unknown_word——整个文本全是生词
 
 错误号定义如下。其他未囊括部分各平台自行新增
     网络错误：
     ​-1 连接错误
     -2 写start错误
     -3 写音频错误
     -4 写stop错误
     -5 读start结果错误
     -6 读stop结果错误
     -7 http评测连接错误
     -8 http评测读错误
     -9 http评测写错误
 
     设备错误
     -1001 录音失败
     -1002 数据源错误——对于允许从文件等数据源(而非麦克风)评测的SDK
 
     ​服务器错误，是服务端返回的错误码。http://192.168.5.120/wiki/index.php/Private_front_service_errcode
 
     全生词错误
     -2001 全生词
 */


//错误码各平台统一，勿轻易增删
typedef NS_ENUM(int, ErrorType)
{
    /*  */
    No_Error                        = 0,
    NO_Define                       = -65534,
    
    NetWork_Connect_Error           = -1,                                           // 连接错误
    NetWork_Start_Write_Error       = -2,                                           // 写start错误
    NetWork_Audio_Write_Error       = -3,                                           // 写音频错误
    NetWork_Stop_Write_Error        = -4,                                           // 写stop错误
    NetWork_Start_Read_Error        = -5,                                           // 读start结果错误
    NetWork_Stop_Read_Error         = -6,                                           // 读stop结果错误
    NetWork_Http_Connect_Error      = -7,                                           // http评测连接错误
    NetWork_Http_Read_Error         = -8,                                           // http评测读错误
    NetWork_Http_Write_Error        = -9,                                           // http评测写错误
    
    Device_Record_Error             = -1001,                                        // 录音失败
    Device_Audio_File_Error         = -1002,                                        // 数据源错误(非麦克风)
    RECORDING_AUDIO_INPUT_ERROR     = -1101,
    RECORDING_AUDIO_SESSION_ERROR   = -1102,
    RECORDING_CATEGORY_ERROR        = -1103,
    RECORDING_ENQUEUE_BUFFER_ERROR  = -1104,
    RECORDING_ALLOC_BUFFER_ERROR    = -1105,
    RECORDING_AUDIO_START_ERROR     = -1106,
    
    Recog_No_Text_In_Dic            = -2001,                                        // 生词错误
        
    //离线识别
    Offline_Recog_Error             = -3001,                                        // 离线识别出错
    OffLine_Recog_No_Memory         = -3002,                                        // 内存不足
    OffLine_Recog_Text_Is_Too_Long  = -3003,                                        // 文本过长
    OffLine_Recog_Text_Is_Empty     = -3004,                                        // 文本为空
    OffLine_Recog_SDK_Expiration    = -3005,                                        // SDK授权过期
    OffLine_Recog_No_Text_In_Dic    = -3006,                                        // 字词不在字典中
    
    Ser_Connect_Engine_Fail         = 57345,                                        // 连接引擎服务失败
    Ser_No_Text_In_Dic              = 57350,                                        // 生词错误
    Ser_Text_Is_Too_Long            = 57351,                                        // 文本过长
    Ser_Engine_Error                = 57352,                                        // 在线引擎出错
    Ser_Illegal_Package             = -6000,                                        // 数据包校验出错
    Ser_Empty_Text                  = 65527,                                        // 文本为空
    Ser_Unknow_Error                = 61440,                                        // 私有协议内部未知错误
    Ser_Connect_Ecode_Fail          = 61441,                                        // 连接转码服务失败
    Ser_Protocal_Read_Error         = 65524,                                        // 协议前8个字节的frame出错
    Ser_Package_Exceed_32M          = 65525,                                        // 单个协议包大于32MB
    Ser_Not_Correct_Client          = 65526,                                        // 不是私有协议的客户端
    Ser_Stop_Error                  = 65532,                                        // 在线引擎Stop返回错误
    Ser_AppKey_Error                = 65533,                                        // Appkey错误
    Ser_Connect_Error               = 65534,                                        // 连接引擎服务错误
    Ser_N2t_Connect_Error           = 8193,                                         // 连接n2t服务出错
    Ser_N2t_Parsing_Error           = 8195,                                         // n2t解析错误
    Ser_N2t_Assembling_Error        = 8198,                                         // n2t组装结果出错,只有enstar和E模式会组装结果
    Ser_Transcoding_Connect_Error   = 49153,                                        // 连接转码服务出错
    Ser_Transcoding_Error           = 49155,                                        // 转码服务转码过程中出错
    Ser_Http_Error                  = 40960,                                        // HTTP评测前端内部错误
};

@interface USCErrorCode : NSObject

+(NSError *)getError:(ErrorType)code;
+(ErrorType)offlineErrorTransform:(int)code; //该方法只用作离线识别错误码的转换
+(ErrorType)onlineErrorTransform:(int)code; //该方法只用作在线识别错误码的转换
+(BOOL)isTextError:(ErrorType)error;//是否为文本相关错误

@end
