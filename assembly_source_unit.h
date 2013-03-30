// assembly_source_unit.h: interface for the assembly_source_unit class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ASSEMBLY_SOURCE_UNIT_H__4003CFF4_DFCA_4960_B47E_4F0DC73AD181__INCLUDED_)
#define AFX_ASSEMBLY_SOURCE_UNIT_H__4003CFF4_DFCA_4960_B47E_4F0DC73AD181__INCLUDED_

/*
********************************************************************************************************
*																									   *
*																									   *
*	MODULE			: <assembly_source_unit.h>														   *	
*																									   *
*	DESCRIPTION		: HCC Compiler Assembly Source Unit class										   *
*																									   *
*	AUTHOR			: Harold L. Marzan																   *	
*																									   *
*	LAST-MODIFIED	: (unknown)																		   *	
*																									   *
********************************************************************************************************
*/
#if !defined(_DEBUG)
#define __STREAM_TO_FILE
#endif


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "HCCLib\corecommon.h"
#include "HCCLib\errors.h"
#include <iostream>
#include <iomanip>

using namespace std;

class assembly_source_unit
{
	__tostream* out_ptr;
	static int m_nMaxCharsPerLine;
	int m_nTextLineLength;	
public:
	assembly_source_unit(const char* output_file) : m_nTextLineLength(0) {

#ifdef __STREAM_TO_FILE	
		out_ptr = new __tofstream(output_file);
#else
		out_ptr = &cout;
#endif

		if(!out_ptr)
			HccErrorManager::AbortTranslation(HccErrorManager::abortAssemblyFileOpenFailed);
	}
	virtual ~assembly_source_unit();
public:
	void close();
	void flush();
	void attach(__tostream* new_out_ptr);
	__tostream* detach();
	assembly_source_unit& operator<<(const __tstring& code)
	{		
		if((unsigned long)code.length() + m_nTextLineLength <= (unsigned long)m_nMaxCharsPerLine){
			*out_ptr << code;
			m_nTextLineLength += code.length();
		}else{
			//*out_ptr << _T('\n') << code; //<-- This lines causes unpredictable syntax errors in MASM when breaking with newlines
			*out_ptr << code;
			m_nTextLineLength = code.length();
		}
		return *this;
	}
	assembly_source_unit& operator<<(const TCHAR ch)
	{
		if(ch==_T('\n')) m_nTextLineLength = 0;

		if(++m_nTextLineLength <= m_nMaxCharsPerLine)
			*out_ptr << ch;			
		else{
			*out_ptr << _T('\n') << ch;			
			m_nTextLineLength = 1;
		}
		return *this;
	}

	assembly_source_unit& operator<<(const int nValue)
	{		
		if(++m_nTextLineLength <= m_nMaxCharsPerLine)
			*out_ptr << setw(9) 
					<< uppercase
					<< setfill('0') 
					<< hex << nValue 
					<< nouppercase
					<< _T("h");
		else{
			*out_ptr << _T('\n') 
					<< setw(9) 
					<< uppercase
					<< setfill('0') 
					<< hex << nValue 
					<< nouppercase
					<< _T("h");
			m_nTextLineLength = 1;
		}
		return *this;
	}

	assembly_source_unit& operator<<(const __int64 nValue)
	{		
		if(++m_nTextLineLength > m_nMaxCharsPerLine)
		{
			*out_ptr << _T('\n');
			m_nTextLineLength = 1;
		}

		*out_ptr << setw(17)
				<< uppercase
				<< setfill('0') 
				<< hex << nValue 
				<< nouppercase
				<< _T("h");

		return *this;
	}

	assembly_source_unit& operator<<(const double dValue)
	{		
		if(++m_nTextLineLength <= m_nMaxCharsPerLine)
			*out_ptr << dValue;
		else{
			*out_ptr << _T('\n') << dValue;
			m_nTextLineLength = 1;
		}
		return *this;
	}


	assembly_source_unit& operator<<( assembly_source_unit& (*fnManip)(assembly_source_unit&))
	{		
		return fnManip(*this);
	}

	friend assembly_source_unit& hex(assembly_source_unit& unit);
	friend assembly_source_unit& dec(assembly_source_unit& unit);
};

assembly_source_unit& endl(assembly_source_unit& unit);
assembly_source_unit& ctab(assembly_source_unit& unit);
assembly_source_unit& space(assembly_source_unit& unit);
assembly_source_unit& comma(assembly_source_unit& unit);


#endif // !defined(AFX_ASSEMBLY_SOURCE_UNIT_H__4003CFF4_DFCA_4960_B47E_4F0DC73AD181__INCLUDED_)
