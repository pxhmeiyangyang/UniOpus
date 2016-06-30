//
//  ASRVadWapper.h
//  usc
//
//  Created by hejinlai on 12-11-15.
//  Copyright (c) 2012年 yunzhisheng. All rights reserved.
//

#ifndef __usc__ASRVadWapper__
#define __usc__ASRVadWapper__

static int VADTimeout = 10000;

class AsrVAD;

class ASRVadWapper {
    
public:
    
    ASRVadWapper();
    ~ASRVadWapper();
    int init();
    void setVadTimeout(int frontTimeout, int backTimeout);
    void reset();
    int isVadTimeout(char *pcm, int len, int *volumn);
    int isActive(char *pcm, int len, int *energy);
private:
    AsrVAD *vad;
        
};



#endif /* defined(__usc__ASRVadWapper__) */
