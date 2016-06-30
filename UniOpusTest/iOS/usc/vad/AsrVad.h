#ifndef __ASR_VAD_H__
#define __ASR_VAD_H__

// caution:
// the first 15ms data should throw away, because some machine always be zeros or very low energy data.

// VAD_STATUS
enum {
	SILENCE = 0, // silence status
	END_VOICE,   // voice end
	ON_VOICE,    // voice persist
	ST_VOICE,    // voice start
	DO_NOTHING   // don't konw what
};

// VAD_PARA_TYPES
enum {
	MINBACKENG = 5,// min back energy, default 5e6 (float)
	MINBACKENGH,   // min back energy higher TH, just larger than this, may voice occur, 5e8(for very noise, if voice is good, set it low, like 5e6) (float)
	PITCHTH,       // pitch threshold, 0.7 (float)
	PITCHSTNUMTH,  // pitch persist length for start usage, 8 (int)
	PITCHENDNUMTH, // pitch drop length for end usage, 50 (int)
	LOWHIGHTH,     // high freq energy vs low freq energy, 0.8 (float)
	MINSIGLEN,     // min signal length for speech, 10 (int)
	MAXSILLEN,     // max silence length, 30 (int)
	SINGLEMAX,     // max single point max in spectral, 50 (float)
	NOISE2YTH,     // gloable noise to signal value threshold, 0.5 (float)
	NOISE2YTHVOWEL,// gloable noise to signal value threshold for vowel part, 0.1 (float)
	VOICEPROBTH,   // voice freq domain prob Th, 0.7 (float)
	USEPEAK,       // use pitch or peak, 0 for pitch, other for peak, 0 (int)
	NOISE2YST,     // noise to y ratio, start point in freq domain, 10 (int)
	RESET_FEND,    // reset type, use this when a file is end, or a voice stream is end.
	RESET_VEND,    // reset type, use this when detect a voice end point.
	PITCHLASTTH    // the pitch last number for speech re-apear, which for transition section or tail, 3 (int)
};

class AsrVAD
{
public:
	/************************************************************************\
	 FUNCTION:  init
	 DESCRIPTION:
	   Do initialization.
	 PARAMETERS: 
	   kHZ:         The speech samplerate/1000.[In]
	 RETURN:
	  0     : Success.
	  Others: Errors.
	 NOTE: just surport 16k now!
	\************************************************************************/
	int init( int kHZ = 16 );
    
	/************************************************************************\
	 FUNCTION:  process
	 DESCRIPTION:
	   Do voice detection.
	 PARAMETERS: 
	   pcm: Voice data.[In]
	   len: Data length.[In]
	   energy: data energy.[Out]
	 RETURN:
	  >=0 : status of vad.
	  < 0 : errors.
	 NOTE: 1. len must be 10ms(160 for 16kHz), if not this, errors, so if the data length
	          is not this, pls do a loop calling or waiting!
	       2. if the voice start status returned, the voice have persistened hundreds of ms,
		      so, the upper calling function must do cache for the voice, 0.5s maybe enough
			  for default setting.
	\************************************************************************/
	int process(const short* pcm, int len, int* energy);

	/************************************************************************\
	 FUNCTION:  getstlength
	 DESCRIPTION:
	   get the voice length last before Start voice detected.
	 PARAMETERS: 
	   No.
	 RETURN:
	  the frame number.
	  < 0 : errors.
	 NOTE: the buffer could be setted large, and use this value as the realy start point.
	       but, the length should be larger than it for thinking about error, such as, 
		   10 frame(100ms) lasted, use 200~300 ms to protect the boundry.
	\************************************************************************/
	int getstlength(){return nLen;}

	/************************************************************************\
	 FUNCTION:  setparas
	 DESCRIPTION:
	   Set parameters of the vad engien.
	 PARAMETERS: 
	   ntype : vad parameters' type, see VAD_PARA_TYPES.[In]
	   val   : parameter's values.[In] 
	 RETURN:
	  None.
	 NOTE: 1. MINBACKENG: minimum of the back energy, it is got by calculate
	          the frame energy of the noise, the default value is got by dongle's voice,
			  if the voice energy is low, the value should be tuned down.
		   2. MINBACKENGH: it is very like above parameter, above is low TH for noise, 
		      this is for high TH, it is for voice start.
		   3. PITCHTH: pitch threshold, high for voice, low for noise,if set zero, 
		      not use pitch, if the voice's quality is not so good, especialy in low 
			  frequnce region, set it to zero, and if the quality is very good, set it
			  higher may cut many high energy noise.
		   4. PITCHSTNUMTH: how many frame contain vowle may consider voice, larger for
		      good quality voice.
		   5. PITCHENDNUMTH: how many frame not contain vowls may consider noise, larger
		      for bad quality voice.
		   6. LOWHIGHTH: high frequency energy divide low frequency energy, for judge vowel,
		      lower for stricter.
		   7. SINGLEMAX: max single point energy, very large means not used, lower for 
		      restricter for voice, it is useful for single period noise, but bad for
			  very noise data, because /u/ /n/ phone just contain 3~4 harmonic, if the 
			  noise is very high, some may lost.
		   8  NOISE2YTH: noise to signal energy ratio threshold for voice, lower for more 
		      restrict, when the noise is very high, tune it high.
		   9  NOISE2YTHVOWEL: like the NOISE2YTH, but it is for vowel speech, so, it should
		      lower, when the noise is very high, tune it high.
		   10 VOICEPROBTH: the speech prob from spectral energy, tune it down when the noise
		      energy is very high.
		   11 USEPEAK: if use freq domain for period detect, default use time domain. If the 
		      noise is very high, the time domain maybe lose, use this maybe better, this method
			  may instead the time domain in future.
		   12 PITCHLASTTH: the pitch last frame number threshold, for distinct tail and middle,
		      if 1 is set, means not use it.
	\************************************************************************/
	void setparas(int ntype, const void* val);

	/************************************************************************\
	 FUNCTION:  reset
	 DESCRIPTION:
	   Do reset.
	 PARAMETERS: 
	   None.
	 RETURN:
	  None.
	 NOTE: when get the end status of the voice, call this for reset.
	\************************************************************************/
	void reset(int ntype = RESET_FEND);

	// constructor & destructor
	AsrVAD() {
		psDataBuf = NULL;
		pvhd = NULL;
	}
	~AsrVAD();

private:
	int  GetEng(const short* psDataIn, int len, int* energy, int* engstatus);
	void idhdbypd(const short* psDataIn, int len);
	void idtlbypd(const short* psDataIn, int len);
	void idhdbypk();
	void idtlbypk();
	int  vadlogic(int nStatus, const short* psDataIn, int len);
	void* pvhd;
	int nStatus;
	int nFrameLen;
	int nFrameSft;
	int nLen;
	int nInitYet;
	short* psDataBuf;
	int nGetStartYet;
	int m_nSilCnt;
	int m_nSigCnt;   // for zero the m_nSilCnt
	int m_nSigCntTh; 

	int nProbInc;
	short psForProbData[1024];
	float m_prob, m_probaux;
	int   nContinuePitch;
	int   nLastPitchSign;
	float m_preprob[8]; // for 3 value max filter
	int   m_preEng[8]; // for 3 value max filter,for hold on
	// parameters
	float m_minBackEng;
	float m_pitchTh;
	int   m_nPitchNumTh;
	float m_flowhighcmpTh;
	int   m_nPitchEndNumTh;
	int   nMinSigLen;
	int   nMaxSilLen;
	float m_fnoise2yTh;
	float m_fVowelnoise2yTh;
	int   m_nusepeak;
	int   m_nstart;
	int   m_nstop;
	float m_feng;
	float m_feng2Th;
	int   m_nProbLastNum;
	int   m_nProbLastTh;
};

#endif
