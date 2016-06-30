#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h> 

#define END_DATA_FIELD_LEN 4
#define KEY_DATA_FIELD_LEN 1
#define LIST_NUM 16
class Encrypter{
    public:
        static int EncodeContent
            (char* key, int keyLen, char* targetData, int targetDTLen, char* encodedData, int encodeDTLen)
            {
                char *chKey = key;
                char *chTarget = targetData;
                if(chKey==NULL || chTarget==NULL||encodedData==NULL||keyLen<=0||encodeDTLen<=0||targetDTLen<=0){
                    printf("key or targetData or encodedData error\n");
                    return -1;
                }
                char *chEncode = (char*)malloc(encodeDTLen);
                if(chEncode==NULL){
                    printf("malloc chEncode error\n");
                    return -2;
                }
                memset(chEncode, 0x00, encodeDTLen);

                //If len of encode data is shorter than target we return -1
                if(encodeDTLen<targetDTLen){
                    printf("encodeDTLen<targetDTLen\n");
                    return -3;
                }

                //add user id in the front of encoded data
                if(keyLen>60){
                    printf("key is too long\n");
                    return -4;
                }
                chEncode[0]=(char)keyLen;
                if(encodeDTLen>=keyLen+1){
                    memcpy(chEncode+1, chKey, keyLen);
                }else{
                    printf("encodeDTLen is too short to load key\n");
                    return -5;
                }
                char para1=0;
                char para2=chKey[0];
                if(encodeDTLen>=keyLen+1+targetDTLen){
                    for(int i=0;i<targetDTLen;i++)
                    {
                        int indexOfKey = i%keyLen;
                        para2 = chKey[indexOfKey];
                        chEncode[i+keyLen+1] = chTarget[i]^para1^para2;	
                        para1 = chEncode[i+keyLen+1];
                    }
                }else{
                    printf("encodeDTLen is not long enough\n");
                    return -6;
                }
 
                int totalLen=keyLen+1+targetDTLen+END_DATA_FIELD_LEN;
                int nwSeqLen = htonl(totalLen);
                if(encodeDTLen>=totalLen){
                    memcpy(encodedData, (char*)&nwSeqLen,END_DATA_FIELD_LEN);
                    memcpy(encodedData+END_DATA_FIELD_LEN, chEncode, keyLen+1+targetDTLen);
                }else{
                    printf("encodeDTLen is not long enough\n");
                    return -7;
                }

                free(chEncode);

                return 0;
            }

        static int DecodeContent
            (char* key, int keyLen, char* encodedData, int encodeDTLen, char*  decodedData, int decodeDTLen, int* realDTLen)
            {
                char* chKey=key;
                char* chEncode=encodedData;

                if(key==NULL||keyLen<=0||encodedData==NULL||encodeDTLen<=0||decodedData==NULL||decodeDTLen<=0){
                    printf("data error \n");
                    return -1;
                }
                /*
                printf("DecodeContent\n");
                printf("keyLen in inner function is %d\n",keyLen);
                printf("key value in inner function is %s\n", chKey);
                printf("encodeDTLen len in inner function is %d\n", encodeDTLen);
                printf("encoded data in inner function is %s\n", chEncode);
                printf("key\n");
                print_rowdata(key,keyLen);
                */
                char *chDecode = (char*)malloc(encodeDTLen);
                if(chDecode==NULL){
                    printf("malloc chDecode error\n");
                    return -2;
                }
                memset(chDecode, 0x00, encodeDTLen);


                //If len of encode data is shorter than target we return -1
                if(decodeDTLen<encodeDTLen)
                    return -3;
                /*
                printf("before decode\n");
                print_rowdata(chDecode,encodeDTLen);
                */
                char para1=0;
                char para2=chKey[0];
                for(int i=0; i<encodeDTLen; i++)
                {
                    int indexOfKey = i%keyLen;
                    para2 = chKey[indexOfKey];
                    chDecode[i] = chEncode[i]^para1^para2;
                    para1 = chEncode[i];
                }
                /*
                printf("after decode\n");
                printf("decode str %s\n",chDecode);
                print_rowdata(chDecode,encodeDTLen);
                printf("Encode Data Length is %d\n", encodeDTLen);
                */
                if(decodeDTLen>=encodeDTLen+END_DATA_FIELD_LEN){
                    memcpy( decodedData, (char*)&encodeDTLen, END_DATA_FIELD_LEN);
                    memcpy( decodedData+END_DATA_FIELD_LEN, chDecode, encodeDTLen);
                }else{
                    printf("decodeDTLen is too short\n");
                    return -4;
                }
                *realDTLen=encodeDTLen+END_DATA_FIELD_LEN;
                /*
                printf("decodedData\n");
                print_rowdata(decodedData,decodeDTLen);
                */
                free(chDecode);
                return 0;
            }

        static int DecodeTotalContent( char* key, int* keyLen, char* encodedData, int encodeDTLen, char* decodedData, int decodeDTLen, int* realDTLen)
        {
            //printf("DecodeTotalContent\n");
            if(encodedData==NULL||encodeDTLen<=0||key==NULL||*(keyLen)<=0||decodedData==NULL||decodeDTLen<=0){
                printf("encode data error\n");
                return -1;
            }
            char* GenalInfo=encodedData;
            int itotalLen=0;
            int nwSeqLen=0;
            if(encodeDTLen>=END_DATA_FIELD_LEN){
                memcpy((char*)&nwSeqLen, GenalInfo, END_DATA_FIELD_LEN);
            }else{
                printf("encodeDTLen is too short\n");
                return -2;
            }
            itotalLen=ntohl(nwSeqLen);
            char* userIdInfo=NULL;
            char lenOfUserid=0;
            if(encodeDTLen>=END_DATA_FIELD_LEN+1){
                userIdInfo=GenalInfo+END_DATA_FIELD_LEN;
                lenOfUserid=userIdInfo[0];
            }else{
                printf("encodeDTLen is too short\n");
                return -2;
            }
            int realEncodeDTLen = 0;
            if(lenOfUserid>0&&itotalLen>END_DATA_FIELD_LEN+1+lenOfUserid){ 
                realEncodeDTLen=itotalLen-END_DATA_FIELD_LEN-1-lenOfUserid;
            }else{
                printf("lenOfUserid or itotalLen error\n");
                return -3;
            }

            if(itotalLen>encodeDTLen)
                return -4;

            /*
            printf("tatol input len is 0x%04x\n", itotalLen);
            printf("userid len is %d\n", lenOfUserid);
            printf("encoded data len is %d\n", realEncodeDTLen);
            printf("decoded data len of storage is %d\n", decodeDTLen);        
            */

            if(encodeDTLen>=END_DATA_FIELD_LEN+1+lenOfUserid){
                memcpy(key+KEY_DATA_FIELD_LEN, userIdInfo+1, lenOfUserid);
                memcpy(key, (char*)&lenOfUserid, KEY_DATA_FIELD_LEN);
                *keyLen=lenOfUserid+KEY_DATA_FIELD_LEN;        
            }else{
                printf("encodeDTLen is too short\n");
                return -2;
            }
            /*
            printf("key rowdata\n");
            print_rowdata(key,*keyLen);
            */
            int status = Encrypter::DecodeContent((userIdInfo+1), lenOfUserid, userIdInfo+1+lenOfUserid, realEncodeDTLen, decodedData, decodeDTLen, realDTLen);
            if(status){
                printf("DecodeContent error status %d\n",status);
                return -5;
            }else
                return 0;
        } 

    static int print_rowdata(char* rowdata, int datalen){
        if(rowdata==NULL||datalen<=0){
            printf("print_rowdata:dataerror\n");
            return -1;
        }
        printf("/**************************ROW DATA START*****************************/\n");
        int i = 0;
        for(i=0; i<datalen;i++){
            if(i%LIST_NUM == 0)
                printf("%04x ",i);
            unsigned char c = *(unsigned char*)(rowdata+i);
            printf(" %02x ",c);
            if(i%LIST_NUM == LIST_NUM-1)
                printf("\n");

        }
        if(datalen%LIST_NUM !=0)
            printf("\n");
        
        printf("/***************************ROW DATA END******************************/\n");    

        return 0;
        
    
    }
};
