// HCCLexer.cpp: implementation of the HCCLexer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HCCLexer.h"

#pragma warning(disable:4786)
#pragma warning(disable:4789)

#include <map>
#include <climits>
#include <cmath>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;	//this is private/unique by file
#endif


using namespace std;

map<__tstring, HCC_TOKEN_TYPE> all_keywords;

TCHAR control_chr[] = {
					_T('t'), //0x09
					_T('r'), //0x0D
					_T('n'), //0x0A
					_T('f'), //0x0C
					_T('b'), //0x08
					_T('\\'),//0x5C
					_T('\"'),//0x22
					_T('\''),//0x27
					_T('0'), // '\0' --> 0x00
			};

__int64 HCCLexer::NumberLexer::maxInteger = _I64_MAX; //INT_MAX;
int HCCLexer::NumberLexer::maxExponent = 308;

//to map every char to its code. Note: every ASCII char in UNICODE is located in the same position.
CharCode charCodeMap[136];

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
HCCLexer::HCCLexer(source_buffer* _source_ptr) : source_ptr(_source_ptr), m_bSilentEscapedString(false)
{
	int i;
	//ccLetter, ccUnderscore, ccDigit, ccSpecial, ccSingleQuote, ccDoubleQuote, ccWhiteSpace, ccEndOfFile, ccError

	for(i=0;i<= sizeof(charCodeMap)/sizeof(charCodeMap[0]); i++) charCodeMap[i] = ccError;

	for(i=_T('a'); i<=_T('z');i++) charCodeMap[i] = ccLetter;
	for(i=_T('A'); i<=_T('Z');i++) charCodeMap[i] = ccLetter;
	for(i=_T('0'); i<=_T('9');i++) charCodeMap[i] = ccDigit;

	charCodeMap[_T('_')] = ccUnderscore;
	charCodeMap[_T('+')] = charCodeMap[_T('-')] = ccSpecial;
	charCodeMap[_T('*')] = charCodeMap[_T('/')] = ccSpecial;
	charCodeMap[_T('%')] = charCodeMap[_T('=')] = ccSpecial;
	charCodeMap[_T('.')] = charCodeMap[_T('^')] = ccSpecial;
	charCodeMap[_T('<')] = charCodeMap[_T('>')] = ccSpecial;
	charCodeMap[_T('(')] = charCodeMap[_T(')')] = ccSpecial;
	charCodeMap[_T('[')] = charCodeMap[_T(']')] = ccSpecial;
	charCodeMap[_T('{')] = charCodeMap[_T('}')] = ccSpecial;
	charCodeMap[_T(':')] = charCodeMap[_T(';')] = ccSpecial;
	charCodeMap[_T('!')] = ccSpecial;
	charCodeMap[_T('~')] = ccSpecial;
	charCodeMap[_T('|')] = charCodeMap[_T('&')] = ccSpecial;
	charCodeMap[_T(',')] = charCodeMap[_T('?')] = ccSpecial;
	

	charCodeMap[_T(' ')] = charCodeMap[_T('\t')]	= ccWhiteSpace;
	charCodeMap[_T('\r')] = charCodeMap[_T('\n')]	= ccWhiteSpace;
	charCodeMap[_T('\0')] = ccWhiteSpace;

	charCodeMap[_T('\'')] = ccSingleQuote;
	charCodeMap[_T('\"')] = ccDoubleQuote;

	if(all_keywords.size()==0)
	{
	//All supported keywords or reserved words
		all_keywords[_T("abstract")]	= HCC_ABSTRACT;
		all_keywords[_T("asm")]		= HCC_ASM;
		all_keywords[_T("auto")]	= HCC_AUTO;
		all_keywords[_T("break")]	= HCC_BREAK;
		all_keywords[_T("case")]	= HCC_CASE;
		all_keywords[_T("catch")]	= HCC_CATCH;
		all_keywords[_T("class")]	= HCC_CLASS;
		all_keywords[_T("const")]	= HCC_CONST;
		all_keywords[_T("continue")] = HCC_CONTINUE;
		all_keywords[_T("debugger")] = HCC_DEBUGGER;
		all_keywords[_T("default")] = HCC_DEFAULT;
		all_keywords[_T("do")]		= HCC_DO;
		all_keywords[_T("except")]	= HCC_EXCEPT;
		all_keywords[_T("else")]	= HCC_ELSE;
		all_keywords[_T("enum")]	= HCC_ENUM;
		all_keywords[_T("for")]		= HCC_FOR;
		all_keywords[_T("get")]		= HCC_GET;
		all_keywords[_T("goto")]	= HCC_GOTO;
		all_keywords[_T("if")]		= HCC_IF;
		all_keywords[_T("import")]	= HCC_IMPORT;

		all_keywords[_T("dynamic_cast")] = HCC_DYNAMIC_CAST; //Jan 4, 2009

		all_keywords[_T("namespace")] = HCC_NAMESPACE;
		all_keywords[_T("private")] = HCC_PRIVATE;
		all_keywords[_T("protected")] = HCC_PROTECTED;
		all_keywords[_T("property")] = HCC_PROPERTY;
		all_keywords[_T("public")]	= HCC_PUBLIC;
		all_keywords[_T("put")]		= HCC_PUT;		
		all_keywords[_T("ref")]		= HCC_REF;
		all_keywords[_T("register")] = HCC_REGISTER;
		all_keywords[_T("return")]	= HCC_RETURN;
		all_keywords[_T("sizeof")]	= HCC_SIZEOF;
		all_keywords[_T("static")]	= HCC_STATIC;
		all_keywords[_T("struct")]	= HCC_STRUCT;
		all_keywords[_T("switch")]	= HCC_SWITCH;
		all_keywords[_T("this")]	= HCC_THIS;
		all_keywords[_T("throw")]	= HCC_THROW;
		all_keywords[_T("try")]		= HCC_TRY;
		all_keywords[_T("typename")] = HCC_TYPENAME;		
		all_keywords[_T("using")]	= HCC_USING;
		all_keywords[_T("volatile")] = HCC_VOLATILE;
		all_keywords[_T("while")]	= HCC_WHILE;
		all_keywords[_T("with")]	= HCC_WITH;	

		all_keywords[_T("virtual")]	= HCC_VIRTUAL;

		all_keywords[_T("extern")]	= HCC_EXTERN;

		//data types
		all_keywords[_T("string")]	= HCC_STRING;
		all_keywords[_T("void")]	= HCC_VOID;
		all_keywords[_T("bool")]	= HCC_BOOL;
		all_keywords[_T("char")]	= HCC_CHAR;		
		all_keywords[_T("short")]	= HCC_SHORT;
		all_keywords[_T("long")]	= HCC_LONG;
		all_keywords[_T("int")]		= HCC_INT;
		all_keywords[_T("Int16")]	= HCC_INT16;
		all_keywords[_T("float")]	= HCC_FLOAT;
		all_keywords[_T("Int32")]	= HCC_INT32;
		all_keywords[_T("Int64")]	= HCC_INT64;
		all_keywords[_T("double")]	= HCC_DOUBLE;
		all_keywords[_T("unsigned")] = HCC_UNSIGNED;
		all_keywords[_T("signed")]	= HCC_SIGNED;		

		//constants
		all_keywords[_T("true")]	= HCC_TRUE;
		all_keywords[_T("false")]	= HCC_FALSE;
		all_keywords[_T("null")]	= HCC_NULL;
		

		
		//special keywords mapped to operators
		all_keywords[_T("div")]	= HCC_DIV;		//integer division
		all_keywords[_T("mod")]	= HCC_MOD_OP;	//integer division modulus
		//boolean operators...
		all_keywords[_T("or")]	= HCC_OR_OP;
		all_keywords[_T("and")]	= HCC_AND_OP;
		all_keywords[_T("xor")]	= HCC_XOR_OP;
		all_keywords[_T("not")]	= HCC_NOT_OP;

		//dynamic memory allocation/deallocation
		all_keywords[_T("new")]		= HCC_NEW;
		all_keywords[_T("destroy")]	= HCC_DESTROY;
	}
}


LPHCC_TOKEN HCCLexer::getWordToken()
{
	source_buffer::iterator& _F = source_ptr->begin();
	source_buffer::iterator& _L = source_ptr->end();

	__uint x = 0;
	LPHCC_TOKEN token_ptr = new HCC_TOKEN;	
	token_ptr->token[x++] = *_F++;
	while(_F!=_L && 
		(charCodeMap[*_F] == ccLetter || 
		 charCodeMap[*_F] == ccDigit || 
		 charCodeMap[*_F] == ccUnderscore)) 
		 token_ptr->token[x++] = *_F++;

	if(x >= MAX_TOKEN_SIZE)
	{
		TCHAR lpszMaxTokenSize[50];
		_stprintf(lpszMaxTokenSize, _T("%d characters."), MAX_TOKEN_SIZE);
		HccErrorManager::Error(HccErrorManager::errTokenExceedsMaxSize, lpszMaxTokenSize);
		x = MAX_TOKEN_SIZE - 1;
	}

	token_ptr->token[x++] = _T('\0');
	token_ptr->value.String = token_ptr->token;
	//verify if this is a keyword (reserved word) instead of an identifier
	CheckForReservedWord(token_ptr->String(), &token_ptr->tokenType);
	return token_ptr;
}

LPHCC_TOKEN HCCLexer::getNumberToken()
{
	return numberLexer.getNumberToken(source_ptr);
}

LPHCC_TOKEN HCCLexer::getStringToken()
{
	source_buffer::iterator& _F = source_ptr->begin();
	source_buffer::iterator& _L = source_ptr->end();

	LPHCC_TOKEN token_ptr = new HCC_TOKEN;
	token_ptr->dataType = HCC_STRING_TYPE;
	token_ptr->tokenType = HCC_STRING_LITERAL;

	int x = 0;
	token_ptr->token[x++] = *_F++;
	volatile bool _Got = false;
	while(_F!=_L && x < MAX_STRING_SIZE)
	{
		if(*_F==_T('\"')){
			//if the next is a ", then save one, else break out
			token_ptr->token[x++] = *_F++;
			if((_Got = (*_F!=_T('\"'))))
				break;
			else
				_F++;
		}

		volatile bool _GotEscapeChr = (*_F==_T('\\'));

		token_ptr->token[x++] = *_F++;
		//
		if(_GotEscapeChr && !m_bSilentEscapedString)
		{
			//if not in this list, generates a lexer error...
			switch(*_F)
			{
			case _T('\"'): //0x22
			case _T('\''): //0x27
			case _T('t'): //0x09
			case _T('r'): //0x0D
			case _T('n'): //0x0A
			case _T('f'): //0x0C
			case _T('b'): //0x08
			case _T('\\'): //0x5C
			case _T('0'): // '\0' --> 0x00
				break;
			default:
				{
					__tstring info = _T("; unknown escape sequence in string: \'\\");
					info += *_F;
					info += _T("\'.");
					HccErrorManager::Error(HccErrorManager::errUnrecognizable, info);
				}
				break;
			};
		}
	}
	token_ptr->bIsDelimiter = true;
	token_ptr->token[x++] = _T('\0');
	token_ptr->value.String = token_ptr->token;

	if(!_Got && _F==_L)		
		HccErrorManager::Error(HccErrorManager::errUnexpectedEndOfFile);	
	return token_ptr;
}

LPHCC_TOKEN HCCLexer::getSpecialToken()
{
	source_buffer::iterator& _F = source_ptr->begin();
	source_buffer::iterator& _L = source_ptr->end();

	LPHCC_TOKEN token_ptr = new HCC_TOKEN;
	token_ptr->dataType = HCC_CHAR_TYPE;
	int x= 0;	
	//special chars could be arithmetic operators	
	token_ptr->token[x++] = *_F;

	switch(*_F)
	{
	case _T('{'): token_ptr->tokenType = HCC_LBLOCK_KEY;	_F++; break;			//{
	case _T('}'): token_ptr->tokenType = HCC_RBLOCK_KEY;	_F++; break;			//}
	case _T('['): token_ptr->tokenType = HCC_LBRACKET;		_F++; break;			//[
	case _T(']'): token_ptr->tokenType = HCC_RBRACKET;		_F++; break;			//]
	case _T('('): token_ptr->tokenType = HCC_LPAREN;		_F++; break;			//(
	case _T(')'): token_ptr->tokenType = HCC_RPAREN;		_F++; break;			//)

	case _T(':'):
		{
		token_ptr->tokenType = HCC_COLON; _F++;								//:
		if(_F!=_L && *_F==_T(':')){
			token_ptr->tokenType = HCC_DOUBLE_COLON;						//::	
			token_ptr->token[x++] = *_F++;
		}
		}
		break;
	case _T(';'): token_ptr->tokenType = HCC_SEMICOLON;		_F++; break;		//;
	case _T('.'): token_ptr->tokenType = HCC_PERIOD;		_F++; break;		//.
	case _T(','): token_ptr->tokenType = HCC_COMMA_OP;		_F++; break;		// ,
	case _T('?'): token_ptr->tokenType = HCC_TERNARY_OP;	_F++; break;		//?

	case _T('+'):
		{
			token_ptr->tokenType = HCC_PLUS_OP; _F++;						//+
			if(_F!=_L)
			{
				if(*_F==_T('+')){
					token_ptr->tokenType = HCC_INCREMENT;					//++
					token_ptr->token[x++] = *_F++;
				}else if(*_F==_T('=')){
					token_ptr->tokenType = HCC_INC_ASSIGN;					//+=
					token_ptr->token[x++] = *_F++;
				}
			}
		}
		break;
	case _T('-'):
		{
			token_ptr->tokenType = HCC_MINUS_OP; _F++;						//-
			if(_F!=_L)
			{
				if(*_F==_T('-')){
					token_ptr->tokenType = HCC_DECREMENT;					//--
					token_ptr->token[x++] = *_F++;
				}else if(*_F==_T('=')){
					token_ptr->tokenType = HCC_DEC_ASSIGN;					//-=
					token_ptr->token[x++] = *_F++;
				}
			}
		}
		break;
	case _T('*'):{
			token_ptr->tokenType = HCC_MUL_OP; _F++;						//*
			if(_F!=_L)
			{
				if(*_F==_T('=')){
					token_ptr->tokenType = HCC_MUL_ASSIGN;					//*=
					token_ptr->token[x++] = *_F++;
				}
			}
		}
		break;
	case _T('/'):
		{
			token_ptr->tokenType = HCC_DIV_OP; _F++;						// /	
			if(_F!=_L)
			{
				if(*_F==_T('=')){
					token_ptr->tokenType = HCC_DIV_ASSIGN;					// /=
					token_ptr->token[x++] = *_F++;
				}
			}
		}
		break;
	case _T('%'):
		{
			token_ptr->tokenType = HCC_MOD_OP; _F++;						//%
			if(_F!=_L)
			{
				if(*_F==_T('=')){
					token_ptr->tokenType = HCC_MOD_ASSIGN;					//%=;
					token_ptr->token[x++] = *_F++;
				}
			}
		}
		break;
	case _T('^'):
		{
			token_ptr->tokenType = HCC_XOR_OP; _F++;						// ^
			if(_F!=_L)
			{
				if(*_F==_T('=')){
					token_ptr->tokenType = HCC_XOR_ASSIGN;					//^=
					token_ptr->token[x++] = *_F++;
				}
			}
		}
		break;
	case _T('='):
		{
			token_ptr->tokenType = HCC_ASSIGN_OP; _F++;						//=
			if(_F!=_L)
			{
				if(*_F==_T('=')){
					token_ptr->tokenType = HCC_EQUAL_OP;					//==
					token_ptr->token[x++] = *_F++;
				}
			}

		}
		break;
	case _T('!'):
		{
			token_ptr->tokenType = HCC_NOT_OP;	_F++;						//!
			if(_F!=_L)
			{
				if(*_F==_T('=')){
					token_ptr->tokenType = HCC_NOT_EQ_OP;					//!=;
					token_ptr->token[x++] = *_F++;
				}
			}
		}
		break;
	case _T('<'):
		{
			token_ptr->tokenType = HCC_LESS_OP; _F++;						//<
			if(_F!=_L)
			{
				if(*_F==_T('=')){
					token_ptr->tokenType = HCC_LESS_EQ_OP;					//<=
					token_ptr->token[x++] = *_F++;
				}else if(*_F==_T('<')){
					token_ptr->tokenType = HCC_LEFT_SHIFT_OP;				//<<
					token_ptr->token[x++] = *_F++;
				}
			}
		}
		break;
	case _T('>'):
		{
			token_ptr->tokenType = HCC_GREATER_OP; _F++;					//>
			if(_F!=_L)
			{
				if(*_F==_T('=')){
					token_ptr->tokenType = HCC_GTER_EQ_OP;					//>=
					token_ptr->token[x++] = *_F++;
				}else if(*_F==_T('>')){
					token_ptr->tokenType = HCC_RIGHT_SHIFT_OP;				//>>
					token_ptr->token[x++] = *_F++;
				}
			}
		}
		break;
	case _T('|'):
		{
			token_ptr->tokenType = HCC_BIT_OR_OP;  _F++;					// |
			if(_F!=_L)
			{
				if(*_F==_T('|')){
					token_ptr->tokenType = HCC_OR_OP;						// ||
					token_ptr->token[x++] = *_F++;
				}else if(*_F==_T('=')){
					token_ptr->tokenType = HCC_BIT_OR_ASSIGN;				//|=
					token_ptr->token[x++] = *_F++;
				}				
			}
		}
		break;
	case _T('&'):
		{
			token_ptr->tokenType = HCC_BIT_AND_OP; _F++;					// &
			if(_F!=_L)
			{
				if(*_F==_T('&')){
					token_ptr->tokenType = HCC_AND_OP;						// &&
					token_ptr->token[x++] = *_F++;
				}else if(*_F==_T('=')){
					token_ptr->tokenType = HCC_BIT_AND_ASSIGN;				// &=
					token_ptr->token[x++] = *_F++;
				}
			}
		}
		break;
	case _T('~'):
		{
			token_ptr->tokenType = HCC_COMPL_OP; _F++;						//~
			/*there is not such thing as ~= operator
			if(_F!=_L)
			{
				if(*_F==_T('=')){
					token_ptr->tokenType = HCC_COMPL_ASSIGN;				//~=
					token_ptr->token[x++] = *_F++;
				}
			}			
			*/
		}
		break;
	}

	token_ptr->bIsDelimiter = true;
	token_ptr->token[x++] = _T('\0');
	token_ptr->value.String = token_ptr->token;

	return token_ptr;
}

LPHCC_TOKEN HCCLexer::getCharToken()
{
	source_buffer::iterator& _F = source_ptr->begin();
	source_buffer::iterator& _L = source_ptr->end();

	LPHCC_TOKEN token_ptr = new HCC_TOKEN;	
	token_ptr->tokenType = HCC_CHARACTER;
	int x = 0;
	token_ptr->token[x++] = *_F++;
	if(_F!=_L)
	{
		if(*_F==_T('\\')) //is a escape char?
		{
			token_ptr->token[x++] = *_F++;
			if(_F!=_L)
			{
				TCHAR chr = *_F; //the control char
				token_ptr->token[x++] = *_F++;
				short ctrl_chr_count = sizeof(control_chr)/sizeof(control_chr[0]);
				int y=0;
				for(;y<ctrl_chr_count; y++){
					if(chr==control_chr[y])
						break;
				}
				if(y < ctrl_chr_count){
					if(*_F==_T('\''))
					{
						token_ptr->token[x++] = *_F++;						
						token_ptr->dataType = HCC_CHAR_TYPE;
						token_ptr->tokenType = HCC_CONTROL_CHAR;

						/* This process is already done by the H++ Parser
						switch(chr)
						{
						case _T('\"'): //0x22
							token_ptr->value.Character = 0x22;
							break;
						case _T('\''): //0x27
							token_ptr->value.Character = 0x27;
							break;
						case _T('t'): //0x09
							token_ptr->value.Character = 0x09; //tab char
							break;
						case _T('r'): //0x0D
							token_ptr->value.Character = 0x0D; //carrier return
							break;
						case _T('n'): //0x0A
							token_ptr->value.Character = 0x0A; //line feed
							break;
						case _T('f'): //0x0C
							token_ptr->value.Character = 0x0C; //form feed
							break;
						case _T('b'): //0x08
							token_ptr->value.Character = 0x08; //back space
							break;
						case _T('\\'): //0x5C
							token_ptr->value.Character = 0x5C; // \
							break;
						case _T('0'): // '\0' --> 0x00
							token_ptr->value.Character = 0x09; // null char
							break;
						};
						*/
					}
					else{
						_F++;
						//TODO: Error Expected single quote
						//HccErrorManager::Error(HccErrorManager::...);
					}
				}else{
					token_ptr->token[x] = _T('\0');
					__tstring info = _T("; unknown escape sequence in string: \'");
					info += token_ptr->token;
					info += _T("\'.");
					HccErrorManager::Error(HccErrorManager::errUnrecognizable, info);
				}

			}else{
				//TODO: Error Unexpected EOF
				HccErrorManager::Error(HccErrorManager::errUnexpectedEndOfFile);
			}
		}else{
			//assign the current char			
			token_ptr->value.Character = *_F;
			token_ptr->token[x++] = *_F++;
			if(_F!=_L)
			{
				if(*_F==_T('\'')){
					token_ptr->token[x++] = *_F++;					
					token_ptr->dataType = HCC_CHAR_TYPE;					
				}
				else{
					_F++;
					//TODO: Error Expected single quote
					//HccErrorManager::Error(HccErrorManager::...);
				}
			}else{
				//TODO: Error Unexpected EOF
				HccErrorManager::Error(HccErrorManager::errUnexpectedEndOfFile);
			}
		}
	}
	token_ptr->token[x++] = _T('\0');
	return token_ptr;
}

LPHCC_TOKEN HCCLexer::getErrorToken()
{
	source_buffer::iterator& _F = source_ptr->begin();
	source_buffer::iterator& _L = source_ptr->end();

	LPHCC_TOKEN token_ptr = new HCC_TOKEN;		
	token_ptr->tokenType = HCC_TOKEN_ERROR;
	token_ptr->value.Character = *_F;
	int x= 0;
	token_ptr->token[x++] = *_F++;
	token_ptr->token[x++] = _T('\0');

	HccErrorManager::Error(HccErrorManager::errUnrecognizable);
	return token_ptr;
}

LPHCC_TOKEN HCCLexer::getToken()
{
	SkipWhiteSpace();
	source_buffer::iterator& _F = source_ptr->begin();
	source_buffer::iterator& _L = source_ptr->end();
	if(_F!=_L)
	{		
		switch(charCodeMap[*_F])
		{
		case ccLetter:
		case ccUnderscore:
			return getWordToken();
			break;
		case ccDigit:
			return getNumberToken();
			break;
		case ccSpecial:
			return getSpecialToken();
			break;
		case ccSingleQuote:
			return getCharToken();
		case ccDoubleQuote:
			return getStringToken();
		case ccError:
			return getErrorToken();
		}
		//impl
	}
	//ret token eof
	LPHCC_TOKEN	eofToken = new HCC_TOKEN;
	eofToken->tokenType = HCC_EOF;
	eofToken->token[0] = _T('\0');
	return eofToken;
}

void HCCLexer::OutputToken(LPHCC_TOKEN token_ptr)
{
	switch(token_ptr->tokenType)
	{
	case HCC_IDENTIFIER:	
		listing << _T("\t>>identifier:\t") << token_ptr->String() << _endl;
		break;
	case HCC_NUMBER:
		{
			TCHAR text[100];
			if(token_ptr->dataType==HCC_INTEGER)
				_stprintf(text, _T("\t>>integer:\t%d"), token_ptr->value.Integer);
			else
				_stprintf(text, _T("\t>>double:\t%g"), token_ptr->value.Double);

			listing << text<< _endl;
		}
		break;
	case HCC_STRING_LITERAL:
		listing << _T("\t>>literal string:\t") << token_ptr->String() << _endl;
		break;
	case HCC_CHARACTER:
		listing << _T("\t>>character:\t") << token_ptr->String() << _endl;
		break;
	case HCC_CONTROL_CHAR:
		listing << _T("\t>>ctrl char:\t") << token_ptr->String() << _endl;
		break;
	case HCC_LBLOCK_KEY:		//{
		listing << _T("\t>>start block:\t") << token_ptr->String() << _endl;
		break;
	case HCC_RBLOCK_KEY:		//}
		listing << _T("\t>>end block:\t") << token_ptr->String() << _endl;
		break;
	case HCC_LBRACKET:			//[
		listing << _T("\t>>left bracket:\t") << token_ptr->String() << _endl;
		break;
	case HCC_RBRACKET:			//]
		listing << _T("\t>>right bracket:\t") << token_ptr->String() << _endl;
		break;
	case HCC_LPAREN:			//(
		listing << _T("\t>>left parentesis:\t") << token_ptr->String() << _endl;
		break;
	case HCC_RPAREN:			//)
		listing << _T("\t>>right parentesis:\t") << token_ptr->String() << _endl;
		break;
	case HCC_COLON:				//:
		listing << _T("\t>>colon:\t") << token_ptr->String() << _endl;
		break;
	case HCC_DOUBLE_COLON:		//::	
		listing << _T("\t>>double colon:\t") << token_ptr->String() << _endl;
		break;
	case HCC_SEMICOLON:			//;
		listing << _T("\t>>semicolon:\t") << token_ptr->String() << _endl;
		break;
	case HCC_PERIOD:			//.
		listing << _T("\t>>period:\t") << token_ptr->String() << _endl;
		break;
	case HCC_EOF:
		listing << _T("End-of-File") << _endl;
		break;
	default:
		{
			if(token_ptr->tokenType >= HCC_COMMA_OP && 
				token_ptr->tokenType <= HCC_COMPL_OP)
				listing << _T("\t>>operator:\t") << token_ptr->String() << _endl;
			else if(token_ptr->tokenType >= HCC_ASM && 
					token_ptr->tokenType <= HCC_WITH)
				listing << _T("\t>>keyword:\t") << token_ptr->String() << _endl;
			else	
				listing << _T("\t>>error:\t") << token_ptr->String() << _endl;
		}
		break;
	};
}

bool HCCLexer::CheckForReservedWord(const TCHAR *word, HCC_TOKEN_TYPE *type_result)
{
	*type_result = HCC_IDENTIFIER;

	map<__tstring, HCC_TOKEN_TYPE>::iterator it_keyword = all_keywords.find(word);
	if(it_keyword != all_keywords.end()){
		*type_result = it_keyword->second;
		return true;
	}	
	return false;
}

LPHCC_TOKEN HCCLexer::NumberLexer::getNumberToken(source_buffer* source_ptr)
{
	source_buffer::iterator& _F = source_ptr->begin();
	source_buffer::iterator& _L = source_ptr->end();

	LPHCC_TOKEN token_ptr = new HCC_TOKEN;

	double value = 0.0; //this value ignores the decimal point

	int wholePlaces		= 0; //the integer part
	int decimalPlaces	= 0; //the digits after the decimal point

	TCHAR exponentSign = _T('+');
	double eValue		= 0.0; //value after the 'E'
	int exponent		= 0; //final value of exponent

	bTooManyDigitsFlag	= (false);
	bIsHexNumber		= (false);

	digitCount = 0;

	token_ptr->tokenType = HCC_TOKEN_ERROR; //until we determine the correct type!
	token_ptr->dataType = HCC_INTEGER; 
	//to save the string representation of this number
	in_ptr = token_ptr->token;

	//Get the whole part first... ie.: 1244 . 6432245 e <+|-> 23
	//WholePlaces keeps track of the number of digits in this part.
	if(!acumulateValue(source_ptr, value, HccErrorManager::errInvalidNumber)){
		//if failed acumulate, then if is a hex (started with 0x):
		if(bIsHexNumber)
			hexLexer.getNumberToken(source_ptr, token_ptr, in_ptr);		
		return token_ptr;
	}
		
	wholePlaces = digitCount;

	//if there is a period character...
	if(_F!=_L && *_F==_T('.')){
		token_ptr->dataType = HCC_FLOATING_POINT; 
		*in_ptr++ = *_F++;

		if(!acumulateValue(source_ptr, value, HccErrorManager::errInvalidFraction)){
			*in_ptr++ = _T('\0');
			return token_ptr;
		}
	}

	decimalPlaces = digitCount - wholePlaces;

	//if we have a fraction part, then determine the exponent if any...
	if(_F!=_L && (*_F==_T('E') || *_F==_T('e'))){
		*in_ptr++ = *_F++;
		if(_F!=_L && (*_F==_T('+') || *_F==_T('-')))
			*in_ptr++ = exponentSign = *_F++;

		//acumulate the value after the 'E' character...
		digitCount = 0;
		if(!acumulateValue(source_ptr, eValue, HccErrorManager::errInvalidExponent)){
			*in_ptr++ = _T('\0');
			return token_ptr;
		}

		if(exponentSign==_T('-'))
			eValue = -eValue;

		if(bTooManyDigitsFlag){
			*in_ptr++ = _T('\0');
			HccErrorManager::Error(HccErrorManager::errTooManyDigits);
			return token_ptr;
		}
	}
	//Calculate and check the final exponent value, and adjust the final number...
 	//                      exp - decimalPlaces 
	//                      |      |  
	// 14.1234e+6 --->exp = 6   -  4 = 2  ---> 141234 * 10e+2
	// 14.1234e-4 --->exp = -4  -  4 = -8 ---> 141234 * 10e-8
	// 0.9743     --->exp = 0   -  4 = -4 --->   9743 * 10e-4
	// 1544e-2    --->exp = -2  -  0 = -2 --->   1544 * 10-2

	exponent = int(eValue) - decimalPlaces;

	// 4 > 308 ||
	// -8 < -308 ? Error();
	if( (exponent + wholePlaces < -maxExponent) ||
		(exponent + wholePlaces > maxExponent)) {
		*in_ptr++ = _T('\0');
		HccErrorManager::Error(HccErrorManager::errFloatingPointOutOfRange);
		return token_ptr;
	}
	//if exists the exponent part...
	if(exponent!=0)
		value *= pow(10.0, exponent);


	if(token_ptr->dataType==HCC_INTEGER){
		if(value < -maxInteger || value > maxInteger){
			*in_ptr++ = _T('\0');
			HccErrorManager::Error(HccErrorManager::errIntegerOutOfRange);
			return token_ptr;
		}
		token_ptr->value.Integer = (int)value;
	}else
		token_ptr->value.Double = value;

	token_ptr->tokenType = HCC_NUMBER;
	*in_ptr++ = _T('\0');

	return token_ptr;

}

bool HCCLexer::NumberLexer::acumulateValue(source_buffer* source_ptr, 
											double& value, HccErrorManager::HccError err)
{
	source_buffer::iterator& _F = source_ptr->begin();
	source_buffer::iterator& _L = source_ptr->end();

	const int maxDigitCount = 20;

	if(_F!=_L){
		if(charCodeMap[*_F]!=ccDigit){
			HccErrorManager::Error(err);
			return false; //failure
		}
		volatile int x_pos = 0;
		do{
			if(++digitCount <= maxDigitCount)
			{
				//10 * prevValue + currValue
				value = 10 * value + (*_F - _T('0'));
				*in_ptr++ = *_F++;
			}else 
				bTooManyDigitsFlag = true;
			x_pos++;
		}while(_F!=_L && charCodeMap[*_F]==ccDigit);

		if(_F!=_L && *_F==_T('x') && x_pos==1){
			bIsHexNumber = true;			
			return false;
		}
		
		return true;
	}
	return false;
}

LPHCC_TOKEN HCCLexer::NumberLexer::HexNumberLexer::getNumberToken(source_buffer* source_ptr, LPHCC_TOKEN token_ptr, TCHAR* in_ptr)
{
	source_buffer::iterator& _F = source_ptr->begin();
	source_buffer::iterator& _L = source_ptr->end();

	assert(token_ptr!=NULL);

	if(token_ptr!=NULL && _F!=_L)
	{
		token_ptr->tokenType = HCC_TOKEN_ERROR;	//until we determine that this is a real hex number
		token_ptr->dataType = HCC_INTEGER;

		*in_ptr++ = *_F++; //the 'x' in 0xABCD hex number...

		char szHexSource[16];
		int max_hex_chars = 16; //max chars to complete a 64-bit integer
		int count = 0;
		while(_F!=_L && 
			 ((charCodeMap[*_F]==ccLetter) || charCodeMap[*_F]==ccDigit) && 
			 count <= max_hex_chars)
		{
			*in_ptr++ = *_F;
			if(*_F!=_T('h') && *_F!=_T('H') && *_F!=_T('L'))
				szHexSource[count++] = (char)*_F;
			//next char...
			_F++;
		}
		
		if(count > 0){			
			AlphaToHex(&token_ptr->value.Integer, szHexSource, count);
			//flag it as a number...
			token_ptr->tokenType = HCC_NUMBER;
		}			
		*in_ptr++ = _T('\0');
	}else
		*in_ptr++ = _T('\0');
	return token_ptr;
}