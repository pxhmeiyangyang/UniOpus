#ifndef _COMPRESSER_H
#define _COMPRESSER_H

class Compresser
{
public:
    virtual ~Compresser(){

    }    

    void setCompressType(unsigned int type){
        this->compressType = type;
    }    
 
    unsigned int getCompressType(){
        return this->compressType;
    }
    
    virtual int decode(char* in,int len,short** speech,unsigned int* speech_len){
        return 0;
    };

private:
    unsigned int compressType;    
};
#endif
