#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "asrclient.h"
int main(int argc,char* argv[])
{
	AsrServiceInterface* asrService = NULL;
	char *buffer;
	FILE* pcmFile = NULL;
	if ( argc != 3 ) {
		printf("usage:sample pcm_file 0/1\n");
		return 1;
	}
	struct timeval tv1,tv2;
	gettimeofday(&tv1,NULL);	
	asrService = (AsrServiceInterface*)asrCreateAsrService("10.34.7.146",8888,8,(char)atoi(argv[2]));
	if ( asrService == NULL ) {
		printf("start asr session error.\n");
		return 2;
	}
	pcmFile = fopen(argv[1],"rb");
	if ( pcmFile == NULL ) {
		printf("error open pcm file for reading.\n");
		return 3;
	} else {
		char buffer[9600];	
		int rc=1;
		while(rc>0 ){
			rc=fread(buffer,1,9600,pcmFile);
			
			if (rc>0) asrService->recognizer(buffer,rc);
			else break;
		}
		gettimeofday(&tv2,NULL);
		printf("time cost:%d ms\n",(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000);
		tv1 = tv2;
		printf("result is:%s\n",asrService->getResult());
	}
	asrDestroyAsrService(asrService);
	gettimeofday(&tv2,NULL);
	printf("time cost:%d ms\n",(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000);
	return 0;
}
