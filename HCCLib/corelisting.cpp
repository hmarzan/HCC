#include "..\StdAfx.h"

#include "corelisting.h"

#include <iostream>
#include <iomanip>
#include <ctime>

#include <windows.h>
//#include <comutil.h>

//to centralize the error handling for the HCC Compiler
#include "errors.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;	//this is private/unique by file
#endif


using namespace std;

source_unit_listing listing;

#pragma warning(disable:4305 4309)
TCHAR source_buffer::EofChar = 0xFF;

__uint source_buffer::source_buffer_iterator::_inPos	= 0;
__uint source_buffer::source_buffer_iterator::_inLevel	= 0;
bool source_buffer::source_buffer_iterator::_bChkNoTab	= false;

// S O U R C E   B U F F E R   C L A S S 
source_buffer::source_buffer(const char* source_file, bool bListing) : 
														file(source_file, ios::in), 														
														m_pListing(&listing),
														m_nLineNumber(0),
														m_nLevel(0),
														m_bListFlag(bListing)
														//_F(file)											
{	
	m_szFileName = new char[MAX_FILENAME + 1];
	strncpy(m_szFileName, source_file, MAX_FILENAME);
	//for the file listing...
	//file.open(source_file, ios_base::in);
	if(file){
		//initialize the program unit listing global object
		if(bListing)
			m_pListing->Initialize(source_file);
		else {
#define MAX_SOURCE_FILE_PATH (MAX_PATH * 4)

			TCHAR lpszFilePath[MAX_SOURCE_FILE_PATH];

#ifdef _UNICODE
				int nConvertedChars = 0;
				mbstowcs_s(&nConvertedChars, 
						   lpszFilePath, 
						   source_file,
						   MAX_SOURCE_FILE_PATH);
#else
				_tcsncpy(lpszFilePath, source_file, MAX_SOURCE_FILE_PATH);
				//
#endif //_UNICODE

			TCHAR* lpszFileName = NULL;		

			TCHAR lpszCompleteFilePath[MAX_SOURCE_FILE_PATH];
			::GetFullPathName(lpszFilePath, //(const TCHAR*)_bstr_t(source_file), 
								MAX_SOURCE_FILE_PATH, 
								lpszCompleteFilePath, 
								&lpszFileName);
			cout << lpszFileName << endl;
		}
		getLine();
		_F = this;
	}else{
		cout << _T("{") << _T("0") << _T("} ")<< source_file << endl;
		HccErrorManager::AbortTranslation(HccErrorManager::abortSourceFileOpenFailed);
	}
}

int source_unit_listing::m_nMaxPrintLineLength	= 80;
int source_unit_listing::m_nMaxLinesPerPage		= 50;

// P R O G R A M   L I S T I N G   C L A S S 
source_unit_listing::source_unit_listing(const char* source_file)
{
	Initialize(source_file);
}

void source_unit_listing::putLine(const __tstring& current, 
									int lineNumber, 
									int nestingLevel, 
									__tostream& out)
{
	if(m_nLineCount == source_unit_listing::m_nMaxLinesPerPage)
			printPageHeader(out);

	out << setw(4) << lineNumber << _T(": ") << nestingLevel 
		<< _T(": ") 
		<< current.substr(0, source_unit_listing::m_nMaxPrintLineLength);

	if((unsigned long)current.length() > (unsigned long)source_unit_listing::m_nMaxPrintLineLength)
	{		
		const TCHAR* lpFinal = _T("");
		if((unsigned long)current.length() > (unsigned long)(source_unit_listing::m_nMaxPrintLineLength * 2))
			lpFinal = _T("...");
		//print a second line with m_nMaxPrintLineLength text...
		out << current.substr(source_unit_listing::m_nMaxPrintLineLength,
							  source_unit_listing::m_nMaxPrintLineLength)
			<< lpFinal
			<< endl;
	}
		
	m_nLineCount++;
}

void source_unit_listing::Initialize(const char *source_file)
{
	m_nLineCount = 0;
	m_nPageNumber = 0;
	delete m_szFileName;
	m_szFileName = new char[MAX_FILENAME];
	strncpy(m_szFileName, source_file, MAX_FILENAME);

	time_t tdate;
	time(&tdate);
	strncpy(date, asctime(localtime(&tdate)), sizeof(date)); //FIXED - Jan 21, 2009
	date[strlen(date)+1] = '\0';

	printPageHeader();//TODO: specify the ostream decendant to use (default: standard cout)
}

void source_unit_listing::printPageHeader(__tostream& out)
{
	const TCHAR formFeedChar = _T('\f');	
	out << formFeedChar << _T("Page ") << ++m_nPageNumber
		<< _T("   ") << m_szFileName << _T("   ") << date
		<< endl
		<< endl;
	m_nLineCount = 0;
}

void source_unit_listing::putLine(const __tstring &text, __tostream& out)
{
	out << text;
}

source_unit_listing& _endl(source_unit_listing& lister)
{lister << _T('\n');return lister;}
