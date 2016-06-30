//
//  ASRClientWrapper.h
//  asr
//
//  Created by yunzhisheng on 12-8-15.
//  Copyright (c) 2012年 yunzhisheng. All rights reserved.
//

#ifndef __asr__ASRClientWrapper__
#define __asr__ASRClientWrapper__

#include "asrclient.h"

class ASRClientWrapper
{
private:
    int count;
    AsrServiceInterface* asrService;
    
public:
    ASRClientWrapper();
    ~ASRClientWrapper();
    
    int init(const char* ip, short port);
    int setValueInt(int id, int value);
    int setValueString(int id, const char *s);
    int start();
    int isActive(char* pcm, int len);
    int recognize(char* pcm, int len);
    int stop();
    char* getResult();
    int cancel();
    int getLastError();
    const char* getSessionId();
};


#endif /* defined(__asr__ASRClientWrapper__) */
