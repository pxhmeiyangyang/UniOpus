#include "AsrVad.h"
#include "string.h"
#include "math.h"
#ifdef COMMON_VAD

int AsrVAD::init()
{
	m_pVAD = initVAD_USC();
	if( m_pVAD==NULL )
		return -1;
	mFrmNum = 0;
	mLeftLen = 0;
	start = -1;
	stop = 0;
	return 0;
}

void AsrVAD::setVadSilTime(int frontSil, int backSil)
{
    if (m_pVAD) {
        setParam_USC(m_pVAD, frontSil, backSil);
    }
}

void AsrVAD::reset(){
	resetVAD_USC(m_pVAD);
	mLeftLen = 0;
	start = -1;
	stop = 0;
	mFrmNum = 0;
}
/*
 *	@return continious silence frames
 */
int AsrVAD::process(short* pcm, unsigned int len,int* energy)
{
	short* curPos = pcm;
	static int frmLen = SM_FRE * FRM_MS;
	int samIn = len;
	int samPush =  frmLen - mLeftLen;
	int ret;
	int type = 0;
	if( samPush> samIn ){
		memcpy( mLeftPcm+mLeftLen,curPos,sizeof(short)*samIn );
		mLeftLen += len;
		return type;
	}

	memcpy( mLeftPcm+mLeftLen,curPos,sizeof(short)*samPush );
	mLeftLen = 0;
	curPos += samPush;
	samIn -= samPush;

	while( true ){
		mFrmNum += 3;
		ret = push2VAD_USC( m_pVAD,mLeftPcm,frmLen,energy );
		if( start==-1 && ret == SPEECH ){
			type = 3;
			start = m_pVAD->front;
		}else if( ret==NONSPEECH ){
			type = 1;
			stop = m_pVAD->end;
			start = m_pVAD->front;
			break;
		}else if( ret==MAX_SIL ){
			type = 2;
			break;
		}
		if( samIn<frmLen )
			break;
		memcpy( mLeftPcm,curPos,sizeof(short)*frmLen );
		curPos += frmLen;
		samIn -= frmLen;
	}

	if( samIn>0 && type == 0 ){
		memcpy( mLeftPcm,curPos,sizeof(short)*samIn );
		mLeftLen = samIn;
	}


	return type;

}

#elif defined(SIMPLE_VAD)

int AsrVAD::init()
{
	mSelf = simple_vad_create();
	if (mSelf == NULL)
        return -1;
	simple_vad_init(mSelf);
	return 0;
}

/*
 *	@return continious silence frames
 */
int AsrVAD::process(short* pcm, unsigned int len, int &speechStartPos)
{
	int nowType;
	int FrmNum = (mLeftLen+len) / mSftLen;
	short* pp = pcm;
	
	if( mFindSpeechStart )
		speechStartPos = 0;
	else
		speechStartPos = -1;

	for(int i=0;i<FrmNum;i++)
	{
		if( 0 == i && mLeftLen > 0)
		{
			memcpy(mLeftPcm+mLeftLen,pcm,(mSftLen-mLeftLen)*sizeof(short));
			nowType = simple_vad_process(mLeftPcm, mSftLen, mSelf);
			pp += mSftLen-mLeftLen;
		}
		else
		{
			nowType = simple_vad_process(pp, mSftLen, mSelf);
			pp += mSftLen;
		}

		if( !mFindSpeechStart && nowType==1 && (mPrevType==0))
		{
			speechStartPos = pp - pcm - mSftLen > 0 ? pp - pcm - mSftLen : 0;
			mFindSpeechStart = 1;
		}

		if(nowType==1)
		{
			mTimeout = 0;
		}
		else if( nowType==0 )
		{
			mTimeout ++ ;
		}
		mPrevType = nowType;
	}

	if (FrmNum > 0)
	{
		mLeftLen = (mLeftLen+len) % mSftLen;
		if( mLeftLen > 0)
		{
			memcpy(mLeftPcm,pp,mLeftLen*sizeof(short));
		}
	}
	else
	{
		memcpy(mLeftPcm+mLeftLen,pp,len*sizeof(short));
		mLeftLen += len;
	}
	
	return mTimeout;
}

#endif
