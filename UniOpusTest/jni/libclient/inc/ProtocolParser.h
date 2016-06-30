/*
 * ProtocolParser.h
 *
 *  Created on: 2012-7-19
 *      Author: dongzhaokun
 */

#ifndef PROTOCOLPARSER_H_
#define PROTOCOLPARSER_H_

#include <stdio.h>

#ifdef LINUX
#include <netinet/in.h>
#elif defined (WIN32)
#include <WinSock2.h>
#endif

#ifdef PPDEBUG
#include "JoeLog.h"
#endif

#include "protocol.h"
#include "ServSupAttrs.h"
#include "log.h"

//using namespace std;

typedef enum
{
	PP_OK = 0,
	PP_ERROR,
	PP_FATAL_ERROR,
	PP_INTERNAL_WARNING,
	PP_BUFFER_OVER_FLOW,
	PP_VERSION_TOO_OLD,
	PP_COMMAND_ERROR,
	PP_COMMUNICATION_ERROR,
	PP_ATTRS_NOT_SUPPORT,
	PP_MSG_FORMAT_ERROR,
}PParserResult;


struct AttrUnit
{
	char	AttrCode;
	char	AttrValue[SERV_ATTRS::ATTR_SINGLE_LEN + 1];
};


struct AttrInfoBlk
{
	//Represent as array Code2IndexTab is not been set flag
	static const unsigned char NOTSETFLAG = 0xFF;

	//SERV_ATTRS::ATTR_NUM limit the server supported attribute number
	struct AttrUnit Attrs[SERV_ATTRS::ATTR_NUM];
	
	// record the current attrs number
	unsigned char AttrNum;
	unsigned char Code2IndexTab[SERV_ATTRS::ATTR_NUM];

	AttrInfoBlk()
	{
		Reset();
	}

	void Reset()
	{
		// there is no attrs on default
		AttrNum = 0;

		// all attrs are NOTSETFLAG(0xFF) on default
		for(int i = 0; i < SERV_ATTRS::ATTR_NUM; i++)
		{
			Code2IndexTab[i] = AttrInfoBlk::NOTSETFLAG;
			memset(Attrs[i].AttrValue, 0, SERV_ATTRS::ATTR_SINGLE_LEN + 1);
		}	
	}
};


struct RecvMsgBlock
{
	unsigned int		msgBodyLen;
	unsigned int		msgFrameLen;
	char*			msgBody;
	int			rsp_status;
	struct AttrInfoBlk	attrBlk;

	RecvMsgBlock()
	{
		msgBodyLen = 0;
		msgFrameLen = 0;
		rsp_status = 0;
		msgBody = NULL;
		attrBlk.Reset();
	}

	void Reset()
	{
		msgBodyLen = 0;
		msgFrameLen = 0;
		rsp_status = 0;
		msgBody = NULL;
		attrBlk.Reset();		
	}

	void ResetBodyInfo()
	{
		msgBodyLen=0;
		msgBody=NULL;		
	}
};



/*
 * 
 */
struct SentMsgBlock
{
	int			req_type;
	struct AttrInfoBlk*	attrsBlk;
	const char*		msgbody;
	unsigned int		msgbodylen;
	char*			finalMsgFrame;

	SentMsgBlock()
	{
		req_type = -1;
		attrsBlk = NULL;
		msgbody = NULL;
		msgbodylen = 0;
		finalMsgFrame = NULL;
	}
};



//This is the base class used to implemented by other class to provided some function to add operation on message which would be sent to user later
struct SentMsgHandler
{
	void* parameter;
	bool (*adjustMsg)(const char* pMsg2Sent, char* pFinalMsg, void* parameter, unsigned int msglen);
	SentMsgHandler()
	{
		adjustMsg = NULL;
		parameter = NULL;
	}
};



int
FindPosOfMagic(char* src,unsigned int len,const char magic);



class ProtocolParser
{
	//**************************************************
	#define RSPErrorStatus(x) x>=0xfff0?true:false
	#define MAXBUFFER4DATAINPUT 4096*1024
	//bodylen: without header struct and tail struct, only msg body, including attributes and msgbody
	//attrslen: attributes numbers (int) memory plus attributes (attribute header and attribute value)
	#define ATTRLENLGBODYLEN(attrslen,bodylen) attrslen>bodylen?true:false

	typedef int ATTRNUMTYPE;

	//for translate byte sequence from host to network or vice versa
	#define FROMHOST2NETWORK 0
	#define FROMNETWORK2HOST 1
	#define REQUESTHEADER 1
	#define RSPHEADER 0
	//for session encrypt or decrypt
	#define ENCRYPTDATA 0
	#define DECRYPTDATA 1

	//for frame integrity check
	static const char MAGICNUM1='M';
	static const char MAGICNUM2='@';
	static const char TAILNUM1='!';
	static const char TAILNUM2='@';

	static const char MASTER_ALLOWED_VERSION = 0x02;
	static const char SECONDARY_ALLOWED_VERSION=0x01;

	typedef enum{
		VERSION_TOO_OLD,
		ERROR_MAGIC,
		BUFFER_OF,
		OTHER_ERROR
	}AnalyResult;

	struct FrameRequestHeader{
		char magic_number1;
		char magic_number2;
		char version1;
		char version2;
		char req_type;
		char compress;
		char resv1;
		char resv2;
		int len;
	};

	struct FrameResponseHeader{
		int status_code;
		int len;
		FrameResponseHeader():status_code(0),len(0)
		{}
	};

	struct Response2User{
		struct FrameResponseHeader header;
		char body[4096];
	};

	struct FrameCommonTailer{
		char tailerFlag1;
		char tailerFlag2;
	};

	struct AttrHeader{
		unsigned char AttrCode;
		int length;
	};

	struct AttrBuff{
		int totlen;
		int reallen;
		char* buff;
	};

	//*****************************************************
	public:
		ProtocolParser();
		~ProtocolParser();
	public:

		unsigned int GetNum4Info() {return sizeof(FrameResponseHeader);}
		unsigned int HowManyBytes2Recv(char* pInfo, struct RecvMsgBlock* pMSGBlk) 
		{
			struct FrameResponseHeader* header=(struct FrameResponseHeader*)pInfo;
			pMSGBlk->rsp_status=ntohl(header->status_code);
			pMSGBlk->msgFrameLen=ntohl(header->len);
			return pMSGBlk->msgFrameLen;
		}
		PParserResult ParseDataFromServer (char* pBuffer, struct RecvMsgBlock* pMSGBlk, int actionTimeout=-1);
		//int ParseDataFromServer(int sentCmd,class AsrSession* pSession,int actionTimeout=-1);
		//PParserResult SendData2Server(int req_type,struct AttrInfoBlk* attrsBlk, char* msgbody,int msgbodylen,int fd,int actionTimeout=-1);
		PParserResult AssembleData(struct SentMsgBlock* pSentMsgBlk, SentMsgHandler* pHandler, unsigned int* frameLen);

	private:
		//internal usage function
		//1. translate from host to network byte sequence or vice versa
		PParserResult TranslateSequence(int headerType, int direction,char *dstData);//headerType can be request or response, direction can be from server2user or user2server
		//2. run state machine

		PParserResult IntegrityAdd(char* pdata);
		bool  GetAttrFromBuff(struct AttrInfoBlk* attrblk,char* pdstBuff,int attrnum,int *attrslen);

		//4. encrypt or decrypt
		//PParserResult Encrypt(int direction);//direction can be encrypt or decrypt
		bool FillAttr2Buff(struct AttrInfoBlk* attrsBlk,char* tmpAttr_buff,int* attrlen);
		//initialization
		PParserResult Reset();

		PParserResult EncryptAttrs(char* attrValue, unsigned int len){
			for (unsigned int i=0; i<len; i++)
				*(attrValue+i)^='@';
			return PP_OK;
		}

		PParserResult DecryptAttrs(char* attrValue, unsigned int len){
			for (unsigned int i=0; i<len; i++)
				*(attrValue+i)^='@';
			return PP_OK;
		}

	#ifdef PPDEBUG
	private:
	    char moduleName[20];
	public:
		JoeLogger m_logger;
	#endif
	/******************************************************
	*For debug perpose
	******************************************************/
	//#define PPDEBUG
	#ifdef PPDEBUG

	#define LOG_ERROR_MSG(x) {\
		m_logger.logV1( LOG_LEVEL::LOG_ERROR, moduleName, "Error", x);}

	//for debug usage, and trace info is int
	#define LOG_INT_TRACE(LEVEL, TAG, TRACE) {\
		char tmp[100];\
		sprintf(tmp, "%d", (int)(TRACE));\
		m_logger.logV1( (LEVEL), moduleName, (TAG), tmp);}	

	#define LOG_STRING_TRACE( LEVLE, TAG, TRACE) {\
		m_logger.logV1( (LEVLE), moduleName, (TAG), (TRACE));}		

	#define LOG_HEAP_ALLO(x) {\
		LOG_INT_TRACE(LOG_LEVEL::LOG_INFO, "HeapAllocate", x);}

	#define LOG_HEAP_FREE(x) {\
		LOG_INT_TRACE(LOG_LEVEL::LOG_INFO, "HeapFree", x);}

	#define LOG_SENT_REQ_TYPE(x) {\
		LOG_INT_TRACE(LOG_LEVEL::LOG_INFO, "SentRequestType", x);}

	#define LOG_PPRSP_STATUS_CODE(x) {\
		LOG_INT_TRACE(LOG_LEVEL::LOG_INFO, "PPReturnCode", x);}

	#define LOG_ASR_RSP_CODE(x) {\
		LOG_INT_TRACE(LOG_LEVEL::LOG_INFO, "AsrRspCode", x);}

	#define LOG_ASR_RSP_BODY_LEN(x) {\
		LOG_INT_TRACE(LOG_LEVEL::LOG_INFO, "AsrRspBodyLen", x);}

	#define LOG_ASR_REQ_BODY_LEN(x) {\
		LOG_INT_TRACE(LOG_LEVEL::LOG_INFO, "AsrRspBodyLen", x);}

	#define LOG_RSP_STATUS(x) {\
		LOG_INT_TRACE(LOG_LEVEL::LOG_INFO, "RSPStatus", x);}

	#define LOG_REQ_TYPE(x) {\
		LOG_INT_TRACE(LOG_LEVEL::LOG_INFO, "RequestType", x);}

	#define LOG_REQ_BODY_LEN(x) {\
		LOG_INT_TRACE(LOG_LEVEL::LOG_INFO, "ReqBodyLen", x);}

	#define LOG_FRAME_LEN(x) {\
		LOG_INT_TRACE(LOG_LEVEL::LOG_INFO, "FrameLen", x);}

	#define LOG_RSP_BODY_LEN(x) {\
		LOG_INT_TRACE(LOG_LEVEL::LOG_INFO, "RspBodyLen", x);}

	#define LOG_ATTRS_TOT_LEN(x) {\
		LOG_INT_TRACE(LOG_LEVEL::LOG_INFO, "AttrToTLen", x);}

	#define LOG_ATTRS_NUM(x) {\
		LOG_INT_TRACE(LOG_LEVEL::LOG_INFO, "AttrToTNumber", x);}

	#define LOG_ATTR_LEN(x) {\
		LOG_INT_TRACE(LOG_LEVEL::LOG_INFO, "AttrLen", x);}

	#define LOG_SENT_NUM(x) {\
		LOG_INT_TRACE(LOG_LEVEL::LOG_INFO, "SentDataNum", x);}

	#define LOG_ALL_ATTRS(x) {\
		for(int i=0;i<x->AttrNum;i++)\
		{\
			LOG_INT_TRACE(LOG_LEVEL::LOG_INFO, "AttrCode", x->Attrs[i].AttrCode);\
			LOG_STRING_TRACE(LOG_LEVEL::LOG_INFO, "AttrValue", x->Attrs[i].AttrValue);}}

	#define LOG_FUNCTIONC_IN(x) {\
		LOG_STRING_TRACE(LOG_LEVEL::LOG_INFO, "FunctionIn", x);}

	#define LOG_FUNCTIONC_OUT(x) {\
		LOG_STRING_TRACE(LOG_LEVEL::LOG_INFO, "FunctionOut", x);}

	/*#define LOG_ERROR_MSG(x) {\
		LOG_ERROR_MSG(x);}*/

	#define LOG_OUTPUT_BUFFER(x) {\
		LOG_STRING_TRACE(LOG_LEVEL::LOG_INFO, "BufferData", x);}

	#define LOG_RSP_MSG_BODY(x) {\
		LOG_STRING_TRACE(LOG_LEVEL::LOG_INFO, "RspMsgBody", x);}

	#define LOG_TINY_INFO(x) {\
		LOG_STRING_TRACE(LOG_LEVEL::LOG_INFO, "TinyInfo", x);}


	#define LOG_ATTR_VALUE(x) {\
		LOG_STRING_TRACE(LOG_LEVEL::LOG_INFO, "AttrValue", x);}

	#define LOG_RESULT_VALUE(x)  {\
		LOG_STRING_TRACE(LOG_LEVEL::LOG_INFO, "Result", x);}
		

	#else

	#define LOG_HEAP_ALLO(x)
	#define LOG_HEAP_FREE(x)
	#define LOG_FUNCTIONC_IN(x)
	#define LOG_FUNCTIONC_OUT(x)
	#define LOG_ERROR_MSG(x) {printf("Error Happen.\n");}
	#define LOG_OUTPUT_BUFFER(x)
	#define LOG_SENT_REQ_TYPE(x)  
	#define LOG_RSP_MSG_BODY(x)  
	#define LOG_PPRSP_STATUS_CODE(x) 
	#define LOG_TINY_INFO(x)
	#define LOG_ASR_RSP_CODE(x)
	#define LOG_ASR_RSP_BODY_LEN(x) 
	#define LOG_ASR_REQ_BODY_LEN(x)
	#define LOG_ATTR_VALUE(x)
	#define LOG_RSP_STATUS(x) 
	#define LOG_REQ_TYPE(x)
	#define LOG_REQ_BODY_LEN(x)
	//#define SHOWATTRVALUE(x)
	#define LOG_ATTRS_TOT_LEN(x)
	#define LOG_RSP_BODY_LEN(x)
	#define LOG_ATTR_LEN(x)
	#define LOG_ALL_ATTRS(x)
	#define LOG_ATTRS_NUM(x)
	#define LOG_SENT_NUM(x)
	#define LOG_RESULT_VALUE(x) 
	#define LOG_FRAME_LEN(x) 
	//for debug usage, and trace info is int
	#define LOG_INT_TRACE(LEVEL, TAG, TRACE) 
	#define LOG_STRING_TRACE( LEVLE, TAG, TRACE)

	#endif
	/******************************************************
	*~For debug perpose
	******************************************************/

};

#endif /* PROTOCOLPARSER_H_ */
