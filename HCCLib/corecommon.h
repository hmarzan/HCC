#ifndef __CORE_HCC_COMMON_FEATURES_h__
#define __CORE_HCC_COMMON_FEATURES_h__

/*
********************************************************************************************************
*																									   *
*																									   *
*	MODULE			: <corecommon.h>																   *	
*																									   *
*	DESCRIPTION		: HCC Compiler utility typedef/type constructs for common char types			   *
*																									   *
*	AUTHOR			: Harold L. Marzan																   *	
*																									   *
*	LAST-MODIFIED	: Jan 04, 2009																		   *	
*																									   *
********************************************************************************************************
*/

#pragma warning(disable:4786)

#include <cstdio>
#include <tchar.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iterator>
#include "ifbiterator.h"

#include <stack>

typedef std::stack<double> RUNTIME_STACK;

using namespace std;

typedef basic_string<TCHAR>			__tstring;
typedef basic_ifstream<TCHAR>		__tifstream;
typedef basic_ofstream<TCHAR>		__tofstream;
typedef basic_istringstream<TCHAR>	__tistring_stream;
typedef basic_ostringstream<TCHAR>	__tostring_stream;

typedef basic_istream<TCHAR>		__tistream;
typedef basic_ostream<TCHAR>		__tostream;

typedef bidir_istreambuf_iterator<TCHAR>	__tifstream_iterator;
typedef ostreambuf_iterator<TCHAR>			__tofstream_iterator;

typedef enum __tagHCC_TOKEN_TYPE //The TokenType type
{
	HCC_TOKEN_ERROR		= 0,
	HCC_IDENTIFIER		= 1,	
	HCC_KEYWORD			= 2,
	HCC_NUMBER			= 3,
	HCC_STRING_LITERAL	= 4,
	HCC_CHARACTER		= 5,
	HCC_CONTROL_CHAR	= 6,
	HCC_EOF				= 7,
	HCC_LINE_MARKER		= 8,

	HCC_LBLOCK_KEY,		//{
	HCC_RBLOCK_KEY,		//}
	HCC_LBRACKET,		//[
	HCC_RBRACKET,		//]
	HCC_LPAREN,			//(
	HCC_RPAREN,			//)

	HCC_COLON,			//:
	HCC_DOUBLE_COLON,	//::	
	HCC_SEMICOLON,		//;
	HCC_PERIOD,			//.
	HCC_COMMA_OP,		// ,
	//A R I T H M E T I C   O P E R A T O R S 
	HCC_PLUS_OP,		//+
	HCC_MINUS_OP,		//-
	HCC_MUL_OP,			//*
	HCC_DIV_OP,			// /	
	HCC_MOD_OP,			//%
	//T H E   S I M P L E   A S S I G N M E N T   O P E R A T O R
	HCC_ASSIGN_OP,		//=
	//R E L A T I O N A L   O P E R A T O R S 
	HCC_LESS_OP,		//<
	HCC_LESS_EQ_OP,		//<=
	HCC_GREATER_OP,		//>
	HCC_GTER_EQ_OP,		//>=
	HCC_EQUAL_OP,		//==
	HCC_NOT_EQ_OP,		//!=
	//A S S I G N M E N T   O P E R A T O R S
	HCC_INCREMENT,		//++
	HCC_DECREMENT,		//--
	HCC_INC_ASSIGN,		//+=
	HCC_DEC_ASSIGN,		//-=
	HCC_MUL_ASSIGN,		//*=
	HCC_DIV_ASSIGN,		///=
	HCC_MOD_ASSIGN,		//%=
	HCC_XOR_ASSIGN,		//^=
	//B O O L E A N   A N D   B I T   O P E R A T O R S 
	HCC_LEFT_SHIFT_OP,		// <<
	HCC_RIGHT_SHIFT_OP,		// >>
	HCC_BIT_OR_OP,		// |
	HCC_BIT_OR_ASSIGN,	// |=
	HCC_OR_OP,			// ||
	HCC_BIT_AND_OP,		// &
	HCC_BIT_AND_ASSIGN,	// &=
	HCC_AND_OP,			// &&
	HCC_XOR_OP,			// ^
	HCC_NOT_OP,			//!
	HCC_COMPL_OP,		//~	

	HCC_TERNARY_OP,		//?

	
	//All supported keywords or reserved words
	HCC_ABSTRACT, HCC_ASM, HCC_AUTO, HCC_BOOL, HCC_BREAK,
	HCC_CASE, HCC_CATCH, HCC_CHAR, HCC_CLASS, HCC_CONST, HCC_CONTINUE,
	HCC_DEBUGGER,
	HCC_DEFAULT, HCC_DESTROY, HCC_DIV, HCC_DO, HCC_DOUBLE, HCC_DYNAMIC_CAST, 
	HCC_EXCEPT, HCC_ELSE, HCC_ENUM, HCC_EXTERN,
	HCC_FALSE, HCC_FLOAT, HCC_FOR, HCC_GET, HCC_GOTO,
	HCC_IF, HCC_IMPORT,
	HCC_INT, HCC_INT16, HCC_INT32, HCC_INT64, HCC_LONG, 
	HCC_NAMESPACE, HCC_NEW, HCC_NULL,
	HCC_PRIVATE, HCC_PROTECTED, HCC_PROPERTY, HCC_PUBLIC, HCC_PUT,
	HCC_REF, HCC_REGISTER, HCC_RETURN, 
	HCC_SHORT, HCC_SIGNED, 	HCC_SIZEOF,	HCC_STATIC,	HCC_STRUCT,	HCC_STRING, HCC_SWITCH,
	HCC_THIS, HCC_THROW, HCC_TRUE,	HCC_TRY, HCC_TYPENAME, 
	HCC_UNSIGNED, HCC_USING, HCC_VIRTUAL, HCC_VOID, HCC_VOLATILE, HCC_WHILE, HCC_WITH,

}HCC_TOKEN_TYPE, *LPHCC_TOKEN_TYPE;

typedef HCC_TOKEN_TYPE HccTokenType;

typedef enum __tagHCC_DATA_TYPE //The DataType type
{
	HCC_UNKNOWN_TYPE	= 0,
	HCC_CHAR_TYPE		= 0x10,	
	HCC_INTEGER			= 0x20,
	HCC_FLOATING_POINT	= 0x40,
	HCC_STRING_TYPE		= 0x80,
	HCC_BOOLEAN			= 0x100,
	HCC_VOID_TYPE		= 0x200,
	HCC_CUSTOM_TYPE		= 0x400,	

}HCC_DATA_TYPE, *LPHCC_DATA_TYPE;

typedef enum __tagHCC_DATA_TYPE_MODIFIER
{
	HCC_NO_MODIFIER		= 0,

	HCC_SIGNED_TYPE		= 3,
	HCC_UNSIGNED_TYPE	= 4,

}HCC_DATA_TYPE_MODIFIER;

typedef HCC_DATA_TYPE DataType;
typedef HCC_DATA_TYPE_MODIFIER DataTypeModifier;

/*
  These enumeration, defines how the identifier was declared in the source code

  STORAGE_SPECIFIER_TYPE
  DECLARATION_TYPE

*/

//V A R I A B L E   S T O R A G E   T Y P E S
typedef enum __tagSTORAGE_SPECIFIER_TYPE
{
	STG_AUTO,
	STG_REGISTER,
	STG_STATIC,
	STG_EXTERN,
	STG_VOLATILE,
}STORAGE_SPECIFIER_TYPE, *LPSTORAGE_SPECIFIER_TYPE;

//we can have a combination of:
//	SCOPE_LOCAL		| SCOPE_HEAP, 
//	SCOPE_GLOBAL	| SCOPE_HEAP
typedef enum __tagHCC_IDENTIFIER_SCOPE_TYPE
{
	SCOPE_LOCAL		= 0x01,
	SCOPE_GLOBAL	= 0x03,
	SCOPE_HEAP		= 0x10,
}HCC_IDENTIFIER_SCOPE_TYPE, *LPHCC_IDENTIFIER_SCOPE_TYPE;


//D E C L A R A T I O N   T Y P E S
typedef enum __tagDECLARATION_TYPE
{
	DECL_UNDEFINED,	
	DECL_VOID,
	DECL_VARIABLE,
	DECL_CONSTANT,
	DECL_BUILDIN_TYPE,				//for all the compiler build-in types
	DECL_NEW_TYPE,					//for class and struct keywords (will be the same behavior for now)
	DECL_NEW_ABSTRACT_TYPE,			//for abstract class and struct keywords
	DECL_VTBL_PTR,					//the vtable pointer
	
	DECL_NEW_FUNC_MEMBER,			//a function in a class
	DECL_NEW_VIRTUAL_FUNC_MEMBER,	//a virtual function in a class
	DECL_NEW_VIRTUAL_ABSTRACT_FUNC_MEMBER,	//a virtual function in an abstract class
	DECL_NEW_STATIC_CLASS_MEMBER,	//a static function in a class	
	DECL_NEW_DATA_MEMBER,			//an attribute in a class
	DECL_NEW_CLASS_CONSTRUCTOR,		//a constructor member
	DECL_UNIQUE_DESTRUCTOR,			//a object's destructor for a class

	DECL_READONLY_PROPERTY,			//a read-only property
	DECL_WRITEONLY_PROPERTY,		//a write-only property
	DECL_READWRITE_PROPERTY,		//a read-write property

	DECL_PARAM_VALUE,		//param by value
	DECL_PARAM_BYREF,		//param by reference
	DECL_PARAM_CONST_BYREF,	//const param by reference

	DECL_PARAM_POINTER,			//param pointer
	DECL_PARAM_CONST_POINTER,	//const param pointer

	DECL_PARAM_ARRAY,		//one-dimension param array
	DECL_PARAM_CONST_ARRAY,	//one-dimension const param array
	DECL_NAMESPACE,	

	DECL_OPERATOR_NEW,		//the new operator for dynamic memory allocation (OOP)
	DECL_OPERATOR_DESTROY,	//the destroy operator for dynamically memory deallocation (OOP)

	DECL_POINTER_VARIABLE,	//for dynamically allocated memory

	//for multiple params console write built-in operators (like Pascal operators).
	DECL_BUILTIN_CONSOLE_WRITE,
	DECL_BUILTIN_CONSOLE_WRITELN,

	DECL_PROC_LABEL,		//for user defined procedure labels where to jump using 'goto' statement

	DECL_OPERATOR_MIN,		//the inline-min between two integer types
	DECL_OPERATOR_MAX,		//the inline-max between two integer types

	DECL_OPERATOR_SIZEOF,	//the inline-sizeof operator

	DECL_SYMBOL_USER_ALIAS,		//to map an identifier to other

	DECL_SYMBOL_DYNAMIC_CAST_OPERATOR, //the dynamic cast operator (OOP)

}DECLARATION_TYPE, *LPDECLARATION_TYPE;

/*
   This enumeration define the "specific" specifier-type that represents an identifier
   S P E C I F I E R   T Y P E S 
*/
typedef enum __tagDECLARE_SPEC_TYPE
{	
	DSPEC_SIMPLE,	//the primitive | build-in data types
	DSPEC_ARRAY,
	DSPEC_CLASS,
	DSPEC_ENUM,
	DSPEC_NAMESPACE,
}DECLARE_SPEC_TYPE, *LPDECLARE_SPEC_TYPE;


typedef enum __tagHCC_FUNC_TYPE
{
	HCC_USER_DEFINED,
	HCC_BUILDIN_NEW_OPERATOR,
	HCC_BUILDIN_DESTROY_OPERATOR,
	HCC_BUILDIN_DYNAMIC_CAST,
	HCC_BUILDIN_TYPE_CONVERT_CAST, //TODO: pointer to int, int to pointer
}HCC_FUNC_TYPE, *LPHCC_FUNC_TYPE;


#define MAX_TOKEN_SIZE 256
#define MAX_STRING_SIZE (1024 * 4) //4096 == 4kb

typedef unsigned int __uint;

typedef union DataValue
{
	__int64	Integer;
	double	Double;
	TCHAR	Character;
	TCHAR*	String;
	bool	Boolean;
} DataValueType;


typedef struct __tagHCC_TOKEN
{
	HccTokenType	tokenType;
	DataType		dataType;	
	bool			bIsDelimiter;
	DataValueType	value;
	TCHAR token[MAX_STRING_SIZE + 1];

	__tagHCC_TOKEN() :  tokenType(HCC_TOKEN_ERROR), 
						dataType(HCC_UNKNOWN_TYPE),
						bIsDelimiter(false) { value.String = 0;}

	~__tagHCC_TOKEN()
		{}

	const TCHAR* String() const
		{return token;}
	const bool IsDelimiter() const
		{return bIsDelimiter;}

}HCC_TOKEN, *LPHCC_TOKEN;

typedef enum __tagCHARACTER_CODE
{
	ccLetter, ccUnderscore, ccDigit, ccSpecial, ccSingleQuote, ccDoubleQuote, 
	ccWhiteSpace, ccEndOfFile, ccError
}CHARACTER_CODE;

typedef CHARACTER_CODE CharCode;

typedef union __tagFLOATING_POINT_CONVERSION
{
	double	dSource;
	__int64 i64Final;
	struct{
		long	dwHighPart;
		long	dwLowPart;
	};
}FLOATING_POINT_CONVERSION;

#define HCC_POINTER_ADDRESSOF	HCC_BIT_AND_OP
#define HCC_POINTER_DEREFERENCE HCC_MUL_OP

#endif //__CORE_HCC_COMMON_FEATURES_h__