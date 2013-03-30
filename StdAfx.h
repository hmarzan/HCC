// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__EA89EB4B_1681_45AB_9A6E_3B8B23CC50B0__INCLUDED_)
#define AFX_STDAFX_H__EA89EB4B_1681_45AB_9A6E_3B8B23CC50B0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#define _CRTDBG_MAP_ALLOC		//to get the memory leak line number

#define _INTEGRAL_MAX_BITS 64

#include <malloc.h>
#include <cstdlib>				//define the memory allocation functions
#include <crtdbg.h>				//redefine the memory allocation functions

#define DEBUG_NEW new(_NORMAL_BLOCK, THIS_FILE, __LINE__)

#pragma warning(disable: 4786)

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__EA89EB4B_1681_45AB_9A6E_3B8B23CC50B0__INCLUDED_)
