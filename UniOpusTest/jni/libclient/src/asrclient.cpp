#include <stdlib.h>

#ifdef LINUX
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/time.h>
#elif defined (WIN32)
#include <WinSock2.h>
#include <Ws2tcpip.h>
const char *inet_ntop(int af, const void *src, char *dst, int cnt);
#endif

#ifndef _SPEEX_CODEC_
#include "Opuswrapper.h"
#else
#include "speexwrapper.h"
#endif

#include "AsrVad.h"
#include "AsrSession.h"
#include "asrclient.h"
#include "log.h"

#ifdef LINUX
#include <sys/time.h>
#elif defined (WIN32)
#include <time.h>
#endif

// For backtrace
#ifdef LINUX_PC_DEBUG
#ifdef LINUX 
#include<execinfo.h>
#endif
#endif

#include"AsrDebugger.h"
#include "Settings.h"     //将服务器地址及端口号变为全局变量,在此引入头文件 - by liujun
//#include<>

#ifdef PPDEBUG
JoeLogger logger;
#endif

#ifdef USE_VAD
//#define DETECT_SPEECH_START
#endif

#define ONE_SECOND 32000
#define SERVICE_TIMEOUT_LIMIT	99

/*
 * This is for debug in gdb
 */
/*
void
asr_backtrace(const char* commit)
{
	//Show where the backtrace happened.
	printf("USC_backtrace[\"%s\"].\n", commit);

	int j, nptrs;
	#define SIZE 100
	void *buffer[100];
	char **strings;

	nptrs = backtrace(buffer, SIZE);
	printf("backtrace() returned %d addresses\n", nptrs);

	// The call backtrace_symbols_fd(buffer, nptrs, STDOUT_FILENO)
	// would produce similar output to the following: 

	strings = backtrace_symbols(buffer, nptrs);
	if(strings == NULL)
	{
		perror("backtrace_symbols");
		exit(EXIT_FAILURE);
	}

	for(j = 0; j < nptrs; j++)
		printf("%s\n", strings[j]);

	free(strings);
}
*/

class AsrService: public AsrServiceInterface
{
#ifdef DETECT_SPEECH_START
    static const int DATA_BUFFER_LEN = 32 * 150;	//150ms
#endif
    static const int PACKAGE_LEN_THRESHOLD = 9600;	//300ms
	int package_len_threshold;
    
public:
#ifndef _SPEEX_CODEC_
    AsrService(const char* uri, short port)
        : server_port(asrServer_port)
        , asr_session(NULL)
        , lastError(SESSION_LOGIC_ERR::ASRCLIENT_SERVICE_NOT_RUNNING)
        , opus(NULL)
#else
    AsrService(const char* uri, short port)
        : server_port(port)
        , asr_session(NULL)
        , lastError(SESSION_LOGIC_ERR::ASRCLIENT_SERVICE_NOT_RUNNING)
        , speex(NULL)
#endif
    {
		package_len_threshold = PACKAGE_LEN_THRESHOLD;
        //strncpy(server_uri, uri, 32);
        strncpy(server_uri, asrServer_domain, 32);
        server_ip[0] = 0;
        
        compress = 8;
        enableVAD = 0;
        
        speexBuf[0]= 0;
        bufLen = 0;
        bufferedPcmLen = 0;
        totalPcmLen = 0;
        
        vadTimeout = 300;
        maxSpeechSecond = 60 * 30;//取消时长限制，暂时将时长设为30min
        waitingResultTimeout = 30;
        
#ifdef USE_VAD
        asr_VAD = NULL;
#endif
#ifdef DETECT_SPEECH_START
        dataBuf = NULL;
        dataLen = 0;
        dataBuf2 = NULL;
        dataLen2 = 0;
#endif
    }
    
    virtual ~AsrService()
    {
        if(asr_session)
	{
            delete asr_session;
        }
#ifdef USE_VAD
        if(asr_VAD)
	{
            delete asr_VAD;
        }
#endif

#ifdef DETECT_SPEECH_START
        if(dataBuf)
	{
            delete [] dataBuf;
        }
        if ( dataBuf2 ) {
            delete [] dataBuf2;
        }
#endif

#ifndef _SPEEX_CODEC_
        if(opus)
	{
            delete opus;
        }
#else
	if(speex)
	{
            delete speex;
        }
#endif
    }
    
    bool getValidIp(char* server_ip,char* server_uri,int url_index);
    
    virtual int setValueInt(int id, int value);
    virtual int setValueString(int id, const char* s);
    virtual const char* getOptionValue(int id);
    virtual int start();
    virtual int recognizer(char* pcm ,int pcm_len);
    virtual int isactive(char* pcm ,int pcm_len);
    virtual int stop();
    virtual char* getResult();
    virtual int cancel();
    virtual int queryResult();
    virtual int getLastError() { return lastError;}
#ifdef TEST_SERVER_USAGE
    virtual int disconnect_tcp();
    virtual int start_without_connect();
    virtual int connect_tcp();
    virtual int stop_without_disconnect();
#endif
    
    void setLastError(int error) { lastError = error;}
#ifdef DETECT_SPEECH_START
    void saveRawData(char* pcm, int len);
#endif
    bool initialize();
    int doCompress(char* pcm, int pcm_len);
    int resumeSession(char* pcm, int pcm_len);
    int sendRemainData();
    
private:
    char server_uri[32];
    char server_ip[32];
    short server_port;
    int compress;
    int enableVAD;
    AsrSession* asr_session;
    int lastError;
    
 #ifndef _SPEEX_CODEC_
    Opus* opus;
 #else
    Speex* speex;
 #endif
    char speexBuf[3200];
    int bufLen;
    int bufferedPcmLen;
    int totalPcmLen;
    int vadTimeout;
    int maxSpeechSecond;
    int waitingResultTimeout;

#ifdef USE_VAD
    AsrVAD* asr_VAD;
#endif
#ifdef DETECT_SPEECH_START
    char *dataBuf;
    int dataLen;
    char *dataBuf2;
    int dataLen2;
#endif
};


bool AsrService::initialize()
{
#ifndef _SPEEX_CODEC_
	opus = new Opus();
	if(opus == NULL)
		return false;
#else
	speex = new Speex();
	if(speex == NULL)
		return false;
	speex->setQuality((int)compress);
#endif
	
	{
#ifdef USE_VAD
		asr_VAD = new AsrVAD();
		if (asr_VAD == NULL)
			return false;
        
		if (asr_VAD->init() != 0)
			return false;
#endif
	}
    
	asr_session = new AsrSession();
	if(asr_session == NULL)
	{
		return false;
	}

	return true;
}

int AsrService::setValueInt(int id, int value)
{
	int status = 0;
	char attr_int_value[100] = {0};

	switch (id)
	{
		case ASR_ENABLE_VAD_ID:
			if(value >= 0 && value <= 1)
			{
				enableVAD = value;
			}
			break;
		case ASR_VAD_TIMEOUT_ID:
			if(value >= 2000 && value <= 10000)
			{
				vadTimeout = value/10;
			}
			break;
		case ASR_MAX_SPEECH_TIMEOUT_ID:
//			if(value >= 10 && value <= 600)
//			{
//				maxSpeechSecond = value;
//			}
			break;
		case ASR_WAITING_SERVICE_TIMEOUT_ID:
			if(value>SERVICE_TIMEOUT_LIMIT || value<0)
			{
				status = INPUT_CHECKER_ERR::ASRCLIENT_INVALID_PARAMETERS;
			}
			else
			{
				sprintf(attr_int_value, "%d", value);
				status = asr_session->sess_set_attr(SERV_SUP_ATTR::SSUP_WAIT_TIMEOUT, attr_int_value);
			}
			break;
		case ASR_WAITING_RESULT_TIMEOUT_ID:
			if(value >= 0)
			{
				waitingResultTimeout = value;
			}
			break;
		case ASR_RESULT_FORMAT_ID:
			if(value == 1)
			{
				status=asr_session->sess_set_attr(SERV_SUP_ATTR::SSUP_RSP_FMT_SET, "json");
			}
			else
			{
				status=asr_session->sess_set_attr(SERV_SUP_ATTR::SSUP_RSP_FMT_SET, "text");
			}
			break;
		case ASR_PCM_COMPRESS_ID:
			if(value >= 0 && value <= 10)
			{
				compress = value;
			}
#ifdef _SPEEX_CODEC_
			speex->setQuality((int)compress);
#endif
			break;
		case ASR_OPT_DEVICE_OS:
			sprintf(attr_int_value, "%d", value);
			status = asr_session->sess_set_attr(SERV_SUP_ATTR::SSUP_ASR_OPT_DEVICE_OS, attr_int_value);
			break;
		case ASR_OPT_CARRIER:
			sprintf(attr_int_value, "%d", value);
			status = asr_session->sess_set_attr(SERV_SUP_ATTR::SSUP_ASR_OPT_CARRIER, attr_int_value);
			break;
		case ASR_OPT_NETWORK_TYPE:
			sprintf(attr_int_value, "%d", value);
			status=asr_session->sess_set_attr(SERV_SUP_ATTR::SSUP_ASR_OPT_NETWORK_TYPE, attr_int_value);			
			break;
		/*case ASR_OPT_NET_PACKAGE_SIZE:
			package_len_threshold = value;
			break;*/
		default:
			return INPUT_CHECKER_ERR::ASRCLIENT_INVALID_PARAMETERS;

	}
	return status;
}

/*
 *
 */
int
AsrService::setValueString(int id, const char* s)
{
	//printf("AsrService::setValueString.\n");
	int status;
	if(s == NULL)
	{
		//printf("NULL para.\n");
		return INPUT_CHECKER_ERR::ASRCLIENT_INVALID_PARAMETERS;
	}

	//printf("~~~~~~~~~~~~~~~~~~~~~~~\n");
	//printf("?? id:%d\n", id);
	//printf("ASR_OPT_COLLECTED_INFO:%d\n", ASR_OPT_COLLECTED_INFO);
	switch (id)
	{
		//USC_OPT_IMEI_ID 0x0c
		case ASR_IMEI_ID:
			status=asr_session->sess_set_attr(SERV_SUP_ATTR::SSUP_IMEI_SET, s);
			break;
		//USC_OPT_APP_KEY 0x09
		case ASR_SERVICE_KEY_ID:
			//printf("!!!!SERV_SUP_ATTR::SSUP_APP_KEY:%d\n", SERV_SUP_ATTR::SSUP_APP_KEY);
			status=asr_session->sess_set_attr(SERV_SUP_ATTR::SSUP_APP_KEY, s);
			break;
		//USC_OPT_PACKAGE_NAME 0x0e
		case ASR_OPT_PACKAGE_NAME:
			status=asr_session->sess_set_attr(SERV_SUP_ATTR::SSUP_ASR_OPT_PACKAGE_NAME, s);
			break;
		//USC_USER_ID 0x12
		case ASR_USER_ID:
			status=asr_session->sess_set_attr(SERV_SUP_ATTR::SSUP_USER_ID, s);
			break;
		//USC_OPT_COLLECTED_INFO 0x13
		case ASR_OPT_COLLECTED_INFO:
			//printf("ASR_OPT_COLLECTED_INFO");
			status=asr_session->sess_set_attr(SERV_SUP_ATTR::SSUP_COLLECTED_INFO, s);
			break;
		//USC_AUDIO_ENCODE_MTD 0x0201
		case ASR_AUDIO_ENCODE_MTD:
			
			/* the sample rate is restricted to 16K whatever the encode method is */
			status=asr_session->sess_set_attr(SERV_SUP_ATTR::SSUP_AUDIO_ENC_METH, s);
			if(status<0)
				break;
			else{
#ifndef _SPEEX_CODEC_
				if(opus){
					int type = AueSet::TranslateInt(s);
					status = opus->change_mode(Opus::WB_MODE, type);
				}
#endif
			}
			break;
		//USC_AUDIO_ENCODE_MTD8K 0x0202
		case ASR_AUDIO_ENCODE_MTD8K:
			/* the sample rate is restricted to 8K */
			status=asr_session->sess_set_attr(SERV_SUP_ATTR::SSUP_AUDIO_ENC_METH, "opus-nb");
			if(status < 0)
				break;
			else
#ifndef _SPEEX_CODEC_
				status = opus->change_mode(Opus::NB_MODE, Opus::NB_MODE);
#endif
			break;			
		//USC_REQ_RSP_ENTITY 0x14
		case ASR_REQ_RSP_ENTITY:
			status=asr_session->sess_set_attr(SERV_SUP_ATTR::SSUP_REQ_RSP_ENTITY, s);
			break;
		// not include in USC_Series
		case ASR_MODLE_TYPE:
			status=asr_session->sess_set_attr(SERV_SUP_ATTR::SSUP_MODEL_TYPE, s);
			break;
		case ASR_ORAL_EVAL_TEXT:
			status=asr_session->sess_set_attr(SERV_SUP_ATTR::SSUP_ORAL_EVAL_TEXT, s);
			//printf("setting SSUP_ORAL_EVAL_TEXT value is %s\n", s);
			break;
		case ASR_ORAL_TASK_TYPE:
			status = asr_session->sess_set_attr(SERV_SUP_ATTR::SSUP_ORAL_TASK_TYPE, s);
			break;
		case ASR_ORAL_CONF_OP1:
			status = asr_session->sess_set_attr(SERV_SUP_ATTR::SSUP_ORAL_EX1, s);
                        break;
		case ASR_ORAL_CONF_OP2:
			status = asr_session->sess_set_attr(SERV_SUP_ATTR::SSUP_ORAL_EX2, s);
			break;
        case ASR_ORAL_AUDIO_URL:
            status = asr_session->sess_set_attr(SERV_SUP_ATTR::SSUP_RSP_AUDIO_URL, s);
            break;
        case ASR_ORAL_SESSION_ID:
            status = asr_session->sess_set_attr(SERV_SUP_ATTR::SSUP_RSP_SESSION_ID, s);
            break;
		default:
			return INPUT_CHECKER_ERR::ASRCLIENT_INVALID_PARAMETERS;
	}
	return status;
}


const char*
AsrService::getOptionValue(int id)
{
	//printf("call AsrService::getOptionValue(%d)\n", id);
	return asr_session->sess_get_attr((unsigned char)id);
}

#pragma mark -  DNS容错 2014-8-19 by liujun

bool AsrService::getValidIp(char* server_ip,char* server_uri,int url_index)
{
	char out_ip[32] = {0};
    
	struct hostent* hptr;
	for (int i = url_index;i < 3;i++)
	{
		if (strcmp(server_uri,server_list[i]) != 0)
		{
			if((hptr = gethostbyname(server_list[i])) == NULL)
			{
				continue;
			}
			inet_ntop(hptr->h_addrtype, *(hptr->h_addr_list), out_ip, sizeof(out_ip));
			if (strcmp(out_ip,server_ip) != 0)
			{
				strcpy(server_uri,server_list[i]);
				strcpy(server_ip,out_ip);
                strcpy(asrServer_domain, server_uri);
                
				return true;
			}
		}
	}
	return false;
}

int
AsrService::start()
{
	//printf("AsrService:start().\n");
	//asr_backtrace("AsrService::start(), backtrace 1");
    
	int ret = 0;
	speexBuf[0] = 0;
	bufLen = 0;
	bufferedPcmLen = 0;
	totalPcmLen = 0;
    
	setLastError(SUCC_CODE::ASRCLIENT_RECOGNIZER_OK);
	
#ifndef _SPEEX_CODEC_
	//printf("AsrService::opus_reset()\n");
	opus->reset();
#else
	speex->reset();
#endif
    
	asr_VAD->reset();
    
	if(server_ip[0] == 0)
	{
		// determine if the uri is ip string
		struct sockaddr_in sa;
		//printf("AsrService::inet_pton\n");
        
		int temp = inet_pton(AF_INET, server_uri, &(sa.sin_addr));
		if(temp == 1)
		{
			//asr_backtrace("AsrService::start(), backtrace 2.1");
			strcpy(server_ip, server_uri);
		}
		else if(temp == 0)
		{
			//asr_backtrace("AsrService::start(), backtrace 2.2");
			// convert uri to IP
            
            //printf("asrclient: gethostbyname error 1\n");
            struct hostent *hptr;
            if( (hptr = gethostbyname(server_uri)) == NULL)
			{
                //取消DNS容错 2015.1.22 by liujun
				//if(!getValidIp(server_ip,server_uri,0))
				{
					LOGE("ASR-JNI", "asrclient: gethostbyname error");
					//printf("asrclient: gethostbyname error 2\n");
					return NETWORK_GENERAL_ERR::ASRCLIENT_GETHOST_BY_NAME_ERROR;
				}
			}
			else
			{
				inet_ntop(hptr->h_addrtype, *(hptr->h_addr_list), server_ip, sizeof(server_ip));
			}
		}
		else
		{
			//asr_backtrace("AsrService::start(), backtrace 2.3");
            LOGE("ASR-JNI", "asrclient: inet_pton error");
            printf("asrclient: inet_pton error\n");
			return NETWORK_GENERAL_ERR::ASRCLIENT_INET_PTON_ERROR;
		}
	}
    
	/*
	 * 1. every start, let's create a new connection
	 */
	ret = asr_session->sess_conn_server((char*)server_ip, (int)server_port);
	//asr_backtrace("AsrService::start(), backtrace 3");
    
	int dns_index = 0;
	while(ret != 0 && dns_index<3)
	{
        //取消DNS容错 2015.2.11 by liujun
//		if(getValidIp(server_ip,server_uri,dns_index++))
//		{
//			ret = asr_session->sess_conn_server((char*)server_ip, (int)server_port);
//		}
//		else
		{
			//to free the fd
			asr_session->sess_disconn_server();
			char err_msg[128];
			sprintf(err_msg, "asrclient: connect error = %d", ret);
			LOGE("ASR-JNI", err_msg);
			//printf("asrclient: connect error= %d\n", ret);
#ifdef LINUX
			setLastError(errno);
#endif
			return ret;
		}
	}
    
	if (ret!=0)
	{
#ifdef LINUX
		setLastError(errno);
#endif
		return ret;
        
	}
	
	/*
	 * 2. After the TCP is established, call the asr_session->start().
	 */
	ret = asr_session->sess_start();
    
	//asr_backtrace("AsrService::start(), backtrace 4");
	if(ret != 0)
	{
		char err_msg[128];
		sprintf(err_msg, "asrclient: start error = %d", ret);
		LOGE("ASR-JNI", err_msg);
		//printf("asrclient: start error=%d\n", ret);
#ifdef LINUX
        setLastError(errno);
#endif
	}
	return ret;
}

int AsrService::recognizer(char*pcm, int pcm_len)
{
	int rt = 0;
	if (totalPcmLen > maxSpeechSecond*ONE_SECOND)
	{
		LOGE("ASR-JNI", "asrclient: max speech");
		printf("asrclient: max speech\n");
#ifdef LINUX
		setLastError(errno);
#endif
		return INPUT_CHECKER_ERR::ASRCLIENT_MAX_SPEECH_TIMEOUT;
	}
	
	totalPcmLen += pcm_len;
	
	rt = resumeSession(pcm, pcm_len);
	if(rt != 0)
	{
		char err_msg[128] = {0};
		sprintf(err_msg, "asrclient: resume error = %d", rt);
		LOGE("ASR-JNI", err_msg);
		//printf("asrclient: resume error=%d\n", rt);
#ifdef LINUX
                setLastError(errno);
#endif

	}

	return rt;
}

int AsrService::isactive(char* pcm, int pcm_len)
{
#ifdef USE_VAD
	if(enableVAD) {

		int volume = 0;
		int status = asr_VAD->process((short*)pcm, pcm_len/sizeof(short), &volume);
		
		if(status == 1 || status == 2) {
			return 0;
		} else if(status == 0) {
			return 1;
		} else {
			LOGE("ASR-JNI", "VAD returns unexpected value.");
			return 0;
		}
	} else {
		return 1;
	}
#else
	return 1;
#endif
}

int AsrService::stop()
{
    //printf("AsrService->stop() in");
	int err = sendRemainData();
	if (err != 0)
	{
		char err_msg[128];
		sprintf(err_msg, "asrclient: send remain error = %d", err);
		LOGE("ASR-JNI", err_msg);
                //printf("asrclient: send remain error=%d\n", err);

#ifdef LINUX    
                setLastError(errno);
#endif

		return err;
	}
	
    //printf("AsrService->stop() sess_stop start : %d\n",waitingResultTimeout);
    
	err = asr_session->sess_stop(waitingResultTimeout);
	if (err != 0) {

		char err_msg[128];
		sprintf(err_msg, "asrclient: stop error = %d", err);
		LOGE("ASR-JNI", err_msg);
		//printf("asrclient: stop error=%d\n", err);

#ifdef LINUX
              setLastError(errno);
#endif        	
		/*
		 * Indeed, dissess_conn_server() is called in ~AsrService().
		 * But to optimize the release of fds, we call it in this position
		 */
		asr_session->sess_disconn_server();

		return err;
	}
    
    //printf("AsrService->stop() sess_stop stop");
	
	/*
	 * Indeed, dissess_conn_server() is called in ~AsrService().
	 * But to optimize the release of fds, we call it in this position
	 */
	asr_session->sess_disconn_server();

	asr_VAD->reset();
	
	return err;
}


char*
AsrService::getResult()
{
	char* result = asr_session->sess_get_res();
	return  result;
}

/*
 *
 */
int
AsrService::cancel()
{
	int status = asr_session->sess_cancel();
#ifdef LINUX
	if (status != 0)
		setLastError(errno);
#endif
	return status;
}

int
AsrService::queryResult()
{
    //printf("AsrService::queryResult -> timeout :%d\n",waitingResultTimeout);
	int status = asr_session->sess_query_res(waitingResultTimeout);
#ifdef LINUX
	if(status != 0)
		setLastError(errno);
#endif
	return status;
}

/*
 *
 */
int
AsrService::resumeSession(char* pcm,int pcm_len)
{
	int ret = 0;
	int buffer_len = doCompress(pcm, pcm_len);
	if ( buffer_len < 0 )
	{
		return INPUT_CHECKER_ERR::ASRCLIENT_COMPRESS_PCM_ERROR;
	}
    
	bufferedPcmLen += pcm_len;
    
	if(bufferedPcmLen >= PACKAGE_LEN_THRESHOLD)
	{
		ret = asr_session->sess_resume(speexBuf, bufLen);
		speexBuf[0] = 0;
		bufLen = 0;
		bufferedPcmLen = 0;
	}

#ifdef LINUX
	if(ret != 0)
		setLastError(errno);
#endif
	
	return ret;
}


/*
 * This function is for test
 */
#ifdef DETECT_SPEECH_START
void AsrService::saveRawData(char* pcm, int len)
{
	if(len > DATA_BUFFER_LEN)
	{
		if(dataBuf == NULL)
		{
			dataBuf = new char[len];
			if(dataBuf == NULL)
			{
			}
		}
		memcpy(dataBuf, pcm, len);
		dataLen = len;
		return;
	}
	
	if(dataBuf == NULL)
	{
		dataBuf = new char[DATA_BUFFER_LEN];
		if(dataBuf == NULL)
		{
		}
	}
    
	if(dataBuf2 == NULL)
	{
		dataBuf2 = new char[DATA_BUFFER_LEN];
		if(dataBuf2 == NULL)
		{
		}
	}
    
	if(dataLen < DATA_BUFFER_LEN)
	{
		int remain = DATA_BUFFER_LEN - dataLen;
		if(remain >= len)
		{
			memcpy(dataBuf+dataLen, pcm, len);
			dataLen += len;
		}
		else
		{
			memcpy(dataBuf+dataLen, pcm, remain);
			dataLen = DATA_BUFFER_LEN;
			memcpy(dataBuf2, pcm+remain, len-remain);
			dataLen2 = len-remain;
		}
	}
	else
	{
		int remain = DATA_BUFFER_LEN - dataLen2;
		if(remain >= len)
		{
			memcpy(dataBuf2+dataLen2, pcm, len);
			dataLen2 += len;
		}
		else
		{
			memcpy(dataBuf2+dataLen2, pcm, remain);
			dataLen2 = DATA_BUFFER_LEN;
			memcpy(dataBuf, pcm+remain, len-remain);
			dataLen = len-remain;
			
			char *tmp = dataBuf;
			dataBuf = dataBuf2;
			dataLen = dataLen2;
			dataBuf2 = tmp;
			dataLen2 = len-remain;
		}
	}
    
}
#endif

int
AsrService::sendRemainData()
{   
	if ( bufLen > 0 )
	{
		int status = asr_session->sess_resume(speexBuf, bufLen);
#ifdef LINUX
		if( status != 0)
			setLastError(errno);
#endif
		
		return status;
	}
	return 0;
}


/*
 * compress the PCM stream, to communicate with ASR server
 */
int
AsrService::doCompress(char* pcm, int pcm_len)
{
	int codec_len = 0;
	int encode = 0;
#ifndef _SPEEX_CODEC_
	encode = opus->encode(pcm, pcm_len, (unsigned char*)speexBuf+bufLen, &codec_len);
#else
	encode = speex->encode(pcm, pcm_len, speexBuf + bufLen,&codec_len);
#endif	
	if ( encode < 0 )
	{
		return -1;
	}
	
	bufLen += codec_len;
	return bufLen;
}

#ifdef TEST_SERVER_USAGE
int
AsrService::disconnect_tcp()
{
	// every stop, let't disconnect
	asr_session->dissess_conn_server();
	return 0;
}

int AsrService::start_without_connect(){
	int ret = 0;
	speexBuf[0]= 0;
	bufLen = 0;
	bufferedPcmLen = 0;
	totalPcmLen = 0;
    
	setLastError(SUCC_CODE::ASRCLIENT_RECOGNIZER_OK);
	
#ifndef _SPEEX_CODEC_
	opus->reset();
#else
	speex->reset();
#endif

	asr_VAD->reset();
	
	if (ret != 0) {

		char err_msg[128];
		sprintf(err_msg, "asrclient: connect error = %d", ret);
		LOGE("ASR-JNI", err_msg);
                printf("asrclient: connect error=%d\n", ret);
#ifdef LINUX    
                setLastError(errno);
#endif

		return ret;
	}
	
	ret = asr_session->sess_start();
	if(ret != 0) {

		char err_msg[128];
		sprintf(err_msg, "asrclient: start error = %d", ret);
		LOGE("ASR-JNI", err_msg);
		printf("asrclient: start error=%d\n", ret);
#ifdef LINUX
                setLastError(errno);
#endif

	}
	return ret;
}

int AsrService::connect_tcp(){

	int ret = 0;
	if(server_ip[0] == 0) {

		// determine if the uri is ip string
		struct sockaddr_in sa;
		int temp = inet_pton(AF_INET, server_uri, &(sa.sin_addr));
		if(temp == 1) {
			strcpy(server_ip, server_uri);
		} else if(temp == 0){
			// convert uri to IP 
			struct hostent* hptr;
			if( (hptr = gethostbyname(server_uri)) == NULL){
				LOGE("ASR-JNI", "asrclient: gethostbyname error");
	            		printf("asrclient: gethostbyname error\n");
				return NETWORK_GENERAL_ERR::ASRCLIENT_GETHOST_BY_NAME_ERROR;
			}
			inet_ntop(hptr->h_addrtype, *(hptr->h_addr_list), server_ip, sizeof(server_ip));
			//printf("After DNS we find out domain %s is ip %s", server_uri, server_ip);
		} else {
	        		LOGE("ASR-JNI", "asrclient: inet_pton error");
	        		printf("asrclient: inet_pton error\n");
			return NETWORK_GENERAL_ERR::ASRCLIENT_INET_PTON_ERROR;
		}
	}

	// every start, let's create a new connection
	ret = asr_session->sess_conn_server((char*)server_ip, (int)server_port);
}

int
AsrService::stop_without_disconnect(){
	int err = sendRemainData();
	if (err != 0) {

		char err_msg[128];
		sprintf(err_msg, "asrclient: send remain error = %d", err);
		LOGE("ASR-JNI", err_msg);
                printf("asrclient: send remain error=%d\n", err);

#ifdef LINUX    
                setLastError(errno);
#endif

		return err;
	}
	
	err = asr_session->sess_stop(waitingResultTimeout);
	if (err != 0) {

		char err_msg[128];
		sprintf(err_msg, "asrclient: stop error = %d", err);
		LOGE("ASR-JNI", err_msg);
		//printf("asrclient: stop error=%d\n", err);

#ifdef LINUX
                setLastError(errno);
#endif        	

		return err;
	}
	
	// every stop, let't disconnect
	//asr_session->dissess_conn_server();
	asr_VAD->reset();
	
	return err;
}

#endif

AsrServiceInterface* asrCreateAsrService(const char* ip,short port)
{
#ifdef	WIN32
	WSAData wsaData;
	int nCode;
	if ((nCode = WSAStartup(MAKEWORD(1, 1), &wsaData)) != 0) {
		LOGE("ASR-JNI", "error initialize winsock.");
		return NULL;
	}
#endif
	
	AsrService* asr_service = new AsrService(ip,port);
	if ( asr_service == NULL )
	{
		LOGE("ASR-JNI", "ASR memory allocation error");
		return NULL;
	}
	if (!asr_service->initialize())
	{
		delete asr_service;
		return NULL;
	}

	return asr_service;
}

void
asrDestroyAsrService(void* asrService)
{
#ifdef WIN32
	WSACleanup();
#endif

	if(asrService == NULL)
		return;

    	AsrService* asr_service = (AsrService*)asrService;
	delete asr_service;
}

#ifdef WIN32
const char *inet_ntop(int af, const void *src, char *dst, int cnt)
{
	if (af == AF_INET)
	{
		struct sockaddr_in in;
		memset(&in, 0, sizeof(in));
		in.sin_family = AF_INET;
		memcpy(&in.sin_addr, src, sizeof(struct in_addr));
		getnameinfo((struct sockaddr *)&in, sizeof(struct sockaddr_in), dst, cnt, NULL, 0, NI_NUMERICHOST);
		return dst;
	}
	else if (af == AF_INET6)
	{
		struct sockaddr_in6 in;
		memset(&in, 0, sizeof(in));
		in.sin6_family = AF_INET6;
		memcpy(&in.sin6_addr, src, sizeof(struct in_addr6));
		getnameinfo((struct sockaddr *)&in, sizeof(struct sockaddr_in6), dst, cnt, NULL, 0, NI_NUMERICHOST);
		return dst;
	}
	return NULL;
}
#endif
