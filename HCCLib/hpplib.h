#ifndef __HPLUSPLUS_LIBRARY_HELPER__
#define __HPLUSPLUS_LIBRARY_HELPER__

#include "corecommon.h"

extern FLOATING_POINT_CONVERSION fp_conv;

inline __int64 FromDoubleToInt64(double dValue)
{
	fp_conv.dSource = dValue;
	return fp_conv.i64Final;
}

inline long LowPartFromDouble(double dValue)
{
	fp_conv.dSource = dValue;
	return fp_conv.dwLowPart;
}

inline long HighPartFromDouble(double dValue)
{
	fp_conv.dSource = dValue;
	return fp_conv.dwHighPart;
}

inline FLOATING_POINT_CONVERSION& FromDouble(double dValue)
{
	fp_conv.dSource = dValue;	
	return fp_conv;
}

inline FLOATING_POINT_CONVERSION& FromInt64(__int64 i64Value)
{
	fp_conv.i64Final = i64Value;
	return fp_conv;
}


#endif //__HPLUSPLUS_LIBRARY_HELPER__