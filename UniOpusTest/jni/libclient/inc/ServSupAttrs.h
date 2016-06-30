/*
* ServSupAttrs.h header file is define some enum of server supporting attributes
*/
/*
* maybe we can make this to be xml format, and can be read from outside without compile the server core module
*/
#ifndef __SERVSUPATTR_H__
#define __SERVSUPATTR_H__

//#include <string>
//#include <iostream>
//using namespace std;
#include <string.h>
#include "Opuswrapper.h"

#define SUPPED 1
#define NOT_SUP 0

struct SERV_SUP_ATTR
{
	static const unsigned char SSUP_ENCRYPT = 0x00;
	static const unsigned char SSUP_DECODE = 0x01;
	static const unsigned char SSUP_AUDIO_ENC_METH = 0x02;	//support user and server point out which audio encode method used
	static const unsigned char SSUP_AUDIO_FMT_SET = 0x03;	//support define format of audio
	static const unsigned char SSUP_RSP_FMT_SET = 0x04;	//support define format of text returned to user
	static const unsigned char SSUP_RSP_ENC_METH = 0x05;	//support define coding format of result, for example, gb2312, utf8 or unicode
	static const unsigned char SSUP_RST_NUM = 0x06;		//support define how many result return to user
	static const unsigned char SSUP_GRAM_FMT_SET = 0x07;	//support define grammar method, url-list, abnf, or grxml
	static const unsigned char SSUP_CFD_SET = 0x08;		//support define limit of zhixindu
	static const unsigned char SSUP_SUB_CAGA_SET = 0x09;	//iat or asr catogery
	static const unsigned char SSUP_ENT_SET = 0x0a;		//support define different engine, sms16k, sms8k,vedio16k, video8k
	static const unsigned char SSUP_WAIT_TIMEOUT = 0x0b;	//support define waiting timeout duration
	static const unsigned char SSUP_IMEI_SET = 0x0c;	//support user set IMEI
	static const unsigned char SSUP_APP_KEY = 0x0d;		//support user sending APP_KEY
	//static const unsigned char SSUP_USER_CARRIED=0x0e;	//support user defined information
	static const unsigned char SSUP_ASR_OPT_PACKAGE_NAME = 0x0e;
	static const unsigned char SSUP_ASR_OPT_CARRIER = 0x0f;
	static const unsigned char SSUP_ASR_OPT_NETWORK_TYPE = 0x10;
	static const unsigned char SSUP_ASR_OPT_DEVICE_OS = 0x11;
	static const unsigned char SSUP_USER_ID = 0x12;
	static const unsigned char SSUP_COLLECTED_INFO = 0x13;
	static const unsigned char SSUP_REQ_RSP_ENTITY = 0x14;
	static const unsigned char SSUP_RSP_AUDIO_URL = 0x15;//deprecated,changed by SSUP_SESSION_ID
	static const unsigned char SSUP_MODEL_TYPE = 0x16;	//choose model type of ASR-Server
	static const unsigned char SSUP_USRDATA_FLAG = 0x17; 
	static const unsigned char SSUP_ORAL_EVAL_TEXT = 0x18;
	static const unsigned char SSUP_ORAL_TASK_TYPE = 0x19;
	static const unsigned char SSUP_ORAL_EX1 = 0x1A;
	static const unsigned char SSUP_ORAL_EX2 = 0x1B;
	static const unsigned char CURRSUPATTRNUM = SSUP_APP_KEY + 1;
	static const unsigned char MAXSUPATTRNUM = 255;		//max attribute number (using long to store attributes code)0~254, 255 is presenting no attribute set
    static const unsigned char SSUP_RSP_SESSION_ID = 0x1c;
};

class BaseAttrSet
{
public:
	virtual bool ValueInSupList(const char* value, unsigned int len){return true;}
};

class ReqRspEntity:public BaseAttrSet
{
	static const unsigned int VALUE_CHAR_LEN=100;
	static const unsigned int REQ_ENTITY_NUM=1;
    
    	//we should consider the end flag '/0' in string
	char list[ReqRspEntity::REQ_ENTITY_NUM][ReqRspEntity::VALUE_CHAR_LEN+1];
        
	bool ValueInSupList(const char* value, unsigned int len)
	{
		if(len > ReqRspEntity::VALUE_CHAR_LEN)
			return false;
        
		for(unsigned int i=0; i<ReqRspEntity::REQ_ENTITY_NUM; i++)
			if(strncmp(value, list[i], len)==0)
				return true;
                
		return false;
	}
public:
    	ReqRspEntity()
	{
		strcpy(list[0],  "req_audio_url");
	}
};


/*
 * Added by lizhongyuan
 */
class DeviceOS: public BaseAttrSet
{
	static const unsigned int kValueCharLen_ = 10;
	bool ValueInSupList(const char* value, unsigned int len)
	{
		if(len > DeviceOS::kValueCharLen_)
		{
			return false;
		}
		return true;
	}
};

class RsqAudioURL:public BaseAttrSet
{
	static const unsigned int VALUE_CHAR_LEN=100;
	bool ValueInSupList(const char* value, unsigned int len)
	{
		if(len > RsqAudioURL::VALUE_CHAR_LEN)
		{
			return false;
		}
		return true;
	}
};

class CollectedInfo:public BaseAttrSet{
	static const unsigned int VALUE_CHAR_LEN=100;
	bool ValueInSupList(const char* value, unsigned int len)
	{
		if(len>CollectedInfo::VALUE_CHAR_LEN)
			return false;
		return true;
	}
};

class UserID:public BaseAttrSet{
	static const unsigned int VALUE_CHAR_LEN=60;
	bool ValueInSupList(const char* value, unsigned int len)
	{
		if(len>UserID::VALUE_CHAR_LEN)
			return false;
		return true;
	}
};

class PackageName:public BaseAttrSet{
	static const unsigned int VALUE_CHAR_LEN=100;
	bool ValueInSupList(const char* value, unsigned int len)
	{
		if(len>PackageName::VALUE_CHAR_LEN)
			return false;
		return true;
	}
};

class ModelType:public BaseAttrSet{

    static const unsigned int MODELTYPE_LIST_NUM=7;
    static const unsigned int VALUE_CHAR_LEN=20;

    char list[ModelType::MODELTYPE_LIST_NUM][ModelType::VALUE_CHAR_LEN+1];

    public:
    ModelType()
    {
        strcpy(list[0],  "general");
        strcpy(list[1],  "letv");
        strcpy(list[2],  "poi");
        strcpy(list[3],  "food");
        strcpy(list[4],  "song");
        strcpy(list[5],  "medical");
        strcpy(list[6],  "movietv");
    }
    bool ValueInSupList(const char* value, unsigned int len)
    {
        if(len>ModelType::VALUE_CHAR_LEN)
            return false;

        for(unsigned int i=0; i<ModelType::MODELTYPE_LIST_NUM; i++)
            if(strncmp(value, list[i], len)==0)
                return true;

        return false;
    }
};

class UserDataFlag:public BaseAttrSet{
    static const unsigned int PERSONALDATAFLAG_LIST_NUM=2;
    static const unsigned int VALUE_CHAR_LEN=20;

    //we should consider the end flag '/0' in string
    char list[UserDataFlag::PERSONALDATAFLAG_LIST_NUM][UserDataFlag::VALUE_CHAR_LEN+1];
    //this parameter is temp used to store input string, cause input char* maybe not end using '/0'

    public:
    UserDataFlag()
    {
        strcpy(list[0],  "open");
        strcpy(list[1],  "close");
    }

    bool ValueInSupList(const char* value, unsigned int len)
    {
        if(len>UserDataFlag::VALUE_CHAR_LEN)
        {
            return false;
        }

        return true;
    }

};


class AueSet:public BaseAttrSet{
    static const unsigned int AUE_LIST_NUM=3;
    static const unsigned int VALUE_CHAR_LEN=100;

    //we should consider the end flag '/0' in string
    char list[AueSet::AUE_LIST_NUM][AueSet::VALUE_CHAR_LEN+1];
    //this parameter is temp used to store input string, cause input char* maybe not end using '/0'
    //char tmpValue[AueSet::VALUE_CHAR_LEN+1];

    public:
    static int TranslateInt(const char* value){
        int result = 0;
        if(!strcmp(value, "opus")){
            result = Opus::WB_MODE;
        }
        else if(!strcmp(value, "opus-nb")){
            result = Opus::NB_MODE;
        }
        else if(!strcmp(value, "speex")){
            result = 0;
        }

        return result;
    }

    public:
    AueSet()
    {
        strcpy(list[0],  "speex");
        strcpy(list[1],  "opus");
        strcpy(list[2],  "opus-nb");
        /*strcpy(list[1],  "speex-wb");
          strcpy(list[2],  "raw");
          strcpy(list[3],  "amr");
          strcpy(list[4],  "amr-fx");
          strcpy(list[5],  "amr-wb");
          strcpy(list[6],  "amr-wb-fx");
          strcpy(list[7],  "feature");*/
    }

    bool ValueInSupList(const char* value, unsigned int len)
    {
        if(len>AueSet::VALUE_CHAR_LEN)
            return false;

        for(unsigned int i=0; i<AueSet::AUE_LIST_NUM; i++)
            if(strncmp(value, list[i], len)==0)
                return true;

        return false;
    }

};

class AufSet:public BaseAttrSet{

    static const unsigned int AUF_LIST_NUM=2;	
    static const unsigned int VALUE_CHAR_LEN=100;

    char list[AufSet::AUF_LIST_NUM][AufSet::VALUE_CHAR_LEN+1];

    public:
    AufSet()
    {
        strcpy(list[0],  "audio/L16/8000");
        strcpy(list[1],  "audio/L16/16000");
        //strcpy(list[2],  "pcm");
    }
    bool ValueInSupList(const char* value, unsigned int len)
    {
        if(len>AufSet::VALUE_CHAR_LEN)
            return false;

        for(unsigned int i=0; i<AufSet::AUF_LIST_NUM; i++)
            if(strncmp(value, list[i], len)==0)
                return true;

        return false;
    }
};

class RstSet:public BaseAttrSet{

    static const unsigned int RST_LIST_NUM=3;	
    static const unsigned int VALUE_CHAR_LEN=20;

    char list[RstSet::RST_LIST_NUM][RstSet::VALUE_CHAR_LEN+1];
    public:
    RstSet()
    {
        strcpy(list[0],  "text");
        strcpy(list[1],  "xml");
        strcpy(list[2],  "json");
    }
    bool ValueInSupList(const char* value, unsigned int len)
    {
        if(len>RstSet::VALUE_CHAR_LEN)
            return false;

        for(unsigned int i=0; i<RstSet::RST_LIST_NUM; i++)
            if(strncmp(value, list[i], len)==0)
                return true;

        return false;
    }
};

class RseSet:public BaseAttrSet{

    static const unsigned int RSE_LIST_NUM=2;
    static const unsigned int VALUE_CHAR_LEN=100;

    char list[RseSet::RSE_LIST_NUM][RseSet::VALUE_CHAR_LEN];
    public:
    RseSet()
    {
        strcpy(list[0],  "gb2312");
        strcpy(list[1],  "utf8");
        //strcpy(list[2],  "unicode");
    }
    bool ValueInSupList(const char* value, unsigned int len)
    {
        if(len>RseSet::VALUE_CHAR_LEN)
            return false;

        for(unsigned int i=0; i<RseSet::RSE_LIST_NUM; i++)
            if(strncmp(value, list[i], len)==0)
                return true;

        return false;
    }
};

class WaitTimeOut:public BaseAttrSet{

    public:
        static const unsigned int VALUE_CHAR_LEN=2;

        //min or max seconds to wait if there is no data transmitting
        static const unsigned int min_to_sec=0;
        static const unsigned int max_to_sec=30;


    public:
        bool ValueInSupList(const char* value, unsigned int len)
        {
            if(len>WaitTimeOut::VALUE_CHAR_LEN)
                return false;

            //translate char* to unsigned tmp
            unsigned int tmp=0;
            for(unsigned int i=0; i<len; i++)
                tmp = tmp*10 + (value[i]-'0');

            if(tmp<=WaitTimeOut::max_to_sec)
                return true;
            else
                return false;
        }

};

/*
class IMEISet:public BaseAttrSet
{
    static const unsigned int VALUE_CHAR_LEN = 16;

    public:
    bool ValueInSupList(const char* value, unsigned int len)
    {
        if(len > IMEISet::VALUE_CHAR_LEN)
            return false;
        else
            return true;
    }
};
*/

/*
 * Altered by lizhongyuan
 * If the length of "deviceID" is longer than 40, cut to 40 bytes
 * So, the ValueInSupList always return true;
 */
class IMEISet: public BaseAttrSet
{
	/*
	 * Indeed, the WebService accept only 40 bytes, but if the users set strings longer than 40 , we should accept them.
	 * So, I set the VALUE_CHAR_LEN to 100 to restrict input
	 */
	static const unsigned int VALUE_CHAR_LEN = 100;
	//static const unsigned int VALUE_CHAR_LEN = 200;
public:
	bool ValueInSupList(const char* value, unsigned int len)
	{
		if(len > IMEISet::VALUE_CHAR_LEN)
			return false;
		else
			return true;
		return true;
	}
};


class AppKeySet:public BaseAttrSet
{
    static const unsigned int VALUE_CHAR_LEN=50;

    public:
    bool ValueInSupList(const char* value, unsigned int len)
    {
        //cout<<"AppKey"<<value<<endl;
        //cout<<"Len"<<(int)len<<endl;
        if(len>AppKeySet::VALUE_CHAR_LEN)
            return false;
        else
            return true;
    }

};

class OralEvalText:public BaseAttrSet{

    static const unsigned int VALUE_CHAR_LEN=5000;

    public:
    bool ValueInSupList(const char* value, unsigned int len)
    {
        if(len>=OralEvalText::VALUE_CHAR_LEN)
            return false;
        else
            return true;
    }

};

class OralConfigOpt1:public BaseAttrSet{

    static const unsigned int VALUE_CHAR_LEN=100;
    public:

    bool ValueInSupList(const char* value, unsigned int len)
    {
        if(len>=OralConfigOpt1::VALUE_CHAR_LEN)
            return false;
        else
            return true;
    }
};

class OralConfigOpt2:public BaseAttrSet{
    static const unsigned int VALUE_CHAR_LEN=100;
    public:
    bool ValueInSupList(const char* value, unsigned int len)
    {
        if(len>=OralConfigOpt2::VALUE_CHAR_LEN)
            return false;
        else
            return true;
    }
};

class OralTaskType:public BaseAttrSet{

    static const unsigned int VALUE_CHAR_LEN=1000;
    
    public:
    bool ValueInSupList(const char* value, unsigned int len)
    {
	if(len>=OralTaskType::VALUE_CHAR_LEN)
		return false;
	else
		return true;
    }
};

class OralSessionId:public BaseAttrSet{
    
    static const unsigned int VALUE_CHAR_LEN=1000;
    
public:
    bool ValueInSupList(const char* value, unsigned int len)
    {
        if(len>=OralSessionId::VALUE_CHAR_LEN)
            return false;
        else
            return true;
    }
};

struct SERV_ATTRS
{
	//server support attributes number limit
	static const unsigned char ATTR_NUM=50;
	static const unsigned int ATTR_SINGLE_LEN=1024*5;

	//Attributes checker for check itself's attribute value are correct or not
	AueSet	gservAUE;
	AufSet		gservAUF;
	RstSet		gservRST;
	RseSet		gservRSE;
	WaitTimeOut	gservWTO;
	IMEISet		gservIMEI;
	AppKeySet	gservKey;
	PackageName	gpackageName;
	DeviceOS	guserOS;
	UserID		guserId;
	CollectedInfo gcollectedInfo;
	ReqRspEntity greqAudioURL;
	RsqAudioURL grspAudioURL;
	ModelType gmodelType;
	OralEvalText goralEvalText;
	OralTaskType goralTaskType;
	OralConfigOpt1 goralConfigOpt1;
	OralConfigOpt2 goralConfigOpt2;
    OralSessionId goralSeesionId;
	char codelist[SERV_ATTRS::ATTR_NUM];
	BaseAttrSet* attrstruct[SERV_ATTRS::ATTR_NUM];

    public:
    SERV_ATTRS()
    {
        BaseAttrSet* tmpAttrStruct[SERV_ATTRS::ATTR_NUM] = {NULL, NULL, &gservAUE/*2*/, &gservAUF/*3*/, &gservRST/*4*/, &gservRSE/*5*/, NULL, NULL, NULL, NULL, \
            NULL, &gservWTO/*0b*/, &gservIMEI/*0c*/, &gservKey/*0d*/, &gpackageName/*0e*/, NULL, NULL, &guserOS/*11*/, &guserId/*12*/, &gcollectedInfo/*13*/,\
                &greqAudioURL/*14*/, &grspAudioURL/*15*/, &gmodelType/*16*/, NULL, &goralEvalText, &goralTaskType, &goralConfigOpt1, &goralConfigOpt2, &goralSeesionId/*1c*/, NULL,\
                NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,\
                NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL };

        char tmpCodeList[SERV_ATTRS::ATTR_NUM]= {NOT_SUP, SUPPED, SUPPED/*2*/, SUPPED/*3*/, SUPPED/*4*/, SUPPED/*5*/, SUPPED, NOT_SUP,NOT_SUP,NOT_SUP,\
            NOT_SUP,SUPPED/*0b*/,  SUPPED/*0c*/,  SUPPED/*0d*/,  SUPPED/*0e*/,  SUPPED,  SUPPED,  SUPPED/*11*/, SUPPED/*12*/, SUPPED/*13*/,\
                SUPPED/*14*/,SUPPED/*15*/,SUPPED/*16*/,NOT_SUP,SUPPED,SUPPED,SUPPED,SUPPED,SUPPED,NOT_SUP,\
                NOT_SUP,NOT_SUP,NOT_SUP,NOT_SUP,NOT_SUP,NOT_SUP,NOT_SUP,NOT_SUP,NOT_SUP,NOT_SUP,\
                NOT_SUP,NOT_SUP,NOT_SUP,NOT_SUP,NOT_SUP,NOT_SUP,NOT_SUP,NOT_SUP,NOT_SUP,NOT_SUP};

        //initial checker pointer array
        for(unsigned int i = 0; i < SERV_ATTRS::ATTR_NUM; i++)
        {
            attrstruct[i]=tmpAttrStruct[i];
            codelist[i]=tmpCodeList[i];
        }

    }

    bool ValueInSupList(unsigned char attr, const char* value, unsigned int len)
    {
        //cout<<"ValueInSupListIn"<<endl;
        if( attr >= SERV_ATTRS::ATTR_NUM)
        {
            //cout<<"ValueInSupListOut/CodeError"<<endl;
            return false;
        }

        if(codelist[attr]==NOT_SUP)
        {
            //cout<<"ValueInSupListOut/CodeNotSup"<<endl;
            return false;
        }
        else if(attrstruct[attr]!=NULL)
        {
            //cout<<"ValueInSupListOut/CheckStructError"<<endl;
            return attrstruct[attr]->ValueInSupList(value, len);
        }
        else
        {
            //cout<<"ValueInSupListOut/NoStructCorrect"<<endl;
            return true;
        }
    }	
};

struct AttrsLimit{

    static const unsigned int attrValueLenLimit=SERV_ATTRS::ATTR_SINGLE_LEN;
    static const unsigned int attrsNumberLimit=SERV_ATTRS::ATTR_NUM;

    static bool OutOfNumberLimit(unsigned int attributeNum){
        return attributeNum>attrsNumberLimit? true: false;
    }

    static bool OutOfLengthLimit(unsigned int valuelength){
        return valuelength>attrValueLenLimit? true: false;
    }

};

#endif

