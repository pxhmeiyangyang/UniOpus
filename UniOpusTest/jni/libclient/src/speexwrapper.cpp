#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "speexwrapper.h"

Speex::~Speex() {
	speex_bits_destroy(&bits);
	speex_encoder_destroy(state);
}
Speex::Speex()
{
	state = speex_encoder_init(&speex_wb_mode);
	speex_bits_init(&bits);	
	left[0]=0;
	left_len=0;
}

void Speex::setQuality(int quality)
{
	//fprintf(stderr,"quality = %d\n",quality);
	if ( quality>10 ) quality=10;
//	speex_encoder_ctl(state, SPEEX_SET_QUALITY, &quality);
	int tmp=1;
	speex_encoder_ctl(state,SPEEX_SET_VBR,&tmp);
//	tmp = 2;
//	speex_encoder_ctl(state,SPEEX_SET_COMPLEXITY,&tmp);
	float qual = quality;
	speex_encoder_ctl(state, SPEEX_SET_VBR_QUALITY, &qual);
}

int Speex::encode_frame(short* pcm, int pcm_len,char* cbits)
{
	short input[FRAME_SIZE];
	int   nbBytes = 0;
	assert(pcm_len <= FRAME_SIZE);

	for(int i=0;i<pcm_len;i++){
		input[i]=pcm[i];
	}
	for(int i=pcm_len;i<FRAME_SIZE;i++)
	{
		input[i]=0.0;
	}

	speex_bits_reset(&bits);
	speex_encode_int(state,input,&bits);
	nbBytes = speex_bits_write(&bits, cbits+sizeof(int), MAX_WB_BYTES);
	memcpy(cbits,(char*)&nbBytes,sizeof(int));
	return nbBytes+sizeof(int);
}

int Speex::encode(char* in,int len,char* spx,int* spx_len,bool force_zip)
{
	short* pcm = (short*)in;
	int pcm_len = (len + left_len)/2;
	int idx=0;

	*spx_len = 0;

	int loop = pcm_len / FRAME_SIZE;
	int new_left_len = (pcm_len % FRAME_SIZE)*2;
	char tmp_spx[MAX_WB_BYTES];
	short frame[FRAME_SIZE];

	for(idx=0;idx<loop;idx++) {
		if ( idx==0 ) {
			short* left_pcm =(short*)left;
			int i=0;
			for(i=0;i<left_len/2;i++) {
				frame[i]=left_pcm[i];
			}
			for(int j=0;j<FRAME_SIZE-left_len/2;j++){
				frame[i+j]=pcm[j];
			}
			int len = encode_frame(frame,FRAME_SIZE,tmp_spx);
			for(i=0;i<len;i++) {
				spx[i+*spx_len] = tmp_spx[i];
			}
			*spx_len += len;
		} else {
			short* one_frame = (short*)&(pcm[idx*FRAME_SIZE-left_len/2]);
			int len = encode_frame(one_frame,FRAME_SIZE,tmp_spx);
			for(int i=0;i<len;i++) spx[i+*spx_len]=tmp_spx[i];
			*spx_len += len;
		}
	}

	left_len = new_left_len;
	//fprintf(stderr,"left_len = %d\n",left_len);
	if ( left_len >0) {
		if ( force_zip == false ) memcpy(left,in+len-left_len,left_len);
		else {
			short* one_frame = (short*)&(pcm[len-left_len/2]);			
			int len = encode_frame(one_frame,FRAME_SIZE,tmp_spx);
			for(int i=0;i<len;i++) spx[i+*spx_len] = tmp_spx[i];
			*spx_len += len;
		}
	}
	return loop;
}

