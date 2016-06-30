#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <unistd.h>

#include <sys/ipc.h>

#include <sys/msg.h>

#include  <time.h>

#define TEXT_SIZE  1000
#define KEY_T_NUM 233

struct Jmsgbuf

{

    long mtype ;

	 int length;

    char mtext[TEXT_SIZE] ;

};

int getMSGID()
{
	int key=KEY_T_NUM;
 	int msqid = msgget( key, 0600|IPC_CREAT ) ;

    	if ( msqid < 0 )
	{

      		perror("create message queue error") ;

        	return -1 ;

    	}
	else
		return msqid;
}

int sendMSG2Q(int msqid,char *pData,int length)

{

    struct Jmsgbuf buf ;

    int flag ;

    int sendlength, recvlength ;
 



	memset((void*)&buf,0x00,sizeof(struct Jmsgbuf));

    buf.mtype = 1 ;

	buf.length=length;
	
    memcpy((void*)buf.mtext,(void*)pData,length) ;

    sendlength =sizeof(struct Jmsgbuf)-sizeof(long);

    flag = msgsnd( msqid, &buf, sendlength , IPC_NOWAIT ) ;

    if ( flag < 0 )

    {

        perror("send message error") ;

        return -1 ;

    }

    return 0 ;

}
