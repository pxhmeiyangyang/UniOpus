//
//  ASRVadWapper.cpp
//  usc
//
//  Created by hejinlai on 12-11-15.
//  Copyright (c) 2012年 yunzhisheng. All rights reserved.
//

#include "ASRVadWapper.h"
#include "string.h"
#include "AsrVad.h"

#define INPUT_BUFFER_LEN	(8000*2)	// 500ms
#define VAD_BUFFER_LEN		(4000*2)	// 250ms


ASRVadWapper::ASRVadWapper()
{
    vad = new AsrVAD();
}

int ASRVadWapper::init()
{
    if (vad != NULL) {
        return vad->init();
    }
    return -1;
}

void ASRVadWapper::setVadTimeout(int frontTimeout, int backTimeout)
{
    if (vad != NULL)
    {
        vad->setVadSilTime(frontTimeout, backTimeout);
    }
}

void ASRVadWapper::reset()
{
    if (vad != NULL) {
        vad->reset();
    }
}

int ASRVadWapper::isActive(char *pcm, int len, int *energy)
{
    int status = vad->process((short*)pcm, len/(sizeof(short)), energy);
    if(status == 1 || status == 2){
        return 0;
    }else if(status == 0 || status == 3){
        return 1;
    }else{
        return 0;
    }
}

int ASRVadWapper::isVadTimeout(char *pcm, int len, int *volumn)
{
    // 拷贝录音数据，防止程序崩溃
    char *pcmData = new char[len];
    memcpy(pcmData, pcm, len);
    
    int offset = 0;
    do
    {
        int readLen = (len-offset)>VAD_BUFFER_LEN?VAD_BUFFER_LEN:(len-offset);
        int r = isActive(pcmData + offset, readLen, volumn);
        if (r == 0)
        {
            // 释放资源
            delete [] pcmData;
            return VADTimeout;
        }
			
			offset += readLen;
    }while(offset < len);
        
    // 释放资源
    delete [] pcmData;
  	return 0;
}

ASRVadWapper::~ASRVadWapper()
{
    delete vad;
    vad = NULL;
}
