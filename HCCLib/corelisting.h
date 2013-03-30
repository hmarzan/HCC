#ifndef __CORE_HCC_SOURCE_LISTING_h__
#define __CORE_HCC_SOURCE_LISTING_h__

#pragma warning(disable:4244)
/*
********************************************************************************************************
*																									   *
*																									   *
*	MODULE			: <corelisting.h>																   *	
*																									   *
*	DESCRIPTION		: HCC Compiler utility classes (source interface, translation unit listing)		   *
*																									   *
*	AUTHOR			: Harold L. Marzan																   *	
*																									   *
*	LAST-MODIFIED	: (unknown)																		   *	
*																									   *
********************************************************************************************************
*/


#include "corecommon.h"

#include <iostream>
#include <iosfwd>

using namespace std;

#define MAX_FILENAME			300
#define MAX_INPUT_BUFFER_SIZE	1024

class source_unit_listing
{
	char* m_szFileName;
	long m_nPageNumber;
	long m_nLineCount;
	char date[64]; //FIXED - when using /S for source file listing, this buffer was too short to receive the date
							 //in the Initialize() member function; Jan 21, 2009

	static int m_nMaxPrintLineLength;
	static int m_nMaxLinesPerPage;

	source_unit_listing(const source_unit_listing&);
	source_unit_listing& operator=(const source_unit_listing&);
public:
	void putLine(const __tstring& text, __tostream& out = cout);
	void printPageHeader(__tostream& out = cout);
	void Initialize(const char* source_file);
	source_unit_listing() : 
				m_nLineCount(0), 
				m_szFileName(0),
				m_nPageNumber(0){}
	explicit source_unit_listing(const char* source_file);
	~source_unit_listing(){delete m_szFileName;}
	void putLine(const __tstring& current, 
				 int lineNumber, 
				 int nestingLevel, 
				 __tostream& out = cout);

	const char* getFileName() const
		{return m_szFileName;}
	const long getPageNumber() const
		{return m_nPageNumber;}
	const long lineCount() const
		{return m_nLineCount;}

	source_unit_listing& operator<<(const __tstring& text)
		{cout << text;return *this;}
	source_unit_listing& operator<<(const TCHAR ch)
		{cout << ch;return *this;}
	source_unit_listing& operator<<(const int _Val)
		{cout << _Val;return *this;}
	source_unit_listing& operator<<(const __uint _Val)
		{cout << _Val;return *this;}
	source_unit_listing& operator<<(const float _Val)
		{cout << _Val;return *this;}
	source_unit_listing& operator<<(const double _Val)
		{cout << _Val;return *this;}
	source_unit_listing& operator<<(source_unit_listing& (*fnmanip)(source_unit_listing&))
	{return fnmanip(*this);}
};

source_unit_listing& _endl(source_unit_listing& lister);

extern source_unit_listing listing;

class source_buffer
{
	source_buffer();
	source_buffer(const source_buffer&);	
	source_buffer& operator=(const source_buffer&);

	__tifstream file;
	//__tifstream_iterator _F;
	//__tifstream_iterator _L;

	source_unit_listing* m_pListing;

	char*	m_szFileName;	
	long	m_nLineNumber;
	bool	m_bListFlag;
	short	m_nLevel;	
	
	TCHAR lpszText[MAX_INPUT_BUFFER_SIZE + 1]; // <stream> + '\n' + '\0'
	static TCHAR EofChar;
public:
	//typedef __tifstream_iterator iterator;
	explicit source_buffer(const char* source_file, bool bListing = false);
	~source_buffer(){ delete []m_szFileName;}

private:
	TCHAR* getLine()
	{
		lpszText[0] = _T('\0');
		if(!file.eof()){
			file.getline(lpszText, MAX_INPUT_BUFFER_SIZE);
			__uint size = file.gcount();			
			if(size > 1 || lpszText[size - 1]==_T('\0')) //this is often the result, except, 
												//when is just one not-null char without a '\n' as the next
			{
				lpszText[size - 1]	= _T('\n');
				lpszText[size]		= _T('\0');
			}else{
				lpszText[size]		= _T('\n');
				lpszText[size + 1]	= _T('\0');
			}
			//produce output for listing if activated
			source_buffer_iterator::_inPos		= (0);
			source_buffer_iterator::_inLevel	= (0);
			source_buffer_iterator::_bChkNoTab	= false;

			++m_nLineNumber;
			if(listingFlag())
				m_pListing->putLine(lpszText, m_nLineNumber, source_buffer::nestingLevel());
			return lpszText;
		}
		return &EofChar;		
	}
public:	
	class source_buffer_iterator;
	friend class source_buffer_iterator;	
	class source_buffer_iterator : 
				public iterator<bidirectional_iterator_tag, TCHAR, char_traits<TCHAR>::off_type> {
	public:
	source_buffer_iterator(source_buffer* _SBuf = 0) :
		  _SBuff(_SBuf), _Got(_SBuf==0), _TabSize(8) , _Val(_E()), _Ptr(0)
		{if(_SBuff!=0) _Ptr = _SBuff->lpszText;}
	source_buffer_iterator(const source_buffer_iterator& _BIt) : _TabSize(8)
		{_SBuff = _BIt._SBuff;  _Ptr = _BIt._Ptr; _Val = _BIt._Val;_Got = _BIt._Got;}

	typedef TCHAR _E;
	typedef _E char_type;
	typedef char_traits<TCHAR> _Tr;
	typedef _Tr traits_type;
	typedef _Tr::int_type int_type;	
	typedef source_buffer_iterator _BuffIt;

	_BuffIt& operator=(source_buffer* _SBuf)		  
		{_SBuff = _SBuf;_Got = (_SBuf==0); _inPos = (0); _inLevel = (0); _Val = (_E()); _Ptr = (0);
		if(_SBuff!=0) 
			_Ptr = _SBuff->lpszText;return *this;}
		const _E operator*()
			{if(!_Got)
				_Peek();
				return _Val;}
		_BuffIt operator++(int)
			{if(!_Got)
				_Peek();
			_BuffIt _Tmp = *this;
			_Inc();
			return _Tmp;}
		_BuffIt& operator++()
			{_Inc();
			return *this;}
		_BuffIt operator--(int)
			{if(!_Got)
				_Peek();
			_BuffIt _Tmp = *this;
			_Dec();
			return _Tmp;}
		_BuffIt operator--()
			{_Dec();
			return *this;}
		bool operator==(const _BuffIt& _BIt)
			{return (_SBuff==0 && _BIt._SBuff==0) ||
				(_SBuff!=0 && _BIt._SBuff!=0);
				}
		bool operator!=(const _BuffIt& _BIt)
			{return !(*this==_BIt);}
		static __uint inputpos()
			{return _inPos;}
		static __uint nesting_level()
		{return _inLevel;}
	private:
	void _Inc()
		{if(*_Ptr!=_T('\0') && *_Ptr!=source_buffer::EofChar)
			_Ptr++;
		 while(*_Ptr==_T('\0'))
			 _Ptr = _SBuff->getLine();			 
		 if(*_Ptr==source_buffer::EofChar){
				_SBuff = 0;_Ptr = 0; _Val = source_buffer::EofChar;
		 }else{
				_IncPos();_Got = false;
			}
		}		
	void _Dec()
		{if(_Ptr!=_SBuff->lpszText){
			--_Ptr;_DecPos();
			_Val = *_Ptr;_Got = true;
			}
		}
	_E _Peek()
	{if(*_Ptr==_T('\0')){
			do
				_Ptr = _SBuff->getLine();
			while(*_Ptr==_T('\0'));
			if(*_Ptr==source_buffer::EofChar){
				_SBuff = 0;_Ptr = 0;_Val = source_buffer::EofChar;
			}else{
				_Val = *_Ptr;
				_Got = true;}
	}else{
				_Val = *_Ptr;
				_Got = true;
			}		
		return _Val;
	}
	void _IncPos()
		{_inPos++;
		 if(_Val=='\t' || _Val==L'\t'){
			_inPos += _TabSize - _inPos%_TabSize;
			if(false==_bChkNoTab)
				_inLevel++;
			return;
		 }
		_bChkNoTab = true;
		}
	void _DecPos()
		{_inPos--;
		 if(_Val=='\t' || _Val==L'\t'){
			_inPos -= _TabSize - _inPos%_TabSize;
			if(false==_bChkNoTab)
				_inLevel--;
			return;
		 }
		 _bChkNoTab = true;
		}
	private:
		_E _Val;
		bool _Got;		
		source_buffer* _SBuff;
		TCHAR * _Ptr;
		static __uint _inPos;
		static __uint _inLevel;
		static bool _bChkNoTab;
		char _TabSize;
		friend class source_buffer;
	};

	typedef source_buffer_iterator iterator;

	source_buffer_iterator _F;
	source_buffer_iterator _L;

	source_buffer_iterator& begin()
		{return _F;}
	source_buffer_iterator end()
		{return _L;}
	//return the current char
	const TCHAR nextChar() //const
		{return *_F;}
	//advance and return the current char...
	const TCHAR getChar()
		{return (_F!=_L) ? *_F++ : _T('\0');}
	//put the current char back (often used to put back a '.')!
	void goBack()
		{--_F;}
	//the current char position
	static __uint inputPosition()
		{return source_buffer_iterator::inputpos();}
	const long lineNumber() const
		{return m_nLineNumber;}
	const char* sourceFile() const
		{return m_szFileName;}
	//to flag to produce the code listing
	const short setListingFlag(bool doListing = true)
		{m_bListFlag = doListing;}
	const bool listingFlag() const
		{return m_bListFlag;}
	static short nestingLevel()
		{return source_buffer_iterator::nesting_level();}
};

#endif //__CORE_HCC_SOURCE_LISTING_h__
