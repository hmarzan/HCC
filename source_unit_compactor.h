// source_unit_compactor.h: interface for the source_unit_compactor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOURCE_UNIT_COMPACTOR_H__C34F5848_E692_4086_95F3_604D44FAD5AA__INCLUDED_)
#define AFX_SOURCE_UNIT_COMPACTOR_H__C34F5848_E692_4086_95F3_604D44FAD5AA__INCLUDED_

/*
********************************************************************************************************
*																									   *
*																									   *
*	MODULE			: <source_unit_compactor.h>														   *	
*																									   *
*	DESCRIPTION		: HCC Compiler Source Unit Compactor class										   *
*																									   *
*	AUTHOR			: Harold L. Marzan																   *	
*																									   *
*	LAST-MODIFIED	: (unknown)																		   *	
*																									   *
********************************************************************************************************
*/

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <iostream>
#include "HCCLib\corecommon.h"

class source_unit_compactor  
{
	int m_nTextLength;
	static int m_nMaxTextLengthPerLine;
	__tostream* output;
public:
	source_unit_compactor(__tostream& _out = cout) : output(&_out), m_nTextLength(0){}
	virtual ~source_unit_compactor();
public:
	source_unit_compactor& operator<<(const __tstring& text){
		if((unsigned long)m_nTextLength + text.length() <= (unsigned long)m_nMaxTextLengthPerLine){
			*output << text;
			m_nTextLength += text.length();
		}else{
			*output << _T('\n') << text;
			m_nTextLength = text.length();
		}
		return *this;
	}
	source_unit_compactor& operator<<(const TCHAR ch){
		if(++m_nTextLength <= m_nMaxTextLengthPerLine)
			*output << ch;
		else{
			*output << _T('\n') << ch;
			m_nTextLength = 1;
		}
		return *this;
	}
	source_unit_compactor& operator<<(source_unit_compactor& (*fnm)(source_unit_compactor&) )
		{return fnm(*this);}
	void flush()
		{output->flush();}
};

source_unit_compactor& _blank(source_unit_compactor& compactor);


#endif // !defined(AFX_SOURCE_UNIT_COMPACTOR_H__C34F5848_E692_4086_95F3_604D44FAD5AA__INCLUDED_)
