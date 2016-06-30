//
//  USCErrorCode.m
//  usc
//
//  Created by 刘俊 on 15/10/8.
//  Copyright © 2015年 yunzhisheng. All rights reserved.
//
#import "USCErrorCode.h"


@implementation USCErrorCode


+(NSError *)getError:(ErrorType)code
{
    switch (code) {
            
        case No_Error:
            return nil;
            
        // 在线错误代码
        case NetWork_Connect_Error:
            return [NSError errorWithDomain:@"连接错误" code:NetWork_Connect_Error userInfo:nil];
            
        case NetWork_Start_Write_Error:
            return [NSError errorWithDomain:@"写start错误" code:NetWork_Start_Write_Error userInfo:nil];
            
        case NetWork_Audio_Write_Error:
            return [NSError errorWithDomain:@"写音频错误" code:NetWork_Audio_Write_Error userInfo:nil];
            
        case NetWork_Stop_Write_Error:
            return [NSError errorWithDomain:@"写stop错误" code:NetWork_Stop_Write_Error userInfo:nil];
            
        case NetWork_Start_Read_Error:
            return [NSError errorWithDomain:@"读start结果错误" code:NetWork_Start_Read_Error userInfo:nil];
            
        case NetWork_Stop_Read_Error:
            return [NSError errorWithDomain:@"读stop结果错误" code:NetWork_Stop_Read_Error userInfo:nil];
            
        case NetWork_Http_Connect_Error:
            return [NSError errorWithDomain:@"http评测连接错误" code:NetWork_Http_Connect_Error userInfo:nil];
            
        case NetWork_Http_Read_Error:
            return [NSError errorWithDomain:@"http评测读错误" code:NetWork_Http_Read_Error userInfo:nil];
            
        case NetWork_Http_Write_Error:
            return [NSError errorWithDomain:@"http评测写错误" code:NetWork_Http_Write_Error userInfo:nil];


        // 在线识别Server端错误码
        case Ser_Unknow_Error:
            return [NSError errorWithDomain:@"私有协议内部未知错误" code:Ser_Unknow_Error userInfo:nil];
            
        case Ser_Connect_Ecode_Fail:
            return [NSError errorWithDomain:@"连接转码服务失败" code:Ser_Connect_Ecode_Fail userInfo:nil];
            
        case Ser_Connect_Engine_Fail:
            return [NSError errorWithDomain:@"连接引擎服务失败" code:Ser_Connect_Engine_Fail userInfo:nil];
            
        case Ser_Illegal_Package:
            return [NSError errorWithDomain:@"数据包校验出错" code:Ser_Illegal_Package userInfo:nil];
            
        case Ser_Empty_Text:
            return [NSError errorWithDomain:@"测评文本为空" code:Ser_Empty_Text userInfo:nil];
            
        case Ser_No_Text_In_Dic:
            return [NSError errorWithDomain:@"生词错误" code:Recog_No_Text_In_Dic userInfo:nil];
            
        case Ser_Text_Is_Too_Long:
            return [NSError errorWithDomain:@"文本过长" code:Ser_Text_Is_Too_Long userInfo:nil];
            
        case Ser_Engine_Error:
            return [NSError errorWithDomain:@"在线引擎出错" code:Ser_Engine_Error userInfo:nil];
            
        case Ser_Protocal_Read_Error:
            return [NSError errorWithDomain:@"协议前8个字节的frame出错" code:Ser_Protocal_Read_Error userInfo:nil];
            
        case Ser_Package_Exceed_32M:
            return [NSError errorWithDomain:@"单个协议包大于32MB" code:Ser_Package_Exceed_32M userInfo:nil];
            
        case Ser_Not_Correct_Client:
            return [NSError errorWithDomain:@"不是私有协议的客户端" code:Ser_Not_Correct_Client userInfo:nil];
            
        case Ser_Stop_Error:
            return [NSError errorWithDomain:@"在线引擎Stop返回错误" code:Ser_Stop_Error userInfo:nil];
            
        case Ser_AppKey_Error:
            return [NSError errorWithDomain:@"Appkey错误" code:Ser_AppKey_Error userInfo:nil];
            
        case Ser_Connect_Error:
            return [NSError errorWithDomain:@"连接引擎服务错误" code:Ser_Connect_Error userInfo:nil];
            
        case Ser_N2t_Connect_Error:
            return [NSError errorWithDomain:@"连接n2t服务出错" code:Ser_N2t_Connect_Error userInfo:nil];
            
        case Ser_N2t_Parsing_Error:
            return [NSError errorWithDomain:@"n2t解析错误" code:Ser_N2t_Parsing_Error userInfo:nil];
            
        case Ser_N2t_Assembling_Error:
            return [NSError errorWithDomain:@"n2t组装结果出错" code:Ser_N2t_Assembling_Error userInfo:nil];
            
        case Ser_Transcoding_Connect_Error:
            return [NSError errorWithDomain:@"连接转码服务出错" code:Ser_Transcoding_Connect_Error userInfo:nil];
            
        case Ser_Transcoding_Error:
            return [NSError errorWithDomain:@"转码服务转码过程中出错" code:Ser_Transcoding_Error userInfo:nil];
            
        case Ser_Http_Error:
            return [NSError errorWithDomain:@"HTTP评测前端内部错误" code:Ser_Http_Error userInfo:nil];
            
            
        // - 设备错误码 -
        case Device_Audio_File_Error:
            return [NSError errorWithDomain:@"读取录音文件出错" code:Device_Audio_File_Error userInfo:nil];
            
        case Device_Record_Error:
        case RECORDING_ALLOC_BUFFER_ERROR:
        case RECORDING_AUDIO_INPUT_ERROR:
        case RECORDING_CATEGORY_ERROR:
        case RECORDING_ENQUEUE_BUFFER_ERROR:
        case RECORDING_AUDIO_SESSION_ERROR:
        case RECORDING_AUDIO_START_ERROR:
            return [NSError errorWithDomain:@"录音错误" code:Device_Record_Error userInfo:nil];
            
            
        // - 离线错误代码 -
        case Offline_Recog_Error:
            return [NSError errorWithDomain:@"离线识别出错" code:Offline_Recog_Error userInfo:nil];
            
        case OffLine_Recog_No_Memory:
            return [NSError errorWithDomain:@"内存不足" code:OffLine_Recog_No_Memory userInfo:nil];
            
        case OffLine_Recog_No_Text_In_Dic:
            return [NSError errorWithDomain:@"字词不在字典中" code:OffLine_Recog_No_Text_In_Dic userInfo:nil];
            
        case OffLine_Recog_Text_Is_Too_Long:
            return [NSError errorWithDomain:@"文本过长" code:OffLine_Recog_Text_Is_Too_Long userInfo:nil];
            
        case OffLine_Recog_Text_Is_Empty:
            return [NSError errorWithDomain:@"文本为空" code:OffLine_Recog_Text_Is_Empty userInfo:nil];
            
        case OffLine_Recog_SDK_Expiration:
            return [NSError errorWithDomain:@"SDK授权过期" code:OffLine_Recog_SDK_Expiration userInfo:nil];
            
        default:
            return [NSError errorWithDomain:@"引擎运行出错" code:NO_Define userInfo:nil];
    }
}

+(ErrorType)offlineErrorTransform:(int)code
{
    /*
     RECOGNIZER_RESULT = 1,
     RECOGNIZER_OK = 0,
     RECOGNIZER_ERROR = -1,
     RECOGNIZER_BAD_STATE = -2,
     RECOGNIZER_BAD_POINTER = -3,
     RECOGNIZER_BAD_DATA = -4,
     RECOGNIZER_NO_MEMORY = -5,
     RECOGNIZER_WORD_OOV = -6,
     RECOGNIZER_SENTENCE_OVERLONG = -7,
     RECOGNIZER_SENTENCE_EMPTY = -8,
     RECOGNIZE_EXPIRATION = -9,
     */
    
    switch (code)
    {
        case 1:
        case 0:
            return No_Error;
            
        case -1:
        case -2:
        case -3:
        case -4:
            return Offline_Recog_Error;
            
        case -5:
            return OffLine_Recog_No_Memory;
            
        case -6:
            return OffLine_Recog_No_Text_In_Dic;
            
        case -7:
            return OffLine_Recog_Text_Is_Too_Long;
            
        case -8:
            return OffLine_Recog_Text_Is_Empty;
            
        case -9:
            return OffLine_Recog_SDK_Expiration;
            
        default:
            return Offline_Recog_Error;
    }
}

+(ErrorType)onlineErrorTransform:(int)code
{
    switch (code)
    {
        case 57345:
            return Ser_Connect_Engine_Fail;
            
        case 57350:
            return Ser_No_Text_In_Dic;
            
        case 57351:
            return Ser_Text_Is_Too_Long;
            
        case 57352:
            return Ser_Engine_Error;
            
        case -6000:
            return Ser_Illegal_Package;
            
        case 65527:
            return Ser_Empty_Text;
            
        case 61440:
            return Ser_Unknow_Error;
            
        case 61441:
            return Ser_Connect_Ecode_Fail;
            
        case 65524:
            return Ser_Protocal_Read_Error;
            
        case 65525:
            return Ser_Package_Exceed_32M;
            
        case 65526:
            return Ser_Not_Correct_Client;
            
        case 65532:
            return Ser_Stop_Error;
            
        case 65533:
            return Ser_AppKey_Error;
            
        case 65534:
            return Ser_Connect_Error;
            
        case 8193:
            return Ser_N2t_Connect_Error;
            
        case 8195:
            return Ser_N2t_Parsing_Error;
            
        case 8198:
            return Ser_N2t_Assembling_Error;
            
        case 49153:
            return Ser_Transcoding_Connect_Error;
            
        case 49155:
            return Ser_Transcoding_Error;
            
        case 40960:
            return Ser_Http_Error;
                        
        default:
            return NO_Define;
    }
}

+(BOOL)isTextError:(ErrorType)error
{
    if (error == Ser_Empty_Text
        ||error == Ser_Text_Is_Too_Long
        ||error == Ser_No_Text_In_Dic)
    {
        return YES;
    }
    else
    {
        return NO;
    }
}

@end
