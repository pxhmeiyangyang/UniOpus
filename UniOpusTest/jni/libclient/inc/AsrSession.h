#ifndef	ASR_SESSION_H
#define	ASR_SESSION_H

#include "ProtocolParser.h"
#ifdef DES_ALGO
#include <openssl/des.h>
#endif
/*#include <iostream>

using namespace std;*/
extern SERV_ATTRS gservATTR;
static const unsigned int MAX_RSP_BODY_LEN=1024*1024;

class AsrSession
{
#ifdef TEST_SERVER_PARSING_PACKET
	struct SFrameRequestHeader
	{
		char	magic_number1;
		char	magic_number2;
		char	version1;
		char	version2;
		char	req_type;
		char	compress;
		char	resv1;
		char	resv2;
		int	len;
	};

	struct SFrameCommonTailer
	{
		char	tailerFlag1;
		char	tailerFlag2;
	};

	struct SAttrHeader
	{
		unsigned char	AttrCode;
		int		length;
	};

	//for frame integrity check
	/*
	static const char	MAGICNUM1_ ='M';
	static const char	MAGICNUM2_ ='@';
	static const char	TAILNUM1_ ='!';
	static const char	TAILNUM2_ ='@';
	*/

	static const char	MASTER_ALLOWED_VERSION = 0x02;
	static const char	SECONDARY_ALLOWED_VERSION=0x01;
#endif

	static const unsigned int kMsgFrameLen_ = (AttrsLimit::attrValueLenLimit)*(AttrsLimit::attrsNumberLimit)+sizeof(int);

public:

	AsrSession();

	~AsrSession();

	//bool isConnected(){return fd>0;}
	int sess_conn_server(char* server_ip, int server_port);
	void sess_close_fd(int* fd_ptr);
	void sess_disconn_server();
	//int start(int compress,int,int,char*,char*);
	int sess_start();
	int start_cb(int statuscode, char* msgbody, unsigned int msglen);
	int sess_resume(char* data,int len);
	int sess_stop(int waitingTime);
	int sess_stop_cb(int statuscode, char* msgbody, unsigned int msglen);
	void sess_set_res(char* mResult)
	{
		sess_result_ = mResult;
		sess_has_res_ = true;
        }
	char* sess_get_res();
	//int sess_get_resBuffLen(){return bufLen;}
	/*char* enlargeResultBuff(int len)
		{
			delete[] result;
			result=new char[len];
			return result;
		}*/
	int
	sess_get_fd(){return sess_fd_;}

	int
	sess_cancel();

	int
	sess_query_res(int waitingTime);

	int
	sess_query_res_cb(int statuscode, char* msgbody, unsigned int msglen);

	int
	sess_call_back(int sentCmd,int statuscode,char* rspmsgbody,unsigned int rspmsglen);

	//struct AttrInfoBlk* getattrblk();
	bool
	sess_chk_vld_attr(struct AttrInfoBlk* attrs);

	bool
	sess_chk_vld_attr(unsigned char attr_code, const char* attr_value);
	
	int
	sess_set_attr(unsigned char attr_code, const char* attr_value);

	char*
	sess_get_attr(unsigned char attr_code);

	void
	sess_set_mtu_len(int len)
	{
		if(len >= 512)
		{
			sess_mtu_len_=len;
		}
	}

		
private:
	int
	sess_status_to_errcode(int status);

	int
	sess_trans_status(int PPstatus);

	bool
	sess_decrypt_txt(char* srcText,int len);

#ifdef DES_ALGO
	int coreDecrypt_des(char* srcText, int len);
#else
	int coreDecrypt_easy(char* srcText, int len);
#endif
	int sess_send_n ( int* socket, char* buffer,int len,int timeout = -1);
	int sess_recv_n ( int* socket, char* buffer,int len,int timeout=-1);
	void sess_clr_attrs()
	{
		if(sess_attr_blk_ != NULL)
		{
			delete sess_attr_blk_;
		}
		sess_attr_blk_ = NULL;
	}

	int sess_chk_attr(unsigned char attrCode, const char* attrValue){
		if(attrCode == SERV_SUP_ATTR::SSUP_APP_KEY)
		{
			if(strlen(attrValue)!=40)
				return -1;
		}
		return 0;
	}
private:

	int			sess_fd_;
	char*			sess_result_;
	bool			sess_has_res_;
	char			sess_rsp_buf_[MAX_RSP_BODY_LEN];
	int bufLen; //length of result
	int			sess_pcm_len_;
	class ProtocolParser	sess_proto_psr_;
	//struct AttrInfoBlk* m_pAttrBlk;
	struct RecvMsgBlock	sess_recv_blk_;
	struct AttrInfoBlk*	sess_attr_blk_;
	unsigned int sess_mtu_len_;
#ifdef PPDEBUG
	char moduleName[20];
#endif


/******************************************************
*For debug perpose
******************************************************/
//#define PPDEBUG
#ifdef PPDEBUG

#define SLOG_ERROR_MSG(x) {\
	sess_proto_psr_.m_logger.logV1( LOG_LEVEL::LOG_ERROR, moduleName, "Error", x);}

//for debug usage, and trace info is int
#define SLOG_INT_TRACE(LEVEL, TAG, TRACE) {\
	char tmp[100];\
	sprintf(tmp, "%d", (int)(TRACE));\
	sess_proto_psr_.m_logger.logV1( (LEVEL), moduleName, (TAG), tmp);}	

#define SLOG_STRING_TRACE( LEVLE,  TAG, TRACE) {\
	sess_proto_psr_.m_logger.logV1( (LEVLE), moduleName, (TAG), (TRACE));}		

#define SLOG_HEAP_ALLO(x) {\
	SLOG_INT_TRACE(LOG_LEVEL::LOG_INFO, "HeapAllocate", x);}

#define SLOG_HEAP_FREE(x) {\
	SLOG_INT_TRACE(LOG_LEVEL::LOG_INFO, "HeapFree", x);}

#define SLOG_SENT_REQ_TYPE(x) {\
	SLOG_INT_TRACE(LOG_LEVEL::LOG_INFO, "SentRequestType", x);}

#define SLOG_PPRSP_STATUS_CODE(x) {\
	SLOG_INT_TRACE(LOG_LEVEL::LOG_INFO, "PPReturnCode", x);}

#define SLOG_ASR_RSP_CODE(x) {\
	SLOG_INT_TRACE(LOG_LEVEL::LOG_INFO, "AsrRspCode", x);}

#define SLOG_ASR_RSP_BODY_LEN(x) {\
	SLOG_INT_TRACE(LOG_LEVEL::LOG_INFO, "AsrRspBodyLen", x);}

#define SLOG_ASR_REQ_BODY_LEN(x) {\
	SLOG_INT_TRACE(LOG_LEVEL::LOG_INFO, "AsrRspBodyLen", x);}

#define SLOG_RSP_STATUS(x) {\
	SLOG_INT_TRACE(LOG_LEVEL::LOG_INFO, "RSPStatus", x);}

#define SLOG_REQ_TYPE(x) {\
	SLOG_INT_TRACE(LOG_LEVEL::LOG_INFO, "RequestType", x);}

#define SLOG_REQ_BODY_LEN(x) {\
	SLOG_INT_TRACE(LOG_LEVEL::LOG_INFO, "ReqBodyLen", x);}

#define SLOG_RSP_BODY_LEN(x) {\
	SLOG_INT_TRACE(LOG_LEVEL::LOG_INFO, "RspBodyLen", x);}

#define SLOG_ATTRS_TOT_LEN(x) {\
	SLOG_INT_TRACE(LOG_LEVEL::LOG_INFO, "AttrToTLen", x);}

#define SLOG_ATTRS_NUM(x) {\
	SLOG_INT_TRACE(LOG_LEVEL::LOG_INFO, "AttrToTNumber", x);}

#define SLOG_ATTR_CODE(x) {\
	SLOG_INT_TRACE(LOG_LEVEL::LOG_INFO, "AttrCode", x);}

#define SLOG_ATTR_LEN(x) {\
	SLOG_INT_TRACE(LOG_LEVEL::LOG_INFO, "AttrLen", x);}

#define SLOG_SENT_NUM(x) {\
	SLOG_INT_TRACE(LOG_LEVEL::LOG_INFO, "SentDataNum", x);}

#define SLOG_ALL_ATTRS(x) {\
	if(x==NULL)\
	{\
		SLOG_STRING_TRACE(LOG_LEVEL::LOG_INFO, "Attrs", "NULL");\
	}\
	else \
	{\
		for(int i=0;i<x->AttrNum;i++)\
		{\
			SLOG_INT_TRACE(LOG_LEVEL::LOG_INFO, "AttrCode", x->Attrs[i].AttrCode);\
			SLOG_STRING_TRACE(LOG_LEVEL::LOG_INFO, "AttrValue", x->Attrs[i].AttrValue);\
		}\
	}}

#define SLOG_FUNCTIONC_IN(x) {\
	SLOG_STRING_TRACE(LOG_LEVEL::LOG_INFO, "FunctionIn", x);}

#define SLOG_FUNCTIONC_OUT(x) {\
	SLOG_STRING_TRACE(LOG_LEVEL::LOG_INFO, "FunctionOut", x);}

/*#define SLOG_ERROR_MSG(x) {\
	SLOG_ERROR_MSG(x);}*/

#define SLOG_OUTPUT_BUFFER(x) {\
	SLOG_STRING_TRACE(LOG_LEVEL::LOG_INFO, "BufferData", x);}

#define SLOG_RSP_MSG_BODY(x) {\
	SLOG_STRING_TRACE(LOG_LEVEL::LOG_INFO, "RspMsgBody", x);}

#define SLOG_TINY_INFO(x) {\
	SLOG_STRING_TRACE(LOG_LEVEL::LOG_INFO, "TinyInfo", x);}


#define SLOG_ATTR_VALUE(x) {\
	SLOG_STRING_TRACE(LOG_LEVEL::LOG_INFO, "AttrValue", x);}

#define SLOG_RESULT_VALUE(x)  {\
	SLOG_STRING_TRACE(LOG_LEVEL::LOG_INFO, "Result", x);}

#define SLOG_CONVERT_INT_TO_STRING(IN, OUT) {\
    OUT=new char[100];\
    sprintf(OUT, "%d", (int)(IN));\}

#define SLOG_FREE_STRING(OUT) {delete[] OUT}

#define SLOG_EVENT_TRACE(MODULE, TAG, TRACE) {\
	sess_proto_psr_.m_logger.logV1( LOG_LEVEL::LOG_DEBUG, (MODULE), (TAG), (TRACE));}	

#else

#define SLOG_HEAP_ALLO(x)
#define SLOG_HEAP_FREE(x)
#define SLOG_FUNCTIONC_IN(x)
#define SLOG_FUNCTIONC_OUT(x)
#define SLOG_ERROR_MSG(x) {printf("Error Happen.\n");}
#define SLOG_OUTPUT_BUFFER(x)
#define SLOG_SENT_REQ_TYPE(x)  
#define SLOG_RSP_MSG_BODY(x)  
#define SLOG_PPRSP_STATUS_CODE(x) 
#define SLOG_TINY_INFO(x)
#define SLOG_ASR_RSP_CODE(x)
#define SLOG_ASR_RSP_BODY_LEN(x) 
#define SLOG_ASR_REQ_BODY_LEN(x)
#define SLOG_ATTR_VALUE(x)
#define SLOG_RSP_STATUS(x) 
#define SLOG_REQ_TYPE(x)
#define SLOG_REQ_BODY_LEN(x)
//#define SHOWATTRVALUE(x)
#define SLOG_ATTRS_TOT_LEN(x)
#define SLOG_RSP_BODY_LEN(x)
#define SLOG_ATTR_LEN(x)
#define SLOG_ALL_ATTRS(x)
#define SLOG_ATTRS_NUM(x)
#define SLOG_SENT_NUM(x)
#define SLOG_RESULT_VALUE(x) 
#define SLOG_ATTR_CODE(x) 
#define SLOG_INT_TRACE(LEVEL, TAG, TRACE)
#define SLOG_STRING_TRACE( LEVLE, TAG, TRACE)
#define SLOG_EVENT_TRACE(MODULE, TAG, TRACE)
#define SLOG_CONVERT_INT_TO_STRING(IN, OUT)
#define SLOG_FREE_STRING(OUT)

#endif
/******************************************************
*~For debug perpose
******************************************************/

};


#endif
