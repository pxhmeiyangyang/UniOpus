#include "JoeLog.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef LINUX
#include <sys/time.h>
#elif defined(WIN32)
#include <time.h>
#endif

#include <string.h>
#include <iostream>


using namespace std;

JoeLogger::JoeLogger()
{
	InitialProperty();
	m_seialLog=new Serial();
	GetCurrentTime(m_ID);
	strcpy(m_owner, "client");
}

JoeLogger::JoeLogger(const char* owner)
{
	InitialProperty();
	m_seialLog=new Serial();	
	GetCurrentTime(m_ID);
	strcpy(m_owner, owner);
}

JoeLogger::~JoeLogger()
{
	delete m_seialLog;
}

JoeLoggerReturnStatus JoeLogger::RecordLogRow(const char *pRow)
{
	char *tmp=new char[strlen(pRow)+100];
	strcpy(tmp,"\0");
	//InternalJoeLoggerReturnStatus IntelResult=INT_NO_PROBLEM;
	JoeLoggerReturnStatus result=NO_PROBLEM;
	result=m_logProperty.pSetTime(m_logProperty.logRowTime);
	if(result==NO_PROBLEM)
	{
		strcat(tmp,"LoggerID.:");
		strcat(tmp,m_ID);
		strcat(tmp,"	");
		strcat(tmp,"SeqNo.:");
		sprintf(tmp+strlen(tmp),"%d",++(m_logProperty.seqNo));
		strcat(tmp,"	");
		strcat(tmp,"Time:");
		strcat(tmp,m_logProperty.logRowTime);
		strcat(tmp,"	");
		strcat(tmp,pRow);
		if(m_seialLog->WriteString(tmp,strlen(tmp)))
			result=ERROR_HAPPENED;
	}

	delete[] tmp;

	return result;
}

int JoeLogger::logV1(int logLevel, const char* module, const char* tag, const char* trace)
{
	static const unsigned int MAX_MSG_LEN=1024;
	
	char tmp[ MAX_MSG_LEN + 1]={0};
	int result = 0;

	m_logProperty.pSetTime(m_logProperty.logRowTime);
	sprintf(tmp, "LoggerID.:%s\towner:%s\tSeqNo.:%d\tTime:%s\tLevel:%d\tmodule:%s\t%s:%s", m_ID, m_owner, ++(m_logProperty.seqNo), m_logProperty.logRowTime, logLevel, module, tag, trace);
	if(m_seialLog->WriteString(tmp, strlen(tmp)))
		result=-1;

	return result;	
}

JoeLoggerReturnStatus GetCurrentTime(char* pTimeS)
{
	struct timeval tcurrent;	
	JoeLoggerReturnStatus result=NO_PROBLEM;
	int joeerror=gettimeofday(&tcurrent,NULL);

	if(!joeerror)
	{
		double tmptime=(double)tcurrent.tv_sec+((double)tcurrent.tv_usec)/1000000;
		char tmp[100];
		sprintf(tmp,"%f",tmptime);
		strcpy(pTimeS,tmp);
	}
	else
		result=ERROR_HAPPENED;
	
	return result;
}

JoeLoggerReturnStatus GetCurrentTimeFullFormat(char* pTimeS)
{

	//time_t timep;
	//time (&timep);
	//printf(¡°%s¡±,ctime(&timep));

	time_t tcurrent;	
	JoeLoggerReturnStatus result=NO_PROBLEM;
	int joeerror=time(&tcurrent);

	if(joeerror)
	{
		//char tmp[100];
		//tmp=ctime(&tcurrent);
		strcpy(pTimeS,ctime(&tcurrent));
	}
	else
		result=ERROR_HAPPENED;
	
	return result;
}

JoeLoggerReturnStatus JoeLogger::TranslateToExternalStatus(InternalJoeLoggerReturnStatus internalStatus)
{
	JoeLoggerReturnStatus result;
	switch (internalStatus)
	{
		case INT_NO_PROBLEM:  //it was OK
			result=NO_PROBLEM;
		break;
		case INT_INTERNAL_DESIGN_ERROR: //internal design error, it will log itself! the caller can treat it as a warning, but can work continuely
			result=INTERNAL_DESIGN_ERROR;
		break;
		case INT_ERROR_HAPPENED: //it caused by other API calling, caller should do some handle
			result=ERROR_HAPPENED;
		break;
		default:
			result=INTERNAL_DESIGN_ERROR;
		break;
	}
	return result;
}

void JoeLogger::InitialProperty()
{
	m_logProperty.pSetTime=GetCurrentTime;
	m_logProperty.seqNo=0;
}

Serial::Serial()
{
	m_type = STDOUTPUT;
	if(m_type==STFILE_SYSTEM)
	{
		//next step: generate a file using the current time as file name
		char tmp[50];
		GetCurrentTimeFullFormat(tmp);
		char filename[100]="JoeLog";
		strcat(filename,tmp);
		strcat(filename,".txt");
		m_filename=filename;
		m_fp=fopen(filename,"at+"); 
	}
	m_seqno=0;
	m_msqid=-1;
}

Serial::Serial(const char* name)
{
	//next step: generate a file using the current time as file name
	char tmp[50];
	GetCurrentTimeFullFormat(tmp);
	char filename[100]="JoeLog";
	strcat(filename,name);
	strcat(filename,tmp);
	strcat(filename,".txt");
	m_filename=filename;
	m_fp=fopen(filename,"at+"); 
	m_type=STFILE_SYSTEM;
	m_seqno=0;
}

Serial::~Serial()
{
	if(m_type==STFILE_SYSTEM)
	{
		fclose(m_fp);
	}
}

int Serial::WriteString(char *pString,int length)
{
	m_seqno++;
	if(m_type==STFILE_SYSTEM)
	{
		pthread_mutex_lock(&lock);
		if(m_fp!=NULL)
		{
			fputs(pString,m_fp);
			fputc('\n',m_fp);
			
			//auto store when record number equal AUTOSTORENUM
			if(m_seqno%AUTOSTORENUM==0)
			{
				fclose(m_fp);
				char tmp[50];
				GetCurrentTimeFullFormat(tmp);
				char filename[100]="JoeLog";
				strcat(filename,tmp);
				strcat(filename,".txt");
				m_filename=filename;
				m_fp=fopen(filename,"at+"); 
			}
		}
		else
			cout<<"File create error or other file related error!"<<endl;
		pthread_mutex_unlock(&lock);
	}
	else if(m_type==STDOUTPUT)
		{
			//cout<<pString<<endl;
			fprintf(stderr, pString);
                        fprintf(stderr, "\n");
                        fflush(stderr);
		}
		else if(m_type==STDIPCMSGQ)
			{
				if(m_msqid!=-1)
					sendMSG2Q(m_msqid,pString,length);
				else
				{
					m_msqid=getMSGID();
					if(m_msqid==-1)
						cout<<"get IPC message queue error!"<<endl;
					else
						sendMSG2Q(m_msqid,pString,length);
				}
			}

	return 0;
}
