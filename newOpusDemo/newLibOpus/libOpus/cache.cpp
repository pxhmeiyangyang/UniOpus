#include "cache.h"
#include "string.h"
#include <stdio.h>

CLCache::CLCache():m_isize(0),m_icapacity(0)
{
    m_pbuf = NULL;
}


CLCache::~CLCache()
{
    delete [] m_pbuf;
    m_pbuf = NULL;
}


bool CLCache::realloc(unsigned int len)
{
    char *tmp = NULL;

    if (0 == m_icapacity)
    {
        tmp = new char[len];
        m_pbuf = tmp;
        m_icapacity = len;
    }
    else
    {
        unsigned int blockNum = 0;

        blockNum = ((m_isize+len)/m_icapacity)+1;
        tmp = new char[blockNum*m_icapacity];

        memcpy(tmp, m_pbuf, m_isize);

        delete [] m_pbuf;
        m_pbuf = tmp;
        m_icapacity = blockNum*m_icapacity;
    }

    return true;
}


void CLCache::add(char *src, unsigned int len)
{
    if ((NULL == src) || (0 == len))
    {
        return ;
    }
    unsigned int free = m_icapacity - m_isize;
    unsigned int need = len*sizeof(char);

    if (need > free)
    {
        realloc(need);
    }

    char *addr = m_pbuf + m_isize;
    memcpy(addr, src, need);

	//for (int i = m_isize; i < 2; i++)
	//	printf("m_pbuf[%d] : %d\n", i, m_pbuf[i]);

    m_isize += need;
}

char *CLCache::sub(unsigned int *len)
{
    if (0 == m_isize)
    {
        if (NULL != len)
        {
            *len = m_isize;
        }
        return NULL;
    }
    else
    {
        if (NULL != len)
        {
            *len = m_isize;
        }
        m_isize = 0;

        return m_pbuf;
    }
}

bool CLCache::empty(void)
{
    if (0 == m_isize)
    {
        return true;
    }
    else
    {
        return false;
    }
}

char CLCache::valueUnsigned(unsigned char *ch, int index)
{
	if (index < 0 || index >= m_isize)
	{
		return -1;
	}

	//printf("index : %d, m_pbuf[index] : %d.", index, m_pbuf[index]);
	*ch = (unsigned char)(m_pbuf[index]);
	return 0;
}

char CLCache::value(char *ch, int index)
{
    if (index < 0||index >= m_isize)
    {
        return -1;
    }

	//printf("index : %d, m_pbuf[index] : %d.", index, m_pbuf[index]);
    *ch = m_pbuf[index];
    return 0;
}

int CLCache::length(void)
{
    return m_isize;
}

void CLCache::print_m_pbuf(void)
{
    printf("\tby 671:\t%s\n", m_pbuf);
}

void CLCache::reset(void)
{
	m_isize = 0;
	m_icapacity = 0;
	delete[] m_pbuf;
	m_pbuf = NULL;
}
