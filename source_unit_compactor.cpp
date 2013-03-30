// source_unit_compactor.cpp: implementation of the source_unit_compactor class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "source_unit_compactor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;	//this is private/unique by file
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
int source_unit_compactor::m_nMaxTextLengthPerLine = 80;

source_unit_compactor::~source_unit_compactor()
{

}

source_unit_compactor& _blank(source_unit_compactor& compactor)
	{compactor << _T(' ');return compactor;}
