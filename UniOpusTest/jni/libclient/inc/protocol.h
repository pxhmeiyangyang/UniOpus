#ifndef	ASR_PROTOCOL_H
//#include <string>
#define	ASR_PROTOCOL_H
#define	MAX_ALLOWED_QUERY_LEN	(4096*1024)
#define	MAX_SUB_CMD_LEN	(512)
#define  MAXATTRLEN (2048)

/*enum ASRSN_ATTR{
		SAMPLERATE,	//sample bit rate
		USERNAME,
		SESSIONKEY
	};*/

const static char START_SESSION_CMD = 0x01;
const static char RESUME_SESSION_CMD = 0x11;
const static char STOP_SESSION_CMD = 0x10;
const static char CANCEL_SESSION_CMD = 0x12;
const static char GET_PARTIAL_RESULT_CMD = 0x13;

enum {
	ASR_SERVICE_BUSY=0xFFFF,			//No free session for user
	ASR_START_SESSION_ERROR=0xFFFE,		//start session error. in most of cases, request head parameters are wrong
	ASR_APP_KEY_CHECK_ERROR = 0xFFFD,	//APP key check fail, that means, it is a invalid key
//	ASR_RESUME_SESSION_ERROR=0xFFFD,
	ASR_STOP_SESSION_ERROR=0xFFFC,		//stop session error.Bad stop session head parameter
	ASR_UNKNOWN_HEAD_ERROR=0xFFFB,		//other errors,program currently not catched.
	ASR_SERVICE_UNAVAI_ERROR=0xFFFA,	//Server is down or can not accept user connection
	ASR_RECOGNIZE_OK=0x00,				//asr recognized user input successfully
	ASR_RECOGNIZE_ERROR=0x01,			//asr recognizing error
	ASR_START_SESSION_OK=0x00,
	ASR_SYSTEM_ERROR=0xFFF0,
	ASR_ATTR_NOT_SUPPORT=0xFFF1,
	ASR_MSG_FORMAT_ERROR=0xFFF2,
	ASR_REQ_NOT_SUPPORT=0xFFF3,
	ASR_BAD_FRAME_FOUND=0xFFF4,
	ASR_FRAME_LEN_TOO_GREAT=0xFFF5,
	ASR_MAGIC_NUM_WRONG=0xFFF6,
    ASR_RESULT_TOO_LONG = 0xFE08,
#ifdef ORAL_EVAL
	ASR_ORAL_MODE_BUT_NO_TEXT_UPLOAD=0xFFF7,
       ASR_ORAL_MODE_EVAL_ERROR = 0xFE01,
       ASR_ORAL_MODE_NOT_COLLECTED_WORD = 0xFE02,
       ASR_ORAL_MODE_EVAL_SPEECH_TOO_LONG = 0xFE03,       
#endif
};

class session_key_words {
	public:
		session_key_words();
		~session_key_words();
	private:

};
//enum { SESSION_STOPPED=0,SESSION_STARTED,SESSION_RESUMED,SESSION_UNUSED};
enum { 
    SESSION_STOPPED=0,
    SESSION_STARTED,
    SESSION_UNUSED};

/*struct session_opt_t {
	int wait_timeout;
	int result_format;
	char imei[16];
	char key[64];
};*/

#endif
