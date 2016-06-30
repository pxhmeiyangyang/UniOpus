/*--------------------------------------------------------------------------------
	Author:	Mu Su
	Email:	sumu2003@gmail.com
	This Pitch tracking algorithm follows the paper belows
	"Accurate short-term analysis of the fundamental frequency and the harmonics-to
	-noise ratio of a sampled sound" by Paul Boersma


	The basic idea is the normalized auto-correlation of a short speech segment divided
	by the auto-correlation of hanning window, followed by pitch candidate extraction
	and DP tracking
  --------------------------------------------------------------------------------*/


#ifndef _PITCH_H_
#define _PITCH_H_
#ifdef __cplusplus
extern "C"
{
#endif

#include "typedef.h"
#include "stdio.h"
#define M_PI	3.14159265358979323846
#define M_SQRT2	1.41421356237309504880
#define MAX_PITCH_CAND	10
#define MAX_BUFLEN_SEC	60


#define MAX_PITCH_P            0.015625
#define MIN_PITCH_P            0.002222
//#define MAX_PITCH_P		0.002222
//#define MIN_PITCH_P		0.001176470588
#define PITCH_DIM	4


#define	MAX_WIN_SIZE	512	//50ms 512

#define PITCH_THRESHOLD	410//Q12,0.05f

#define CC_PITCH_SHIFT	20
#define CC_PITCH_QN_FFT	(Int64)(1.0 *( 1<<CC_PITCH_SHIFT )/ M_SQRT2)
#define CC_PITCH_SHIFT_U	15
#define CC_PITCH_QN_FFT_U	(Int64)(1.0 *( 1<<CC_PITCH_SHIFT_U )/ M_SQRT2)
#define CC_PITCH_LOG_Q	(Int32)((1<<13)*0.69314718055994530941723212145818)
#define CC_HZ_QN	(1<<6)
//#define PITCH_DEBUG
#define SAMP_KHZ	16
typedef struct TagBWIIR_S{
	double	a[9];
	double	b[9];
	double	xp[8];
	double	yp[8];
}BWIIR_S;

typedef struct TagFRMDATA{
	UInt32	iEn;	/** normalized energy */
	Int32	nCand;	/** candidate numbers */

	Int32	nHz[MAX_PITCH_CAND];	/** F0 candidate for every frame */ 
	Int32	nRm[MAX_PITCH_CAND];	/** correlation coefficient candidate for every frame */

	/************************************************************************/
	/* Dynamic Tracking Result                                              */
	/************************************************************************/
	Int32	prevCand[MAX_PITCH_CAND];
	Int32	score[MAX_PITCH_CAND];
	Int32	time;

	/* circular buffer back-pointer */
	Int32 prevIdx;
}FRMDATA;

typedef struct TagPITCHDATA{
	Int32	nKHz;	/** sampling frequency in KHz */
	Int32	nWinMs;	/** process window length in sec. */
	Int32	nSftMs;	/** process window shift in sec. */

	FRMDATA*	pFrmData;

	Int32	nFrmBufSize;

	Int32	nFrmSft;	/** frame shift samples */
	Int32	nFrmlen;	/** frame length samples */

	Int32	nMinLag;	/** minimun lag for ACF search */
	Int32	nMaxLag;	/** maximun lag for ACF search */

//		float	fHanWin[MAX_WIN_SIZE];
//		float	fRmWin[MAX_WIN_SIZE];
	Int32	nRmWin[MAX_WIN_SIZE];

	BWIIR_S* pIIR;
	
#ifdef PITCH_DEBUG
	int	nMaxPitchCand;
#endif
	Int32* frameBuffer;
	Int32* traceBuffer;
	Int32 firsTime;
	Int32 frameCounter;
}PITCH;


#define NB_FRAME_THRESHOLD_LTF 10
#define LAMBDA_LTF 0.97
#define FEATURE_THRESHOLD_UPD_LTF 1000
#define FEATURE_FLOOR 90
#define MIN_FRAME 10
#define LAMBDA_LTF_HIGHER_E 0.99
#define CN_FRAME_THRESHOLD 5
#define MIN_SPEECH_FRAME_HANGOVER 4
#define EN_THRESHOLD_UPD_LTE 20

#define THRESHOLD_VAD_LOW 3000 //4500
#define THRESHOLD_VAD_HGH 4000 //4500

#define EN_THRESHOLD_VAD 40
#define EN_THRESHOLD_LOW 10

#define HANGOVER 100

#define FEATURE_NUMBER 6000

#define SPEECH 1
#define NONSPEECH -1
#define HOLDING -2
#define WAITING 0
#define MAX_SIL	2

typedef struct feature_S{
	Int32 Vo;
	Int32 En;
}feature;

typedef struct VAD_S {
	int   nbSpeechFrame;
	int   nbSilFrame;
	int   meanFt;
	int   flagVAD;
	int   hangOver;
	int   frameIndex;
	int	  frameSkip;
	float lambdaLTF;
	
	PITCH * engine;
	feature * features;
	int   number;
	int   front;
	int   end;
	
	float meanEn;
	float speechEn;
	float maxEn;
	int maxFSil;
	int maxBSil;
}VAD;


PITCH* initPITCH_USC( Int32 nFrmMs,Int32 nSftMs );

void delPITCH_USC( void* handle );

int push2PITCH_USC( void* handle,Int16* pWaveBuffer,Int32 nSamples,Int32 Flush , feature* features, int* pnumber,int* ave);
	
void resetPITCH_USC( void* handle );


VAD * initVAD_USC();

void setParam_USC(VAD* handler,int maxFSil,int maxBSil);

int push2VAD_USC(VAD* hanlder, short* pWaveBuffer, int nSmaples,int* ave);

void delVAD_USC(VAD* handler);

void resetVAD_USC( VAD* handler );
    
    
    
#ifdef __cplusplus
}
#endif

#endif /* _PITCH_H_ */
