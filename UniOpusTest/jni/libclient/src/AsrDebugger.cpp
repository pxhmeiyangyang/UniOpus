#include"AsrDebugger.h"

#include<stdio.h>
#include<stdlib.h>

#ifdef LINUX
#include<execinfo.h>
#include<unistd.h>




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
	//would produce similar output to the following: 

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
#endif
