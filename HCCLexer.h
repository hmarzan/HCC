// HCCLexer.h: interface for the HCCLexer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HCCLEXER_H__E9AAE4C9_7B3B_4680_9FD9_061BB1B054F1__INCLUDED_)
#define AFX_HCCLEXER_H__E9AAE4C9_7B3B_4680_9FD9_061BB1B054F1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/*
********************************************************************************************************
*																									   *
*																									   *
*	MODULE			: <hcclexer.h>																	   *	
*																									   *
*	DESCRIPTION		: HCC Compiler Lexer class (token retrieval and lexical analysis for HC)		   *
*																									   *
*	AUTHOR			: Harold L. Marzan																   *	
*																									   *
*	LAST-MODIFIED	: (unknown)																		   *	
*																									   *
********************************************************************************************************
*/

#include "HCCLib\errors.h"
#include "HCCLib\corecommon.h"
#include "HCCLib\corelisting.h"

class HCCLexer  
{
	source_buffer * const source_ptr;
public:
 	HCCLexer(source_buffer* _source_ptr);		
	virtual ~HCCLexer() {}

	class NumberLexer
	{
		static __int64 maxInteger;
		static int maxExponent;		
		int digitCount;
		bool bTooManyDigitsFlag;
		bool bIsHexNumber;
		TCHAR* in_ptr;
		bool acumulateValue(source_buffer* source_ptr, double& value, HccErrorManager::HccError err);
	public:
		NumberLexer() : bTooManyDigitsFlag(false), digitCount(0), in_ptr(0) {}
		~NumberLexer(){}

		LPHCC_TOKEN getNumberToken(source_buffer* source_ptr);

		class HexNumberLexer
		{		
			unsigned char BinaryToHex(char ch)
			{
				if ( ch >= '0' && ch <= '9' )
				{
					return ( ch - '0' );
				}

				if ( ch >= 'A' && ch <= 'F' )
				{
					return ( ch - 'A' + 0xA );
				}

				if (ch >= 'a' && ch <= 'f' )
				{
					return ( ch - 'a' + 0xA );
				}
				return -1; //to flag and error to the lexer
			}

			void AlphaToHex(__int64* pi64Dest, char *szSource, int count)
			{
				if(szSource==0 || pi64Dest==0)
					return;
				__tstring info = _T(": \'");
				info += szSource;	//TODO: char to wchar_t conversion is needed 'cause of UNICODE issues
				info += _T("\';");

				*pi64Dest = 0; //init to zero
				char nByte = 0;
				char* szDest = (char*)pi64Dest;

				while (count > 0)
				{
					char AH = BinaryToHex(*szSource++);
					char AL = 0;
					count--;
					if(AH==-1)
					{
						HccErrorManager::Error(HccErrorManager::errInvalidHexNumber, info);
					}	
					nByte = AH;
					if(count > 0)
					{
						nByte = (AH << 4);
						//at the end of the hex definition?
						if(*szSource=='h' || *szSource=='\0')
							break;
						AL = BinaryToHex(*szSource++);
						if(AL==-1)
						{
							HccErrorManager::Error(HccErrorManager::errInvalidHexNumber, info);
						}
						nByte += AL;						
						count--;

						*szDest = nByte;
						//at the end of the hex definition?
						if(*szSource=='h' || *szSource=='\0')
							break;

						if(count > 0)
						{
							if(count > 1)
							{
								*pi64Dest = *pi64Dest << 8;
							}else{
								*pi64Dest = *pi64Dest << 4;
							}
						}
						continue;
					}
					//sum the last nibble...
					*pi64Dest += nByte;
					break;		
				}    
			}
		public:
			HexNumberLexer() {}
			~HexNumberLexer(){}

			LPHCC_TOKEN getNumberToken(source_buffer* source_ptr, LPHCC_TOKEN token_ptr, TCHAR* in_ptr);
			
		} hexLexer;

		
	} numberLexer;
private:
	bool CheckForReservedWord(const TCHAR* word, HCC_TOKEN_TYPE* type_result);
	LPHCC_TOKEN getErrorToken();
	LPHCC_TOKEN getCharToken();
	LPHCC_TOKEN getSpecialToken();
	LPHCC_TOKEN getStringToken();
	LPHCC_TOKEN getNumberToken();
	LPHCC_TOKEN getWordToken();

public:
	source_buffer* sourceBuf() const
		{return source_ptr;}
	void OutputToken(LPHCC_TOKEN token_ptr);
	LPHCC_TOKEN getToken();

	void SkipWhiteSpace(void){
		source_buffer::iterator& _F = source_ptr->begin();
		source_buffer::iterator& _L = source_ptr->end();
SKIP_WS_AND_COMMENTS:
		while(_F!=_L && _istspace(*_F))
			_F++;
		/* 
			Also skip this type of C++ comment style
		*//*		another comment in test		*/
		if(_F!=_L)
			if(*_F==_T('/')){
				_F++;//skip it
				if(_F!=_L)
				{					
					if(*_F==_T('*')){
						TCHAR prev = _T('\0');
						_F++;
						while(_F!=_L){							
							if(prev==_T('*') && *_F==_T('/'))
								break;
							prev = *_F++;
						}
						if(_F!=_L)
						{
							if(*_F==_T('/')){
								_F++;
								goto SKIP_WS_AND_COMMENTS;
							}
								
						}else{
							//Unexpected EOF
							HccErrorManager::Error(HccErrorManager::errUnexpectedEndOfFile);
						}
					}else if(*_F==_T('/')){ //an inline comment
							_F++;
							while(_F!=_L && *_F!=_T('\n'))
								_F++;
							goto SKIP_WS_AND_COMMENTS;
					}else
						_F--;

				}else{
					//Unexpected EOF
					HccErrorManager::Error(HccErrorManager::errUnexpectedEndOfFile);
				}
			}		
	}
public:
	bool m_bSilentEscapedString;
};

#endif // !defined(AFX_HCCLEXER_H__E9AAE4C9_7B3B_4680_9FD9_061BB1B054F1__INCLUDED_)
