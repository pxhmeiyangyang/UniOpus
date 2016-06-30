#include <string.h>
#include <stdlib.h>
#ifdef	LINUX
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#elif defined (WIN32)
#include <Winsock2.h>
#endif

#include <errno.h>
#include <assert.h>
#include <stdio.h>
#ifdef	OS_HAS_POLL
#include <poll.h>
#endif

#ifdef PRINT_TRACE
#include <pthread.h>
#endif

#include "asrclient.h"
#include "protocol.h"
#include "AsrSession.h"
#include "time.h"

#include "log.h"
#include "ServSupAttrs.h"

#if defined (PPDEBUG) || defined (TEST_SERVER_USAGE)
#include <iostream>
using namespace std;
#include <arpa/inet.h>
#endif

//for debug
#include "AsrDebugger.h"

#ifndef	EQ
#define	EQ ==
#endif

#ifdef PRINT_TRACE
#include <pthread.h>
pthread_mutex_t lock;
int fdNum = 0;
char initMutex = 0;
#endif

#define	MAX_CONN_TIMEOUT	10

SERV_ATTRS	gservATTR;		// todo: confirm global var

AsrSession::AsrSession()
{
	sess_mtu_len_ = 1500;			// todo: confirm MTU
	sess_result_ = NULL;
	sess_attr_blk_ = NULL;
#ifdef PPDEBUG
        strcpy(moduleName, "session");
#endif
	
	sess_fd_ = 0;
	sess_pcm_len_ = 0;
	sess_has_res_ = false;

#ifdef PRINT_TRACE
	if(initMutex==0){
		printf("init mutex\n");
		pthread_mutex_init(&lock, NULL);	
		initMutex = 1;
	}
#endif
}


/*
 *
 */
AsrSession::~AsrSession()
{
	SLOG_FUNCTIONC_IN("~AsrSession");
	
	// Make sure all the fds are closed.
	//sess_close_fd(&sess_fd_);
	sess_disconn_server();

	if(sess_attr_blk_ != NULL)
	{
		delete sess_attr_blk_;
		sess_attr_blk_ = NULL;
	}
	
	SLOG_FUNCTIONC_OUT("~AsrSession");
}


/*
 * Altered by lizhongyuan
 */
int
AsrSession::sess_conn_server(char* server_ip, int server_port)
{
	//printf("AsrSession:sess_conn_server.\n");
	
	SLOG_FUNCTIONC_IN("sess_conn_server");
	SLOG_TINY_INFO(server_ip);

	const int errMsgLen = 128-1;
	char			err_msg[128] = {0};
	int			connect_ret;

	// for construct socket
	struct sockaddr_in		asr_server;
	int			server_fd;

	//for construct the address
	memset(&asr_server,0,sizeof(asr_server));
	asr_server.sin_family = AF_INET;
	asr_server.sin_addr.s_addr = inet_addr(server_ip);
	asr_server.sin_port = htons(server_port);
	

	//printf("before allocate fd.\n");

	// 1. construct the socket
	server_fd = socket(AF_INET,SOCK_STREAM,0);
#ifdef PRINT_TRACE
        printf("thread ID:%ld create fd:%d\n", (long)(pthread_self()),server_fd);
	pthread_mutex_lock(&lock);
	fdNum ++;
	pthread_mutex_unlock(&lock);
#endif
	if(server_fd < 0)
	{
		LOGE("SES-JNI", "create socket error.");
		SLOG_FUNCTIONC_OUT("sess_conn_server/create_socket_error");
		sess_close_fd(&server_fd);
		return SOCKET_CONN_ERR::ASRCLIENT_CREATE_SOCKET_FAIL;
	}
	//printf("after allocate fd.\n");

	snprintf(err_msg, errMsgLen, "socket created: fd = %d", server_fd);
	//printf("We use this %d fd to connect our recognize server.\n", server_fd);    
	

	// 2. set the fd to O_NONBLOCK
#ifdef iOS
	int ios_set = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_NOSIGPIPE, (void *)&ios_set, sizeof(int));
#endif

#ifdef	LINUX
	int fd_flag = fcntl(server_fd, F_GETFL, 0);
	// old: if (fcntl(server_fd, F_SETFL, fd_flag|O_NDELAY) < 0)
	if(fcntl(server_fd, F_SETFL, fd_flag|O_NONBLOCK) < 0)
	{
		snprintf(err_msg, errMsgLen, "set server socket nonblocking error, fd = %d", server_fd);
		LOGE("SES-JNI", err_msg);
		SLOG_FUNCTIONC_OUT("sess_conn_server/socket_nonblocking_error");
		sess_close_fd(&server_fd);
                #ifdef PRINT_TRACE
                printf("thread ID:%ld close fd:%d\n", (long)(pthread_self()),server_fd);
                #endif
		return SOCKET_CONN_ERR::ASRCLIENT_CHANGE_SOCKET_ATTR_FAIL;
	}
#endif

	errno = 0;

	// 3. connect, if failed, immediately
	connect_ret =  connect(server_fd,(const struct sockaddr*)&asr_server,sizeof(struct sockaddr_in));
#ifdef	LINUX
	if(connect_ret < 0 && (errno != EAGAIN && errno != EINPROGRESS) ) 
#else
	if(connect_ret < 0 && (errno != EAGAIN) ) 
#endif
	{
		snprintf(err_msg, errMsgLen, "error connect, fd = %d, err = %d", server_fd, connect_ret);
		LOGE("SES-JNI", err_msg);
		SLOG_FUNCTIONC_OUT("sess_conn_server/connect_ASR_error");
		SLOG_EVENT_TRACE("socket", "action,status", "connect,fail");
		//printf("AsrSession:sess_conn_server(). Failed to connect(errno is neither EAGAIN nor EINPROGRESS).\n");

		sess_close_fd(&server_fd);
                #ifdef PRINT_TRACE
                printf("thread ID:%ld close fd:%d\n", (long)(pthread_self()),server_fd);
                #endif
		return SOCKET_CONN_ERR::ASRCLIENT_COMMUNICATION_ERROR ;
	}
	
#ifndef OS_HAS_POLL			
	fd_set		wfds;
	struct timeval	tv;
	
	FD_ZERO(&wfds);
	FD_SET(server_fd, &wfds);
	
	/* Wait up to five seconds. */
	tv.tv_sec = MAX_CONN_TIMEOUT;
	tv.tv_usec = 0;

	if(select(server_fd + 1, NULL, &wfds, NULL, &tv) <= 0)	// 0 for timeout, <0 for errors
	{

		snprintf(err_msg, errMsgLen, "select error, fd = %d", server_fd);
		LOGE("SES-JNI", err_msg);
		
		SLOG_EVENT_TRACE("socket", "action,status", "connect,fail");
		//printf("AsrSession::sess_conn_server(). Failed to select.\n");
		
		// to make sure the fd is free.
		sess_close_fd(&server_fd);

/*
#ifdef LINUX
		close(server_fd);
#elif defined (WIN32)
		closesocket(server_fd);
#endif
*/
		#ifdef PRINT_TRACE
		printf("**********select error, error no is %d\n",errno);
		#endif
		//sess_close_fd(&server_fd);
                #ifdef PRINT_TRACE
                printf("thread ID:%ld close fd:%d\n", (long)(pthread_self()),server_fd);
                #endif
		return SOCKET_CONN_ERR::ASRCLIENT_CONNECT_SELECT_FAIL;
	}

	// now, check the fd is really able to write
	int		error;

#ifdef LINUX
	socklen_t	len = sizeof(error);
#elif defined (WIN32)
	int len = sizeof(error);
#endif

	int		code;
	if(FD_ISSET(server_fd, &wfds))
	{
		code = getsockopt(server_fd, SOL_SOCKET, SO_ERROR, (char*)&error, &len);
		/* 如果发生错误，Solaris实现的getsockopt返回-1， 
		 * * 把pending error设置给errno. Berkeley实现的 
		 * * getsockopt返回0, pending error返回给error.  
		 * * 我们需要处理这两种情况 */
		if(code < 0 || error)
		{
			if(error)
				errno = error;
			#ifdef PRINT_TRACE
			printf("**********getsockopt error, error no is %d, return code is %d\n",error,code);
			#endif
			sess_close_fd(&server_fd);
                	#ifdef PRINT_TRACE
                	printf("thread ID:%ld close fd:%d\n", (long)(pthread_self()),server_fd);
                	#endif
			return SOCKET_CONN_ERR::ASRCLIENT_CONNECT_SELECT_FAIL;
		}
	}
	else
	{
		#ifdef PRINT_TRACE
		printf("**********sess_conn_server error FD_ISSET fail, error no is %d\n",errno);
		#endif
		sess_close_fd(&server_fd);
                #ifdef PRINT_TRACE
                printf("thread ID:%ld close fd:%d\n", (long)(pthread_self()),server_fd);
                #endif
		return SOCKET_CONN_ERR::ASRCLIENT_CONNECT_SELECT_FAIL;
	}
#else
	struct pollfd fds[1];
	struct timespec tv;
	const struct timespec* tout;
	int retval;
	tv.tv_sec = MAX_CONN_TIMEOUT;	

	fds[0].fd = server_fd;
	fds[0].events = POLLOUT ;
	
	fds[0].revents = 0;
	sigset_t origmask;

	//retval = ppoll(fds, sizeof(fds)/sizeof(struct pollfd), &tv, NULL);
	tout = &tv;
	retval = poll(fds, sizeof(fds)/sizeof(struct pollfd), MAX_CONN_TIMEOUT*1000);
	if( retval <=0 )
	{
		snprintf(err_msg, errMsgLen, "ppoll error, fd = %d", server_fd);
		LOGE("SES-JNI", err_msg);
		SLOG_EVENT_TRACE("socket", "action,status", "connect,fail");		

		sess_close_fd(&server_fd);

		return SOCKET_CONN_ERR::ASRCLIENT_PPOLL_FAIL;
	}
#endif

#ifdef	LINUX
	if(fcntl(server_fd, F_SETFL, fd_flag) < 0)
	{
		snprintf(err_msg, errMsgLen, "set socket blocking error, fd = %d", server_fd);
		LOGE("SES-JNI", err_msg);
		SLOG_EVENT_TRACE("socket", "action,status", "connect,fail");		

		sess_close_fd(&server_fd);

		return SOCKET_CONN_ERR::ASRCLIENT_CHANGE_SOCKET_ATTR_FAIL;
	}
#endif

	sess_fd_ = server_fd;

#if defined (PPDEBUG) || defined (TEST_SERVER_USAGE)
	struct sockaddr_in	guest;  
	socklen_t guest_len = sizeof(guest);  
	getsockname(server_fd, (struct sockaddr*)&guest, &guest_len);  
	char* clientIP = inet_ntoa(guest.sin_addr);

	unsigned short	clientPort = ntohs(guest.sin_port);
	 
#endif
	 
	SLOG_FUNCTIONC_OUT("sess_conn_server");
	SLOG_EVENT_TRACE("socket", "action,status", "connect,succ");

	return SUCC_CODE::ASRCLIENT_CREATE_SERVICE_OK;
}


void
AsrSession::sess_close_fd(int* fd_ptr)
{
	if(*fd_ptr > 0)
	{
#ifdef	LINUX
		close(*fd_ptr);
		#ifdef PRINT_TRACE
        	printf("thread ID:%ld close fd:%d\n", (long)(pthread_self()),*fd_ptr);
		int nowNum = 0;
		pthread_mutex_lock(&lock);
		fdNum --;
		nowNum = fdNum;
		pthread_mutex_unlock(&lock);
		printf("now fd num is %d\n",nowNum);
		#endif
#endif
#ifdef	WIN32
		closesocket(*fd_ptr);
#endif
	}
	*fd_ptr = 0;
}

void 
AsrSession::sess_disconn_server()
{
	sess_close_fd(&sess_fd_);
}


int
AsrSession::sess_start()
{
	//printf("AsrSession::sess_start()\n");
	//asr_backtrace("AsrSession::start<1>");
	SLOG_FUNCTIONC_IN("AsrSession_Start");

	int start_timeout = 10;	// experimental

	int resultcode = 0;

	sess_pcm_len_ = 0;
	
	struct SentMsgBlock	send_msg_blk;
	send_msg_blk.finalMsgFrame = new char[kMsgFrameLen_];
	if(send_msg_blk.finalMsgFrame == NULL)
	{
		return SESSION_LOGIC_ERR::ASRCLIENT_FATAL_ERROR;
	}

	unsigned int		infoAttrLen = 0;
	send_msg_blk.req_type = START_SESSION_CMD;

#ifndef _SPEEX_CODEC_
	//set default audio encode format equal "opus"
	if(sess_attr_blk_->Code2IndexTab[SERV_SUP_ATTR::SSUP_AUDIO_ENC_METH]==AttrInfoBlk::NOTSETFLAG)
	{
		sess_set_attr(SERV_SUP_ATTR::SSUP_AUDIO_ENC_METH, "opus");
	}
#endif
	
	send_msg_blk.attrsBlk = sess_attr_blk_;

	SLOG_TINY_INFO((sess_attr_blk_->Attrs[sess_attr_blk_->Code2IndexTab[SERV_SUP_ATTR::SSUP_IMEI_SET]].AttrValue));
	//*******************************************//
	//using sess_attr_blk_ to translate attributes to lower layer
	//*******************************************//
	
	int status_code;
	if((status_code = sess_proto_psr_.AssembleData(&send_msg_blk, NULL, &infoAttrLen)) != PP_OK)
	{
		delete []send_msg_blk.finalMsgFrame;


		// Make sure the fd is free.
		// close(fd);


		return sess_trans_status(status_code);
	}

	SLOG_ALL_ATTRS(send_msg_blk.attrsBlk);

	//If AssembleData Ok, we will call sess_send_n function to send data, and we will cut the msg into several segment as MTU define
	unsigned int send_buf_len=0;
	int nflag=0;
	int sentNumPerTime=0;

	while(send_buf_len<infoAttrLen)
	{
		sentNumPerTime = (infoAttrLen - send_buf_len)>sess_mtu_len_ ? sess_mtu_len_ : (infoAttrLen - send_buf_len);
		if((nflag = sess_send_n(&sess_fd_, send_msg_blk.finalMsgFrame + send_buf_len, sentNumPerTime, start_timeout))<0)
		{
			LOGE("SES-JNI", "start: call sess_send_n error");
			
			delete []send_msg_blk.finalMsgFrame;
			printf("sess_send_n failed in AsrSerssion::start(), ret:%d\n", nflag);
			//printf("We use this %d fd to start session.\n", fd);
			return nflag;
		}

		//printf("We use this %d fd to start session.\n", fd);
		SLOG_SENT_NUM(nflag);
		send_buf_len+=nflag;
	}

	//asr_backtrace("AsrSession::start<3>");

	SLOG_EVENT_TRACE("socket", "action,status", "send,succ");
	SLOG_EVENT_TRACE("session", "action,status", "start_session,succ");
	 
	//After start session, we clear attributes
	sess_clr_attrs();

	//firstly, we receive the header of rsp, and know how many bytes we need to receive later
	int InfoNum=sess_proto_psr_.GetNum4Info();
	char* Info = new char[InfoNum];
	if(Info == NULL)
	{
		delete []send_msg_blk.finalMsgFrame;
		return SESSION_LOGIC_ERR::ASRCLIENT_FATAL_ERROR;
	}

	//int nflag=0;
	if((nflag = sess_recv_n(&sess_fd_, Info, InfoNum)) < 0)
	{
		LOGE("SES-JNI", "start: call sess_recv_n error");
		
		delete []send_msg_blk.finalMsgFrame;
		delete []Info;


		// Make sure the fd is free.
		// close(fd);


		return nflag;
	}


	//asr_backtrace("AsrSession::start<4>");


	//We firstly get the len of msg(with attribute) and rsp status code
	int len2recv=sess_proto_psr_.HowManyBytes2Recv(Info, &sess_recv_blk_);

	//if receive data len is greater than 2048, we return error
	//but we can also accept this later version
	if(len2recv >= MAX_RSP_BODY_LEN)
	{
		//asr_backtrace("AsrSession::start<4.1>");
		delete []send_msg_blk.finalMsgFrame;
		delete []Info;
		

		// Make sure the fd is free.
		//close(fd);

		return SESSION_LOGIC_ERR::ASRCLIENT_SERVER_RETURN_TOO_LONG_RESP;
	}

	if(len2recv != 0)
	{
		//asr_backtrace("AsrSession::start<4.2>");
		if((nflag=sess_recv_n(&sess_fd_, sess_rsp_buf_, len2recv), 5000) < 0)	// fail
		{
			LOGE("SES-JNI", "start: call sess_recv_n error 2");
			
			delete []send_msg_blk.finalMsgFrame;
			delete []Info;


			// Make sure the fd is free.
			//close(fd);


			return nflag;
		}

		SLOG_EVENT_TRACE("socket", "action,status", "recv,succ");
			 
		//cut it, it is very important, cause we just point result to sess_rsp_buf_'s msgbody start index, so we should add end flag
		sess_rsp_buf_[len2recv]=0;
		sess_recv_blk_.ResetBodyInfo();

		if((resultcode = sess_proto_psr_.ParseDataFromServer(sess_rsp_buf_, &sess_recv_blk_)) != PP_OK)
		{
			delete []send_msg_blk.finalMsgFrame;
			delete []Info;


			// Make sure the fd is free.
			//close(fd);


			return sess_trans_status(resultcode);
		}

		char tmp[100];
		snprintf(tmp, 100-1, "response,%d", sess_recv_blk_.rsp_status);
		SLOG_EVENT_TRACE("session", "action,code", tmp);


		//asr_backtrace("AsrSession::start<4.3>");
	
	
		resultcode = sess_call_back(START_SESSION_CMD, sess_recv_blk_.rsp_status, sess_recv_blk_.msgBody, sess_recv_blk_.msgBodyLen);
	}

	delete []send_msg_blk.finalMsgFrame;
	delete []Info;

	SLOG_FUNCTIONC_OUT("AsrSession_Start");
	return resultcode;
}


int
AsrSession::start_cb(int statuscode, char* msgbody, unsigned int msglen)
{

	SLOG_FUNCTIONC_IN("AsrSession_Start_cb");
	if ( statuscode != SUCC_CODE::ASRCLIENT_START_SESSION_OK)
	{
		return sess_status_to_errcode(statuscode);
	}

	if(msglen!=0)
	{
		SLOG_ERROR_MSG("MsgLen_is_Zero");
		LOGE("SES-JNI", "start_cb: msglen is not zero.");
	}

	SLOG_FUNCTIONC_OUT("AsrSession_Start_cb");
	return SUCC_CODE::ASRCLIENT_CREATE_SERVICE_OK;
}

/*
 * send messages, whick catch the PCM data,  to AsrServer
 */
int
AsrSession::sess_resume(char* data, int len)
{
	SLOG_FUNCTIONC_IN("AsrSession_Resume");
	SLOG_REQ_BODY_LEN(len);
	sess_pcm_len_ += len;

	PParserResult		status_code;
   	unsigned int		frameLen = 0;
	struct SentMsgBlock	send_msg_blk;

	send_msg_blk.finalMsgFrame = new char[kMsgFrameLen_+len];
	if(send_msg_blk.finalMsgFrame == NULL)
	{
		return SESSION_LOGIC_ERR::ASRCLIENT_FATAL_ERROR;
	}

	send_msg_blk.msgbody = data;
	send_msg_blk.msgbodylen = len;
	send_msg_blk.req_type = RESUME_SESSION_CMD;
	send_msg_blk.attrsBlk = sess_attr_blk_;
	
	//int status_code=sess_proto_psr_->SendData2Server(START_SESSION_CMD,&attrblk,(char*)&session_opt,sizeof(session_opt_t),fd,start_timeout);
	//If AssembleData happen error, we return with some error code msg
	if((status_code=sess_proto_psr_.AssembleData(&send_msg_blk, NULL, &frameLen))!=PP_OK)
	{
		delete []send_msg_blk.finalMsgFrame;
		return sess_trans_status(status_code);
	}

	//If AssembleData Ok, we will call sess_send_n function to send data, and we will cut the msg into several segment as MTU define
	//int sentTime= (infoAttrLen%sess_mtu_len_==0) ? infoAttrLen/sess_mtu_len_ : (infoAttrLen/sess_mtu_len_+1) ;
	unsigned int send_buf_len=0;
	int nflag=0;
	//Because, we will not only sent out Header and attributes, but also data field, so infoAttrLen has to be counted.
	//infoAttrLen+=len;
	int sentNumPerTime=0;

	while(send_buf_len<frameLen)
	{
		sentNumPerTime = (frameLen - send_buf_len) > sess_mtu_len_ ? sess_mtu_len_ : (frameLen - send_buf_len);
        
        //printf("-----------------------------------start-----------------------------------\n");
        
		if((nflag = sess_send_n(&sess_fd_, send_msg_blk.finalMsgFrame+send_buf_len, sentNumPerTime))<0)
		{
			LOGE("SES-JNI", "sess_resume: call sess_send_n error");
			delete []send_msg_blk.finalMsgFrame;
			return nflag;
		}
        
        //printf("-----------------------------------stop-----------------------------------\n");

		send_buf_len+=nflag;
	}

	sess_clr_attrs();
    
    //printf("-----------------------------------sess_clr_attrs-----------------------------------\n");

	delete []send_msg_blk.finalMsgFrame;

	SLOG_FUNCTIONC_OUT("AsrSession_Resume");
    
    //printf("-----------------------------------sess_trans_status-----------------------------------\n");
    
	return sess_trans_status(status_code);
}


int
AsrSession::sess_stop(int waitingTime) 
{
    //printf("AsrSession::sess_stop()\n");
	SLOG_FUNCTIONC_IN("AsrSession_Stop");
	int resultcode=0;

	PParserResult		status_code;;
	struct SentMsgBlock	send_msg_blk;
	send_msg_blk.finalMsgFrame = new char[kMsgFrameLen_];
	if(send_msg_blk.finalMsgFrame == NULL)
	{
		return SESSION_LOGIC_ERR::ASRCLIENT_FATAL_ERROR;
	}

	unsigned int infoAttrLen=0;
	send_msg_blk.req_type = STOP_SESSION_CMD;
	send_msg_blk.attrsBlk=sess_attr_blk_;
	
	//int status_code=sess_proto_psr_->SendData2Server(START_SESSION_CMD,&attrblk,(char*)&session_opt,sizeof(session_opt_t),fd,start_timeout);
	//If AssembleData happen error, we return with some error code msg
	if((status_code=sess_proto_psr_.AssembleData(&send_msg_blk, NULL, &infoAttrLen))!=PP_OK)
	{
		delete []send_msg_blk.finalMsgFrame;
		return sess_trans_status(status_code);
	}

	//If AssembleData Ok, we will call sess_send_n function to send data, and we will cut the msg into several segment as MTU define
	//int sentTime= (infoAttrLen%sess_mtu_len_==0) ? infoAttrLen/sess_mtu_len_ : (infoAttrLen/sess_mtu_len_+1) ;
	unsigned int send_buf_len=0;
	int nflag=0;
	int sentNumPerTime=0;

	while(send_buf_len<infoAttrLen)
	{
		SLOG_INT_TRACE(LOG_LEVEL::LOG_INFO, "InfoAttrLen", infoAttrLen);
		sentNumPerTime = (infoAttrLen - send_buf_len)>sess_mtu_len_ ? sess_mtu_len_ : (infoAttrLen - send_buf_len);
		SLOG_INT_TRACE(LOG_LEVEL::LOG_INFO, "sentNumPerTime", sentNumPerTime);
		if((nflag = sess_send_n(&sess_fd_, send_msg_blk.finalMsgFrame+send_buf_len, sentNumPerTime))<0)
		{
			LOGE("SES-JNI", "stop: call sess_send_n error");
			
			delete []send_msg_blk.finalMsgFrame;
			return nflag;
		}

		send_buf_len+=nflag;
	}

	sess_clr_attrs();

	//firstly, we receive the header of rsp, and know how many bytes we need to receive later
	int InfoNum=sess_proto_psr_.GetNum4Info();
	char* Info=new char[InfoNum];
	if(Info == NULL)
	{
		delete []send_msg_blk.finalMsgFrame;
		return SESSION_LOGIC_ERR::ASRCLIENT_FATAL_ERROR;
	}

	if((nflag = sess_recv_n(&sess_fd_, Info, InfoNum, waitingTime))<0)
	{
		LOGE("SES-JNI", "stop: call sess_recv_n error");
		
		delete []send_msg_blk.finalMsgFrame;
		delete []Info;
		return nflag;
	}


	//We firstly get the len of msg(with attribute) and rsp status code
	//struct RecvMsgBlock sess_recv_blk_;
	int len2recv=sess_proto_psr_.HowManyBytes2Recv(Info, &sess_recv_blk_);

	//if receive data len is greater than 2048, we return error
	//but we can also accept this later version
	if(len2recv>=MAX_RSP_BODY_LEN)
	{
		delete []send_msg_blk.finalMsgFrame;
		delete []Info;
		return SESSION_LOGIC_ERR::ASRCLIENT_SERVER_RETURN_TOO_LONG_RESP;
	}

	if(len2recv!=0)
	{		
		if((nflag = sess_recv_n(&sess_fd_, sess_rsp_buf_, len2recv, waitingTime))<0)
		{
			LOGE("SES-JNI", "stop: call sess_recv_n error 2");
			
			delete []send_msg_blk.finalMsgFrame;
			delete []Info;
			return nflag;
		}

		//cut it, it is very important, cause we just point result to sess_rsp_buf_'s msgbody start index, so we should add end flag
		sess_rsp_buf_[len2recv]=0;
		sess_recv_blk_.ResetBodyInfo();

		if((status_code=sess_proto_psr_.ParseDataFromServer(sess_rsp_buf_, &sess_recv_blk_))!=PP_OK)
		{
			delete []send_msg_blk.finalMsgFrame;
			delete []Info;
			return sess_trans_status(status_code);
		}

		resultcode=sess_call_back(STOP_SESSION_CMD, sess_recv_blk_.rsp_status, sess_recv_blk_.msgBody, sess_recv_blk_.msgBodyLen);
	}

	delete []send_msg_blk.finalMsgFrame;
	delete []Info;

	SLOG_RESULT_VALUE(result);
	SLOG_ASR_RSP_CODE(resultcode);

	SLOG_FUNCTIONC_OUT("AsrSession_Stop");
	return resultcode;	
}

int
AsrSession::sess_stop_cb(int statuscode, char * rspmsgbody, unsigned int msglen)
{
	SLOG_FUNCTIONC_IN("AsrSession_StopCB");
	SLOG_RSP_BODY_LEN(msglen);
	SLOG_RSP_STATUS(statuscode);
	if(statuscode!= SUCC_CODE::ASRCLIENT_RECOGNIZER_OK)
		return sess_status_to_errcode(statuscode);

	SLOG_RSP_MSG_BODY(rspmsgbody);

	if(sess_decrypt_txt(rspmsgbody,msglen)!=true)
		return SESSION_LOGIC_ERR::ASRCLIENT_CONTENT_DECODE_ERROR;

	sess_set_res(rspmsgbody);

	SLOG_RESULT_VALUE(result);

	SLOG_FUNCTIONC_OUT("AsrSession_StopCB");
	return SUCC_CODE::ASRCLIENT_RECOGNIZER_OK;
}


int
AsrSession::sess_cancel() 
{
	//PParserResult is an enum type variable
	PParserResult		res_status;
	
	//
	struct SentMsgBlock	send_msg_blk;
	unsigned int		attrs_len = 0;

	// for loop send
	unsigned int		send_buf_len = 0;
	int			send_cur_len = 0;
	int			send_len_per_time = 0;

	//store the final msg
	send_msg_blk.finalMsgFrame = new char[kMsgFrameLen_];
	if(send_msg_blk.finalMsgFrame == NULL)
	{
		return SESSION_LOGIC_ERR::ASRCLIENT_FATAL_ERROR;
	}
	send_msg_blk.req_type = CANCEL_SESSION_CMD;
	send_msg_blk.attrsBlk = sess_attr_blk_;

	// 1. Call AssembleData to build the SentMsgBlock
	if((res_status = sess_proto_psr_.AssembleData(&send_msg_blk, NULL, &attrs_len)) != PP_OK)
	{
		delete []send_msg_blk.finalMsgFrame;
		return sess_trans_status(res_status);
	}


	// 2. loop send
	while(send_buf_len < attrs_len)
	{
		send_len_per_time = (attrs_len - send_buf_len) > sess_mtu_len_ ? sess_mtu_len_ : (attrs_len - send_buf_len);
		if((send_cur_len = sess_send_n(&sess_fd_, send_msg_blk.finalMsgFrame + send_buf_len, send_len_per_time)) < 0)
		{
			LOGE("SES-JNI", "sess_cancel: call sess_send_n error");
			delete []send_msg_blk.finalMsgFrame;
			return send_cur_len;
		}

		send_buf_len += send_cur_len;
	}
	
	// 3. clear attrs & delete mem
	sess_clr_attrs();
	delete []send_msg_blk.finalMsgFrame;

	return sess_trans_status(res_status);
}


/*
 * Get the result block by block
 */
int
AsrSession::sess_query_res(int waitingTime)
{
    //printf("AsrSession::sess_query_res()\n");
	SLOG_FUNCTIONC_IN("AsrSession_QueryPartialResult");

	//
	PParserResult	parse_res;
	int		query_res = 0;

	//
	PParserResult	status_code;
   	unsigned int	infoAttrLen = 0;

	//
	unsigned int	send_buf_len=0;
	int		nflag = 0;
	int		sentNumPerTime = 0;

	//
	struct SentMsgBlock	send_msg_blk;

	// construct the blk with GET_PARTIAL_RESULT_CMD
	send_msg_blk.finalMsgFrame = new char[kMsgFrameLen_];
	if(send_msg_blk.finalMsgFrame == NULL)
	{
		return SESSION_LOGIC_ERR::ASRCLIENT_FATAL_ERROR;
	}

	send_msg_blk.req_type = GET_PARTIAL_RESULT_CMD;
	send_msg_blk.attrsBlk = sess_attr_blk_;
	
	// 1. Assemble the blk
	if((status_code = sess_proto_psr_.AssembleData(&send_msg_blk, NULL, &infoAttrLen))!=PP_OK)
	{
		delete []send_msg_blk.finalMsgFrame;
		return sess_trans_status(status_code);
	}

	// 2. loop send
	while(send_buf_len < infoAttrLen)
	{
		sentNumPerTime = (infoAttrLen - send_buf_len) > sess_mtu_len_ ? sess_mtu_len_ : (infoAttrLen - send_buf_len);
		if((nflag = sess_send_n(&sess_fd_, send_msg_blk.finalMsgFrame+send_buf_len, sentNumPerTime)) < 0)
		{
			LOGE("SES-JNI", "query: call sess_send_n error");
            
			delete []send_msg_blk.finalMsgFrame;
			return nflag;
		}

		send_buf_len+=nflag;
	}

	sess_clr_attrs();
	
	//firstly, we receive the header of rsp, and know how many bytes we need to receive later
	int InfoNum=sess_proto_psr_.GetNum4Info();
	char* Info=new char[InfoNum];
	if(Info == NULL)
	{
		delete []send_msg_blk.finalMsgFrame;
		return SESSION_LOGIC_ERR::ASRCLIENT_FATAL_ERROR;
	}

	if((nflag=sess_recv_n(&sess_fd_, Info, InfoNum, waitingTime))<0)
	{
		LOGE("SES-JNI", "query: call sess_recv_n error");
        
		delete []send_msg_blk.finalMsgFrame;
		delete []Info;
		return nflag;
	}


	//We firstly get the len of msg(with attribute) and rsp status code
	//struct RecvMsgBlock sess_recv_blk_;
	int len2recv=sess_proto_psr_.HowManyBytes2Recv(Info, &sess_recv_blk_);

	//if receive data len is greater than 2048, we return error
	//but we can also accept this later version
	if(len2recv>=MAX_RSP_BODY_LEN)
	{
		delete []send_msg_blk.finalMsgFrame;
		delete []Info;
		return SESSION_LOGIC_ERR::ASRCLIENT_SERVER_RETURN_TOO_LONG_RESP;
	}

	if(len2recv!=0)
	{		
		if((nflag=sess_recv_n(&sess_fd_, sess_rsp_buf_, len2recv, waitingTime))<0)
		{
			LOGE("SES-JNI", "query: call sess_recv_n error 2");
			
			delete []send_msg_blk.finalMsgFrame;
			delete []Info;
			return nflag;
		}

		//cut it, it is very important, cause we just point result to sess_rsp_buf_'s msgbody start index, so we should add end flag
		sess_rsp_buf_[len2recv]=0;
		sess_recv_blk_.ResetBodyInfo();

		if((parse_res = sess_proto_psr_.ParseDataFromServer(sess_rsp_buf_, &sess_recv_blk_))!=PP_OK)
		{
			delete []send_msg_blk.finalMsgFrame;
			delete []Info;
			return sess_trans_status(parse_res);
		}

		query_res = sess_call_back(GET_PARTIAL_RESULT_CMD, sess_recv_blk_.rsp_status, sess_recv_blk_.msgBody, sess_recv_blk_.msgBodyLen);
	}

	delete []send_msg_blk.finalMsgFrame;
	delete []Info;

	SLOG_RESULT_VALUE(result);
	SLOG_ASR_RSP_CODE(query_res);

	SLOG_FUNCTIONC_OUT("AsrSession_QueryPartialResult");

	return query_res;	
}


int AsrSession::sess_query_res_cb(int statuscode, char * rspmsgbody, unsigned int msglen)
{

	SLOG_FUNCTIONC_IN("AsrSession_QueryPartialResultCB");
	SLOG_RSP_BODY_LEN(msglen);

	if(statuscode!= SUCC_CODE::ASRCLIENT_RECOGNIZER_OK)
	{
		SLOG_FUNCTIONC_OUT("AsrSession_QueryPartialResultCB/statusNotOK");
		return sess_status_to_errcode(statuscode);
	}

	if(msglen!=0)
	{
		SLOG_TINY_INFO("Has_Data");
		
		if(sess_decrypt_txt(rspmsgbody,msglen)!=true)			
		{
			sess_close_fd(&sess_fd_);
			return SESSION_LOGIC_ERR::ASRCLIENT_CONTENT_DECODE_ERROR;
		}

		sess_set_res(rspmsgbody);
		//SLOG_ASR_RSP_CODE(ASR_RECOGNIZER_PARTIAL_RESULT);
		SLOG_FUNCTIONC_OUT("AsrSession_QueryPartialResultCB_WithData");
		return SUCC_CODE::ASRCLIENT_RECOGNIZER_PARTIAL_RESULT;
	}

	SLOG_FUNCTIONC_OUT("AsrSession_QueryPartialResultCB_NoData");
	return SUCC_CODE::ASRCLIENT_RECOGNIZER_OK;
}


int
AsrSession::sess_trans_status(int PPstatus)
{
	int	result_code = SOCKET_CONN_ERR::ASRCLIENT_COMMUNICATION_ERROR;

	switch (PPstatus)
	{
		case PP_OK:
			result_code= SUCC_CODE::ASRCLIENT_RECOGNIZER_OK;
			break;
		case PP_ERROR:
			result_code = INTERNAL_ERR::ASRCLIENT_FATAL_ERROR;
			break;
		case PP_FATAL_ERROR:
			result_code = INTERNAL_ERR::ASRCLIENT_FATAL_ERROR;
			break;
		case PP_COMMUNICATION_ERROR:
			result_code = SOCKET_CONN_ERR::ASRCLIENT_COMMUNICATION_ERROR;
			break;
		case PP_ATTRS_NOT_SUPPORT:
			result_code = PROTOCOL_ERR::ASRCLIENT_ATTRS_NOT_SUPPORT;
			break;
		case PP_MSG_FORMAT_ERROR:
			result_code=PROTOCOL_ERR::ASRCLIENT_MSG_FORMAT_ERROR;
			break;
	}

	return result_code;
}

int
AsrSession::sess_status_to_errcode(int status)
{
	int	errorCode = 0;
	switch(status){
		case ASR_SERVICE_BUSY:
			errorCode = PROTOCOL_ERR::ASRCLIENT_SERVICE_BUSY;
			break;
		case ASR_START_SESSION_ERROR:	//start session error. in most of cases, request head parameters are wrong
			errorCode = PROTOCOL_ERR::ASRCLIENT_START_SESSION_ERROR;
			break;
		case ASR_APP_KEY_CHECK_ERROR://APP key check fail, that means, it is a invalid key
			errorCode = SESSION_LOGIC_ERR::ASRCLIENT_AUTHORIZE_ERROR;
			break;
		case ASR_STOP_SESSION_ERROR:		//stop session error.Bad stop session head parameter
			errorCode = PROTOCOL_ERR::ASRCLIENT_STOP_SESSION_ERROR;
			break;
		case ASR_UNKNOWN_HEAD_ERROR:		//other errors,program currently not catched.
			errorCode = PROTOCOL_ERR::ASRCLIENT_UNKNOWN_HEAD_ERROR;
			break;
		case ASR_SERVICE_UNAVAI_ERROR:		//Server is down or can not accept user connection
			errorCode = PROTOCOL_ERR::ASRCLIENT_SERVICE_UNAVAI_ERROR;
			break;		
		case ASR_RECOGNIZE_ERROR:			//asr recognizing error
			errorCode = PROTOCOL_ERR::ASRCLIENT_RECOGNIZE_ERROR;
			break;	
		case ASR_SYSTEM_ERROR:
			errorCode = PROTOCOL_ERR::ASRCLIENT_SYSTEM_ERROR;
			break;	
		case ASR_ATTR_NOT_SUPPORT:
			errorCode = PROTOCOL_ERR::ASRCLIENT_ATTRS_NOT_SUPPORT;
			break;	
		case ASR_MSG_FORMAT_ERROR:
			errorCode = PROTOCOL_ERR::ASRCLIENT_MSG_FORMAT_ERROR;
			break;	
		case ASR_REQ_NOT_SUPPORT:
			errorCode = PROTOCOL_ERR::ASRCLIENT_COMMAND_ERROR;
			break;	
		case ASR_BAD_FRAME_FOUND:
			errorCode = PROTOCOL_ERR::ASRCLIENT_MSG_FORMAT_ERROR;
			break;	
		case ASR_FRAME_LEN_TOO_GREAT:
			errorCode = PROTOCOL_ERR::ASRCLIENT_MSG_FORMAT_ERROR;
			break;	
		case ASR_MAGIC_NUM_WRONG:	
			errorCode = PROTOCOL_ERR::ASRCLIENT_MSG_FORMAT_ERROR;
			break;
#ifdef ORAL_EVAL
		case ASR_ORAL_MODE_BUT_NO_TEXT_UPLOAD:
			errorCode = ORAL_EVAL_ERR::ASRCLIENT_ORAL_MODE_BUT_NO_TEXT_UPLOAD;
			break;
		case ASR_ORAL_MODE_EVAL_ERROR:
       			errorCode = ORAL_EVAL_ERR::ASRCLIENT_ORAL_MODE_EVAL_ERROR;
			break;
		case ASR_ORAL_MODE_NOT_COLLECTED_WORD:
       			errorCode = ORAL_EVAL_ERR::ASRCLIENT_ORAL_MODE_NOT_COLLECTED_WORD;
			break;
		case ASR_ORAL_MODE_EVAL_SPEECH_TOO_LONG:
       			errorCode = ORAL_EVAL_ERR::ASRCLIENT_ORAL_MODE_EVAL_SPEECH_TOO_LONG;   
			break;
        	case ASR_RESULT_TOO_LONG:
            		errorCode = PROTOCOL_ERR::ASRCLIENT_RESULT_TOO_LONG;
            		break;
#endif
	}
	return errorCode;
}

bool
AsrSession::sess_decrypt_txt(char* srcText,int len)
{
	SLOG_FUNCTIONC_IN("AsrSession_sess_decrypt_txt");
	bool deresult=true;
	
	if(len==0)
	{
		SLOG_FUNCTIONC_OUT("AsrSession_sess_decrypt_txt_NoData");
		return true;
	}
	
	if(sess_recv_blk_.attrBlk.Code2IndexTab[SERV_SUP_ATTR::SSUP_ENCRYPT]!=AttrInfoBlk::NOTSETFLAG)
	{

#ifdef DES_ALGO
		coreDecrypt_des(srcText, len);
#else
		coreDecrypt_easy(srcText, len);
#endif
		
	}

	SLOG_FUNCTIONC_OUT("AsrSession_sess_decrypt_txt_WithData");
	return deresult;
}

#ifdef DES_ALGO
int
AsrSession::coreDecrypt_des(char* srcText, int len)
{
	unsigned char* encryptText = new unsigned char[len];
	if(encryptTest == NULL)
	{
		return SESSION_LOGIC_ERR::ASRCLIENT_FATAL_ERROR;
	}

	DES_cblock	Civ;
	memset((void*)&Civ,5,sizeof(DES_cblock));
	DES_key_schedule iKs;
	DES_set_key((const_DES_cblock*)sess_recv_blk_.attrBlk.Attrs[sess_recv_blk_.attrBlk.Code2IndexTab[SERV_SUP_ATTR::SSUP_ENCRYPT]].AttrValue,&iKs);

	memcpy((char*)encryptText,srcText,len);
	
	int bitnum=8;
	DES_cfb_encrypt(encryptText,(unsigned char*)srcText,bitnum,(long)len,&iKs,&Civ,DES_DECRYPT);
	
	delete []encryptText;	
	return 0;
}
#else

int
AsrSession::coreDecrypt_easy(char* srcText, int len)
{
	unsigned int index_src, index_key;
	const char* key=sess_recv_blk_.attrBlk.Attrs[sess_recv_blk_.attrBlk.Code2IndexTab[SERV_SUP_ATTR::SSUP_ENCRYPT]].AttrValue;
	const int key_len=strlen(sess_recv_blk_.attrBlk.Attrs[sess_recv_blk_.attrBlk.Code2IndexTab[SERV_SUP_ATTR::SSUP_ENCRYPT]].AttrValue);
	index_key=0;
	
	for(index_src=0; index_src<len; index_src++)
	{
		srcText[index_src]^= key[index_key];
		index_key= (index_key+1)%key_len;
	}
	
	return 0;
}
#endif

int AsrSession::sess_call_back(int sentCmd,int statuscode, char * rspmsgbody, unsigned int rspmsglen)
{
	SLOG_FUNCTIONC_IN("AsrSession_sess_call_back");

	int resultcode = SUCC_CODE::ASRCLIENT_RECOGNIZER_OK;
	switch(sentCmd)
	{
		case START_SESSION_CMD:
			resultcode = start_cb(statuscode,rspmsgbody,rspmsglen);
			break;
		case RESUME_SESSION_CMD:
			resultcode = SUCC_CODE::ASRCLIENT_RECOGNIZER_OK;
			break;
		case STOP_SESSION_CMD:
			resultcode = sess_stop_cb(statuscode,rspmsgbody,rspmsglen);
			break;
		case CANCEL_SESSION_CMD:
			resultcode = SUCC_CODE::ASRCLIENT_RECOGNIZER_OK;
			break;
		case GET_PARTIAL_RESULT_CMD:
			resultcode = sess_query_res_cb(statuscode,rspmsgbody,rspmsglen);
			break;
	}

	SLOG_ASR_RSP_CODE(resultcode);
	SLOG_FUNCTIONC_OUT("AsrSession_sess_call_back");
	return resultcode;
}

/*struct AttrInfoBlk* AsrSession::getattrblk()
{
	return m_pAttrBlk;
}*/

bool AsrSession::sess_chk_vld_attr(struct AttrInfoBlk* attrsBlk)
{
	for(int i=0;i<attrsBlk->AttrNum;i++)
		if(gservATTR.ValueInSupList(attrsBlk->Attrs[i].AttrCode, attrsBlk->Attrs[i].AttrValue, strlen(attrsBlk->Attrs[i].AttrValue))==false)
			return false;
	
	return true;		
}

bool AsrSession::sess_chk_vld_attr(unsigned char attr_code, const char* attr_value){
	SLOG_FUNCTIONC_IN("sess_chk_vld_attr");
	if(gservATTR.ValueInSupList(attr_code, attr_value, strlen(attr_value))==false)
	{
		SLOG_FUNCTIONC_OUT("sess_chk_vld_attr/CheckFail");
		return false;
	}
	else
	{
		//add asrSession level check
		if(sess_chk_attr(attr_code, attr_value)!=0)
		{
			SLOG_FUNCTIONC_OUT("sess_chk_vld_attr/asrSessionCheckFail");
			return false;
		}
		SLOG_FUNCTIONC_OUT("sess_chk_vld_attr/OK");
		return true;
	}
}


/*
 *
 */
int
AsrSession::sess_send_n(int* socket_fd, char* buffer, int buffer_len, int timeout)
{
	//printf("\n\nsess_send_n.\n");
	//printf("Para,  buffer_len: %d,\n", buffer_len);
	SLOG_FUNCTIONC_IN("sess_send_n");
	
	int errno_num;
	char	err_msg[128] = {0};
	int errMsgLen = 128-1;
	int	rt = 0;
	int	sent = 0;
	int	retval;

	do
	{
#ifndef OS_HAS_POLL
		//printf("has POLL, use select!!!!!!!!\n");
		fd_set		wfds;
		struct timeval	tv;
		
		FD_ZERO(&wfds);
		FD_SET(*socket_fd, &wfds);
		
		/* Wait up to five seconds. */
		tv.tv_sec = timeout;
		tv.tv_usec = 0;
		retval = select(*socket_fd + 1, NULL, &wfds, NULL, (timeout == -1) ? NULL : &tv);
		if(retval <= 0 && errno EQ EINTR)
		{
			if(timeout > 0)
			{
				timeout -= tv.tv_sec;
			}
			continue;
		}
		else if(retval <= 0)
		{
			snprintf(err_msg, errMsgLen, "sess_send_r select timeout, fd = %d, timeout = %d, errorno = %d", *socket_fd, timeout, errno);
			LOGE("SES-JNI", err_msg);

			sess_close_fd(socket_fd);
			
			return SOCKET_CONN_ERR::ASRCLIENT_WRITE_SELECT_FAIL;
		}


		// now, check the fd is really able to write
		int		error;

#ifdef LINUX
		socklen_t	len = sizeof(error);
#elif defined (WIN32)
		int	len = sizeof(error);
#endif

#else
		struct pollfd fds[1];
		struct timespec tv;
		const struct timespec* tout;
		tv.tv_sec = timeout;	

		fds[0].fd = *socket_fd;
		fds[0].events = POLLOUT;		
		fds[0].revents = 0;
		
		sigset_t origmask;
		//retval = ppoll (fds, sizeof(fds)/sizeof(struct pollfd), (timeout==-1)?NULL:&tv, NULL);
		tout = &tv;
		retval = poll (fds, sizeof(fds)/sizeof(struct pollfd), timeout*1000);
		if ( retval <=0 ){

			snprintf(err_msg, errMsgLen, "sess_send_n ppoll timeout, fd = %d, timeout = %d, errorno = %d", *socket_fd, timeout, errno);
			LOGE("SES-JNI", err_msg);
			
			sess_close_fd(socket_fd);
			//printf("3333\n");
			return SOCKET_CONN_ERR::ASRCLIENT_WRITE_SELECT_FAIL;
		}
#endif
		
		//printf("---------------------------------------------\n");
		//printf("buffer_len: %d\n.", buffer_len);
		//printf("sent: %d\n.", sent);
		rt = send(*socket_fd, buffer+sent, buffer_len - sent, 0);
        
        //printf("buffer : %zu | sent : %d | buffer_len : %d\n",strlen(buffer), sent, buffer_len);
        //printf("buffer_len - sent: %d | result ：%d \n.", buffer_len - sent, rt);

		if ( rt > 0 )
			sent += rt;
#ifdef LINUX
		else if ( rt < 0 && errno EQ EINTR )
			continue;
#endif
		else
		{
			//errno_num = WSAGetLastError();

			snprintf(err_msg, errMsgLen, "sess_send_n send error, fd = %d, errorno = %d", *socket_fd, errno);
			LOGE("SES-JNI", err_msg);
			
			//printf("Failed in AsrSession::sess_send_n: %d\n", rt);
			//printf("Windows errno:%d\n", errno_num);
			return SOCKET_CONN_ERR::ASRCLIENT_WRITE_TIMEOUT;

		}

	}while (sent < buffer_len );

	SLOG_SENT_NUM(sent);
	SLOG_FUNCTIONC_OUT("sess_send_n");
	return sent;
}

int
AsrSession::sess_recv_n(int* socket, char* buffer,int buffer_len,int timeout)
{
	char	err_msg[128];
	int errMsgLen = 128-1;
	int	recv_rt=0;
	int	recv_len = 0;
	int timedout = timeout;
	do
	{
		int retval;
#ifndef OS_HAS_POLL			
		fd_set rfds;
		struct timeval tv;
		
		FD_ZERO(&rfds);
		FD_SET(*socket, &rfds);
		
		// Wait up to five seconds.
		tv.tv_sec = timedout;
		tv.tv_usec = 0;
		retval = select(*socket+1, &rfds, NULL, NULL, (timeout==-1)?NULL:&tv);
        
        //printf("sess_recv_n - > timeout : %d | result : %d\n",timedout, retval);
        
        if ( retval <=0 && errno EQ EINTR )
		{
			if ( timedout >0 ) timedout -= tv.tv_sec;
			continue;
		}
		else if ( retval <=0 )
		{
			sprintf(err_msg, "sess_recv_n select timeout, fd = %d, timeout = %d, errorno = %d", *socket, timeout, errno);
			LOGE("SES-JNI", err_msg);
            
            //printf("READ_SELECT_FAIL : %d | timeout : %ld\n",retval, tv.tv_sec);
			
			return SOCKET_CONN_ERR::ASRCLIENT_READ_SELECT_FAIL;
		}

#else
		struct pollfd fds[1];
		const struct timespec* tout;
		struct timespec tv;
		tv.tv_sec = timeout;	

		fds[0].fd = *socket;
		fds[0].events = POLLIN ;
		
		fds[0].revents = 0;
		sigset_t origmask;
		//retval = ppoll (fds, sizeof(fds)/sizeof(struct pollfd), (timeout==-1)?NULL:&tv,NULL);
		tout = &tv;
		retval = poll (fds, sizeof(fds)/sizeof(struct pollfd), timeout*1000);
		if ( retval <=0 ) {
			snprintf(err_msg, errMsgLen, "sess_recv_r ppoll timeout, fd = %d, timeout = %d, errorno = %d", socket, timeout, errno);
			LOGE("SES-JNI", err_msg);
			
			return SOCKET_CONN_ERR::ASRCLIENT_READ_SELECT_FAIL;
		}
#endif
		recv_rt = recv(*socket, buffer + recv_len, buffer_len - recv_len, 0);
        //printf("\nAsrSession::sess_recv_n : %d\n",recv_rt);
		if(recv_rt > 0) recv_len += recv_rt;
		else{
			snprintf(err_msg, errMsgLen, "sess_recv_n recv error, fd = %d, errorno = %d", socket, errno);
			LOGE("SES-JNI", err_msg);
			
			return SOCKET_CONN_ERR::ASRCLIENT_READ_TIMEOUT;
		}
	}while (recv_len < buffer_len );

#ifdef PRINT_TRACE
        printf("thread ID:%ld recv_len:%ld buffer_len:%ld\n", (long)(pthread_self()), recv_len, buffer_len);
#endif

	return recv_len;
}

char* AsrSession::sess_get_res()
{
	SLOG_FUNCTIONC_IN("AsrSession_sess_get_res");
	SLOG_FUNCTIONC_OUT("AsrSession_sess_get_res");
	if(sess_has_res_)
		return sess_result_;
	else
		return NULL;
}


/*
 * Altered by lizhongyuan
 * if the user set the "deviceID"/SERV_SUP_ATTR::SSUP_IMEI_SET, get the top 40bytes
 */
int
AsrSession::sess_set_attr(unsigned char _attr_code, const char* _attr_value)
{
	//printf("AsrSession::sess_set_attr(%d, %s)\n", (int)_attr_code, _attr_value);
	SLOG_FUNCTIONC_IN("sess_set_attr");
	SLOG_ATTR_VALUE(_attr_value);	

	// 1. check the value
	if(sess_chk_vld_attr(_attr_code, _attr_value)==false)
	{
		SLOG_FUNCTIONC_OUT("sess_set_attr/ValidCheckFail");
		return PROTOCOL_ERR::ASRCLIENT_ATTRS_NOT_SUPPORT;
	}

	// 2. 
	if(sess_attr_blk_ == NULL)
	{
		//printf("sess_attr_blk_ is NUL");
		sess_attr_blk_ = new struct AttrInfoBlk;
#ifdef PRINT_TRACE
        	printf("thread ID:%ld new sess_attr_blk_:%p\n", (long)(pthread_self()), sess_attr_blk_);
#endif
		if(sess_attr_blk_ == NULL)
		{
#ifdef PRINT_TRACE
                	printf("thread ID:%ld new sess_attr_blk_ fail\n", (long)(pthread_self()));
#endif
			return SESSION_LOGIC_ERR::ASRCLIENT_FATAL_ERROR;
		}
	}


	if( strlen(_attr_value) >= SERV_ATTRS::ATTR_SINGLE_LEN )
	{
		return PROTOCOL_ERR::ASRCLIENT_ATTR_LEN_TOO_LONG;
	}

	// 3.
	if(sess_attr_blk_->Code2IndexTab[_attr_code] == AttrInfoBlk::NOTSETFLAG)
	{
		//printf("  sess_attr_blk_->Code2IndexTab[%d] is 0xFF.\n", (int)_attr_code);
		sess_attr_blk_->Attrs[sess_attr_blk_->AttrNum].AttrCode = _attr_code;	//for example SSUP_APP_KEY = 9
		//printf("  now, the sess_attr_blk_->Attrs[%d].AttrCode is %d\n", sess_attr_blk_->AttrNum, (int)_attr_code);
		if(_attr_code == SERV_SUP_ATTR::SSUP_IMEI_SET)
		{
			strncpy(sess_attr_blk_->Attrs[sess_attr_blk_->AttrNum].AttrValue, _attr_value, 40);
		}
		else
		{
			strcpy(sess_attr_blk_->Attrs[sess_attr_blk_->AttrNum].AttrValue, _attr_value);
			//printf("  and, the sess_attr_blk_->Attrs[%d].AttrValue is %s\n", sess_attr_blk_->AttrNum, sess_attr_blk_->Attrs[sess_attr_blk_->AttrNum].AttrValue);
		}

		//update the array Code2IndexTab
		sess_attr_blk_->Code2IndexTab[_attr_code] = sess_attr_blk_->AttrNum;
		//printf("  sess_attr_blk_->Code2IndexTab[%d]:%d\n", _attr_code, sess_attr_blk_->Code2IndexTab[_attr_code]);
		sess_attr_blk_->AttrNum++;
	}
	else
	{
		//printf("  sess_attr_blk_->Code2IndexTab[%d] isn't 0xff, it's %d.\n", (int)_attr_code, sess_attr_blk_->Attrs[sess_attr_blk_->AttrNum].AttrCode);
		sess_attr_blk_->Attrs[sess_attr_blk_->AttrNum].AttrCode = _attr_code;
		if(_attr_code == SERV_SUP_ATTR::SSUP_IMEI_SET)
		{
			strncpy(sess_attr_blk_->Attrs[sess_attr_blk_->Code2IndexTab[_attr_code]].AttrValue, _attr_value, 40);
		}
		else
		{
			strcpy(sess_attr_blk_->Attrs[sess_attr_blk_->Code2IndexTab[_attr_code]].AttrValue, _attr_value);
		}
	}

	SLOG_FUNCTIONC_OUT("sess_set_attr");		
	return 0;
}

char*
AsrSession::sess_get_attr(unsigned char _attr_code)
{
	//printf("AsrSession::sess_get_attr(%d)\n", (int)_attr_code);
	SLOG_FUNCTIONC_IN("sess_get_attr");
	
	/*
	if(sess_recv_blk_.attrBlk == NULL)
	{
		SLOG_FUNCTIONC_OUT("sess_get_attr/no_attrBlk");
		printf("sess_recv_blk_.attrBlk is NULL\n");
		return NULL;
	}
	*/

	/*user can get the attrs after receive the result*/
	if(sess_recv_blk_.attrBlk.Code2IndexTab[_attr_code] == AttrInfoBlk::NOTSETFLAG)
	{
		SLOG_FUNCTIONC_OUT("sess_get_attr/not_set");
		//printf("sess_recv_blk_.attrBlk.Code2IndexTab[%d] is NOTSETFLAG\n", (int)_attr_code);
		return NULL;
	}
	else
	{
		SLOG_FUNCTIONC_OUT("sess_get_attr");
		return sess_recv_blk_.attrBlk.Attrs[sess_recv_blk_.attrBlk.Code2IndexTab[_attr_code]].AttrValue;
	}

	/*	
	//user will get the attrs all the time
	if(sess_attr_blk_->Code2IndexTab[_attr_code] == AttrInfoBlk::NOTSETFLAG)
	{
		SLOG_FUNCTIONC_OUT("sess_get_attr/not_set");
		printf("sess_recv_blk_.attrBlk.Code2IndexTab[%d] is NOTSETFLAG\n", (int)_attr_code);
		return NULL;
	}
	else
	{
		SLOG_FUNCTIONC_OUT("sess_get_attr");
		return sess_attr_blk_->Attrs[sess_attr_blk_->Code2IndexTab[_attr_code]].AttrValue;
	}
	*/
}
