#include "..\StdAfx.h"

#include "coreicode.h"
#include "errors.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;	//this is private/unique by file
#endif


//int icode_generator::line_number = 0;

TCHAR* symbolStrings[] = {
	NULL, //HCC_TOKEN_ERROR		= 0,
	NULL, //HCC_IDENTIFIER		= 1,	
	NULL, //HCC_KEYWORD			= 2,
	NULL, //HCC_NUMBER			= 3,
	NULL, //HCC_STRING_LITERAL	= 4,
	NULL, //HCC_CHARACTER		= 5,
	NULL, //HCC_CONTROL_CHAR	= 6,
	NULL, //HCC_EOF				= 7,
	NULL, //HCC_LINE_MARKER		= 8,


	_T("{"), //HCC_LBLOCK_KEY,		//{
	_T("}"), //HCC_RBLOCK_KEY,		//}
	_T("["), //HCC_LBRACKET,		//[
	_T("]"), //HCC_RBRACKET,		//]
	_T("("), //HCC_LPAREN,			//(
	_T(")"), //HCC_RPAREN,			//)

	_T(":"), //HCC_COLON,			//:
	_T("::"), //HCC_DOUBLE_COLON,	//::	
	_T(";"), //HCC_SEMICOLON,		//;
	_T("."), //HCC_PERIOD,			//.
	_T(","), //HCC_COMMA_OP,		// ,
	//A R I T H M E T I C   O P E R A T O R S 
	_T("+"), //HCC_PLUS_OP,		//+
	_T("-"), //HCC_MINUS_OP,		//-
	_T("*"), //HCC_MUL_OP,			//*
	_T("/"), //HCC_DIV_OP,			// /	
	_T("%"), //HCC_MOD_OP,			//%
	//T H E   S I M P L E   A S S I G N M E N T   O P E R A T O R
	_T("="), //HCC_ASSIGN_OP,		//=
	//R E L A T I O N A L   O P E R A T O R S 
	_T("<"), //HCC_LESS_OP,		//<
	_T("<="), //HCC_LESS_EQ_OP,		//<=
	_T(">"), //HCC_GREATER_OP,		//>
	_T(">="), //HCC_GTER_EQ_OP,		//>=
	_T("=="), //HCC_EQUAL_OP,		//==
	_T("!="), //HCC_NOT_EQ_OP,		//!=
	//A S S I G N M E N T   O P E R A T O R S
	_T("++"), //HCC_INCREMENT,		//++
	_T("--"), //HCC_DECREMENT,		//--
	_T("+="), //HCC_INC_ASSIGN,		//+=
	_T("-="), //HCC_DEC_ASSIGN,		//-=
	_T("*="), //HCC_MUL_ASSIGN,		//*=
	_T("/="), //HCC_DIV_ASSIGN,		///=
	_T("%="), //HCC_MOD_ASSIGN,		//%=
	_T("^="), //HCC_XOR_ASSIGN,		//^=
	//B O O L E A N   A N D   B I T   O P E R A T O R S 
	_T("<<"), //HCC_LEFT_SHIFT_OP,		// <<
	_T(">>"), //HCC_RIGHT_SHIFT_OP,		// >>
	_T("|"), //HCC_BIT_OR_OP,		// |
	_T("|="), //HCC_BIT_OR_ASSIGN,	// |=
	_T("||"), //HCC_OR_OP,			// ||
	_T("&"), //HCC_BIT_AND_OP,		// &
	_T("&="), //HCC_BIT_AND_ASSIGN,	// &=
	_T("&&"), //HCC_AND_OP,			// &&
	_T("^"), //HCC_XOR_OP,			// ^
	_T("!"), //HCC_NOT_OP,			//!
	_T("~"), //HCC_COMPL_OP,		//~
	_T("?"),	//	HCC_TERNARY_OP,		//?
	
	//All supported keywords or reserved words
	_T("abstract"), //HCC_ABSTRACT
	_T("asm"), //HCC_ASM, 
	_T("auto"), //HCC_AUTO, 
	_T("bool"), //HCC_BOOL, 
	_T("break"), //HCC_BREAK,	
	_T("case"), //HCC_CASE, 
	_T("catch"), //HCC_CATCH, 
	_T("char"), //HCC_CHAR, 
	_T("class"), //HCC_CLASS, 
	_T("const"), //HCC_CONST, 
	_T("continue"), //HCC_CONTINUE,
	_T("debugger"), //HCC_DEBUGGER
	_T("default"), //HCC_DEFAULT, 
	_T("destroy"), //HCC_DESTROY,
	_T("div"), //HCC_DIV, 
	_T("do"), //HCC_DO, 
	_T("double"), //HCC_DOUBLE, 
	_T("dynamic_cast"), //HCC_DYNAMIC_CLAST,
	_T("except"), //HCC_EXCEPT, 
	_T("else"), //HCC_ELSE, 
	_T("enum"), //HCC_ENUM,
	_T("extern"), //HCC_EXTERN,
	_T("false"), //HCC_FALSE, 
	_T("float"), //HCC_FLOAT, 
	_T("for"), //HCC_FOR, 
	_T("get"), //HCC_GET, 
	_T("goto"), //HCC_GOTO,
	_T("if"), //HCC_IF, 
	_T("import"), //HCC_IMPORT,
	_T("int"), //HCC_INT, 
	_T("Int16"), //HCC_INT16, 
	_T("Int32"), //HCC_INT32, 
	_T("Int64"), //HCC_INT64, 
	_T("long"), //HCC_LONG, 
	_T("namespace"), //HCC_NAMESPACE, 
	_T("new"), //HCC_NEW,
	_T("null"), //HCC_NULL,
	_T("private"), //HCC_PRIVATE, 
	_T("protected"), //HCC_PROTECTED, 
	_T("property"), //HCC_PROPERTY, 
	_T("public"), //HCC_PUBLIC, 
	_T("put"), //HCC_PUT,
	_T("ref"), //HCC_REF,
	_T("register"), //HCC_REGISTER, 
	_T("return"), //HCC_RETURN, 
	_T("short"), //HCC_SHORT, 
	_T("signed"), //HCC_SIGNED, 
	_T("sizeof"), //HCC_SIZEOF,	
	_T("static"), //HCC_STATIC,	
	_T("struct"), //HCC_STRUCT,	
	_T("string"), //HCC_STRING,
	_T("switch"), //HCC_SWITCH,
	_T("this"), //HCC_THIS, 
	_T("throw"), //HCC_THROW, 
	_T("true"), //HCC_TRUE,	
	_T("try"), //HCC_TRY, 
	_T("typename"), //HCC_TYPENAME, 
	_T("unsigned"), //HCC_UNSIGNED, 
	_T("using"), //HCC_USING, 
	_T("virtual"), //HCC_VIRTUAL
	_T("void"), //HCC_VOID, 
	_T("volatile"), //HCC_VOLATILE, 
	_T("while"), //HCC_WHILE, 
	_T("with"), //HCC_WITH,
};

icode_generator::icode_generator(const icode_generator& icode)
{
	int length = icode.get_pos();
	cursor = code_ptr = new char[length];
	//copy the icode binary data...
	memcpy(code_ptr, icode.code_ptr, length);
}

void icode_generator::check_bounds(__uint sz)
{
	if(cursor + sz >= &code_ptr[CODE_SEGMENT_MAX_SIZE]){
		HccErrorManager::Error(HccErrorManager::errCodeSegmentOverflow);
		HccErrorManager::AbortTranslation(HccErrorManager::abortCodeSegmentOverflow);
	}
}

void icode_generator::append(HCC_TOKEN_TYPE type)
{
	assert(type!=HCC_TOKEN_ERROR);
	//this process cannot support any kind of errors
	if(HccErrorManager::errorCount() > 0)
		return;

	wchar_t code = type;
	check_bounds(sizeof(wchar_t));
	memcpy(cursor, &code, sizeof(wchar_t));
	cursor += sizeof(wchar_t);
}
void icode_generator::append(const Symbol* symbol)
{
	//this process cannot support any kind of errors
	if(HccErrorManager::errorCount() > 0)
		return;
	
	check_bounds(sizeof(int));
	//save this address no matter what symbol table it belongs to...
	int nAddress = reinterpret_cast<int>(symbol);
	memcpy(cursor, &nAddress, sizeof(int));
	cursor += sizeof(int);
}
void icode_generator::insert_line_marker(void)
{
	//this process cannot support any kind of errors
	if(HccErrorManager::errorCount() > 0)
		return;

	wchar_t last_code;
	cursor -= sizeof(wchar_t);
	memcpy(&last_code, cursor, sizeof(wchar_t));

	HCC_TOKEN_TYPE token_type = HCC_LINE_MARKER;
	check_bounds(sizeof(wchar_t) + sizeof(int));
	//insert the line marker, and the line number...
	wchar_t code = token_type;
	memcpy(cursor, &code, sizeof(wchar_t));
	cursor += sizeof(wchar_t);
	//get the current line number from the source buffer object...
	if(source_ptr!=NULL)
		line_number = source_ptr->lineNumber();
	//
	memcpy(cursor, &line_number, sizeof(int));
	cursor += sizeof(int);

	//insert the last token code...
	memcpy(cursor, &last_code, sizeof(wchar_t));
	cursor += sizeof(wchar_t);
}

bool icode_generator::IsDataType(HCC_TOKEN_TYPE type)
{
		//for all new declarations of stack variables with primitive types...
	switch(type)
	{
	case HCC_BOOL:
	case HCC_CHAR:
	case HCC_SHORT:
	case HCC_LONG:
	case HCC_INT:
	case HCC_INT16:
	case HCC_FLOAT:
	case HCC_INT32:
	case HCC_INT64:
	case HCC_DOUBLE:
	case HCC_UNSIGNED:
	case HCC_SIGNED:
	case HCC_STRING:
		return true;
		break;
	}
	return false;
}

bool icode_generator::IsStorageSpec(HCC_TOKEN_TYPE type)
{
	//for all new declarations of variables with storage specifier types...
	switch(type)
	{
	case HCC_AUTO:
	case HCC_REGISTER:
	case HCC_STATIC:
	case HCC_VOLATILE:
	case HCC_VIRTUAL:
		return true;
		break;
	}
	return false;
}


HCC_TOKEN* icode_generator::getToken(void)
{
	check_bounds(0);
	HCC_TOKEN* token_ptr = new HCC_TOKEN;
	//do loop to extract the line markers	
	HCC_TOKEN_TYPE token_type = HCC_TOKEN_ERROR;

	do{
		memcpy((void*)&token_type, cursor, sizeof(wchar_t));		
		cursor += sizeof(wchar_t);

		if(token_type==HCC_LINE_MARKER){			
			memcpy(&line_number, cursor, sizeof(int));
			cursor += sizeof(int);
		}
	}while(token_type==HCC_LINE_MARKER || IsDataType(token_type) || IsStorageSpec(token_type));
	//
	token_ptr->tokenType = token_type;
	switch(token_type)
	{
	case HCC_NUMBER:
	case HCC_STRING_LITERAL:
	case HCC_CHARACTER:
	case HCC_CONTROL_CHAR:

	case HCC_IDENTIFIER:
	case HCC_SIZEOF:
	
	case HCC_TRUE:
	case HCC_FALSE:
	case HCC_NULL:
		{			
			symbol_ptr = get_symbol();
			_tcsncpy(token_ptr->token, symbol_ptr->String().c_str(),
						symbol_ptr->String().length());
			token_ptr->token[symbol_ptr->String().length()] = _T('\0');
			token_ptr->dataType = symbol_ptr->getDataType();
			//must set the data type for the internal representation of data
			//and for correct evaluation by the H++ interpreter...
			switch(token_type)
			{
			case HCC_NUMBER:
				{
				if(symbol_ptr->getDataType()==HCC_INTEGER)
					token_ptr->value.Integer = symbol_ptr->getDeclDefinition().constant.value.Integer; // (int)symbol_ptr->getValue();
				else if(symbol_ptr->getDataType()==HCC_FLOATING_POINT)
					token_ptr->value.Double = symbol_ptr->getDeclDefinition().constant.value.Double; //symbol_ptr->getValue();
				}
				break;
			case HCC_CHARACTER:
			case HCC_CONTROL_CHAR:
				token_ptr->value.Character = symbol_ptr->getDeclDefinition().constant.value.Character;
				break;
			case HCC_BOOLEAN:
				token_ptr->value.Boolean = symbol_ptr->getDeclDefinition().constant.value.Boolean;
				break;
			default:
				//for all other types...
				token_ptr->value.Integer = symbol_ptr->getDeclDefinition().constant.value.Integer; 
				break;
			}

		}
		break;
	default:
		{
			symbol_ptr = NULL;
			if(token_type>HCC_LINE_MARKER && token_type<=HCC_WITH)
				_tcscpy(token_ptr->token, symbolStrings[token_type]);			
			else if(token_type < HCC_TOKEN_ERROR || token_type > HCC_WITH)
				token_ptr->tokenType = HCC_EOF;
			
		}
		break;
	};
	return token_ptr;
}

Symbol* icode_generator::get_symbol()
{
	int nAddress = 0;
	memcpy((void*)&nAddress, cursor, sizeof(int));
	cursor += sizeof(int);
	Symbol* sym_ptr = reinterpret_cast<Symbol*>(nAddress);
	return sym_ptr;
}

icode_generator g_icode_gen;