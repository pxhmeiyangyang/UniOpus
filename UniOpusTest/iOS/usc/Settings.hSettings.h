//
//  Settings.h
//  usc
//
//  Created by 刘俊 on 14-8-22.
//  Copyright (c) 2014年 yunzhisheng. All rights reserved.
//

//版本号（更新规则：结构有大的调整第一位增加，需求改动或离线引擎更新第二位增加，修复bug及小改动第三位增加；第二位增加时第三位归零）
#define OralSdkVersion                 @"2.16.15"

//识别模式（1为混合，0为纯在线）
#define MIX_RECOGNIZE 1

//-AppKey- 调试用：lcaq6253mwcfrn3wri7g34ip7grx6hbhr65fiza6
#define AppKey @"lcaq6253mwcfrn3wri7g34ip7grx6hbhr65fiza6"

//教育服务器域名及端口
#define oral_domain @"eval1.hivoice.cn"
#define oral_port  8085
#define PirvateProtocal_IP @[@"120.132.147.115", @"114.141.158.124", @"117.121.55.36"]
//#define oral_domain @"eeval.hivoice.cn"
//#define oral_port  8085 
//#define PirvateProtocal_IP @[@"120.132.147.1151", @"1114.141.158.124", @"1117.121.55.36"]

//http
#define HTTP_IP @[@"114.141.158.125", @"120.132.147.118", @"117.121.55.39"]

static char asrAudioFileServer_domain[32] = "edu.hivoice.cn";
static short asrAudioFileServer_port = 80;

//debug log 开关
#define USC_BUG 1

//自定义log写文件(在USC_BUG = 1时才能切换)
#define CUSTOM_LOG 0
