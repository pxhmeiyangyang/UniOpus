/*
 * ProtocolParser.cpp
 *
 *  Created on: 2012-7-19
 *      Author: dongzhaokun
 */
//#include <iostream>
//#include <string.h>



#include "ProtocolParser.h"
#include "protocol.h"

#include <errno.h>
#include "asrclient.h"
#include "log.h"
//#include <stdio.h>
//#include <string.h>
//#include <stdint.h>

#ifdef WIN32

#include <winsock.h>
#include <WinSock2.h>
#include <WinSock.h>
#include <time.h>

#pragma comment(lib,"ws2_32.lib")

#endif

using namespace std;

int
FindPosOfMagic(char* src,unsigned int len,const char magic)
{
	int result=-1;

	for(unsigned int i=0;i<len;i++)
	{
		if(*(src+i)==magic)
		{
			result=i;
			break;
		}
	}

	return result;
}

ProtocolParser::ProtocolParser()
{
/*#ifdef PPDEBUG
	m_logger=new JoeLogger();
#endif
	LOG_HEAP_ALLO(sizeof(JoeLogger));*/

	/*m_tmpAttr_buff.totlen=MAXATTRLEN;
	m_tmpAttr_buff.reallen=0;
	m_tmpAttr_buff.buff=new char[m_tmpAttr_buff.totlen];

	//m_msgbody=new char[1204];
	//msgbylen=1024;*/

	//SETINTERNALSTATUS(m_internalStatus,PP_OK);
#ifdef PPDEBUG
    strcpy(moduleName, "Protocol");
#endif

}

ProtocolParser::~ProtocolParser()
{
	//cout<<"Releasing ProtocolParser!"<<endl;
	//LOG_HEAP_FREE(sizeof(JoeLogger));
/*#ifdef PPDEBUG
	delete 	m_logger;
#endif*/

	/*delete[] m_tmpAttr_buff.buff;
	delete[] m_msgbody;*/
}


PParserResult
ProtocolParser::IntegrityAdd(char* pdata)
{
	LOG_FUNCTIONC_IN("IntegrityAdd");
	LOG_FUNCTIONC_OUT("IntegrityAdd");
	return PP_OK;
}

PParserResult
ProtocolParser::ParseDataFromServer(char* pBuffer, struct RecvMsgBlock* pMSGBlk, int actionTimeout)
{
	LOG_FUNCTIONC_IN("ParseDataFromServer");
	PParserResult	result = PP_OK;
	int		attrs_len = 0;
	int		attrnum;
		
	LOG_ASR_RSP_BODY_LEN(pMSGBlk->msgFrameLen);
	LOG_ASR_RSP_CODE(pMSGBlk->rsp_status);

	if(pMSGBlk->msgFrameLen==0)
	{
		LOG_TINY_INFO("RspBodyLenIsZero");
	}
	else
	{
		LOG_ASR_RSP_CODE(pMSGBlk->rsp_status);
		
		//m_msgbody[pMSGBlk->msgFrameLen]=0;
		attrnum = ntohl(*(int*)pBuffer);			

		LOG_TINY_INFO("EnterGetAttrFromBuff");
		GetAttrFromBuff(&(pMSGBlk->attrBlk), pBuffer, attrnum, &attrs_len);
		pMSGBlk->msgBody = pBuffer + attrs_len;
		pMSGBlk->msgBodyLen = pMSGBlk->msgFrameLen - attrs_len;
		*(pMSGBlk->msgBody + pMSGBlk->msgBodyLen) = 0;
		LOG_TINY_INFO("QuitGetAttrFromBuff");

		LOG_PPRSP_STATUS_CODE(result);
	}

	LOG_FUNCTIONC_OUT("ParseDataFromServer");
	return result;
}


//m_bufferBlock.buff has translate the bytes sequence from network to host


/*
 * firstly, check that the frame is not been corrupted
 * secondly, check that attribute length is normal
 */
bool
ProtocolParser::GetAttrFromBuff(struct AttrInfoBlk*	attrblk,
				     char*			pdstBuff,
				     int			attrnum,
				     int*			attrslen)
{
	bool	result = true;
	int	offset = sizeof(ATTRNUMTYPE);

	if(attrnum==0)
	{

		if(attrslen!=NULL)
		{
			*attrslen=sizeof(ATTRNUMTYPE);
		}
		return result;
	}
		
	struct AttrHeader* pSLAttr=(struct AttrHeader*)(pdstBuff+offset);
	//string attrValue;

	offset+=sizeof(struct AttrHeader)*attrnum;
	int tmpattrslen=sizeof(ATTRNUMTYPE)+sizeof(struct AttrHeader)*attrnum;	
	
	for(int i=0;i<attrnum;i++)
	{
		/*
		attributes format

		|     int     |    sizeof(struct AttrHeader)* attrnum    |   lengthofstring*attrnum |
		| attrnum |      (struct AttrHeader)* attrnum           |    attrvalue * attrnum    |
		
		*/
		pSLAttr->length=ntohl(pSLAttr->length);
		tmpattrslen +=pSLAttr->length;
		
		if(attrblk->Code2IndexTab[pSLAttr->AttrCode]==AttrInfoBlk::NOTSETFLAG)
		{
			attrblk->Attrs[attrblk->AttrNum].AttrCode=pSLAttr->AttrCode;
		   LOG_ATTRS_TOT_LEN(pSLAttr->length);
			LOG_TINY_INFO("In0xFF");		
			
			//we decrypt attributes value here
			DecryptAttrs(pdstBuff+offset, pSLAttr->length);
			
			memcpy(attrblk->Attrs[attrblk->AttrNum].AttrValue, pdstBuff+offset, pSLAttr->length);
			attrblk->Attrs[attrblk->AttrNum].AttrValue[pSLAttr->length]=0;
			attrblk->Code2IndexTab[pSLAttr->AttrCode]=attrblk->AttrNum;
			LOG_TINY_INFO(attrblk->Attrs[attrblk->AttrNum].AttrValue);
			attrblk->AttrNum++;			
		}
		else
		{		
			LOG_TINY_INFO("In~0xFF");
			LOG_ATTRS_TOT_LEN(pSLAttr->length);

			//we decrypt attributes value here
			DecryptAttrs(pdstBuff+offset, pSLAttr->length);
			
			memcpy(attrblk->Attrs[attrblk->Code2IndexTab[pSLAttr->AttrCode]].AttrValue, pdstBuff+offset, pSLAttr->length);
			attrblk->Attrs[attrblk->Code2IndexTab[pSLAttr->AttrCode]].AttrValue[pSLAttr->length]=0;
			attrblk->Code2IndexTab[pSLAttr->AttrCode]=i;
		}
		
		offset+= pSLAttr->length;		
		pSLAttr++;
	}

	LOG_ALL_ATTRS(attrblk);

	if(attrslen!=NULL)
		*attrslen=tmpattrslen;

	LOG_ATTRS_TOT_LEN(tmpattrslen);
	
	LOG_FUNCTIONC_OUT("GetAttrFromBuff");
	return result;
}

PParserResult ProtocolParser::TranslateSequence(int headerType, int direction,char *dstData)
{
	LOG_FUNCTIONC_IN("TranslateSequence");
	PParserResult result=PP_OK;

	if(headerType==RSPHEADER && direction==FROMHOST2NETWORK)
		{
			struct FrameResponseHeader* rsq_head;

			rsq_head=(struct FrameResponseHeader *)dstData;
			rsq_head->len= htonl(rsq_head->len);
		}

	LOG_FUNCTIONC_OUT("TranslateSequence");
	return result;
}

bool
ProtocolParser::FillAttr2Buff(struct AttrInfoBlk* attrsBlk,char* tmpAttr_buff,int* attrs_len)
{
	LOG_FUNCTIONC_IN("FillAttr2Buff");
	bool result=true;
	int tmpreallen=0;
	
	if(attrsBlk==NULL)
	{
		*(int*)(tmpAttr_buff)=0;
		tmpreallen=sizeof(ATTRNUMTYPE);
		if(attrs_len!=NULL)
			*attrs_len=tmpreallen;

		return result;
	}

	//We encrypt attributes here
	/*if(attrsBlk!=NULL)
	{
		//unsigned char index=m_attrBlk->Code2IndexTab[SERV_SUP_ATTR::SSUP_APP_KEY];
		unsigned char attrsNum=attrsBlk->AttrNum;
		for(unsigned char index=0; index<attrsNum; index++)
		{
			//It is very ugly!! We should figure out a better way to handle it 
			//static const unsigned char KEYLEN=40;
			unsigned int attrLen=attrsBlk->Attrs[index].AttrValue.length();
			char* tmp=new char[attrLen+1];
			strncpy(tmp, attrsBlk->Attrs[index].AttrValue.c_str(), attrLen);
			for(unsigned int i=0; i<attrLen; i++)
				tmp[i]^='@';
			tmp[attrLen]=0;
			attrsBlk->Attrs[index].AttrValue.assign(tmp);
		}
	}*/

	//map<char, string>::iterator attr_Iter;
	//memset((void*)tmpAttr_buff,0,m_tmpAttr_buff.totlen);
	struct AttrHeader* attrCS=(struct AttrHeader *)(tmpAttr_buff+sizeof(ATTRNUMTYPE));
	//char* attrsBuffer=(char*) attrCS;
    char *attrsValue=tmpAttr_buff+sizeof(ATTRNUMTYPE)+sizeof(struct AttrHeader)*(attrsBlk->AttrNum + 1);//for additional 0x1d attribute
	//fill total attributes number and set reallen of buffer
    *(int*)(tmpAttr_buff)=htonl(attrsBlk->AttrNum + 1);//for additional 0x1d attribute
	tmpreallen=sizeof(ATTRNUMTYPE);

	//we have to check if attribute's length is out of limit, we didn't do it yet!!!!!!!!!!!!!!!!!!!!
    int i =0;
    for(;i<attrsBlk->AttrNum;i++)
	{
		/*
		attributes format

		|     int     |    sizeof(struct AttrHeader)* attrnum    |   lengthofstring*attrnum |
		| attrnum |      (struct AttrHeader)* attrnum           |    attrvalue * attrnum    |
		
		*/
       int lenOfAttr=strlen(attrsBlk->Attrs[i].AttrValue);

		attrCS->AttrCode=attrsBlk->Attrs[i].AttrCode;
		attrCS->length=htonl(lenOfAttr);
		LOG_ATTR_LEN(lenOfAttr);
		memcpy(attrsValue, attrsBlk->Attrs[i].AttrValue, lenOfAttr);

		//printf("%d attr:%s\n", i, attrsBlk->Attrs[i].AttrValue);
		//printf("%d attr:%s len:%d\n", i, attrsValue, lenOfAttr);

		//We encrypt attribute value here
		EncryptAttrs(attrsValue, lenOfAttr);
		
		LOG_ATTR_VALUE(attrsValue);
		attrCS ++;
		attrsValue +=lenOfAttr;
		tmpreallen +=sizeof(struct AttrHeader)+lenOfAttr;		
	}
    //add 0x1d:"1"
    
    attrCS->AttrCode = 0x1d;
    attrCS->length = htonl(1);
    memcpy(attrsValue, "1", 1);
    EncryptAttrs(attrsValue, 1);
    tmpreallen += sizeof(struct AttrHeader) + 1;

    
	if(attrs_len!=NULL)
		*attrs_len=tmpreallen;
	
	LOG_FUNCTIONC_OUT("FillAttr2Buff");
	return result;
}

/*
 *
 */
PParserResult
ProtocolParser::AssembleData(struct SentMsgBlock*	pSentMsgBlk,
			     SentMsgHandler*		pHandler,
			     unsigned int*		frameLen)
{
	LOG_FUNCTIONC_IN("AssembleData");
	
	// 1. set the return value
	PParserResult	result = PP_OK;
	int		attrs_len = 0;

	// 2. pSentMsgBlk's header
	struct FrameRequestHeader*	req_header = (struct FrameRequestHeader*)pSentMsgBlk->finalMsgFrame;
	memset(req_header, 0, sizeof(struct FrameRequestHeader));

	//unsigned int	req_head_len = sizeof(struct FrameRequestHeader);
	int	req_head_len = sizeof(struct FrameRequestHeader);
	
	// 3. set header
	req_header->magic_number1 = MAGICNUM1;
	req_header->magic_number2 = MAGICNUM2;
	req_header->version1 = MASTER_ALLOWED_VERSION;
	req_header->version2 = SECONDARY_ALLOWED_VERSION;
	req_header->req_type = pSentMsgBlk->req_type;
	req_header->compress = 1;	//compress ;	

	// 4. add some log infomation
	IntegrityAdd((char*)&req_header);

	LOG_TINY_INFO("EnterFill");

	//5. fill the header's attrs into buff
	FillAttr2Buff(pSentMsgBlk->attrsBlk, pSentMsgBlk->finalMsgFrame + sizeof(struct FrameRequestHeader), &attrs_len);
	req_head_len += attrs_len;	// 

	LOG_TINY_INFO("EnterFill");
	LOG_ATTRS_TOT_LEN(attrs_len);

	//6. Fill the body data into buff
	if(pSentMsgBlk->msgbodylen>0)
	{
		memcpy(pSentMsgBlk->finalMsgFrame + req_head_len,
		       pSentMsgBlk->msgbody,
		       pSentMsgBlk->msgbodylen);
		req_head_len += pSentMsgBlk->msgbodylen;

		if(pHandler!=NULL)
		{
			pHandler->adjustMsg(pSentMsgBlk->msgbody,
					    pSentMsgBlk->finalMsgFrame + sizeof(struct FrameRequestHeader) + attrs_len,
					    pHandler->parameter,
					    pSentMsgBlk->msgbodylen);			
		}
	}

	LOG_REQ_BODY_LEN(req_head_len);

	// 7. Fill tail into buff
	struct FrameCommonTailer*	tail = (struct FrameCommonTailer*)(pSentMsgBlk->finalMsgFrame + req_head_len);
	tail->tailerFlag1 = TAILNUM1;
	tail->tailerFlag2 = TAILNUM2;
	req_head_len += sizeof(FrameCommonTailer);


	req_header->len = htonl( req_head_len - (int)sizeof(struct FrameRequestHeader));


	*frameLen = req_head_len;

	LOG_REQ_BODY_LEN(req_head_len);
	LOG_FUNCTIONC_OUT("AssembleData");

	return result;	
}

