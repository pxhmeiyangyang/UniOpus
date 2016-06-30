#ifndef JOELOG_H_
#define JOELOG_H_

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "Joe_tst.h"


#include <pthread.h>


using namespace std;

typedef enum {
	NO_PROBLEM, //it was OK
	INTERNAL_DESIGN_ERROR, //internal design error, it will log itself! the caller can treat it as a warning, but can work continuely
	ERROR_HAPPENED, //it caused by other API calling, caller should do some handle
	TOTALNUM_JLRETURNSTATUS=ERROR_HAPPENED
}JoeLoggerReturnStatus;

/*
* Serial class provides persistence mechanism for other class or module
*/
class Serial
{

typedef enum StorageType{
	STFILE_SYSTEM,
	STDATABASE,
	STDOUTPUT,
	STDIPCMSGQ
}StorageType;

static const unsigned int AUTOSTORENUM=5;

public:
	Serial();
	//to set file system as persistence storage
	Serial(const char* fname);
	~Serial();
//to write string into storage system
	int WriteString(char *pString, int length);
//to read string from the storage system
	int ReadString(char *pString, int length, char*DestString);
private:
	FILE *m_fp;
	StorageType m_type;
	char *m_filename;
	unsigned int m_seqno;
	pthread_mutex_t lock;
	int m_msqid;
};


JoeLoggerReturnStatus GetCurrentTime(char *pCurrentTime);
JoeLoggerReturnStatus GetCurrentTimeFullFormat(char* pTimeS);

class JoeLogger
{


typedef enum{
	INT_NO_PROBLEM, //it was OK
	INT_INTERNAL_DESIGN_ERROR, //internal design error, it will log itself! the caller can treat it as a warning, but can work continuely
	INT_ERROR_HAPPENED, //it caused by other API calling, caller should do some handle
	INT_TOTALNUM_JLRETURNSTATUS=ERROR_HAPPENED
}InternalJoeLoggerReturnStatus;

struct LogSystemProperty{
	char logRowTime[100];
	unsigned int seqNo;
	//string logRowLevel;
	JoeLoggerReturnStatus (* pSetTime)(char *);
};


public:
	JoeLogger();
	JoeLogger(const char* name);
	~JoeLogger();
	JoeLoggerReturnStatus RecordLogRow(const char *pRow);
	int logV1(int logLevel, const char* module, const char* tag, const char* trace);
	//property initialization action, for example, set property seting function pointer
	void InitialProperty();
	
private:
	struct LogSystemProperty m_logProperty;
	Serial* m_seialLog;
	char m_ID[50];
	char m_owner[10];

//function
private:

	JoeLoggerReturnStatus TranslateToExternalStatus(InternalJoeLoggerReturnStatus internalStatus);
};

#endif 


