#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc , char** argv)
{
    /*
	char* listFile = argv[1];
	short buff[480];
	char wav[1024];
	FILE* fvoice = NULL;
	FILE* flist = NULL;
	int nFrmLen = 220;
	int n,silF = 500,silB = 100;
	int ret;
	AsrVAD* pEngine = new AsrVAD;
	ret = pEngine->init( silF,silB );
	if( ret!=0 ){
		printf( "cannot launch VAD\n" );
		return -1;
	}

		
	flist = fopen( listFile,"rt" );
	if( flist==NULL ){
		fprintf( stderr,"cannot open file %s\n",listFile );
		return -1;
	}

	while( fgets( wav,1024,flist ) ){
		FILE* fvoice = NULL;
		char* pos = strtok( wav," \t\r\n" );
		if( pos==NULL )
			continue;
		fvoice = fopen(pos,"rb");
		if( fvoice==NULL ){
			fprintf( stderr,"cannot open file %s\n",wav );
			continue;
		}
		fseek(fvoice, 44, SEEK_SET);
		//int flush = 0;
		pEngine->reset();
		printf( "process %s ... ",wav );
		while( (n = fread(buff, sizeof(short), nFrmLen, fvoice)) != 0 ){
			ret = pEngine->process(buff, n);
			if(ret == 1) {
//				break;
			}else if( ret==2 ){
				printf("max sil %d reached\n",silF);
				break;
			}
		}
		fclose(fvoice);
		printf( "done!\t" );
//		if( ret == NONSPEECH )
		printf("%f %f\n", pEngine->start * 0.01, pEngine->stop * 0.01);


	}
	if( pEngine )
		delete pEngine;
	fclose( flist );
	return 0;
     */
    return 1;
}
