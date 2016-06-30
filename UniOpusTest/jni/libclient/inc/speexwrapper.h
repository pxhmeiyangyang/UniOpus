#ifndef	_SPEEX_H
#define	_SPEEX_H
#include "speex/speex.h"

#ifdef	__cplusplus
extern "C" {
#endif

static const int FRAME_SIZE	= 320;
#define	MAX_WB_BYTES	(FRAME_SIZE*2)
class Speex
{
	public:
		Speex();
		~Speex();
		int encode(char* in,int in_len,char* spx,int* spx_len,bool force_zip=false); //cbits is user supplied buffer to hold speex encode result

		int encode_frame(short* in,int in_len,char* cbits);	//cbits is user supplied buffer to hold speex encode result
		void setQuality(int quality) ;
		void reset(){left_len = 0;}
	private:
		void  *state;
		SpeexBits bits;
		int quality;
		char left[FRAME_SIZE*2];
		int left_len;
};
#ifdef	__cplusplus
}
#endif
#endif
