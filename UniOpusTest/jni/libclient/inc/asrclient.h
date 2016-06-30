#include "log.h"

#ifndef	ASR_CLIENT_H
#define	ASR_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

    /*enum {
      ASR_AUTHORIZE_ERROR				= -8,
      ASR_VAD_TIMEOUT					= -7,
      ASR_MAX_SPEECH_TIMEOUT			= -6,
      ASR_SERVICE_NOT_RUNNING			= -5,
      ASR_MEM_ALLOCATION_ERROR		= -4,
      ASR_COMPRESS_PCM_ERROR			= -3,
      ASR_COMMUNICATION_ERROR			= -2,
      ASR_FATAL_ERROR					= -1,
      ASR_ATTRS_FORMAT_ERROR			= -9,
      ASR_INTERNAL_ERROR				= -10,
      ASR_RECOGNIZER_OK				= 0,
      ASR_RECOGNIZER_NO_RESULT		= 1,
      ASR_RECOGNIZER_PARTIAL_RESULT	= 2,	
      };*/

enum
{
	ASR_ENABLE_VAD_ID			= 0,	// 0 OR 1, default value 0 (disable)
	ASR_VAD_TIMEOUT_ID			= 1,	// (this value only valid when vad is enabled) from 500 to 10000, default value 3000 (means 3000ms)
	ASR_MAX_SPEECH_TIMEOUT_ID		= 2,	// from 10 to 600, default value 60  (means 60s)
	ASR_WAITING_SERVICE_TIMEOUT_ID		= 3,	// 0 means always wait, default value 20 (means 20s)
	ASR_WAITING_RESULT_TIMEOUT_ID		= 4,	// 0 means always wait, default value 30 (means 30)
	ASR_RESULT_FORMAT_ID			= 5,	// default value 0 ( 0 means "text", 1 means "json")
	ASR_PCM_COMPRESS_ID			= 6,	// from 0 to 10, default value 8
	ASR_DISABLE_PARTIAL_RESULT_ID		= 7,
	ASR_AUDIO_ENCODE_MTD			= 0x0201, //= 16,
	ASR_AUDIO_ENCODE_MTD8K			= 0x0202, //= 19,
	ASR_IMEI_ID				= 0x0c, //= 8,
	ASR_SERVICE_KEY_ID			= 0x0d, //= 9,
	ASR_OPT_PACKAGE_NAME			= 0x0e, //= 10,
	ASR_OPT_CARRIER				= 0x0f, //= 12,
	ASR_OPT_NETWORK_TYPE			= 0x10, //= 13,
	ASR_OPT_DEVICE_OS			= 0x11, //= 11,
	ASR_USER_ID				= 0x12, //= 14,
	ASR_OPT_COLLECTED_INFO			= 0x13, //= 15,
	ASR_REQ_RSP_ENTITY              	= 0x14, //= 17,
	ASR_MODLE_TYPE                  	= 0x16, //= 18,

	ASR_ORAL_EVAL_TEXT 			= 0x17,
	ASR_USRDATA_FLAG                        = 0x18,
	ASR_OPT_NET_PACKAGE_SIZE                = 0x19,
	ASR_ORAL_TASK_TYPE			= 0x1A,
	ASR_ORAL_CONF_OP1			= 0x1B,
	ASR_ORAL_CONF_OP2			= 0x1C,
    ASR_ORAL_AUDIO_URL			= 0x15,
    ASR_ORAL_SESSION_ID         = 0x1d,
};

// device OS
typedef enum {
    DEVICE_OS_Android 				= 0,
    DEVICE_OS_iOS 				= 1,
    DEVICE_OS_WP8				= 2,
}DEVICE_OS;
    
// network type
typedef enum {
    NETWORK_TYPE_NONE				= 0,
    NETWORK_TYPE_WIFI				= 1,
    NETWORK_TYPE_3G				= 2,
    NETWORK_TYPE_2G				= 3,
    // iOS无法区分2G还是3G
    NETWORK_TYPE_WWAN				= 4,
}NETWORK_TYPE;
    

class 	AsrServiceInterface
{
public:
	AsrServiceInterface()
	{
		usc_count = 0;
	}

	virtual ~AsrServiceInterface()
	{

	}

	virtual int setValueInt(int id, int value) = 0;
	virtual int setValueString(int id, const char* s) = 0;
	virtual const char* getOptionValue(int id) = 0;
	virtual int start() = 0;
	virtual int recognizer(char*pcm, int pcm_len) = 0;//used to recognize pcm data
	virtual int isactive(char*pcm, int pcm_len) = 0;
	virtual char* getResult() = 0;					//fetch result
	virtual int stop() = 0;
	virtual int getLastError() = 0;					//get error code when error occured
	virtual int cancel() = 0;
	virtual int queryResult() = 0;
#ifdef TEST_SERVER_USAGE
	virtual int disconnect_tcp()=0;
	virtual int start_without_connect()=0;
	virtual int connect_tcp()=0;
	virtual int stop_without_disconnect()=0;
#endif
       
public:
        int usc_count;
        int usc_disablePResult;
};

AsrServiceInterface*	asrCreateAsrService(const char* ip,short port);//quality from 0 to 10, the bigger, the better
void	asrDestroyAsrService(void* asrService);
//int		asrGetGlobalLastError();		//when asrCreateAsrService failed, use this to get error code

#ifdef __cplusplus
}
#endif

#endif
