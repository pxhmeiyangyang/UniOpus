#include "stdio.h"
#include "string.h"
#include "uni_opus_decoder.h"

//namespace uni {

enum { WB_FRAME_SIZE=320, NB_FRAME_SIZE=160 };

Opus::~Opus()
{
    delete srcCache;
    delete dstCache;

    if (encodeFlag)
    {
        opus_encoder_destroy((OpusEncoder *)handle);
    }
    else
    {
	    opus_decoder_destroy((OpusDecoder *)handle);
    }
}

Opus::Opus(int wb_mode, bool opt)
{
	int err = 0;
    frame_size = (wb_mode == WB_MODE) ? WB_FRAME_SIZE : NB_FRAME_SIZE; //wb_mode / 50;
    sample_rate = (wb_mode == WB_MODE) ? 16000 : 8000; //wb_mode; 
    decHeadSize = 2;

    encodeFlag = opt;

    if (encodeFlag)
    {
        //compress
        opus_int32 skip = 0;
	    handle = (long)opus_encoder_create(sample_rate,1,OPUS_APPLICATION_VOIP,&err);

	    if( err != OPUS_OK )
        {
		    fprintf(stderr, "cannnot create opus encoder: %s\n", opus_strerror(err));
		    handle = 0;
		    return;
	    }

        opus_encoder_ctl((OpusEncoder *)handle, OPUS_SET_BITRATE(16000));
        //opus_encoder_ctl((OpusEncoder *)handle, OPUS_SET_BITRATE(14000));
	    opus_encoder_ctl((OpusEncoder *)handle, OPUS_SET_BANDWIDTH(OPUS_AUTO));
	    opus_encoder_ctl((OpusEncoder *)handle, OPUS_SET_VBR(1));
	    opus_encoder_ctl((OpusEncoder *)handle, OPUS_SET_COMPLEXITY(10));
	    opus_encoder_ctl((OpusEncoder *)handle, OPUS_SET_INBAND_FEC(0));
	    opus_encoder_ctl((OpusEncoder *)handle, OPUS_SET_FORCE_CHANNELS(OPUS_AUTO));
	    opus_encoder_ctl((OpusEncoder *)handle, OPUS_SET_DTX(0));
	    opus_encoder_ctl((OpusEncoder *)handle, OPUS_SET_PACKET_LOSS_PERC(0));
	    opus_encoder_ctl((OpusEncoder *)handle, OPUS_GET_LOOKAHEAD(&skip));
	    opus_encoder_ctl((OpusEncoder *)handle, OPUS_SET_LSB_DEPTH(16));
    }
    else
    {
        //extract
	    handle = (long)opus_decoder_create( sample_rate,1,&err );
	    if( err != OPUS_OK )
        {
		    fprintf(stderr, "cannnot create opus decoder: %s\n", opus_strerror(err));
		    handle = 0;
		    return;
	    }

    }

    srcCache = new CLCache;
    dstCache = new CLCache;
}

bool Opus::headRecved(void)
{
    if (encodeFlag)
    {
        return true;
    }
    else
    {
        return srcCache->length()>=decHeadSize?true:false;
    }
}

//
int Opus::decHeadValue(int *errNo)
{
    unsigned char ch0 = 0;
	unsigned char ch1 = 0;

    *errNo = Opus::SUCCESS;

    if (srcCache->length() < decHeadSize)
    {
        *errNo = Opus::INTERNAL_CALL_INVALID;
        return 0;
    }

	if (0 != srcCache->valueUnsigned(&ch0, 0))
    {
        *errNo = Opus::INTERNAL_CALL_INVALID;
        return 0;
    }

	if (0 != srcCache->valueUnsigned(&ch1, 1))
    {
        *errNo = Opus::INTERNAL_CALL_INVALID;
        return 0;
    }

    int encFrameLen = (int)(((unsigned short)ch1<<8) | (unsigned short)ch0);
   
	//printf("encFrameLen : %d, limit : %d. ch0 : %d. ch1 : %d\n", encFrameLen, frame_size*sizeof(short), ch0, ch1);

    if(encFrameLen > frame_size*sizeof(short) || encFrameLen < 0)
    {
		fprintf(stderr, "Invalid payload length %d, limit %d.\n", encFrameLen, frame_size*sizeof(short));
        *errNo = Opus::SRC_DATA_ERROR;
        return 0;
    }

    return encFrameLen;
}

char *Opus::getFrameBody(unsigned int *len)
{
    unsigned int totalLen = 0;
    char *total = srcCache->sub(&totalLen);
    char *body = NULL;
    
    /*
        for (int i=0;i<totalLen;++i)
        {
            printf("%d ",*(total+i));
        }
        */

    if (encodeFlag)
    {
        body = new char[totalLen];
        memcpy(body, total, totalLen);
        *len = totalLen;
    }
    else
    {
        body = new char[totalLen-2];
        memcpy(body, total+2, totalLen-2);
        *len = totalLen-2;
    }

    return body;
}

void Opus::releaseFrameBody(char *body)
{
    delete []body;
}


bool Opus::frameCompletely(int *errNo)
{
    if (encodeFlag)
    {
        return (frame_size*sizeof(short) == srcCache->length()?true:false);
    }
    else
    {
        int decHeadLen = 0;
        decHeadLen = decHeadValue(errNo);
        if (SUCCESS != *errNo)
        {
            return false;
        }

        if (decHeadLen+decHeadSize == srcCache->length())
        {
            return true;
        }

        return true;
    }
}

//0,’˝∫√£ª’˝ ˝£¨ £”‡£ª∏∫ ˝£¨–Ë“™
int Opus::feedFrameData(char *input, unsigned int len, int *errNo, bool *startProcess)
{
    if (NULL == input || len <= 0)
    {
        if (NULL != errNo)
        {
            *errNo = INPUT_PARAMS_INVALID;
        }

        return 0;
    }

    *startProcess = false;
    *errNo = SUCCESS;
    int rc = 0;

    if (encodeFlag)
    {
        const int needLen = frame_size*sizeof(short) - srcCache->length();
        const int feedLen = needLen<len?needLen:len;
        srcCache->add(input, feedLen);
        rc = len-needLen;

        if (rc >= 0)
        {
            *startProcess = true;
        }

        return (rc);
    }
    else
    {
        if (headRecved())
        {
            int headLen = decHeadValue(errNo);
            if (SUCCESS != *errNo)
            {
                return 0;
            }
            const int needLen = headLen + decHeadSize - srcCache->length();
            const int feedLen = needLen<len?needLen:len;
            srcCache->add(input, feedLen);
            rc = len-needLen;

            if (rc >= 0)
            {
                *startProcess = true;
            }

            return (rc);
        }
        else
        {
            const int needLen = decHeadSize - srcCache->length();
            const int feedLen = needLen<len?needLen:len;
            srcCache->add(input, feedLen); 
            rc = len-needLen;

            return (rc);
        }
    }
}

int Opus::encodeFrame(void)
{
	int i = 0;
	short nbytes = 0;
    int errNo = 0;
    
    if (frameCompletely(&errNo))
    {
        unsigned int frameLen = 0;
        char *frame = getFrameBody(&frameLen);

        unsigned char tmpEncBuf[642] = {0};
        
        nbytes = opus_encode((OpusEncoder *)handle, (opus_int16 *)frame, frameLen/sizeof(short), tmpEncBuf+decHeadSize, 640);
        //printf("nbytes %d\n",nbytes);
	    //swap frame-head
	    if(nbytes>frame_size*2 || nbytes < 0)
        {
		    return OPUS_ENCODE_ERROR;
        }
	    tmpEncBuf[0] = nbytes & 0xFF;
	    tmpEncBuf[1] = (nbytes >>8)  & 0xFF;

        dstCache->add((char *)tmpEncBuf, nbytes+decHeadSize);

        /*
        for (int i = 0;i<dstCache->length();++i)
        {
            char ch = 0;
            dstCache->value(&ch, i);
            printf("%d ", ch);
        }
        */

        releaseFrameBody(frame);
    }
    else
    {
        return INTERNAL_CALL_INVALID;
    }
    
	return SUCCESS;
}


int Opus::encode(char* src, int srcLen, char **dst, unsigned int* dstLen)
{
    int errNo = 0;
    bool startProcess = false;
    char *curPos = src;
    int lastDataLen = srcLen;

    while((lastDataLen = feedFrameData(curPos, lastDataLen, &errNo, &startProcess)) >= 0)
    {
        if (SUCCESS != errNo)
        {
            return errNo;
        }

        if (startProcess)
        {
            errNo = encodeFrame();
            if (SUCCESS != errNo)
            {
                return errNo;
            }
        }

        if (0 >= lastDataLen)
        {
            lastDataLen = 0;
            break;
        }
        curPos = src + srcLen - lastDataLen;
    }

    *dst = dstCache->sub(dstLen);
    
    return SUCCESS;
}

int Opus::decodeFrame(void)
{
	int i = 0;
	short nshorts = 0;
    int errNo = 0;


    if (frameCompletely(&errNo))
    {
        unsigned int frameLen = 0;
        char *frame = getFrameBody(&frameLen);

        /*
        printf("\n");
        for (int i=0;i<frameLen;++i)
        {
            printf("%d ",*(frame+i));
        }
        */

        short tmpDecBuf[320*4] = {0};

        nshorts = opus_decode((OpusDecoder *)handle, (unsigned char *)frame, frameLen, tmpDecBuf, frame_size, 0);
        //printf("nbytes %d\n",nshorts);
	    //swap frame-head
	    if(nshorts>frame_size || nshorts < 0)
        {
		    return OPUS_DECODE_ERROR;
        }

        dstCache->add((char *)tmpDecBuf, nshorts*sizeof(short));

        releaseFrameBody(frame);
    }
    else
    {
        return INTERNAL_CALL_INVALID;
    }

            
    return SUCCESS;
}

int Opus::decode(char* src, int srcLen, char** dst, unsigned int* dstLen)
{
    int errNo = 0;
    bool startProcess = false;
    char *curPos = src;
    int lastDataLen = srcLen;

    while((lastDataLen = feedFrameData(curPos, lastDataLen, &errNo, &startProcess)) >= 0)
    {
        if (SUCCESS != errNo)
        {
            return errNo;
        }

        if (startProcess)
        {
            errNo = decodeFrame();
            if (SUCCESS != errNo)
            {
                return errNo;
            }
        }

        if (0 >= lastDataLen)
        {
            lastDataLen = 0;
            break;
        }
        curPos = src + srcLen - lastDataLen;
    }

    *dst = dstCache->sub(dstLen);
    
    return SUCCESS;
}
void Opus::cacheReset()
{
	if (srcCache != NULL)
		srcCache->reset();
	if (dstCache != NULL)
		dstCache->reset();
}

//}
