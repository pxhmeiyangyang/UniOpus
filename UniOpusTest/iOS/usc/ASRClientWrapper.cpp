//
//  ASRClientWrapper.cpp
//  asr
//
//  Created by yunzhisheng on 12-8-15.
//  Copyright (c) 2012年 yunzhisheng. All rights reserved.
//

#include "ASRClientWrapper.h"
//#include <iostream.h>
#include <iostream>
#include "ServSupAttrs.h"


#define INPUT_BUFFER_LEN	(8000*2)	// 500ms
#define VAD_BUFFER_LEN		(4000*2)	// 250ms


ASRClientWrapper::ASRClientWrapper()
    : count(0)
    , asrService(NULL)
{
    
}

ASRClientWrapper::~ASRClientWrapper()
{
    asrDestroyAsrService(asrService);
	asrService = NULL;
}

int ASRClientWrapper::init(const char* ip, short port)
{ 
	int r = SUCC_CODE::ASRCLIENT_RECOGNIZER_OK;

  	asrService = (AsrServiceInterface*)asrCreateAsrService(ip,port);
  	if(asrService == NULL)
    {
        r = INTERNAL_ERR::ASRCLIENT_CREATE_ASRSERVER_ERROR;
    }
  		
  	return r;
}

int ASRClientWrapper::setValueInt(int id , int value)
{
	int r = INTERNAL_ERR::ASRCLIENT_CREATE_ASRSERVER_ERROR;
	
	if( asrService != NULL)
	{
		r = asrService->setValueInt(id, value);
	}
	return r;
    
}

int ASRClientWrapper::setValueString(int id , const char* s)
{
	int r = INTERNAL_ERR::ASRCLIENT_CREATE_ASRSERVER_ERROR;
	
	if( asrService != NULL)
	{
		r = asrService->setValueString(id, s);
	}
	return r;
}


int ASRClientWrapper::start()
{
	int r = INTERNAL_ERR::ASRCLIENT_CREATE_ASRSERVER_ERROR;
    
  	if( asrService != NULL)
  	{
		count = 0;
  		r = asrService->start();
  	}
  	return r;
}

int ASRClientWrapper::isActive(char* pcm, int len)
{
    int r = INTERNAL_ERR::ASRCLIENT_CREATE_ASRSERVER_ERROR;
	
  	if( asrService != NULL)
  	{
        // 拷贝录音数据，防止程序崩溃
        char *pcmData = (char *)malloc(len);
        memcpy(pcmData, pcm, len);
        
  		int offset = 0;
		r = SUCC_CODE::ASRCLIENT_RECOGNIZER_OK;
		do
		{
			int readLen = (len-offset)>VAD_BUFFER_LEN?VAD_BUFFER_LEN:(len-offset);
	  		r = asrService->isactive(pcmData + offset, readLen);
			if (r == 0)
			{
               	 	// 释放资源
                		free(pcmData);
				return INPUT_CHECKER_ERR::ASRCLIENT_VAD_TIMEOUT;
			}
			
			offset += readLen;
		}while(offset < len);
        
        // 释放资源
        free(pcmData);
  	}
  	return r;
}

int ASRClientWrapper::recognize(char* pcm, int len)
{
	int r = INTERNAL_ERR::ASRCLIENT_CREATE_ASRSERVER_ERROR;
	
  	if( asrService != NULL)
  	{
        // 拷贝录音数据，防止程序崩溃
        char *pcmData = (char *)malloc(len);
        memcpy(pcmData, pcm, len);
        
        //printf("%s\n",pcmData);
        
  		int offset = 0;
		r = SUCC_CODE::ASRCLIENT_RECOGNIZER_OK;
		do
		{
			int readLen = (len-offset)>INPUT_BUFFER_LEN?INPUT_BUFFER_LEN:(len-offset);
	  		r = asrService->recognizer(pcmData + offset, readLen);
			if (r<0)
			{
                // 释放资源
                free(pcmData);
				return r;
			}
			
			offset += readLen;
		}while(offset < len);
        
		count += len;
		if(count > INPUT_BUFFER_LEN*4)
		{
			count = 0;
			r = asrService->queryResult();
		}
        
        // 释放资源
        free(pcmData);
  	}
  	return r;
}

int ASRClientWrapper::stop()
{
	int r = INTERNAL_ERR::ASRCLIENT_CREATE_ASRSERVER_ERROR;
  	if( asrService != NULL)
  	{
  		r = asrService->stop();
  	}
	return r;
}

int ASRClientWrapper::cancel()
{
	int r = INTERNAL_ERR::ASRCLIENT_CREATE_ASRSERVER_ERROR;
  	if( asrService != NULL)
  	{
  		r = asrService->cancel();
  	}
	return r;
}

char* ASRClientWrapper::getResult()
{
    char *r = NULL;
  	if( asrService != NULL)
  	{
  		r = asrService->getResult();
  	}
	return r;
}

int ASRClientWrapper::getLastError()
{
    int r = INTERNAL_ERR::ASRCLIENT_CREATE_ASRSERVER_ERROR;
  	if( asrService != NULL)
  	{
  		r = asrService->getLastError();
  	}
	return r;
}

const char* ASRClientWrapper::getSessionId()
{
    const char *r = NULL;
  	if( asrService != NULL)
  	{
  		r = asrService->getOptionValue(SERV_SUP_ATTR::SSUP_RSP_SESSION_ID);
  	}
	return r;
}


