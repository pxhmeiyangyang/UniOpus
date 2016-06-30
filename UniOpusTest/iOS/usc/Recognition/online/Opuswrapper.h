#ifndef _OPUS_WRAPPER_H
#define _OPUS_WRAPPER_H

//using namespace std;

#ifndef _SPEEX_CODEC_
#include "opus.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum { WB_FRAME_SIZE=320, NB_FRAME_SIZE=160 };

class Opus{
	public:
		static const int WB_MODE = 1;
		static const int NB_MODE = 2;
	public:
		Opus( int in_mode=WB_MODE,int out_mode=WB_MODE );
		~Opus();
		
		int set_mode(int in_mode,int out_mode);
		int change_mode(int in_mode,int out_mode);
        
        void release();
		
#ifdef _ENCODE_ONLY_
		int encode(char* in,int len,unsigned char* opus,int* opus_len);
		//int encode(char* in,int len,vector<unsigned char>& opus);
		int encode_frame(char* in,unsigned char* cbits);
#else
		int decode(char* in,int len,short** speech,int* speech_len);
		int decode_frame(char* in,int* len,short* frame,int* frame_len);
#endif
		void reset();
	private:
		//int sample_rate;
		int frame_size;
		
		int buf_len;
		char buffer[WB_FRAME_SIZE*2];
		int current_in_mode; 
		int current_out_mode; 
#ifdef _ENCODE_ONLY_
		OpusEncoder* enc;
//#else
		OpusDecoder* dec;
		short dec_head_size;
#endif
		
};

#ifdef __cplusplus
}
#endif

#endif

