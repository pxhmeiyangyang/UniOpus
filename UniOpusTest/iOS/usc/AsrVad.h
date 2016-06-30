#ifndef __ASR_VAD_H__
#define __ASR_VAD_H__

#ifdef COMMON_VAD
#include "decision.h"
#define USE_VAD
#elif defined(SIMPLE_VAD)
#include "simple_vad.h"
#define USE_VAD
#endif

#ifdef COMMON_VAD

#define ASR_VAD_TIMEOUT -7

class AsrVAD
{
	static const int SM_FRE = 16;
	static const int FRM_MS = 30;
	static const int SFT_MS = 10;
public:
	int start;
	int stop;

	AsrVAD()
	{
		m_pVAD = NULL;
		mLeftLen = 0;
		mFrmNum = 0;
		start = 0;
		stop = 0;
	}
	~AsrVAD()
	{
		if( m_pVAD != NULL)
			delVAD_USC(m_pVAD);

	}
    
	int init();
    void setVadSilTime(int frontSil, int backSil);
    
	int process(short* pcm, unsigned int len,int* energy);
	void reset();
    int getEng(const short* psDataIn, int len, int* energy);
private:
	VAD* m_pVAD;
	short mLeftPcm[SM_FRE * FRM_MS];
	int mLeftLen;
	int mFrmNum;
};

#elif defined(SIMPLE_VAD)

class AsrVAD
{
	static const int SM_FRE = 16;
	static const int FRM_MS = 25;
	static const int SFT_MS = 10;
public:
	AsrVAD():mFrmLen(SM_FRE * FRM_MS),mSftLen(SM_FRE * SFT_MS),
		mLeftLen(0),mPrevType(0),mFindSpeechStart(0),mTimeout(0)
	{

	}
	~AsrVAD()
	{
		if (mSelf != NULL)
		{
			simple_vad_delete(mSelf);
		}
	}
	int init();
	int process(short* pcm, unsigned int len, int &speechStartPos);
private:
	int mFrmLen;
	int mSftLen;
	short mLeftPcm[SM_FRE * SFT_MS];
	int mLeftLen;
	int mPrevType;
	int mFindSpeechStart;
	int mTimeout;
	SimpleVadParams_t* mSelf;
};

#endif

#endif
