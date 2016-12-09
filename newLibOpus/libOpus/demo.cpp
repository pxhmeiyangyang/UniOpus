#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uni_opus_decoder.h"
//#include "Opuswrapper.h"

#define DEFAULT_LEN 640

int decode(const char* input, const char* output);
int encode(const char* input, const char* output);

int main()
{
    //encode("../liping.pcm", "../lipingencode.opus");
    decode("../liping.opus", "../lipingdecode.pcm");
    getchar();
    return 0;
}


int encode(const char* input, const char* output)
{
    FILE* fresult = fopen(output, "w");
    if (fresult == NULL) {
        fprintf(stderr, "Cann't Create File %s\n", output);
        return -2;
    }
    FILE* fpcm = fopen(input, "rb");
    if (fpcm == NULL) {
        fprintf(stderr, "Cann't Open File %s\n", input);
        return -3;
    }
    //uni::Opus* opus = new uni::Opus(uni::Opus::WB_MODE, uni::Opus::WB_MODE);
    Opus* opus = new Opus(Opus::WB_MODE, true);
    char buff[DEFAULT_LEN];
    //unsigned char* out = new unsigned char[DEFAULT_LEN];
    //int retLen;
    char* out = NULL;
    unsigned int retLen;
    int allret = 0;
    int bufalllen = 0;
    while (true){
        //memset(out, 0, DEFAULT_LEN);
        int nRead = fread(buff, sizeof(char), sizeof(buff), fpcm);
        if (nRead <= 0) {
            break;
        }
        //for (int i = 0; i < nRead; i++)
        //{
        //    printf(" %d", buff[i]);
        //}
        int ret = opus->encode(buff, nRead, &out, &retLen);
        bufalllen += nRead;
        allret += retLen;
        printf("\nbufflen : %d, retlen : %d, ret : %d\n", nRead, retLen, ret);
        //for (int i = 0; i < retLen; i++)
        //{
        //    printf(" %d", out[i]);
        //}
        fwrite(out, sizeof(char), retLen, fresult);

    }
    printf("ALL len buf : %d, ret : %d\n", bufalllen, allret);
    //delete[] out;
    fclose(fresult);
    fclose(fpcm);
    delete opus;
    return 0;
}

int decode(const char* input, const char* output)
{
    FILE* fresult = fopen(output, "w");
    if (fresult == NULL) {
        fprintf(stderr, "Cann't Create File %s\n", output);
        return -2;
    }
    FILE* fpcm = fopen(input, "rb");
    if (fpcm == NULL) {
        fprintf(stderr, "Cann't Open File %s\n", input);
        return -3;
    }

    Opus* opus = new Opus(Opus::WB_MODE, false);

    char buff[1024];
    char* out = NULL;
    unsigned int retLen;
    while (true){

        int nRead = fread(buff, sizeof(char), sizeof(buff), fpcm);
        if (nRead <= 0) {
            break;
        }
        int ret = opus->decode(buff, nRead, &out, &retLen);
        printf("input : %d\toutput : %d\tret : %d\n", nRead, retLen, ret);
        fwrite(out, sizeof(char), retLen, fresult);

    }
    fclose(fresult);
    fclose(fpcm);
    delete opus;
    return 0;
}


