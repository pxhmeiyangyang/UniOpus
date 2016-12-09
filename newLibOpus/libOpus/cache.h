#ifndef __CACHE_H__
#define __CACHE_H__

class CLCache
{
public:
    CLCache();
    ~CLCache();

public:
    void add(char *src, unsigned int len);
    char *sub(unsigned int *len);
    bool empty(void);
    char value(char *ch, int index);
	char valueUnsigned(unsigned char *ch, int index);
    int length(void);

	// add by 671 for test
    void print_m_pbuf(void);
	/* reset buffer by lqy */
	void reset(void);

private:
    bool realloc(unsigned int len);

private:
    char *m_pbuf;
    unsigned int m_icapacity;
    unsigned int m_isize;
};

#endif
