#ifndef __TYPEDEF_H_
#define __TYPEDEF_H_

#ifdef _WIN32
typedef __int16 Int16;
typedef __int32	Int32;
typedef unsigned __int32 UInt32;
typedef	__int64	Int64;
#elif defined __GNUC__ || defined LINUX
typedef short Int16;
typedef signed int	Int32;
typedef unsigned int UInt32;
typedef	long long	Int64;
#else
    #error "What the hell is your building platform"
#endif

#define CC_NUM_CEP_COEFFS	13
#define CC_ONLINE_DELAY	6

#define CC_FFT_SHIFT	29
#define CC_FIX_QN_FFT	(INT_64)(1.0 *( 1<<CC_FFT_SHIFT )/ M_SQRT2)

#endif
