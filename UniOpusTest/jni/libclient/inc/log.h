#ifndef __LOG_H__
#define __LOG_H__

/*struct err_code_range{
    static const int SUCC = 0;
    static const int SOCKET_CONN_START = 10000;
    static const int SOCKET_CONNN_END = 10999;
    static const int SESSION_LOGIC_START = 20000;
    static const int SESSION_LOGIC_END = 20999;
    static const int INPUT_CHECKER_START = 30000;
    static const int INPUT_CHECKER_END = 30999;
    static const int INTERNAL_ERROR_START = 40000;
    static const int INTERNAL_ERROR_END = 40999;
};*/

struct SUCC_CODE{
    static const int ASRCLIENT_RECOGNIZER_OK = 0;
    static const int ASRCLIENT_CREATE_SERVICE_OK = 0;
    static const int ASRCLIENT_START_SESSION_OK = 0;
    static const int ASRCLIENT_RECOGNIZER_NO_RESULT = 1;
    static const int ASRCLIENT_RECOGNIZER_PARTIAL_RESULT = 2;
};

struct SOCKET_CONN_ERR{  

    static const int ASRCLIENT_COMMUNICATION_ERROR = -10001;
    static const int ASRCLIENT_CREATE_SOCKET_FAIL = -10002;
    static const int ASRCLIENT_CHANGE_SOCKET_ATTR_FAIL = -10003;

    static const int ASRCLIENT_CONNECT_SELECT_FAIL = -10004;
    static const int ASRCLIENT_WRITE_SELECT_FAIL = -10005;
    static const int ASRCLIENT_READ_SELECT_FAIL = -10006;
    static const int ASRCLIENT_WRITE_TIMEOUT = -10007;
    static const int ASRCLIENT_READ_TIMEOUT = -10008;
    
    static const int ASRCLIENT_PPOLL_FAIL = -10009;

};

struct SESSION_LOGIC_ERR
{
	static const int ASRCLIENT_AUTHORIZE_ERROR = -20001;
	static const int ASRCLIENT_SERVICE_NOT_RUNNING = -20002;
	static const int ASRCLIENT_SERVER_RETURN_TOO_LONG_RESP = -20003;
	static const int ASRCLIENT_SERVER_COMM_ERROR = -20004;
	static const int ASRCLIENT_CONTENT_DECODE_ERROR = -20005;
	static const int ASRCLIENT_FATAL_ERROR = -20006;
};

struct INPUT_CHECKER_ERR{
    static const int ASRCLIENT_VAD_TIMEOUT = -30001;
    static const int ASRCLIENT_MAX_SPEECH_TIMEOUT = -30002;
    static const int ASRCLIENT_COMPRESS_PCM_ERROR = -30003;
    static const int ASRCLIENT_INVALID_PARAMETERS = -30004;
};

struct INTERNAL_ERR{
    static const int ASRCLIENT_FATAL_ERROR = -40001;
    static const int ASRCLIENT_MEM_ALLOCATION_ERROR = -40002;    
    static const int ASRCLIENT_CREATE_ASRSERVER_ERROR = -40003;
};

struct PROTOCOL_ERR{
    static const int ASRCLIENT_SERVICE_BUSY = -50001;			//No free session for user
    static const int ASRCLIENT_START_SESSION_ERROR = -50002;		//start session error. in most of cases, request head parameters are wrong
    //	ASR_RESUME_SESSION_ERROR=0xFFFD,
    static const int ASRCLIENT_STOP_SESSION_ERROR = -50003;		//stop session error.Bad stop session head parameter
    static const int ASRCLIENT_UNKNOWN_HEAD_ERROR = -50004;		//other errors,program currently not catched.
    static const int ASRCLIENT_SERVICE_UNAVAI_ERROR = -50005;	//Server is down or can not accept user connection

    static const int ASRCLIENT_RECOGNIZE_ERROR = -50006;			//asr recognizing error

    static const int ASRCLIENT_SYSTEM_ERROR = -50007;
    static const int ASRCLIENT_ATTRS_NOT_SUPPORT = -50008;
    static const int ASRCLIENT_MSG_FORMAT_ERROR = -50009;

    static const int ASRCLIENT_VERSION_TOO_OLD = -50010;
    static const int ASRCLIENT_COMMAND_ERROR = -50011;
    static const int ASRCLIENT_RESULT_TOO_LONG = -51012;

    static const int ASRCLIENT_ATTR_LEN_TOO_LONG = -51013;
};

#define ORAL_EVAL
#ifdef ORAL_EVAL
struct ORAL_EVAL_ERR
{

    static const int ASRCLIENT_ORAL_MODE_BUT_NO_TEXT_UPLOAD = -11001;
    static const int ASRCLIENT_ORAL_MODE_EVAL_ERROR = -11002;
    static const int ASRCLIENT_ORAL_MODE_NOT_COLLECTED_WORD = -11003;
    static const int ASRCLIENT_ORAL_MODE_EVAL_SPEECH_TOO_LONG = -11004;

};
#endif

struct NETWORK_GENERAL_ERR{
    static const int ASRCLIENT_INET_PTON_ERROR = -70001;
    static const int ASRCLIENT_GETHOST_BY_NAME_ERROR = -70002;
};

struct LOG_LEVEL{
    static const int LOG_ERROR=3;
    static const int LOG_DEBUG=2;
    static const int LOG_INFO=1;
};

#ifdef LIBLOG

#ifdef __ANDROID__
#include <android/log.h>
#define LOGI(tag, msg) __android_log_print(ANDROID_LOG_INFO, tag, msg)
#define LOGW(tag, msg) __android_log_print(ANDROID_LOG_WARN, tag, msg)
#define LOGE(tag, msg) __android_log_print(ANDROID_LOG_ERROR, tag, msg)
#else	// __ANDROID__
#define LOGI(tag, msg) fprintf(stderr, "INFO - %s\t: %s\n", tag, msg);
#define LOGW(tag, msg) fprintf(stderr, "WARN - %s\t: %s\n", tag, msg);
#define LOGE(tag, msg) fprintf(stderr, "ERR  - %s\t: %s\n", tag, msg);
#endif	// __ANDROID__

#else	// LIBLOG

#define LOGI(tag, msg)
#define LOGW(tag, msg)
#define LOGE(tag, msg)

#endif	// LIBLOG

#endif	// __LOG_H__


