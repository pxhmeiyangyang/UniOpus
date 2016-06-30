#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Opuswrapper.h"

Opus::~Opus(){
#ifdef _ENCODE_ONLY_
	opus_encoder_destroy(enc);
#else
	opus_decoder_destroy(dec);
#endif
}

Opus::Opus(int in_mode,int out_mode){
	set_mode(in_mode,out_mode);
}
/*
 *	1. in _ENCODE_ONLY_ mode,in_mode specify input stream mode[WB|NB],out_mode specify encoded
 *	   stream mode[WB|NB]
 *	2. in _DECODE_ONLY_ mode,only in_mode is used to specify the input stream mod[WB|NB]
 *	3. only [WB->WB,WB->NB,NB->NB] are legal combinations
 */
int Opus::set_mode(int in_mode,int out_mode) {
	
	int err = 0;
	opus_int32 skip = 0;

	buf_len = 0;
	if( in_mode==NB_MODE && out_mode==WB_MODE ){
		fprintf(stderr,"force out_mode to NB_MODE when in_mode is NB_MODE\n");
		out_mode = NB_MODE;
	}

	current_in_mode = in_mode;
	current_out_mode = out_mode;
	
	memset(buffer, 0x0, WB_FRAME_SIZE*2);
	
	frame_size = (in_mode==WB_MODE)?WB_FRAME_SIZE:NB_FRAME_SIZE;
	int sample_rate = (in_mode==WB_MODE)?16000:8000;
	int bitrate = (out_mode==WB_MODE)?16000:8000;
	
#ifdef _ENCODE_ONLY_
	enc = opus_encoder_create(sample_rate, 1, OPUS_APPLICATION_VOIP, &err);
	if( err != OPUS_OK ){
		fprintf(stderr, "cannnot create opus encoder: %s\n", opus_strerror(err));
		enc = NULL;
		return -1;
	}
	
	if(in_mode==WB_MODE)
    {
		if(out_mode==WB_MODE)
			opus_encoder_ctl(enc, OPUS_SET_BANDWIDTH(OPUS_BANDWIDTH_WIDEBAND));
		else
			opus_encoder_ctl(enc, OPUS_SET_BANDWIDTH(OPUS_BANDWIDTH_NARROWBAND));
	}
    else
    {
		opus_encoder_ctl(enc, OPUS_SET_BANDWIDTH(OPUS_AUTO));
	}
	opus_encoder_ctl(enc, OPUS_SET_BITRATE(bitrate));
	opus_encoder_ctl(enc, OPUS_SET_VBR(1));
	opus_encoder_ctl(enc, OPUS_SET_COMPLEXITY(10));
	opus_encoder_ctl(enc, OPUS_SET_INBAND_FEC(0));
	opus_encoder_ctl(enc, OPUS_SET_FORCE_CHANNELS(OPUS_AUTO));
	opus_encoder_ctl(enc, OPUS_SET_DTX(0));
	opus_encoder_ctl(enc, OPUS_SET_PACKET_LOSS_PERC(0));
	opus_encoder_ctl(enc, OPUS_GET_LOOKAHEAD(&skip));
	opus_encoder_ctl(enc, OPUS_SET_LSB_DEPTH(16));
#else
	dec = opus_decoder_create(sample_rate, 1, &err );
	if( err != OPUS_OK ){
		fprintf(stderr, "cannnot create opus decoder: %s\n", opus_strerror(err));
		dec = NULL;
		return -1;
	}
#endif	
	return 0;
}

int Opus::change_mode(int in_mode,int out_mode){
	if( in_mode==NB_MODE && out_mode==WB_MODE ){
		fprintf(stderr,"force out_mode to NB_MODE when in_mode is NB_MODE\n");
		out_mode = NB_MODE;
	}
	if(in_mode == current_in_mode && out_mode == current_out_mode)
		return 0;
	
#ifdef _ENCODE_ONLY_
	opus_encoder_destroy(enc);
#else
	opus_decoder_destroy(dec);
#endif	
	
	set_mode(in_mode,out_mode);
	
	return 0;
}

void Opus::release()
{
    opus_encoder_destroy(enc);
    enc = nullptr;
}

void Opus::reset(){
#ifdef _ENCODE_ONLY_
	opus_encoder_ctl(enc, OPUS_RESET_STATE);
    
    opus_int32 skip = 0;
    opus_encoder_ctl(enc, OPUS_SET_BITRATE_REQUEST, 16000);
    opus_encoder_ctl(enc, OPUS_SET_VBR_REQUEST, 1);
    opus_encoder_ctl(enc, OPUS_SET_COMPLEXITY_REQUEST, 10);
    opus_encoder_ctl(enc, OPUS_SET_INBAND_FEC_REQUEST, 0);
    opus_encoder_ctl(enc, OPUS_SET_FORCE_CHANNELS_REQUEST, OPUS_AUTO);
    opus_encoder_ctl(enc, OPUS_SET_DTX_REQUEST, 0);
    opus_encoder_ctl(enc, OPUS_SET_PACKET_LOSS_PERC_REQUEST, 0);
    opus_encoder_ctl(enc, OPUS_GET_LOOKAHEAD_REQUEST, &skip);
    opus_encoder_ctl(enc, OPUS_SET_LSB_DEPTH_REQUEST, 16);
    
#else
	opus_decoder_ctl(dec, OPUS_RESET_STATE);
#endif
	buf_len = 0;
}

#ifdef _ENCODE_ONLY_
int Opus::encode(char* in,int len,unsigned char* opus,int* opus_len)
{
	int frame_bytes = (frame_size <<1);
	int i,nbytes,push_len = frame_bytes - buf_len;
	char* pos = in;
	unsigned char tmp[640];
	unsigned char* des = opus;
	unsigned char* src = tmp;

	*opus_len = 0;
	if( push_len>len ){
		memcpy(buffer+buf_len,pos,len);
		buf_len += len;
		return buf_len;
	}

	memcpy(buffer+buf_len,pos,push_len);
	pos += push_len;
	len -= push_len;
	buf_len = 0;
	
	//process buffer
	nbytes = encode_frame( buffer,tmp );
	if( nbytes==-1 ){
		fprintf( stderr,"error encoding\n" );
		return -1;
	}
	for( i=nbytes;i--; ){
		*des++ = *src++;
	}
	*opus_len += nbytes;

	while( len > frame_bytes ){
		nbytes = encode_frame( pos, tmp);
		if( nbytes==-1 ){
			fprintf( stderr,"error encoding\n" );
			return -1;
		}
		pos += frame_bytes;
		len -= frame_bytes;
		src = tmp;
		for( i=nbytes;i--; ){
			*des++ = *src++;
		}
		*opus_len += nbytes;
	}

	//copy leftover to buffer for next round
	if( len>0 ){
		memcpy(buffer,pos,len);
		buf_len = len;
	}
	return buf_len;
}

int Opus::encode_frame( char* in,unsigned char* cbits ){
    
	short nbytes;
	short* frame = (short* )in;
	nbytes = opus_encode(enc,frame,frame_size,cbits+sizeof(short),640-sizeof(short));
	//swap frame-head
	if( nbytes > frame_size * 2 || nbytes < 0)
		return -1;
	cbits[0] = nbytes & 0xFF;
	cbits[1] = (nbytes >>8)  & 0xFF;
	return (int)(nbytes+sizeof(short));
}
#else
int Opus::decode_frame( char* in,int* len,short* frame,int* frame_len ){
//	int opus_len = (int)(((unsigned short)in[0]<<8) | (unsigned short)in[1]);
	int opus_len = (int)*(short*)in;
	in += 2;
	if( opus_len > frame_size*2 || opus_len < 0 ){
		fprintf( stderr,"Invalid payload length %d\n",opus_len );
		return -1;
	}
	*frame_len = opus_decode(dec,(unsigned char* )in,opus_len,frame,frame_size,0);
	*len = opus_len+sizeof(short);
	return 0;
}


int Opus::decode(char* in,int len,short** speech,int* speech_len){
	int capacity = *speech_len;
	int dec_size;
	short* out = *speech;
	int i,output_samples = 0;
	while( true ){
		int frame_len;
		int rc = decode_frame( in,&dec_size,(short* )buffer,&frame_len );	// todo: buf overflow, if encoder frame is large than decoder pre-def.
		if( rc<0 ){
			fprintf( stderr,"decode one frame wrong\n" );
			break;
		}
		//need more room for decoded data?
		if( output_samples + frame_len > capacity ){
			int offset = out - *speech;
			capacity = output_samples + frame_len;
			capacity *= 2;
			*speech = (short* )realloc( *speech,capacity*sizeof(short) );
			if( *speech==NULL ){
				fprintf(stderr,"cannot realloc %d bytes for more room\n",capacity);
				break;
			}
			out = *speech + offset;
		}
		memcpy( out,buffer,frame_len*sizeof(short) );
		in += dec_size;
		len -= dec_size;
		out += frame_len;
		output_samples += frame_len;
		if( len <= 0 )
			break;
	}

	if( len!=0 ){
		return -1;
	}
	return output_samples;
}
#endif

