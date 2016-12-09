#ifndef _OPUS_WRAPPER_H_
#define _OPUS_WRAPPER_H_

#include "opus.h"
#include "Compresser.h"
#include "cache.h"
//namespace uni {

#ifdef __cplusplus
extern "C" {
#endif


class Opus:public Compresser{
	public:
        static const int WB_MODE = 1;
        static const int NB_MODE = 2;
        static const int SAMPLE_RATE_8K = 8000;
        static const int SAMPLE_RATE_12K = 12000;
        static const int SAMPLE_RATE_16K = 16000;
        static const int SAMPLE_RATE_24K = 24000;
        static const int SAMPLE_RATE_48K = 48000;

	public:
		Opus(int wb_mode=16000, bool encode=true);
		~Opus();

		int encode(char* src, int srcLen, char **dst, unsigned int* dstLen);
		int encodeFrame(void);
		
        int decode(char* src, int srcLen, char** dst, unsigned int* dstLen);
        int decodeFrame(void);

		/* added by lqy for cache reset avoid the decode & encode error */
		void cacheReset();

public:
    enum {
        SUCCESS = 0,
    };
    enum errorCode
    {
        INPUT_PARAMS_INVALID = -99,
        INTERNAL_CALL_INVALID,
        SRC_DATA_ERROR,
        OPUS_ENCODE_ERROR,
        OPUS_DECODE_ERROR,
        JNI_INPUT_NULL_INVALID,
        JNI_INPUT_HANDLE_INVALID,
        JNI_INPUT_SRC_LENGTH_INVALID,
        JNI_INPUT_MODE_INVALID,
    };
	private:
        bool headRecved(void);
        int decHeadValue(int *errNo);
        char *getFrameBody(unsigned int *len);
        void releaseFrameBody(char *body);
        bool frameCompletely(int *errNo);
        int needFrameLen(void);
        int feedFrameData(char *input, unsigned int len, int *errNo, bool *startProcess);

	private:
        int sample_rate;
        int frame_size;
        int decHeadSize;

        long handle;

        bool encodeFlag;

        CLCache *srcCache;
        CLCache *dstCache;

};

#ifdef __cplusplus
}
#endif

//}

#endif

