// HCCParser.cpp: implementation of the HCCParser class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HCCParser.h"

#include <windows.h>
//#include <comutil.h>

#include "HCCIntermediate.h"	//for the member function's icode's
#include <cassert>
#include <climits>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;	//this is private/unique by file
#endif


extern ICodeResponsible theICodeResponsible;

#define CLASS_VPTR_VTBL_NAME _T("@@class_vptr_vtbl@@__")
#define MAX_ARRAY_DIMENSION 2
#define HPP_CLASS_DESTRUCTOR _T("Destructor")

extern map<__tstring, HCC_TOKEN_TYPE> all_keywords;

//T H E   V O I D   T Y P E 
TypeSpecifier* HccTypeChecker::ts_type_void_ptr		= NULL;
//I N T E G R A L   T Y P E S 
TypeSpecifier* HccTypeChecker::ts_type_boolean_ptr	= NULL;
TypeSpecifier* HccTypeChecker::ts_type_char_ptr		= NULL;
TypeSpecifier* HccTypeChecker::ts_type_short_ptr	= NULL;
TypeSpecifier* HccTypeChecker::ts_type_long_ptr		= NULL;
TypeSpecifier* HccTypeChecker::ts_type_int_ptr		= NULL;
TypeSpecifier* HccTypeChecker::ts_type_Int16_ptr	= NULL;
TypeSpecifier* HccTypeChecker::ts_type_Int32_ptr	= NULL;
TypeSpecifier* HccTypeChecker::ts_type_Int64_ptr	= NULL;
TypeSpecifier* HccTypeChecker::ts_type_unsigned_ptr	= NULL;
TypeSpecifier* HccTypeChecker::ts_type_signed_ptr	= NULL;

//F L O A T I N G   P O I N T   T Y P E S
TypeSpecifier* HccTypeChecker::ts_type_float_ptr	= NULL;
TypeSpecifier* HccTypeChecker::ts_type_double_ptr	= NULL;

	//U N S I G N E D   I N T E G E R S 
TypeSpecifier* HccTypeChecker::ts_type_uchar_ptr	= NULL;
TypeSpecifier* HccTypeChecker::ts_type_ushort_ptr	= NULL;
TypeSpecifier* HccTypeChecker::ts_type_ulong_ptr	= NULL;

//S T R I N G   T Y P E 
TypeSpecifier* HccTypeChecker::ts_type_string_ptr	= NULL;

/*
template<typename _Ty>
_Ty& max(_Ty& x, _Ty& y)
{return (x > y ? x : y);}
*/

set<__tstring> imported_transl_units;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
bool HCCParser::bEnableXREF = false;
bool HCCParser::m_bParserInit = false;
//the main entry point pointer...
Symbol* HCCParser::main_entry_point_ptr = NULL;

LPHCC_TOKEN HCCParser::getToken(void)
{
	delete token_ptr;	
	token_ptr	= lexer->getToken();
	token_type	= token_ptr->tokenType;
	return token_ptr;
}

LPHCC_TOKEN HCCParser::getTokenAppend(void)
{
	if(!bPrevTokenManuallyAdded)
		getToken();
	icode_ptr->append(token_type);
	bPrevTokenManuallyAdded = false;
	return token_ptr;
}

LPHCC_TOKEN HCCParser::getTokenIf(HCC_TOKEN_TYPE token, ParserError error)
{
	if(token==token_type)
		return getToken();
	HccErrorManager::Error(error);
	return 0;
}

LPHCC_TOKEN HCCParser::getTokenAppendIf(HCC_TOKEN_TYPE token, ParserError error)
{
	if(token==token_type)
		return getTokenAppend();
	HccErrorManager::Error(error);
	return 0;
}


void HCCParser::Parse()
{
	symbol_table_ptr->set_source_buffer(lexer->sourceBuf());
	icode_ptr->set_source_buffer(lexer->sourceBuf());
	//extract the first token...
	getTokenAppend();
	if(token_type==HCC_EOF){
		delete token_ptr;
		HccErrorManager::Error(HccErrorManager::errUnexpectedEndOfFile);
		return;
	}
	if(token_type!=HCC_TOKEN_ERROR)
		//parse a statement list until we reach Eof...
		ParseStatementList(HCC_TOKEN_ERROR);

	TCHAR text[256];
	_stprintf(text, _T("%20d source lines.\n"), lexer->sourceBuf()->lineNumber());
	listing << text;
	_stprintf(text, _T("%20d syntax errors.\n"), HccErrorManager::errorCount());
	listing << text;

	//C R O S S   R E F E R E N C E   I N F O 
	if(bEnableXREF)
	{
		listing << _T("Unit Cross References:") 
			 << _endl
			 << _endl
			 << *symbol_table_ptr 
			 << _endl;
	}
}

void HCCParser::CompactUnit()
{
	bool bPrevDelimiter = true;
	bool bCurrentDelimiter = false;
	do{
		getToken();
		if(token_type!=HCC_TOKEN_ERROR){
			bCurrentDelimiter = token_ptr->IsDelimiter();
			if(!bPrevDelimiter && !bCurrentDelimiter)
				*unit_compactor_ptr << _blank;

			*unit_compactor_ptr << token_ptr->String();
			bPrevDelimiter = token_ptr->IsDelimiter();			
		}else{			
			//TODO:Error Recuperation Mode....			
		}

	}while(token_type!=HCC_EOF);
	delete token_ptr;
	token_ptr = NULL;
	unit_compactor_ptr->flush();
}

void HCCParser::Init()
{
	if(!HCCParser::m_bParserInit){
		//register all the keywords in the symbol table
		//to avoid the use of reserved words in the H++ programs...
		map<__tstring, HCC_TOKEN_TYPE>::iterator it_keyw = all_keywords.begin();
		while(it_keyw != all_keywords.end()){
			Symbol* symbol_ptr = symbol_table_ptr->insert(it_keyw->first);
			symbol_ptr->setType(HCC_KEYWORD);
			it_keyw++;
		}
		Symbol* symbol_ptr = NULL;

		//T H E   V O I D   T Y P E 
		symbol_ptr = symbol_table_ptr->find(_T("void"));
		symbol_ptr->getDeclDefinition().set_identifier_type(DECL_VOID);
		symbol_ptr->getTypeSpecifier().setDataType(HCC_VOID_TYPE, HCC_NO_MODIFIER, 0);
		symbol_ptr->getTypeSpecifier().setTypeName(symbol_ptr->getName());

		//T H E   V O I D   T Y P E 
		HccTypeChecker::ts_type_void_ptr		= &symbol_ptr->getTypeSpecifier();


		//T H E   B O O L E A N   V A L U E S
		Symbol* symb_false = symbol_ptr = symbol_table_ptr->find(_T("false"));
		symbol_ptr->getDeclDefinition().set_identifier_type(DECL_CONSTANT);
		symbol_ptr->getDeclDefinition().constant.value.Integer = 0;
		
		Symbol* symb_true = symbol_ptr = symbol_table_ptr->find(_T("true"));
		symbol_ptr->getDeclDefinition().set_identifier_type(DECL_CONSTANT);
		symbol_ptr->getDeclDefinition().constant.value.Integer = 1;

		symb_false->setValue(0);//to be deleted
		symb_true->setValue(1);//to be deleted

		//T H E   B O O L   T Y P E 
		symbol_ptr = symbol_table_ptr->find(_T("bool"));
		symbol_ptr->getDeclDefinition().set_identifier_type(DECL_BUILDIN_TYPE);
		symbol_ptr->getTypeSpecifier().set_specifier(DSPEC_ENUM);
		symbol_ptr->getTypeSpecifier().enumeration.max_enum_const_value = 1;//only 0 and 1
		symbol_ptr->getTypeSpecifier().setDataType(HCC_BOOLEAN, HCC_UNSIGNED_TYPE, sizeof(bool));
		symbol_ptr->getTypeSpecifier().setTypeName(symbol_ptr->getName());
		
		HccTypeChecker::ts_type_boolean_ptr	= &symbol_ptr->getTypeSpecifier();

		//false
		symbol_ptr->getTypeSpecifier().enumeration.values.push_back(symb_false);
		//true
		symbol_ptr->getTypeSpecifier().enumeration.values.push_back(symb_true);
		//the constant specifier
		symb_false->getTypeSpecifier().setBaseTypeSpec(HccTypeChecker::ts_type_boolean_ptr);
		symb_true->getTypeSpecifier().setBaseTypeSpec(HccTypeChecker::ts_type_boolean_ptr);

		symb_false->getTypeSpecifier().setDataType(HCC_BOOLEAN, HCC_UNSIGNED_TYPE,  sizeof(bool));
		symb_true->getTypeSpecifier().setDataType(HCC_BOOLEAN, HCC_UNSIGNED_TYPE, sizeof(bool));


		//T H E   C H A R   T Y P E
		symbol_ptr = symbol_table_ptr->find(_T("char"));
		symbol_ptr->getDeclDefinition().set_identifier_type(DECL_BUILDIN_TYPE);
		symbol_ptr->getTypeSpecifier().setDataType(HCC_CHAR_TYPE , HCC_SIGNED_TYPE, sizeof(TCHAR));
		symbol_ptr->getTypeSpecifier().setTypeName(symbol_ptr->getName());
		
		HccTypeChecker::ts_type_char_ptr			= &symbol_ptr->getTypeSpecifier();

		//T H E   I N T E G E R   T Y P E S 
		symbol_ptr = symbol_table_ptr->find(_T("short"));
		symbol_ptr->getDeclDefinition().set_identifier_type(DECL_BUILDIN_TYPE);
		symbol_ptr->getTypeSpecifier().setDataType(HCC_INTEGER , HCC_SIGNED_TYPE, sizeof(short)); //HCC_SHORT_TYPE
		symbol_ptr->getTypeSpecifier().setTypeName(symbol_ptr->getName());

		HccTypeChecker::ts_type_short_ptr		= &symbol_ptr->getTypeSpecifier();

		symbol_ptr = symbol_table_ptr->find(_T("long"));
		symbol_ptr->getDeclDefinition().set_identifier_type(DECL_BUILDIN_TYPE);
		symbol_ptr->getTypeSpecifier().setDataType(HCC_INTEGER , HCC_SIGNED_TYPE, sizeof(long)); //HCC_LONG_TYPE
		symbol_ptr->getTypeSpecifier().setTypeName(symbol_ptr->getName());

		HccTypeChecker::ts_type_long_ptr			= &symbol_ptr->getTypeSpecifier();

		symbol_ptr = symbol_table_ptr->find(_T("int"));
		symbol_ptr->getDeclDefinition().set_identifier_type(DECL_BUILDIN_TYPE);
		symbol_ptr->getTypeSpecifier().setDataType(HCC_INTEGER , HCC_SIGNED_TYPE, sizeof(int)); //HCC_LONG_TYPE
		symbol_ptr->getTypeSpecifier().setTypeName(symbol_ptr->getName());

		HccTypeChecker::ts_type_int_ptr			= &symbol_ptr->getTypeSpecifier();

		symbol_ptr = symbol_table_ptr->find(_T("Int16"));
		symbol_ptr->getDeclDefinition().set_identifier_type(DECL_BUILDIN_TYPE);
		symbol_ptr->getTypeSpecifier().setDataType(HCC_INTEGER , HCC_SIGNED_TYPE, sizeof(short)); //HCC_SHORT_TYPE
		symbol_ptr->getTypeSpecifier().setTypeName(symbol_ptr->getName());

		HccTypeChecker::ts_type_Int16_ptr		= &symbol_ptr->getTypeSpecifier();

		symbol_ptr = symbol_table_ptr->find(_T("Int32"));
		symbol_ptr->getDeclDefinition().set_identifier_type(DECL_BUILDIN_TYPE);
		symbol_ptr->getTypeSpecifier().setDataType(HCC_INTEGER , HCC_SIGNED_TYPE, sizeof(long)); //HCC_LONG_TYPE
		symbol_ptr->getTypeSpecifier().setTypeName(symbol_ptr->getName());

		HccTypeChecker::ts_type_Int32_ptr		= &symbol_ptr->getTypeSpecifier();


		symbol_ptr = symbol_table_ptr->find(_T("Int64"));
		symbol_ptr->getDeclDefinition().set_identifier_type(DECL_BUILDIN_TYPE);
		symbol_ptr->getTypeSpecifier().setDataType(HCC_INTEGER , HCC_SIGNED_TYPE, sizeof(__int64));
		symbol_ptr->getTypeSpecifier().setTypeName(symbol_ptr->getName());

		HccTypeChecker::ts_type_Int64_ptr		= &symbol_ptr->getTypeSpecifier();

		//T H E   F L O A T I N G   P O I N T   D A T A   T Y P E S 
		symbol_ptr = symbol_table_ptr->find(_T("float"));
		symbol_ptr->getDeclDefinition().set_identifier_type(DECL_BUILDIN_TYPE);
		symbol_ptr->getTypeSpecifier().setDataType(HCC_FLOATING_POINT, HCC_NO_MODIFIER, sizeof(float));
		symbol_ptr->getTypeSpecifier().setTypeName(symbol_ptr->getName());
		
		HccTypeChecker::ts_type_float_ptr		= &symbol_ptr->getTypeSpecifier();

		symbol_ptr = symbol_table_ptr->find(_T("double"));
		symbol_ptr->getDeclDefinition().set_identifier_type(DECL_BUILDIN_TYPE);
		symbol_ptr->getTypeSpecifier().setDataType(HCC_FLOATING_POINT, HCC_NO_MODIFIER, sizeof(double));
		symbol_ptr->getTypeSpecifier().setTypeName(symbol_ptr->getName());

		HccTypeChecker::ts_type_double_ptr	= &symbol_ptr->getTypeSpecifier();

		//specifics for integer types

		symbol_ptr = symbol_table_ptr->find(_T("unsigned"));
		symbol_ptr->getDeclDefinition().set_identifier_type(DECL_BUILDIN_TYPE);
		symbol_ptr->getTypeSpecifier().setDataType(HCC_INTEGER , HCC_UNSIGNED_TYPE, sizeof(unsigned int));
		symbol_ptr->getTypeSpecifier().setTypeName(symbol_ptr->getName());

		HccTypeChecker::ts_type_unsigned_ptr	= &symbol_ptr->getTypeSpecifier();

		symbol_ptr = symbol_table_ptr->find(_T("signed"));
		symbol_ptr->getDeclDefinition().set_identifier_type(DECL_BUILDIN_TYPE);
		symbol_ptr->getTypeSpecifier().setDataType(HCC_INTEGER , HCC_SIGNED_TYPE, sizeof(int));
		symbol_ptr->getTypeSpecifier().setTypeName(symbol_ptr->getName());

		HccTypeChecker::ts_type_signed_ptr	= &symbol_ptr->getTypeSpecifier();

		//U N S I G N E D   I N T E G E R   T Y P E S 
		
		//T H E   C H A R   T Y P E
		symbol_ptr = symbol_table_ptr->insert(UNSIGNED_CHAR_NAME);
		symbol_ptr->getDeclDefinition().set_identifier_type(DECL_BUILDIN_TYPE);
		symbol_ptr->getTypeSpecifier().setDataType(HCC_CHAR_TYPE , HCC_UNSIGNED_TYPE, sizeof(TCHAR));
		symbol_ptr->getTypeSpecifier().setTypeName(symbol_ptr->getName());
		
		HccTypeChecker::ts_type_uchar_ptr			= &symbol_ptr->getTypeSpecifier();

		//T H E   I N T E G E R   T Y P E S 
		symbol_ptr = symbol_table_ptr->insert(UNSIGNED_SHORT_NAME);
		symbol_ptr->getDeclDefinition().set_identifier_type(DECL_BUILDIN_TYPE);
		symbol_ptr->getTypeSpecifier().setDataType(HCC_INTEGER , HCC_UNSIGNED_TYPE, sizeof(short)); //HCC_SHORT_TYPE
		symbol_ptr->getTypeSpecifier().setTypeName(symbol_ptr->getName());

		HccTypeChecker::ts_type_ushort_ptr		= &symbol_ptr->getTypeSpecifier();

		symbol_ptr = symbol_table_ptr->insert(UNSIGNED_LONG_NAME);
		symbol_ptr->getDeclDefinition().set_identifier_type(DECL_BUILDIN_TYPE);
		symbol_ptr->getTypeSpecifier().setDataType(HCC_INTEGER , HCC_UNSIGNED_TYPE, sizeof(long)); //HCC_LONG_TYPE
		symbol_ptr->getTypeSpecifier().setTypeName(symbol_ptr->getName());

		HccTypeChecker::ts_type_ulong_ptr			= &symbol_ptr->getTypeSpecifier();
		//
		//T H E   S T R I N G   T Y P E
		symbol_ptr = symbol_table_ptr->find(_T("string"));
		symbol_ptr->getDeclDefinition().set_identifier_type(DECL_BUILDIN_TYPE);
		//to avoid deleting this pointer in the Symbol destructor (was nulled)...
		symbol_ptr->getDeclDefinition().constant.value.String = 0;
		symbol_ptr->getTypeSpecifier().setDataType(HCC_STRING_TYPE, HCC_NO_MODIFIER, sizeof(int));		

		//the string type is defined as an array of chars
		symbol_ptr->getTypeSpecifier().set_specifier(DSPEC_ARRAY);
		symbol_ptr->getTypeSpecifier().array.pItemType = HccTypeChecker::ts_type_char_ptr;

		HccTypeChecker::ts_type_string_ptr	= &symbol_ptr->getTypeSpecifier();
		symbol_ptr->getTypeSpecifier().setTypeName(symbol_ptr->getName());
		//the base type is itself, to behave as all the arrays do...
		symbol_ptr->getTypeSpecifier().setBaseTypeSpec(HccTypeChecker::ts_type_string_ptr);

		//the 'null' constant
		symbol_ptr = symbol_table_ptr->find(_T("null"));
		symbol_ptr->getDeclDefinition().set_identifier_type(DECL_CONSTANT);

		symbol_ptr->getTypeSpecifier().setDataType(HCC_INTEGER, HCC_UNSIGNED_TYPE, sizeof(int));
		symbol_ptr->getTypeSpecifier().setTypeName(_T("int"));
		symbol_ptr->getDeclDefinition().constant.value.Integer = 0;		
		symbol_ptr->getTypeSpecifier().setBaseTypeSpec(&symbol_ptr->getTypeSpecifier());

		//the operators new and destroy (already defined as language keywords)

		//the new operator for dynamic memory allocation
		symbol_ptr = symbol_table_ptr->find(_T("new"));
		symbol_ptr->getDeclDefinition().set_identifier_type(DECL_OPERATOR_NEW);
		symbol_ptr->getDeclDefinition().function.fnType = HCC_BUILDIN_NEW_OPERATOR;
		symbol_ptr->getDeclDefinition().function.return_type = HccTypeChecker::ts_type_Int32_ptr;

		//the destroy operator for dynamically memory deallocation
		symbol_ptr = symbol_table_ptr->find(_T("destroy"));
		symbol_ptr->getDeclDefinition().set_identifier_type(DECL_OPERATOR_DESTROY);
		symbol_ptr->getDeclDefinition().function.fnType = HCC_BUILDIN_DESTROY_OPERATOR;
		symbol_ptr->getDeclDefinition().function.return_type = HccTypeChecker::ts_type_void_ptr;

		//C O N S O L E :: W R I T E
		symbol_ptr = symbol_table_ptr->insert(_T("Console::Write"));
		symbol_ptr->getDeclDefinition().set_identifier_type(DECL_BUILTIN_CONSOLE_WRITE);
		symbol_ptr->getDeclDefinition().function.return_type = HccTypeChecker::ts_type_void_ptr;


		//C O N S O L E :: W R I T E L N
		symbol_ptr = symbol_table_ptr->insert(_T("Console::WriteLn"));
		symbol_ptr->getDeclDefinition().set_identifier_type(DECL_BUILTIN_CONSOLE_WRITELN);
		symbol_ptr->getDeclDefinition().function.return_type = HccTypeChecker::ts_type_void_ptr;

		SymbolTable<__tstring, Symbol>* symbl_tbl_ptr = NULL;
		//I N L I N E - M I N  O P E R A T O R  F O R  I N T E G E R   T Y P E S 
		symbl_tbl_ptr = new SymbolTable<__tstring, Symbol>(_T("__inline_function__"));
		symbol_ptr = symbol_table_ptr->insert(_T("min"));
		symbol_ptr->getDeclDefinition().set_identifier_type(DECL_OPERATOR_MIN);		//the inline-min between two integer types
		symbol_ptr->getDeclDefinition().function.return_type = HccTypeChecker::ts_type_int_ptr;
		symbol_ptr->getDeclDefinition().function.symbol_table_ptr = symbl_tbl_ptr;
													
		symbol_ptr->getTypeSpecifier().setBaseTypeSpec(HccTypeChecker::ts_type_int_ptr);

		//P A R A M E T E R S 
		Symbol* param_A_ptr = symbol_ptr->getDeclDefinition().function.symbol_table_ptr->insert(_T("A"));
		param_A_ptr->getDeclDefinition().set_identifier_type(DECL_PARAM_VALUE);
		param_A_ptr->getDeclDefinition().set_identifier_scope(SCOPE_LOCAL);
		param_A_ptr->getTypeSpecifier().setBaseTypeSpec(HccTypeChecker::ts_type_int_ptr);

		Symbol* param_B_ptr = symbol_ptr->getDeclDefinition().function.symbol_table_ptr->insert(_T("B"));
		param_B_ptr->getDeclDefinition().set_identifier_type(DECL_PARAM_VALUE);
		param_B_ptr->getDeclDefinition().set_identifier_scope(SCOPE_LOCAL);
		param_B_ptr->getTypeSpecifier().setBaseTypeSpec(HccTypeChecker::ts_type_int_ptr);

		//F U N C T I O N   D E F I N I T I O N
		symbol_ptr->getDeclDefinition().function.locals.stack_params.params.clear();
		symbol_ptr->getDeclDefinition().function.locals.stack_params.params.push_back(param_A_ptr);
		symbol_ptr->getDeclDefinition().function.locals.stack_params.params.push_back(param_B_ptr);
		symbol_ptr->getDeclDefinition().function.locals.stack_params.total_params_size = (2 * sizeof(int));
		


		//I N L I N E - M A X  O P E R A T O R  F O R  I N T E G E R   T Y P E S 
		symbl_tbl_ptr = new SymbolTable<__tstring, Symbol>(_T("__inline_function__"));
		symbol_ptr = symbol_table_ptr->insert(_T("max"));
		symbol_ptr->getDeclDefinition().set_identifier_type(DECL_OPERATOR_MAX);		//the inline-max between two integer types
		symbol_ptr->getDeclDefinition().function.return_type = HccTypeChecker::ts_type_int_ptr;
		symbol_ptr->getDeclDefinition().function.symbol_table_ptr = symbl_tbl_ptr;
		
		symbol_ptr->getTypeSpecifier().setBaseTypeSpec(HccTypeChecker::ts_type_int_ptr);

		//P A R A M E T E R S 
		param_A_ptr = symbol_ptr->getDeclDefinition().function.symbol_table_ptr->insert(_T("A"));
		param_A_ptr->getDeclDefinition().set_identifier_type(DECL_PARAM_VALUE);
		param_A_ptr->getDeclDefinition().set_identifier_scope(SCOPE_LOCAL);
		param_A_ptr->getTypeSpecifier().setBaseTypeSpec(HccTypeChecker::ts_type_int_ptr);

		param_B_ptr = symbol_ptr->getDeclDefinition().function.symbol_table_ptr->insert(_T("B"));
		param_B_ptr->getDeclDefinition().set_identifier_type(DECL_PARAM_VALUE);
		param_B_ptr->getDeclDefinition().set_identifier_scope(SCOPE_LOCAL);
		param_B_ptr->getTypeSpecifier().setBaseTypeSpec(HccTypeChecker::ts_type_int_ptr);

		//F U N C T I O N   D E F I N I T I O N
		symbol_ptr->getDeclDefinition().function.locals.stack_params.params.clear();
		symbol_ptr->getDeclDefinition().function.locals.stack_params.params.push_back(param_A_ptr);
		symbol_ptr->getDeclDefinition().function.locals.stack_params.params.push_back(param_B_ptr);
		symbol_ptr->getDeclDefinition().function.locals.stack_params.total_params_size = (2 * sizeof(int));

		//S I Z E O F   O P E R A T O R  - T H E   I D E N T I F I E R   S P E C I F I C   
		//S I Z E   I N   B Y T E S		
		symbol_ptr = symbol_table_ptr->find(_T("sizeof"));
		symbol_ptr->getDeclDefinition().set_identifier_type(DECL_OPERATOR_SIZEOF);
		symbol_ptr->getTypeSpecifier().setDataType(HCC_INTEGER, HCC_UNSIGNED_TYPE, sizeof(int));
		symbol_ptr->getDeclDefinition().function.return_type = HccTypeChecker::ts_type_int_ptr;	
		symbol_ptr->getTypeSpecifier().setBaseTypeSpec(HccTypeChecker::ts_type_int_ptr);				

		//D Y N A M I C   C A S T   O P E R A T O R   ( O O P ) - Jan 4, 2009
		symbol_ptr = symbol_table_ptr->find(_T("dynamic_cast"));
		symbol_ptr->getDeclDefinition().set_identifier_type(DECL_SYMBOL_DYNAMIC_CAST_OPERATOR);
		symbol_ptr->getDeclDefinition().function.fnType = HCC_BUILDIN_DYNAMIC_CAST;
		symbol_ptr->getDeclDefinition().function.return_type = HccTypeChecker::ts_type_Int32_ptr;


		HCCParser::m_bParserInit = true;
	}
}

HCC_TOKEN_TYPE HCCParser::ParseStatement(Symbol *function_ptr)
{
	/*
		BNF

					statement:				
					
					labeled-statement			|
					expression-statement		|
					compound-statement			|
					selection-statement			|
					iteration-statement			|
					jump-statement				|
					declaration-statement		|
					namespace-block				|
					with-block					|
					try-block					|
					empty-statement

					empty-statement:
							; | eof						

					expression-statement:
							expression;				|
							assignment-expression

					assignment-expression:
							identifier = expression-statement
	*/

#ifdef __HCC_TOKEN_DEBUG_OUTPUT__
	lexer->OutputToken(token_ptr);
#endif //__HCC_TOKEN_DEBUG_OUTPUT__	
	//
	//
	skipEmptyStatements();
	icode_ptr->insert_line_marker();
	HCC_TOKEN_TYPE stmt_type = token_type;
	switch(stmt_type)
	{
		case HCC_IDENTIFIER:
		case HCC_NUMBER:
		case HCC_INCREMENT:
		case HCC_DECREMENT:
		case HCC_DESTROY:
		case HCC_POINTER_ADDRESSOF:
		case HCC_POINTER_DEREFERENCE:
		case HCC_LPAREN: //for (expr) without assignments
		case HCC_PLUS_OP:
		case HCC_MINUS_OP:
		{
			//all type of expression-statement
			//++x; --y; x += 5; 5 + 4;
			HCC_PARSE_TREE* tree_ptr = parseCommaExprList(function_ptr);
			delete tree_ptr;
		}
		break;
			//S T A T E M E N T S 
		case HCC_IF:			stmt_type = parseIfStatement(function_ptr);			break;
		case HCC_WHILE:			parseWhile(function_ptr);				break;
		case HCC_DO:			parseDoWhile(function_ptr);				break;
		case HCC_FOR:			parseFor(function_ptr);					break;
		case HCC_SWITCH:		parseSwitch(function_ptr);				break;
			//{ statement-list; }
		case HCC_LBLOCK_KEY:	parseCompoundStatement(function_ptr);	break;			
		case HCC_TRY:			parseTryBlock(function_ptr);			break;
		case HCC_WITH:			parseWithStatement(function_ptr);		break;	
			
			//I N C O M P L E T E   S T A T E M E N T S 
		case HCC_ELSE:	
			{				
				HccErrorManager::Error(HccErrorManager::errInvalidStatement, _T("; \'else\' without \'if\' statement."));
				getToken();
				//parse it anyway...
				if(token_type==HCC_LBLOCK_KEY)
					parseCompoundStatement(function_ptr);
				else
					ParseStatement(function_ptr);
				stmt_type = HCC_SEMICOLON;
			}
			break;
		case HCC_CASE:
			{
				HccErrorManager::Error(HccErrorManager::errInvalidStatement, _T("; \'case\' without \'switch\' statement."));
				//parse it anyway...
				parseCaseBranch(function_ptr);
				if(token_type==HCC_BREAK)
					getToken();
			}
			break;			
		case HCC_CATCH:
			{				
				HccErrorManager::Error(HccErrorManager::errInvalidStatement, _T("; \'catch\' without \'try\' statement."));

				getToken();
				//(
				if(getTokenAppendIf(HCC_LPAREN, HccErrorManager::errMissingLeftParen)==0){
					Resync(HCC_RPAREN);		
				}
				//expr				
				ParseExprList(function_ptr);
				//)
				if(getTokenAppendIf(HCC_RPAREN, HccErrorManager::errMissingRightParen)==0){
					Resync(HCC_LBLOCK_KEY);		
				}
				if(token_type==HCC_LBLOCK_KEY)	//{
						parseCompoundStatement(function_ptr); //{ statement-list }

				stmt_type = HCC_SEMICOLON;
			}
			break;
			//D E C L A R A T I O N S
		case HCC_NAMESPACE:		parseNameSpace();						break;
		case HCC_USING:			parseUsingNamespace(function_ptr);		break;
		//data types
		case HCC_VOID:
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
		//for locals/globals
		case HCC_AUTO:
		case HCC_REGISTER:
		case HCC_STATIC:		
		case HCC_TYPENAME:
		case HCC_VOLATILE:
		//for new types
		case HCC_CLASS:
		case HCC_STRUCT:
		case HCC_CONST:
		case HCC_ENUM:
		case HCC_EXTERN:
			{
				parseDeclarations(function_ptr);
			}
			break;
		case HCC_IMPORT:
			{
				parseImportStatement();
			}
			break;
		case HCC_DEBUGGER:
			{
				//to bring the debugger to the current user-program
				getTokenAppend();
			}
			break;
		default:
			{
				//jump-statement
				if(IsJumpStatement()) 
					parseJumpStatement(function_ptr);
				else
				{
					if(token_type!=HCC_RBLOCK_KEY)
						//unknown statement
						HccErrorManager::Error(HccErrorManager::errInvalidStatement);
				}
			}
		break;
	};
	return stmt_type;
}

LPHCC_PARSE_TREE HCCParser::parseCommaExprList(Symbol *function_ptr)
{
	LPHCC_PARSE_TREE parse_tree = NULL;
	// ','	::* expr1, expr2, expr3,..., expr_n <-- this one is the only left in the stack 
	parse_tree = ParseExprList(function_ptr);
	while(token_type==HCC_COMMA_OP)
	{
		delete parse_tree;		
		getTokenAppend();
		parse_tree = ParseExprList(function_ptr);
	}
	//*
	return parse_tree;
}

//-----------------------------------------------
// ParseRelationalExprList	- Parse an expression (also with: binary relational 
//  							operators | binary bitwise operators, between expressions )
//-----------------------------------------------
LPHCC_PARSE_TREE HCCParser::ParseRelationalExprList(Symbol *function_ptr)
{
	LPHCC_PARSE_TREE tree_ptr = NULL;
	//simple expression
	LPHCC_PARSE_TREE left_ptr = ParseExpr(function_ptr);
	switch(token_type)
	{
		//R E L A T I O N A L  O P
		//G R E A T E R   P R E C E D E N C E 
	case HCC_LESS_OP:			//<
	case HCC_LESS_EQ_OP:		//<=
	case HCC_GREATER_OP:		//>
	case HCC_GTER_EQ_OP:		//>=
		//L E S S   P R E C E D E N C E 
	case HCC_EQUAL_OP:			//==
	case HCC_NOT_EQ_OP:			//!=
		//B I T W I S E   P R E C E D E N C E 
	case HCC_BIT_AND_OP:		// &
	case HCC_XOR_OP:			// ^
	case HCC_BIT_OR_OP:			// |
		{
			tree_ptr = new HCC_PARSE_TREE(NULL, token_type);
			getTokenAppend();
			//simple expression
			tree_ptr->left_ptr	= left_ptr;
			tree_ptr->right_ptr = ParseExpr(function_ptr);
			//Strong Type Checking...
			if(tree_ptr->right_ptr!=NULL && tree_ptr->right_ptr->type_ptr!=NULL)
			{
				if(tree_ptr->token_op!=HCC_BIT_AND_OP && 
					tree_ptr->token_op!=HCC_XOR_OP &&
					tree_ptr->token_op!=HCC_BIT_OR_OP)
				{
					//BEGIN - FIXED Jan 17, 2009
					TypeSpecifier* left_type_operand_ptr	= tree_ptr->left_ptr->type_ptr;
					TypeSpecifier* right_type_operand_ptr	= tree_ptr->right_ptr->type_ptr;

					if(tree_ptr->left_ptr->is_pointer_expr())
						left_type_operand_ptr	= HccTypeChecker::ts_type_int_ptr;
					else if(tree_ptr->right_ptr->is_pointer_expr())
						right_type_operand_ptr	= HccTypeChecker::ts_type_int_ptr;
						
					//relational operators are always boolean
					HccTypeChecker::CheckRelOpOperands(left_type_operand_ptr, right_type_operand_ptr);
					//
					tree_ptr->type_ptr	= HccTypeChecker::ts_type_boolean_ptr;

					//END - FIXED Jan 17, 2009
				}else{
					//check for integer values				
					//
					// &, ^, | only allowed on integral types
					if(false == HccTypeChecker::IntegerOperands(tree_ptr->left_ptr->type_ptr, 
																tree_ptr->right_ptr->type_ptr))
						HccErrorManager::Error(HccErrorManager::errIncompatibleTypes, _T(", operands must be integer types only."));
					//the expected type spec
					tree_ptr->type_ptr	= HccTypeChecker::ts_type_int_ptr;
				}
			}else{
				HccErrorManager::Error(HccErrorManager::errIncompatibleTypes);
				//to flag an error in the next expr level...
				tree_ptr->type_ptr	= HccTypeChecker::ts_type_int_ptr;
			}

			left_ptr = tree_ptr;
		}
		break;
	};
	return left_ptr;
}

//-----------------------------------------------
// ParseExprList	- Parse an expression (for &&, || and ternary '?')
//-----------------------------------------------
LPHCC_PARSE_TREE HCCParser::ParseExprList(Symbol *function_ptr)
{
	LPHCC_PARSE_TREE tree_ptr = NULL;
	//icode_ptr->insert_line_marker();
	//simple expression
	LPHCC_PARSE_TREE left_ptr = ParseRelationalExprList(function_ptr);
	switch(token_type)
	{
	case HCC_AND_OP:			// &&
	case HCC_OR_OP:				// || 											
		{
			tree_ptr = new HCC_PARSE_TREE(NULL, token_type);
			getTokenAppend();
			//simple expression
			tree_ptr->left_ptr	= left_ptr;
			tree_ptr->right_ptr = ParseExprList(function_ptr);
			//Strong Type Checking...
			if(tree_ptr->right_ptr!=NULL && tree_ptr->right_ptr->type_ptr!=NULL)
			{
				HccTypeChecker::CheckBoolean(tree_ptr->left_ptr->type_ptr, 
											 tree_ptr->right_ptr->type_ptr);
				//
				tree_ptr->type_ptr	= HccTypeChecker::ts_type_boolean_ptr;
			}else{
				HccErrorManager::Error(HccErrorManager::errIncompatibleTypes);
				//to flag an error in the next expr level...
				tree_ptr->type_ptr	= HccTypeChecker::ts_type_int_ptr;
			}

			left_ptr = tree_ptr;
		}
		break;
	case HCC_TERNARY_OP:
		{
			is_ternary_expression = true;
			tree_ptr = new HCC_PARSE_TREE(NULL, token_type);			
			getTokenAppend();
			HccTypeChecker::CheckBoolean(left_ptr->type_ptr);			
			//
			tree_ptr->expr1_ptr = left_ptr;
			// expr1 ? expr2 : expr3
			tree_ptr->expr2_ptr = ParseExprList(function_ptr);
			tree_ptr->type_ptr = tree_ptr->expr2_ptr->type_ptr;
			if(token_type==HCC_COLON){
				getTokenAppend();
				tree_ptr->expr3_ptr = ParseExprList(function_ptr);
				//the expression type
				if(tree_ptr->expr2_ptr->type_ptr!=NULL &&
					tree_ptr->expr3_ptr->type_ptr!=NULL)
					tree_ptr->type_ptr = 
						HccTypeChecker::GetResultTypeFromExprTypes(tree_ptr->expr2_ptr->type_ptr,
																	tree_ptr->expr3_ptr->type_ptr);
				else{
					if(tree_ptr->expr2_ptr->type_ptr==NULL)
						tree_ptr->type_ptr = tree_ptr->expr3_ptr->type_ptr;
					else
						tree_ptr->type_ptr = tree_ptr->expr2_ptr->type_ptr;
				}
			}else if(token_type==HCC_EOF){
				HccErrorManager::Error(HccErrorManager::errUnexpectedEndOfFile);
				tree_ptr->type_ptr = HccTypeChecker::ts_type_int_ptr;
			}
			else{
				HccErrorManager::Error(HccErrorManager::errInvalidExpression, 
										_T(" missing \':\' after expression"));
				//
				tree_ptr->type_ptr = HccTypeChecker::ts_type_int_ptr;
			}

			left_ptr = tree_ptr;

			is_ternary_expression = false;
		};
		break;
	};
	//what could make the parser to resynchonize correctly 
	//after a syntax error in an expression?
	HCC_TOKEN_TYPE tokens[] = {	
		HCC_SEMICOLON,
		HCC_COLON,
		HCC_COMMA_OP,
		HCC_RPAREN,
		HCC_RBRACKET,
		HCC_RBLOCK_KEY,
	};
	Resync(tokens, sizeof(tokens)/sizeof(tokens[0]));
	return left_ptr;
}

//-----------------------------------------------
// ParseExpr	- Parse simple expressions 
//				  with binary operators: + | - | or
//-----------------------------------------------
LPHCC_PARSE_TREE HCCParser::ParseExpr(Symbol *function_ptr)
{
	LPHCC_PARSE_TREE tree_ptr = NULL;
	HCC_TOKEN_TYPE unaryOp = HCC_PLUS_OP;

	//If unary op - | + , skip...
	if(IsUnarySign())
	{
		unaryOp = token_type;
		getTokenAppend();
	}

	LPHCC_PARSE_TREE left_ptr = ParseTerm(function_ptr);
	for(;left_ptr!=NULL;)
		switch(token_type)
	{
	case HCC_PLUS_OP:				// +
	case HCC_MINUS_OP:				// -
		//B I T   S H I F T I N G   O P
	case HCC_LEFT_SHIFT_OP:			// <<
	case HCC_RIGHT_SHIFT_OP:		// >>	
		{
			tree_ptr = new HCC_PARSE_TREE(NULL, token_type);
			//skip this operator
			getTokenAppend();
			tree_ptr->left_ptr	= left_ptr;

			//BEGIN - OPTIMIZED Jan 17, 2009
			/*
				shift_expression :: identifier op shift_value |
									integer	   op shift_value;

				shift_value		 :: integer | identifier;
			*/
			if(tree_ptr->token_op==HCC_LEFT_SHIFT_OP || 
				tree_ptr->token_op==HCC_RIGHT_SHIFT_OP)
			{
				HCC_TOKEN_TYPE token = token_type;
				Symbol* symbol_ptr = NULL;
				if(token_type==HCC_NUMBER)
				{
					symbol_ptr = AppendNumber(token_ptr);
					getTokenAppend();
				}else if(token_type==HCC_IDENTIFIER)
				{
					symbol_ptr = getSymbolFromIdentifier(true, function_ptr);

					appendSymbolToICode(HCC_TOKEN_ERROR, symbol_ptr);
				}else{
					HccErrorManager::Error(HccErrorManager::errInvalidExpression, 
						_T(": right operand in shift expression must be an identifier or an integer value up to 8-bits of size, like in: \"integer-expression <shift-op> [identifier | number]\"."));
				}

				Resync(HCC_SEMICOLON);
				//
				if(symbol_ptr!=NULL)
				{
					tree_ptr->right_ptr = new HCC_PARSE_TREE(symbol_ptr, token);
					//we're assumming that this variable or number, must be a 8-bits value in a 16-bits variable
					if(symbol_ptr->getTypeSpecifier().getBaseTypeSpec()->getDataTypeSize() > sizeof(short))
						HccErrorManager::Error(HccErrorManager::errInvalidExpression, 
						_T(": right operand in shift expression must be and integer up to 8-bits of size."));
				}

			}else
				tree_ptr->right_ptr = ParseTerm(function_ptr);
			//END - OPTIMIZED Jan 17, 2009

			//TODO: for error types
			tree_ptr->type_ptr	= HccTypeChecker::ts_type_int_ptr;

			//Syntax Checking...
			if(tree_ptr->right_ptr!=NULL && tree_ptr->right_ptr->type_ptr!=NULL)
			{
				if(HccTypeChecker::IntegerOperands(tree_ptr->left_ptr->type_ptr, 
													tree_ptr->right_ptr->type_ptr))
				{
					tree_ptr->type_ptr	= HccTypeChecker::ts_type_int_ptr;
				}else{
					if(tree_ptr->token_op==HCC_LEFT_SHIFT_OP || 
						tree_ptr->token_op==HCC_RIGHT_SHIFT_OP)
					{
						HccErrorManager::Error(HccErrorManager::errBitShiftingOperandsMustBeIntegers);
						HccErrorManager::Error(HccErrorManager::errIncompatibleTypes);
					}else{					
							/*
								double <op> <double>
								int <op> double
								double <op> int							
							*/
						//are floating points?
						if(HccTypeChecker::FloatingPointOperands(tree_ptr->left_ptr->type_ptr, 
																	tree_ptr->right_ptr->type_ptr))
						{
							//using the highest type...
							tree_ptr->type_ptr	= HccTypeChecker::ts_type_double_ptr;
						}else
							HccErrorManager::Error(HccErrorManager::errIncompatibleTypes);
					}
				}
			}else
				HccErrorManager::Error(HccErrorManager::errInvalidExpression);
			//next sub-expr
			left_ptr = tree_ptr;
		}
		break;
	default:
		return left_ptr;
		break;
	};
	
	return left_ptr;
}

//-----------------------------------------------
//	ParseTerm	- Parse a expression with
//				  binary operators : * | / | div | % | and
//-----------------------------------------------
LPHCC_PARSE_TREE HCCParser::ParseTerm(Symbol *function_ptr)
{
	LPHCC_PARSE_TREE tree_ptr = NULL;

	LPHCC_PARSE_TREE left_ptr = ParseFactor(function_ptr);
	for(;left_ptr!=NULL;)
		switch(token_type)
	{
	case HCC_MUL_OP:			// *
	case HCC_DIV_OP:			// /	
	case HCC_DIV:				// Integer Division	
	case HCC_MOD_OP:			// %
		{
			tree_ptr = new HCC_PARSE_TREE(NULL, token_type);
			//skip this operator
			getTokenAppend();
			tree_ptr->left_ptr	= left_ptr;
			tree_ptr->right_ptr = ParseFactor(function_ptr);

			//Syntax Checking...
			if(tree_ptr->right_ptr!=NULL && tree_ptr->right_ptr->type_ptr!=NULL &&
				tree_ptr->left_ptr!=NULL && tree_ptr->left_ptr->type_ptr!=NULL)
			{
				if(HccTypeChecker::IntegerOperands(tree_ptr->left_ptr->type_ptr, 
													tree_ptr->right_ptr->type_ptr))
				{
					//BEGIN - FIXED Dec 25, 2008
					if(tree_ptr->token_op==HCC_DIV_OP)
						tree_ptr->type_ptr	= HccTypeChecker::ts_type_double_ptr;
					else
						tree_ptr->type_ptr	= HccTypeChecker::ts_type_int_ptr;
					//END - FIXED Dec 25, 2008
					//
				}else{
					if(tree_ptr->token_op==HCC_DIV || tree_ptr->token_op==HCC_MOD_OP){
						HccErrorManager::Error(HccErrorManager::errMustBeIntegerOperands);
						HccErrorManager::Error(HccErrorManager::errIncompatibleTypes);
						//TODO: for error types
						tree_ptr->type_ptr	= HccTypeChecker::ts_type_int_ptr;
					}
					else{
						if(HccTypeChecker::FloatingPointOperands(tree_ptr->left_ptr->type_ptr, 
													tree_ptr->right_ptr->type_ptr))
						{
							tree_ptr->type_ptr	= HccTypeChecker::ts_type_double_ptr;
						}else{
							HccErrorManager::Error(HccErrorManager::errIncompatibleTypes);
							//TODO: for error types
							tree_ptr->type_ptr	= HccTypeChecker::ts_type_int_ptr;
						}
					}
				}
			}else
				HccErrorManager::Error(HccErrorManager::errInvalidExpression);

			//next sub-expr
			left_ptr = tree_ptr;
		}
		break;
	default:
		return left_ptr;
		break;
	};
	return left_ptr;
}

//----------------------------------------------
//	ParseFactor		- Parse a factor in (identifier | number | string | !<factor> | (<expr>)  )
//
//			and:
//
//	assignment-statement:
//		 id = expr
//
//		...it can be:
//
//	labeled-statement:
//		identifier : statement
//-------------------------------------------
LPHCC_PARSE_TREE HCCParser::ParseFactor(Symbol *function_ptr)
{
#ifdef __HCC_TOKEN_DEBUG_OUTPUT__
	lexer->OutputToken(token_ptr);
#endif //__HCC_TOKEN_DEBUG_OUTPUT__
	//
	LPHCC_PARSE_TREE node_ptr = NULL;
	SYMBOL_TABLE::LPSYMBOL symbol_ptr	= NULL;
	HCC_TOKEN_TYPE prev_token			= HCC_TOKEN_ERROR;
	__tstring token_id;	
	bool bArraySubscript = false;
	bool bPointerIndirection = false;
	TypeSpecifier* arrayItemTypeSpec = NULL;
	switch(token_type)
	{
		case HCC_NUMBER:
		{
			symbol_ptr = AppendNumber(token_ptr);
			//the node in the syntax tree...
			node_ptr = new HCC_PARSE_TREE(symbol_ptr, HCC_NUMBER);			
		}
		break;
		case HCC_IDENTIFIER:
		{
			token_id = token_ptr->String();
			//use the current namespace...			
			prev_token	= token_type;
			symbol_ptr = getSymbolFromIdentifier(false, function_ptr);
			if(symbol_ptr!=NULL)
			{
				//if this identifier is a user-defined type, then this must be a declaration					
				//N E W   V A R I A B L E   D E C L A R A T I O N
				if(symbol_ptr->getDeclDefinition().identifier_type()==DECL_NEW_TYPE ||
					symbol_ptr->getDeclDefinition().identifier_type()==DECL_NEW_ABSTRACT_TYPE)
				{
					icode_ptr->put_back(); //the HCC_IDENTIFIER is taken off the MIR code...
					parseVariableDeclarations(function_ptr, &symbol_ptr->getTypeSpecifier());
					return 0;
				}
				else
				{
					switch(symbol_ptr->getDeclDefinition().identifier_type())
					{
					case DECL_VARIABLE:
					case DECL_POINTER_VARIABLE:			//pointers and dynamic arrays
					case DECL_CONSTANT:

					case DECL_NEW_FUNC_MEMBER:			//a function in a class
					case DECL_NEW_VIRTUAL_FUNC_MEMBER:	//a virtual function in a class
					case DECL_NEW_STATIC_CLASS_MEMBER:	//a static function in a class	

					case DECL_NEW_DATA_MEMBER:	//an attribute in a class

					case DECL_PARAM_VALUE:		//param by value
					case DECL_PARAM_BYREF:		//param by reference
					case DECL_PARAM_CONST_BYREF://const param by reference

					case DECL_PARAM_POINTER:			//param pointer
					case DECL_PARAM_CONST_POINTER:		//const param pointer

					case DECL_PARAM_ARRAY:		//one-dimension param array
					case DECL_PARAM_CONST_ARRAY://one-dimension const param array


					case DECL_READONLY_PROPERTY:		//a read-only property
					case DECL_WRITEONLY_PROPERTY:		//a write-only property
					case DECL_READWRITE_PROPERTY:		//a read-write property

					case DECL_PROC_LABEL:				//a proc label block

					case DECL_OPERATOR_MIN:				//the inline-min between two integer types
					case DECL_OPERATOR_MAX:				//the inline-max between two integer types


						goto __CONTINUE_PARSE_EXPRESSION;
						break;
					case DECL_SYMBOL_USER_ALIAS:
						{
							symbol_ptr = symbol_ptr->getOwner();
							assert(symbol_ptr!=NULL);
							goto __CONTINUE_PARSE_EXPRESSION;	
						}
						break;
					//for multiple params console write
					case DECL_BUILTIN_CONSOLE_WRITE:
					case DECL_BUILTIN_CONSOLE_WRITELN:
						{
							node_ptr = ParseCallWriteWriteLnBuiltInFunction(symbol_ptr, function_ptr);
							goto FACTOR_EXIT;
						}
						break;
					case DECL_NEW_VIRTUAL_ABSTRACT_FUNC_MEMBER:
						{
							//BEGIN -FIXED Mar 04, 2009
							//determine the virtual/abstract state of a class
							analyzeClassVirtualState(active_class_ptr);
							if(false==IsAbstractClass(active_class_ptr))
								HccErrorManager::Error(HccErrorManager::errAbstractClassFunctionMemberCall);
							//END -FIXED Mar 04, 2009
						}
						break;
					default:
						//Error: unknown or undefined identifier
						{
							HccErrorManager::Error(HccErrorManager::errInvalidExpression);
						}
						break;
					}
				}
			}
__CONTINUE_PARSE_EXPRESSION:
			if(symbol_ptr!=NULL)
			{
				appendSymbolToICode(HCC_TOKEN_ERROR, symbol_ptr);
				//
__ASSIGNMENT_EXPRESSION:
				//A S S I G N M E N T - E X P R E S S I O N S 
				switch(token_type)
				{
				case HCC_INCREMENT:		//postfix ++
				case HCC_DECREMENT:		//postfix --
					{
						Symbol* id_ptr = symbol_ptr;
						//the node in the syntax tree...
						node_ptr = new HCC_PARSE_TREE(NULL, token_type);

						getTokenAppend();
						node_ptr->left_ptr  = new HCC_PARSE_TREE(id_ptr /*symbol_ptr*/, prev_token);

						if(node_ptr->left_ptr!=NULL)
							node_ptr->type_ptr	= node_ptr->left_ptr->type_ptr;

						DECLARATION_TYPE id_type = id_ptr->getDeclDefinition().identifier_type();

						if(id_type==DECL_CONSTANT				||
							id_type==DECL_READONLY_PROPERTY		||
						//for now, we only support simple assignments to write properties...
							id_type==DECL_WRITEONLY_PROPERTY	||		//a write-only property
							id_type==DECL_READWRITE_PROPERTY	||		//a read-write property
							id_type==DECL_PARAM_CONST_BYREF)
						{
							if(id_type==DECL_READONLY_PROPERTY)
								HccErrorManager::Error(HccErrorManager::errReadOnlyPropertyInvalidAssignment);
							else
							{
								if( id_type==DECL_WRITEONLY_PROPERTY	||	//a write-only property
								 id_type==DECL_READWRITE_PROPERTY)		//a read-write property
								{
									HccErrorManager::Error(HccErrorManager::errInvalidAssignment, 
														  _T(": writable properties can only be assigned through operator \'=\'."));
								}
							//
								HccErrorManager::Error(HccErrorManager::errMustBe_LValue_Identifier);
							}
						}

						if(id_type==DECL_POINTER_VARIABLE)
						{
							node_ptr->left_ptr->set_is_pointer_expr(true);
						}
					}
					break;
				case HCC_ASSIGN_OP:			// =
				case HCC_INC_ASSIGN:		// +=
				case HCC_DEC_ASSIGN:		// -=
				case HCC_MUL_ASSIGN:		// *=
				case HCC_DIV_ASSIGN:		// /=
				case HCC_MOD_ASSIGN:		// %=
				case HCC_XOR_ASSIGN:		// ^=
				case HCC_BIT_OR_ASSIGN:		// |=
				case HCC_BIT_AND_ASSIGN:	// &=
					{

						//for now, we only support simple assignments to write properties...
						if( (symbol_ptr->getDeclDefinition().identifier_type()==DECL_WRITEONLY_PROPERTY	||	//a write-only property
							 symbol_ptr->getDeclDefinition().identifier_type()==DECL_READWRITE_PROPERTY)		//a read-write property
							&&
							token_type!=HCC_ASSIGN_OP)
						{
							HccErrorManager::Error(HccErrorManager::errInvalidAssignment, _T(": writable properties can only be assigned through operator \'=\'."));
						}

						if(symbol_ptr->getDeclDefinition().identifier_type()!=DECL_CONSTANT &&
							symbol_ptr->getDeclDefinition().identifier_type()!=DECL_READONLY_PROPERTY &&
							symbol_ptr->getDeclDefinition().identifier_type()!=DECL_PARAM_CONST_BYREF &&
							symbol_ptr->getDeclDefinition().identifier_type()!=DECL_PARAM_CONST_POINTER)
						{
							//the node in the syntax tree...
							node_ptr = new HCC_PARSE_TREE(NULL, token_type);
							// P O I N T E R   A N D   D Y N A M I C   A R R A Y S
							if(
								(
								 symbol_ptr->getDeclDefinition().identifier_type()==DECL_POINTER_VARIABLE	||
								 symbol_ptr->getDeclDefinition().identifier_type()==DECL_PARAM_ARRAY		||
								 symbol_ptr->getDeclDefinition().identifier_type()==DECL_PARAM_CONST_ARRAY	||
								 symbol_ptr->getDeclDefinition().identifier_type()==DECL_PARAM_POINTER		||
								 symbol_ptr->getDeclDefinition().identifier_type()==DECL_PARAM_CONST_POINTER ||
								 (
								   symbol_ptr->getDeclDefinition().identifier_type()==DECL_NEW_DATA_MEMBER &&
								   symbol_ptr->getDeclDefinition().user_data.bDataMemberIsPointer
								 )
								)
								&& false==bPointerIndirection)
							{
								if(token_type!=HCC_ASSIGN_OP)
									HccErrorManager::Error(HccErrorManager::errInvalidAssignment, 
														_T("Pointers and dynamic arrays can only be assigned using operator \'=\'."));
								if(symbol_ptr->getTypeSpecifier().specifier()==DSPEC_ARRAY)
								{
									if(false==bArraySubscript)
									{	
										if(false==IsDynamicArray(&symbol_ptr->getTypeSpecifier()))
										{
											HccErrorManager::Error(HccErrorManager::errCannotAssignToFullyDefinedArray, symbol_ptr->getCompleteName());
										}
										parseArrayPointerVariableOrDataMember(symbol_ptr, 
																			  getArrayScalarType(&symbol_ptr->getTypeSpecifier()),
																			  getArrayDimesionsFromType(&symbol_ptr->getTypeSpecifier()),
																			  function_ptr, 
																			  active_class_ptr, true);
									}else{
										getTokenAppend(); // =
										node_ptr->left_ptr  = new HCC_PARSE_TREE(NULL, prev_token);
										node_ptr->left_ptr->type_ptr = 
															getArrayScalarType(&symbol_ptr->getTypeSpecifier());
										node_ptr->right_ptr = ParseExprList(function_ptr);										
									}
								}else{
									TypeSpecifier* base_type_ptr = symbol_ptr->getTypeSpecifier().getBaseTypeSpec();
									node_ptr->left_ptr  = new HCC_PARSE_TREE(NULL, HCC_IDENTIFIER);
									node_ptr->left_ptr->type_ptr = base_type_ptr;

									parsePointerVariableOrDataMember(symbol_ptr, 
																	  base_type_ptr,
																	  function_ptr, 
																	  active_class_ptr, true);
									//the same to silent the strong type checking system
									node_ptr->right_ptr = new HCC_PARSE_TREE(symbol_ptr, HCC_IDENTIFIER);
									node_ptr->right_ptr->set_is_pointer_expr(true);
									node_ptr->set_is_pointer_expr(true);
								}
								goto FACTOR_EXIT;								
							}

							//this tree forms an assignment
							node_ptr->left_ptr  = new HCC_PARSE_TREE(symbol_ptr, prev_token);

							getTokenAppend();	//assign op
							node_ptr->right_ptr = ParseExprList(function_ptr);

							if(node_ptr->right_ptr!=NULL && node_ptr->right_ptr->type_ptr)
							{
								TypeSpecifier* left_type_spec_ptr = node_ptr->left_ptr->type_ptr;
								TypeSpecifier* right_type_spec_ptr = node_ptr->right_ptr->type_ptr;

								if(!!left_type_spec_ptr)
								{
									//for arrays, just use the scalar/array definition type for type checking									
									if(left_type_spec_ptr->specifier()==DSPEC_ARRAY)
										left_type_spec_ptr = getArrayScalarType(left_type_spec_ptr);

									TypeSpecifier* result_expr_type_ptr =
										HccTypeChecker::GetResultTypeFromExprTypes(left_type_spec_ptr, 
																				   right_type_spec_ptr);

									if(left_type_spec_ptr->getDataType()==HCC_INTEGER
										&&
										result_expr_type_ptr->getDataType()==HCC_FLOATING_POINT)
									{
										HccWarningManager::Warning(HccWarningManager::warnImplicitConversionPossibleLossOfData, 
																	_T("Do explicit type conversion to avoid loss of data."));
									}else
										HccTypeChecker::CheckCompatibleAssignment(left_type_spec_ptr,
																				right_type_spec_ptr,
																				HccErrorManager::errInvalidAssignment, 
																				_T("(must have the same type as l-value)."));

									//in H++, the following operators are only allowed between integer operands...
									switch(node_ptr->token_op)
									{
									case HCC_MOD_ASSIGN:		// %=
									case HCC_XOR_ASSIGN:		// ^=
									case HCC_BIT_OR_ASSIGN:		// |=
									case HCC_BIT_AND_ASSIGN:	// &=
										{
											if(!HccTypeChecker::IntegerOperands(left_type_spec_ptr, right_type_spec_ptr))
											{
												HccErrorManager::Error(HccErrorManager::errMustBeIntegerOperands);
												HccErrorManager::Error(HccErrorManager::errIncompatibleTypes);
											}
										}
										break;
									}
								}else
									HccErrorManager::Error(HccErrorManager::errInvalidAssignment);
							}else
								HccErrorManager::Error(HccErrorManager::errInvalidAssignment);

							node_ptr->type_ptr	= node_ptr->left_ptr->type_ptr;
						}else{
							if(symbol_ptr->getDeclDefinition().identifier_type()==DECL_READONLY_PROPERTY)
								HccErrorManager::Error(HccErrorManager::errReadOnlyPropertyInvalidAssignment);
							else
								HccErrorManager::Error(HccErrorManager::errMustBe_LValue_Identifier);
						}
					}
					break;					
				case HCC_COLON:				//labeled-statement
					{
						if(!is_ternary_expression)
						{
							__tstring info = _T(": \'");
							info += symbol_ptr->String();
							info += _T("\' ");
							if(symbol_ptr->getDeclDefinition().identifier_type()!=DECL_PROC_LABEL)
								HccErrorManager::Error(HccErrorManager::errRedeclaredIdentifier, info);
							
							const __tstring& _label = symbol_ptr->getName();
							set<__tstring>& proc_labels = map_proc_labels[function_ptr];
							if(proc_labels.find(_label)!=proc_labels.end())
							{
								info += _T(" (already exists).");
								HccErrorManager::Error(HccErrorManager::errLabelRedefined, info);
							}else
								proc_labels.insert(_label);							
											
							//skip the ':' token
							getTokenAppend();
							//
							//next: parse statement;
							ParseStatement(function_ptr);
						}else{
							node_ptr = new HCC_PARSE_TREE(symbol_ptr, prev_token);
						}
					}
					break;					
				case HCC_PERIOD:
					{
						TypeSpecifier* baseTypeSpec = symbol_ptr->getTypeSpecifier().getBaseTypeSpec();
						node_ptr = parseObjectInstanceMember(baseTypeSpec, function_ptr);
						if(IsAbstractClass(symbol_ptr))
							HccErrorManager::Error(HccErrorManager::errAbstractClassFunctionMemberCall);						
					}
					break;
				case HCC_LBRACKET:	//[
					{
						//accessing an array element...
						TypeSpecifier* baseTypeSpec = symbol_ptr->getTypeSpecifier().getBaseTypeSpec();
						//
						arrayItemTypeSpec = parseArraySubscript(baseTypeSpec, function_ptr);
						bArraySubscript = true;
						//if an assignment...
						goto __ASSIGNMENT_EXPRESSION;
					}
					break;
				case HCC_LPAREN:
					{
						node_ptr = parseFunctionCall(symbol_ptr, function_ptr);
					}
					break;
				default:
					{
						if(bArraySubscript){
							node_ptr = new HCC_PARSE_TREE(NULL, prev_token);
							node_ptr->type_ptr = arrayItemTypeSpec;
						}else{
							node_ptr = new HCC_PARSE_TREE(symbol_ptr, prev_token);

							//if is a pointer, and the next token is either '+' or '-', parse a pointer expression.
							if(
								symbol_ptr->getDeclDefinition().identifier_type()==DECL_POINTER_VARIABLE 
								||
								(symbol_ptr->getDeclDefinition().identifier_type()==DECL_NEW_DATA_MEMBER &&
									symbol_ptr->getDeclDefinition().user_data.bDataMemberIsPointer)
								)
							{
								//flag this expr as a pointer expr.
								node_ptr->set_is_pointer_expr(true);
								if(token_type == HCC_PLUS_OP || 
									token_type == HCC_MINUS_OP) //+ | - 
								{
									node_ptr = parsePointerExpression(function_ptr, node_ptr);
									//
								}else if(token_type == HCC_EQUAL_OP ||
										token_type == HCC_NOT_EQ_OP) // == | !=
								{
									//to let the the expression flow
								}
								else
								{
									HCC_TOKEN_TYPE tokens[] = {	
										HCC_SEMICOLON,
										HCC_RPAREN,
										HCC_COMMA_OP,
										HCC_RBRACKET,
										HCC_RBLOCK_KEY,
									};
									Resync(tokens, sizeof(tokens)/sizeof(tokens[0]));
								}
							}
						}
					}
					break;
				};
				goto FACTOR_EXIT;
			}else{
				if(function_ptr!=NULL)
				{
					//for labels, we must be inside a member function/class function
					//insert label in the local symbol table...
					Symbol* _label_ptr = 
								function_ptr->getDeclDefinition().function.symbol_table_ptr->insert(token_id);
					
					if(_label_ptr!=NULL)
					{
						//marked as a proc block label
						_label_ptr->setType(HCC_IDENTIFIER);
						_label_ptr->getDeclDefinition().set_identifier_type(DECL_PROC_LABEL);
						_label_ptr->getDeclDefinition().set_identifier_scope(SCOPE_LOCAL);
						
						const __tstring& _label = _label_ptr->getName();
						set<__tstring>& proc_labels = map_proc_labels[function_ptr];
						//insert it as already declared label block for this procedure...
						proc_labels.insert(_label);
						//
						if(token_type!=HCC_COLON)
						{
							find_symbol(last_identifier.c_str());
						}					
						appendSymbolToICode(HCC_TOKEN_ERROR, _label_ptr);
						getTokenAppend(); //skip the ':' token
						//next: parse statement;
						ParseStatement(function_ptr);
					}
				}
				goto FACTOR_EXIT;				
			}			
		}
		break;
		case HCC_LPAREN:
		{
			//skip this ')'
			getTokenAppend();
			//the node in the syntax tree...
			node_ptr = parseCommaExprList(function_ptr);
			//
			if(token_type!=HCC_RPAREN){
				HccErrorManager::Error(HccErrorManager::errMissingRightParen);
				goto FACTOR_EXIT;
			}
		}
		break;
		case HCC_CONTROL_CHAR:
			{
				symbol_ptr = symbol_table_ptr->find(token_ptr->String());
				if(symbol_ptr==NULL)
				{
					symbol_ptr = symbol_table_ptr->insert(token_ptr->String());
					symbol_ptr->setType(token_type);
					//
					int length = _tcslen(token_ptr->String()) - 2;	// '\x'
					assert(length==2);
					switch(token_ptr->String()[2])
					{
					case _T('t'):
						symbol_ptr->getDeclDefinition().constant.value.Character = _T('\t');
						break;
					case _T('r'):
						symbol_ptr->getDeclDefinition().constant.value.Character = _T('\r');
						break;
					case _T('n'):
						symbol_ptr->getDeclDefinition().constant.value.Character = _T('\n');
						break;
					case _T('f'):
						symbol_ptr->getDeclDefinition().constant.value.Character = _T('\f');
						break;
					case _T('b'):
						symbol_ptr->getDeclDefinition().constant.value.Character = _T('\b');
						break;
					case _T('0'):
						symbol_ptr->getDeclDefinition().constant.value.Character = _T('\0');
						break;
					default:
						symbol_ptr->getDeclDefinition().constant.value.Character = token_ptr->String()[2];
						break;
					}
					//type changed to char...
					symbol_ptr->getTypeSpecifier().setBaseTypeSpec(HccTypeChecker::ts_type_char_ptr);
				}
				//the node in the syntax tree...
				node_ptr = new HCC_PARSE_TREE(symbol_ptr, token_type);
				//
				//add this symbol to the intermediate code...
				icode_ptr->append(symbol_ptr);
				getTokenAppend();
				goto FACTOR_EXIT;
			}
			break;
		case HCC_CHARACTER:		
		case HCC_STRING_LITERAL:
		{			
			symbol_ptr = symbol_table_ptr->find(token_ptr->String());
			if(symbol_ptr==NULL)
			{
				symbol_ptr = symbol_table_ptr->insert(token_ptr->String());
				symbol_ptr->setType(token_type);
				//
				int length = _tcslen(token_ptr->String()) - 2;
				if(length==1){
					symbol_ptr->getDeclDefinition().set_identifier_type(DECL_CONSTANT);
					symbol_ptr->getDeclDefinition().constant.value.Character = token_ptr->String()[1];
					//type changed to char...
					symbol_ptr->getTypeSpecifier().setBaseTypeSpec(HccTypeChecker::ts_type_char_ptr);
					//
					symbol_ptr->setType(HCC_CHARACTER);
				}else{
					TCHAR* string_ptr = new TCHAR[length + 1];
					symbol_ptr->getDeclDefinition().constant.value.String = string_ptr;
					CopyQuotedString(string_ptr, token_ptr->String());
					//changed type to represent an array of characters...
					symbol_ptr->getTypeSpecifier().setStringSpec(length);
				}
			}
			//the node in the syntax tree...
			node_ptr = new HCC_PARSE_TREE(symbol_ptr, token_type);
			//
			//add this symbol to the intermediate code...
			icode_ptr->append(symbol_ptr);
			getTokenAppend();
			goto FACTOR_EXIT;
		}
		break;
		case HCC_TRUE:
		case HCC_FALSE:
		case HCC_NULL:
		{
			//for constant assignments 
			//the node in the syntax tree...
			symbol_ptr = find_symbol(token_ptr->String());
			node_ptr = new HCC_PARSE_TREE(symbol_ptr, token_type);
			//
			node_ptr->type_ptr = symbol_ptr->getTypeSpecifier().getBaseTypeSpec();

			//add this symbol to the intermediate code...
			icode_ptr->append(symbol_ptr);
			getTokenAppend();
			goto FACTOR_EXIT;
		}
		break;
		//P R E F I X   O P E R A T O R S  (T H E   H I G H E S T   P R E C E D E N C E )
		case HCC_NOT_OP:		//!  : !1;   == 0
		case HCC_COMPL_OP:		//~  : ~0x0; == 0xFFFFFFFF;
			{
				HCC_TOKEN_TYPE prefix_op = token_type;
				node_ptr = new HCC_PARSE_TREE(NULL, token_type);
				getTokenAppend();
				node_ptr->right_ptr = ParseFactor(function_ptr);

				if(node_ptr->right_ptr!=NULL && node_ptr->right_ptr->type_ptr)
				{
					if(HCC_NOT_OP==prefix_op)
						node_ptr->type_ptr	= HccTypeChecker::ts_type_boolean_ptr;
					else
						node_ptr->type_ptr	= node_ptr->right_ptr->type_ptr;			

					if(HccTypeChecker::IntegerOperands(node_ptr->type_ptr, node_ptr->type_ptr)==false)
					{
						HccTypeChecker::CheckBoolean(node_ptr->type_ptr, node_ptr->type_ptr);
					}
				}
				goto FACTOR_EXIT;
			}
			break;
		case HCC_INCREMENT:		//++ : ++id; == (id = id + 1)
		case HCC_DECREMENT:		//-- : --id; == (id = id - 1)		
		{

			//M U S T   B E   A N   I D E N T I F I E R
			HCC_TOKEN_TYPE prefix_op = token_type;
			node_ptr = new HCC_PARSE_TREE(NULL, token_type);
			getTokenAppend(); //skip			

			node_ptr->right_ptr = ParseFactor(function_ptr);			

			if(node_ptr->right_ptr!=NULL && node_ptr->right_ptr->type_ptr)
			{
				node_ptr->type_ptr	= node_ptr->right_ptr->type_ptr;

				if(node_ptr->right_ptr->symbol_ptr!=NULL)
				{
					if(node_ptr->right_ptr->symbol_ptr->getType()!=HCC_IDENTIFIER)
						HccErrorManager::Error(HccErrorManager::errInvalidExpression, _T("prefix ++ operator can only be applied to identifiers."));

					Symbol* id_ptr = node_ptr->right_ptr->symbol_ptr;

					DECLARATION_TYPE id_type = id_ptr->getDeclDefinition().identifier_type();

					if(id_type==DECL_CONSTANT				||
						id_type==DECL_READONLY_PROPERTY		||
					//for now, we only support simple assignments to write properties...
						id_type==DECL_WRITEONLY_PROPERTY	||		//a write-only property
						id_type==DECL_READWRITE_PROPERTY	||		//a read-write property
						id_type==DECL_PARAM_CONST_BYREF)
					{
						if(id_type==DECL_READONLY_PROPERTY)
							HccErrorManager::Error(HccErrorManager::errReadOnlyPropertyInvalidAssignment);
						else
						{
							if( id_type==DECL_WRITEONLY_PROPERTY	||	//a write-only property
							 id_type==DECL_READWRITE_PROPERTY)		//a read-write property
							{
								HccErrorManager::Error(HccErrorManager::errInvalidAssignment, 
													  _T(": writable properties can only be assigned through operator \'=\'."));
							}
						//
							HccErrorManager::Error(HccErrorManager::errMustBe_LValue_Identifier);
						}
					}
				}
			}else 
				assert(0);
			
			goto FACTOR_EXIT;
		}
		break;
		//BEGIN - ADDED - Jan 1 2009
		case HCC_NEW:
			{
				node_ptr = new HCC_PARSE_TREE(NULL, token_type);
				TypeSpecifier* base_type_ptr = (function_ptr!=NULL ? function_ptr->getDeclDefinition().function.return_type : NULL);
				node_ptr->type_ptr = parseNewInstance(NULL,
													  base_type_ptr,
													  NULL,
													  function_ptr,
													  false);
				goto FACTOR_EXIT; //for empty-expression-statement
			}
		break;
		//END - ADDED - Jan 1 2009
		case HCC_DESTROY:
			{
				//destroy a dynamically allocated object
				bool bIsDynamicArray = false;
				getTokenAppend();
				//destroy object1;
				//destroy array1;
				//destroy []array2
				if(HCC_LBRACKET==token_type)
				{
					getTokenAppend(); //[
					getTokenAppendIf(HCC_RBRACKET, HccErrorManager::errMissingRightBracket); //]

					bIsDynamicArray = true;
				}
				//dynamic_cast(...) | identifier;
				if(HCC_DYNAMIC_CAST==token_type)
				{
					parseDynamicCastOperator(NULL, NULL, function_ptr);
					//
				}else if((symbol_ptr = getSymbolFromIdentifier(true, function_ptr))!=NULL)
				{
					DeclarationType declType = symbol_ptr->getDeclDefinition().identifier_type();
					if(declType!=DECL_POINTER_VARIABLE)
					{
						if(declType==DECL_NEW_DATA_MEMBER) 
						{
							if(false==symbol_ptr->getDeclDefinition().user_data.bDataMemberIsPointer)
								HccErrorManager::Error(HccErrorManager::errCanOnlyDestroyPointerOrArrayTypes);
						}else
							HccErrorManager::Error(HccErrorManager::errCanOnlyDestroyPointerOrArrayTypes);
					}
					//for array of objects, we must notify the user the correct use of destroy operator
					if(symbol_ptr->getTypeSpecifier().specifier()==DSPEC_ARRAY)
					{
						if(symbol_ptr->getTypeSpecifier().array.bIsDynamicArray && !bIsDynamicArray)
						{
							if(symbol_ptr->getTypeSpecifier().array.pItemType->specifier()==DSPEC_CLASS)
							{
								HccWarningManager::Warning(HccWarningManager::warnMustUseBracketsForArrayObjectsDtorCall);
							}
						}
					}
					//add symbol to icode
					icode_ptr->append(symbol_ptr);
				}
				goto FACTOR_EXIT;
			}
			break;
		case HCC_DYNAMIC_CAST: //dynamic_cast(type-spec, variable) -->dynamic_cast(variable)
			{
				node_ptr = new HCC_PARSE_TREE(NULL, token_type);
				node_ptr->type_ptr = parseDynamicCastOperator(NULL, NULL, function_ptr);
				goto FACTOR_EXIT;
			}
			break;
		case HCC_POINTER_ADDRESSOF:
			{

				//type-spec ^ pointer = &variable;
				HCC_TOKEN_TYPE prefix_op = token_type;
				node_ptr = new HCC_PARSE_TREE(NULL, token_type);
				getTokenAppend();	//& <identifier>
				if(token_type==HCC_IDENTIFIER)
				{
					if((symbol_ptr = getSymbolFromIdentifier(true, function_ptr))!=NULL)
					{
						node_ptr->right_ptr = new HCC_PARSE_TREE(symbol_ptr, HCC_IDENTIFIER); // ParseFactor(function_ptr);
						node_ptr->right_ptr->set_is_pointer_expr(true);
						
						appendSymbolToICode(HCC_TOKEN_ERROR, symbol_ptr); //ADDED/FIXED Jan 27, 2009

						//BEGIN - ADDED/FIXED Feb 22, 2009
						if(HCC_LBRACKET==token_type)	//[
						{
							//accessing an array element...
							TypeSpecifier* baseTypeSpec = symbol_ptr->getTypeSpecifier().getBaseTypeSpec();
							//
							TypeSpecifier* arrayItemTypeSpec = parseArraySubscript(baseTypeSpec, function_ptr);
							//
						}
						//END - ADDED/FIXED Feb 22, 2009
					}
					if(node_ptr->right_ptr!=NULL && node_ptr->right_ptr->type_ptr)
						node_ptr->type_ptr	= node_ptr->right_ptr->type_ptr;
					else				
						HccErrorManager::Error(HccErrorManager::errInvalidPointerExpression);
				}
				else
					HccErrorManager::Error(HccErrorManager::errInvalidPointerExpression, _T("; identifier expected after operator \'&\'."));								
				//parse as a pointer expression
				node_ptr = parsePointerExpression(function_ptr, node_ptr);

				HCC_TOKEN_TYPE tokens[] = {	
					HCC_SEMICOLON,
					HCC_RPAREN,
					HCC_COMMA_OP,
					HCC_RBRACKET,
					HCC_RBLOCK_KEY,
				};
				Resync(tokens, sizeof(tokens)/sizeof(tokens[0]));

				goto FACTOR_EXIT;
			}
			break;
		case HCC_POINTER_DEREFERENCE:
			{
				HCC_TOKEN_TYPE prefix_op = token_type;				
				getTokenAppend();	//* <identifier> | * <ptr-expr>
				if(token_type==HCC_IDENTIFIER)
				{
					Symbol* symbl_pointer_ptr = getSymbolFromIdentifier(true, function_ptr);
					if(symbl_pointer_ptr!=NULL)
					{
						symbol_ptr = symbl_pointer_ptr;
						//add to intermediate code...
						appendSymbolToICode(HCC_TOKEN_ERROR, symbol_ptr);

						//BEGIN - FIXED hlm 1, 2009
						DECLARATION_TYPE declType = symbol_ptr->getDeclDefinition().identifier_type();
						if(DECL_POINTER_VARIABLE!=declType)
						{
							if(declType==DECL_NEW_DATA_MEMBER && 
								false==symbol_ptr->getDeclDefinition().user_data.bDataMemberIsPointer)
							{
								HccErrorManager::Error(HccErrorManager::errInvalidPointerExpression);
							}
						}
						//END - FIXED hlm 1, 2009

						bPointerIndirection = true;
						//if an assignment expression...
						goto __ASSIGNMENT_EXPRESSION;	// *id_ptr = expr;
					}else
						HccErrorManager::Error(HccErrorManager::errInvalidPointerExpression);
				}
				HCC_TOKEN_TYPE tokens[] = {	
					HCC_SEMICOLON,
					HCC_RPAREN,
					HCC_COMMA_OP,
					HCC_RBRACKET,
					HCC_RBLOCK_KEY,
				};
				Resync(tokens, sizeof(tokens)/sizeof(tokens[0]));
				goto FACTOR_EXIT;
			}
			break;
		case HCC_SIZEOF:
			{
				symbol_ptr = symbol_table_ptr->find(token_ptr->String());
				getToken();
				node_ptr = parseSizeOfExpr(symbol_ptr, function_ptr);
				goto FACTOR_EXIT;
			}
		break;
		case HCC_SEMICOLON:
			goto FACTOR_EXIT; //for empty-expression-statement
			break;
		default:
		{
			//invalid expression			
			HccErrorManager::Error(HccErrorManager::errInvalidExpression);
			HCC_TOKEN_TYPE tokens[] = {	
				HCC_SEMICOLON,
				HCC_COLON,
				HCC_RPAREN,
				HCC_RBRACKET,
				HCC_RBLOCK_KEY,
			};
			Resync(tokens, sizeof(tokens)/sizeof(tokens[0]));
			goto FACTOR_EXIT;
		}
		break;
	};

	//the next token
	getTokenAppend();
FACTOR_EXIT:

	return node_ptr;
}

void HCCParser::TestLexer()
{
	do{
		getToken();
		if(token_type!=HCC_TOKEN_ERROR){
			lexer->OutputToken(token_ptr);
			if(token_type==HCC_IDENTIFIER){
				Symbol* symbol_ptr = symbol_table_ptr->insert(token_ptr->String());
				if(symbol_ptr!=NULL){
					if(token_ptr->dataType==HCC_FLOATING_POINT)
						symbol_ptr->setValue(token_ptr->value.Double);
					symbol_ptr->setType(token_ptr->tokenType);
					symbol_ptr->setDataType(token_ptr->dataType);
				}
			}			
		}
		else{						
			//TODO:Error Recuperation Mode....
		}

	}while(token_type!=HCC_EOF);
	delete token_ptr;
	token_ptr = NULL;

	TCHAR text[256];
	_stprintf(text, _T("%20d source lines.\n"), lexer->sourceBuf()->lineNumber());
	listing << text;
	_stprintf(text, _T("%20d syntax errors.\n"), HccErrorManager::errorCount());
	listing << text;

}

HCC_TOKEN_TYPE HCCParser::ParseStatementList(HCC_TOKEN_TYPE terminator, Symbol *function_ptr)
{
	HCC_TOKEN_TYPE stmt_type = HCC_VOID;
	/*
		BNF:
				statement-list:
						statement					|
						statement-list  statement						

				statement:
					labeled-statement			|
					expression-statement		|
					compound-statement			|
					selection-statement			|
					iteration-statement			|
					jump-statement				|
					declaration-statement		|
					try-block					|
					empty-statement

				empty-statement:
						; | eof						
	*/
	do{
		stmt_type = ParseStatement(function_ptr);
		//if the current token is not a semicolon, it's a syntax error.
		//if the current is an identifier/keyword (the start of the next statement), then : missing ';'
		//in this case, skip all tokens until we find a semicolon, or the Eof.		
		if(StatementRequiresSemicolon(stmt_type) && 
			token_type!=HCC_SEMICOLON && token_type!=HCC_EOF){
			//R E S Y N C H R O N I Z E / R E S U M E   P A R S E R			
			//missing ';'
			__tstring info = _T("before \'");
			info += token_ptr->token;
			info += _T("\'.");
			HccErrorManager::Error(HccErrorManager::errMissingSemicolon, info);			
			Resync(HCC_SEMICOLON);					
			//eliminate all occurrencies of semicolons before parsing the next statement...
			//this is called: eliminating empty statements!
			while(token_type==HCC_SEMICOLON) //i.e.:  a = 4;;; ;; ;
				getToken();					
		}else{
			//eliminate all occurrencies of semicolons before parsing the next statement...
			//this is called: eliminating empty statements!
			if(token_type==HCC_SEMICOLON)
				getTokenAppend();
			//remove empty statements...
			skipEmptyStatements();
		}
	}while(token_type!=terminator && 
		   token_type!=HCC_EOF);
	//if Eof token...
	if(token_type==HCC_EOF)
	{
		delete token_ptr;
		token_ptr = NULL;	
	}
	return stmt_type;
}

// I T E R A T I O N - S T A T E M E N T 


//--------------------------------------------
//	parseDoWhile()	- parses do-while statements like:
//
//			do{
//				statement-list;
//			}
//			while(expr);
//	or
//			do
//				statement;
//			while(expr);
//
//--------------------------------------------
void HCCParser::parseDoWhile(Symbol *function_ptr)
{
	//rule 1 for H++ statements: cannot be implemented outside a function scope
	if(function_ptr==NULL)
		HccErrorManager::Error(HccErrorManager::errStatementsMustBeInFunctionScope);

	getTokenAppend(); //do
	skipEmptyStatements();
	if(token_type==HCC_LBLOCK_KEY)
		parseCompoundStatement(function_ptr); //{ statement-list; }
	else if(IsNewStatement())		//statement
	{
		// simple-statement ;
		ParseStatement(function_ptr);
		//the semicolon
		if(getTokenAppendIf(HCC_SEMICOLON, HccErrorManager::errMissingSemicolon)==0){
			Resync(HCC_SEMICOLON);		
			return;
		}
		skipEmptyStatements();
	}else{
		//recover from this possible error...		
		Resync(HCC_SEMICOLON);
		return;
	}
	//while
	if(getTokenAppendIf(HCC_WHILE, HccErrorManager::errMissingWhile)==0){
		Resync(HCC_SEMICOLON);
		return;
	}
	//(
	if(getTokenAppendIf(HCC_LPAREN, HccErrorManager::errMissingLeftParen)==0){
		Resync(HCC_SEMICOLON);
		return;
	}
	//expr	
	HCC_PARSE_TREE* tree_ptr = ParseExprList(function_ptr);
	if(tree_ptr!=NULL && tree_ptr->type_ptr!=NULL)
		HccTypeChecker::CheckBoolean(tree_ptr->type_ptr);
	delete tree_ptr;
	//)
	if(getTokenAppendIf(HCC_RPAREN, HccErrorManager::errMissingRightParen)==0){
		Resync(HCC_SEMICOLON);
		return;
	}
}

//--------------------------------------------
//
//	parseWhile()	- parses while statements like:
//
//				while(expr)
//				{
//					statement-list;
//				}
//
//		or
//
//				while(expr)
//					statement;
//--------------------------------------------
void HCCParser::parseWhile(Symbol *function_ptr)
{	
	//rule 1 for H++ statements: cannot be implemented outside a function scope
	if(function_ptr==NULL)
		HccErrorManager::Error(HccErrorManager::errStatementsMustBeInFunctionScope);

	getTokenAppend(); //while
	//(
	if(getTokenAppendIf(HCC_LPAREN, HccErrorManager::errMissingLeftParen)==0){
		Resync(HCC_RPAREN);
	}
	//expr	
	HCC_PARSE_TREE* tree_ptr = ParseExprList(function_ptr);
	if(tree_ptr!=NULL && tree_ptr->type_ptr!=NULL)
		HccTypeChecker::CheckBoolean(tree_ptr->type_ptr);
	delete tree_ptr;
	//)
	if(getTokenAppendIf(HCC_RPAREN, HccErrorManager::errMissingRightParen)==0){
		HCC_TOKEN_TYPE tokens[] = {
			HCC_IDENTIFIER,
			HCC_SEMICOLON,
			HCC_LBLOCK_KEY,
			HCC_RBLOCK_KEY			
		};
		Resync(tokens, sizeof(tokens)/sizeof(tokens[0]));		
	}
	if(token_type==HCC_LBLOCK_KEY)
		parseCompoundStatement(function_ptr);	//{ statement-list; }
	else if(IsNewStatement())		//statement
		ParseStatement(function_ptr);
	else if(token_type!=HCC_SEMICOLON){
		//TODO:error recuperation
		Resync(HCC_SEMICOLON);
	}
}

//--------------------------------------------
//
//	parseFor()	-	parses a for statement like:
//
//			for(;;)
//				compound-statement	| 
//				statement	;		| 
//				empty-statement
//
//		or
//			for(expr1;expr2;expr3)
//				compound-statement	| 
//				statement	;		| 
//				empty-statement
//
//--------------------------------------------
void HCCParser::parseFor(Symbol *function_ptr)
{
	//rule 1 for H++ statements: cannot be implemented outside a function scope
	if(function_ptr==NULL)
		HccErrorManager::Error(HccErrorManager::errStatementsMustBeInFunctionScope);

	getTokenAppend();//for
	// (
	if(getTokenAppendIf(HCC_LPAREN, HccErrorManager::errMissingLeftParen)==0){
		Resync(HCC_SEMICOLON);
		return;
	}
	//expr1	
	/*
	HCC_PARSE_TREE* expr1_ptr = parseCommaExprList(function_ptr);
	delete expr1_ptr;
	*/
	//variable-decl | expr1;
	if(token_type!=HCC_SEMICOLON)
	{
		ParseStatement(function_ptr);
	}
	// ;
	if(getTokenAppendIf(HCC_SEMICOLON, HccErrorManager::errMissingSemicolon)==0){
		Resync(HCC_SEMICOLON);		
	}
	//expr2	
	HCC_PARSE_TREE* expr2_ptr = ParseExprList(function_ptr);
	if(expr2_ptr!=NULL && expr2_ptr->type_ptr!=NULL)
		HccTypeChecker::CheckBoolean(expr2_ptr->type_ptr);

	delete expr2_ptr;
	// ;
	if(getTokenAppendIf(HCC_SEMICOLON, HccErrorManager::errMissingSemicolon)==0){
		Resync(HCC_RPAREN);		
	}
	
	//if not an empty-expression...
	if(token_type!=HCC_RPAREN)
	{		
		//expr3		
		HCC_PARSE_TREE* expr3_ptr = parseCommaExprList(function_ptr);
		delete expr3_ptr;
	}
	//)
	if(getTokenAppendIf(HCC_RPAREN, HccErrorManager::errMissingRightParen)==0){
		Resync(HCC_RPAREN);		
	}

	if(token_type==HCC_LBLOCK_KEY)
		parseCompoundStatement(function_ptr);	//{ statement-list; }
	else if(IsNewStatement())		//statement
		ParseStatement(function_ptr);
	else if(token_type!=HCC_SEMICOLON){
		//TODO:error recuperation
		Resync(HCC_SEMICOLON);
	}

}

// S E L E C T I O N - S T A T E M E N T 
//--------------------------------------------
//
//	parseIfStatement()	- parses if statements like:
//
//					if(expr)
//						compound-statement | statement ;
//
//			or
//					if(expr)
//						statement ;
//			or
//					if(expr)
//						statement ;
//					else
//						statement ;
//			or
//					if(expr)
//						compound-statement | statement ;
//					else
//						compound-statement | statement ;
//
//--------------------------------------------
HCC_TOKEN_TYPE HCCParser::parseIfStatement(Symbol *function_ptr)
{	
	//rule 1 for H++ statements: cannot be implemented outside a function scope
	if(function_ptr==NULL)
		HccErrorManager::Error(HccErrorManager::errStatementsMustBeInFunctionScope);

	//assert(function_ptr!=NULL);
	HCC_TOKEN_TYPE last_stmt = token_type;
	//'I F'   B L O C K 
	getTokenAppend();// if
	//(
	if(getTokenAppendIf(HCC_LPAREN, HccErrorManager::errMissingLeftParen)==0){
		Resync(HCC_SEMICOLON);
		return HCC_TOKEN_ERROR;
	}
	//expr	
	HCC_PARSE_TREE* tree_ptr = ParseExprList(function_ptr);
	if(tree_ptr!=NULL && tree_ptr->type_ptr!=NULL)
		HccTypeChecker::CheckBoolean(tree_ptr->type_ptr);

	delete tree_ptr;
	//)
	if(getTokenAppendIf(HCC_RPAREN, HccErrorManager::errMissingRightParen)==0){
		Resync(HCC_SEMICOLON);
		return HCC_TOKEN_ERROR;
	}
	
	if(token_type==HCC_LBLOCK_KEY)
	{
		parseCompoundStatement(function_ptr);	//{ statement-list; }		
	}
	else if(IsNewStatement()){		//simple-statement requires a ';'
		last_stmt = ParseStatement(function_ptr);

		if(token_type==HCC_ELSE)
		{		
			HccErrorManager::Error(HccErrorManager::errMissingSemicolon, " before keyword \'else\'\n");
			Resync(HCC_SEMICOLON);			
		}	
		// ;
		if(getTokenAppendIf(HCC_SEMICOLON, HccErrorManager::errMissingSemicolon)==0){
			Resync(HCC_SEMICOLON);		
		}
		//to silent the parser in the stmt-list check for semicolon...
		if(last_stmt==HCC_BREAK || last_stmt==HCC_CONTINUE || last_stmt==HCC_RETURN)
			last_stmt = HCC_IF;
	}
	//'E L S E'   B L O C K 
	if(token_type==HCC_ELSE)
	{
		getTokenAppend();//else
		if(token_type==HCC_LBLOCK_KEY)
			parseCompoundStatement(function_ptr);	//{ statement-list; }
		else if(IsNewStatement())		//statement
			last_stmt = ParseStatement(function_ptr);
		else if(token_type!=HCC_SEMICOLON){			
			Resync(HCC_SEMICOLON);			
		}
	}

	//to silent the parser in the stmt-list check for semicolon...
	if(last_stmt==HCC_IDENTIFIER)
		last_stmt = HCC_IF;

	return last_stmt;
}

void HCCParser::parseSwitch(Symbol *function_ptr)
{
	//rule 1 for H++ statements: cannot be implemented outside a function scope
	if(function_ptr==NULL)
		HccErrorManager::Error(HccErrorManager::errStatementsMustBeInFunctionScope);

	getTokenAppend(); //switch
	//(
	if(getTokenAppendIf(HCC_LPAREN, HccErrorManager::errMissingLeftParen)==0){
		Resync(HCC_RPAREN);		
	}
	//expr	
	HCC_PARSE_TREE* tree_ptr = ParseExprList(function_ptr);
	if(tree_ptr!=NULL && tree_ptr->type_ptr!=NULL)
		if(HccTypeChecker::IntegerOperands(tree_ptr->type_ptr, tree_ptr->type_ptr)==false)
		{
			HccErrorManager::Error(HccErrorManager::errInvalidConstant, _T(", must be an integer value."));
			//
		}

	delete tree_ptr;
	//)
	if(getTokenAppendIf(HCC_RPAREN, HccErrorManager::errMissingRightParen)==0){
		Resync(HCC_LBLOCK_KEY);		
	}

	// {
	if(getTokenAppendIf(HCC_LBLOCK_KEY, HccErrorManager::errMissingStartBlock)==0)
	{
		Resync(HCC_RBLOCK_KEY);
		return;
	}

	volatile HCC_TOKEN_TYPE prev_block_type = HCC_TOKEN_ERROR;
	while(1)
	{	
		if(token_type==HCC_CASE || token_type==HCC_DEFAULT){
			//to determine if the default was the last block; else, yield an error...
			if(prev_block_type==HCC_DEFAULT)
			{
				if(token_type==HCC_DEFAULT)
					HccErrorManager::Error(HccErrorManager::errToManySwitchDefaultBlocks);
				else
					HccErrorManager::Error(HccErrorManager::errDefaultBlockMustBeLastInSwitch);
			}
			prev_block_type = token_type;
			parseCaseBranch(function_ptr);			
		}

		if(token_type==HCC_SEMICOLON){
			getTokenAppend();
			skipEmptyStatements();			
		}
		if(token_type==HCC_BREAK){
			getTokenAppend();
		}

		if(token_type==HCC_SEMICOLON){
			getTokenAppend();
			skipEmptyStatements();			
		}
		
		if(token_type!=HCC_CASE && token_type!=HCC_DEFAULT)
			break;
	}

	// }
	if(getTokenAppendIf(HCC_RBLOCK_KEY, HccErrorManager::errMissingEndBlock)==0)
	{
		Resync(HCC_SEMICOLON);
		return;
	}
}

void HCCParser::parseCaseBranch(Symbol *function_ptr)
{
	//rule 1 for H++ statements: cannot be implemented outside a function scope
	if(function_ptr==NULL)
		HccErrorManager::Error(HccErrorManager::errStatementsMustBeInFunctionScope);

	volatile bool unarySign = false;
	HCC_TOKEN_TYPE sign = HCC_TOKEN_ERROR;
	if(token_type==HCC_DEFAULT)
	{
		getTokenAppend();
		if(getTokenAppendIf(HCC_COLON, HccErrorManager::errMissingColon)==0){
			Resync(HCC_SEMICOLON);
			return;
		}
		goto PARSE_BRANCH_BLOCK;
	}else{
		if(getTokenAppendIf(HCC_CASE, HccErrorManager::errMissingCaseInSwitch)==0)
		{
			HCC_TOKEN_TYPE tokens[] = {
								HCC_CASE,
								HCC_BREAK,
								HCC_RBLOCK_KEY,
								HCC_SEMICOLON,
							};
			Resync(tokens, sizeof(tokens)/sizeof(tokens[0]));
			return;
		}
	}
	//		
	if((unarySign = IsUnarySign())) //TODO
	{
		sign = token_type;
		getTokenAppend();
	}
	if(token_type!=HCC_NUMBER && token_type!=HCC_CHARACTER && token_type!=HCC_IDENTIFIER)
	{
		__tstring info = _T("constant | number | char, before \'case\'.");
		HccErrorManager::Error(HccErrorManager::errMissingIdentifier, info);
		Resync(HCC_COLON);
	}else{			
		if(token_type==HCC_IDENTIFIER)
		{
			Symbol* id_ptr = NULL;
			if((id_ptr = getSymbolFromIdentifier(false, function_ptr))==0)
				HccErrorManager::Error(HccErrorManager::errUndeclaredIdentifier);
			else{				
				icode_ptr->append(id_ptr);				

				if(id_ptr->getDeclDefinition().identifier_type()!=DECL_CONSTANT)
					HccErrorManager::Error(HccErrorManager::errInvalidConstant);
			}
		}else if(token_type==HCC_NUMBER)
		{
			if(token_ptr->dataType!=HCC_INTEGER){
				__tstring info = _T(" must be an integer type.");
				HccErrorManager::Error(HccErrorManager::errInvalidConstant, info);
			}
			Symbol* snum_ptr = AppendNumber(token_ptr);
			getToken();
		}		

		//append the colon as a separator between the branch condition and the branch code...
		icode_ptr->append(HCC_COLON);
	}

	if(getTokenAppendIf(HCC_COLON, HccErrorManager::errMissingColon)==0){
			HCC_TOKEN_TYPE tokens[] = {
								HCC_CASE,
								HCC_BREAK,
								HCC_LBLOCK_KEY,
								HCC_RBLOCK_KEY,
								HCC_SEMICOLON,
							};
			Resync(tokens, sizeof(tokens)/sizeof(tokens[0]));
	}
	
PARSE_BRANCH_BLOCK:
	//avoid processing the break statement...
	if(token_type==HCC_BREAK)
		return;
	else if(token_type==HCC_LBLOCK_KEY)
		parseCompoundStatement(function_ptr);	//{ statement-list; }
	else if(IsNewStatement())		//statement
		ParseStatement(function_ptr);
}


HCC_TOKEN_TYPE HCCParser::parseCompoundStatement(Symbol *function_ptr)
{	
	/*
		//Statement of this form:

		'{' statement-seq_opt '}'

		//where statement-seq_opt is:

		statement-seq:

			statement
			statement-seq  statement
	*/
	getTokenAppend(); // {
	HCC_TOKEN_TYPE stmt = ParseStatementList(HCC_RBLOCK_KEY, function_ptr); //{ statement-list }
	if(getTokenAppendIf(HCC_RBLOCK_KEY, HccErrorManager::errMissingEndBlock)==0){ //}
		Resync(HCC_RBLOCK_KEY);
	}
	return stmt;
}

void HCCParser::Resync(HCC_TOKEN_TYPE token)
{
	Resync(&token, 1);
}

void HCCParser::Resync(HCC_TOKEN_TYPE *tokens, int sz)
{
	bool errorFlag = true;	
	for(int x=0;x<sz && errorFlag; x++)
		errorFlag = (tokens[x]!=token_type) && !IsNewStatement();

	if(errorFlag){
		//unexpected token
		__tstring info;
		ParserError errorCode = token_type == HCC_EOF 
												? HccErrorManager::errUnexpectedEndOfFile 
												: HccErrorManager::errUnexpectedToken; //unexpected token								

		if(errorCode==HccErrorManager::errUnexpectedToken){
			info = _T("\'");
			info += token_ptr->token;
			info += _T("\'.");
		}		
		HccErrorManager::Error(errorCode, info);
		//Skip tokens...
		//Error Recuperation Mode: 
		//P A N I C  M O D E  R E C U P E R A T I O N...					
		int y= 0;
		while(tokens[y]!=token_type			&&
				!IsNewStatement()			&&
				token_type!=HCC_SEMICOLON	&& 
				token_type!=HCC_EOF)
		{
			getToken();
			y = (y + 1) % sz;
		}

		if(token_type==HCC_EOF && 
			errorCode!=HccErrorManager::errUnexpectedEndOfFile)
		{
			HccErrorManager::Error(HccErrorManager::errUnexpectedEndOfFile);
		}
	}
}


bool HCCParser::IsNewStatement()
{	
	return (token_type==HCC_IF 			|| 
			token_type==HCC_WHILE 		||
			token_type==HCC_DO 			||
			token_type==HCC_FOR 		||
			token_type==HCC_IDENTIFIER	||
			token_type==HCC_LBLOCK_KEY	||
			token_type==HCC_INCREMENT	||
			token_type==HCC_DECREMENT	||

			//jump-statements
			token_type==HCC_RETURN		||
			token_type==HCC_BREAK		||
			token_type==HCC_CONTINUE	||
			token_type==HCC_GOTO		||
			token_type==HCC_WITH		||
			token_type==HCC_NAMESPACE	||
			token_type==HCC_DEBUGGER);
}

void HCCParser::skipEmptyStatements()
{
	//remove all of the next empty statements...
	if(token_type==HCC_SEMICOLON)
	{
		while(token_type==HCC_SEMICOLON) //i.e.:  a = 4;;; ;; ;
			getToken();
	}
}

bool HCCParser::IsJumpStatement()
{
	return (token_type==HCC_BREAK		||
			token_type==HCC_CONTINUE	||
			token_type==HCC_GOTO		||
			token_type==HCC_RETURN);
}

void HCCParser::parseJumpStatement(Symbol *function_ptr)
{
	//rule 1 for H++ statements: cannot be implemented outside a function scope
	if(function_ptr==NULL)
		HccErrorManager::Error(HccErrorManager::errStatementsMustBeInFunctionScope);

	switch(token_type)
	{
	case HCC_BREAK:
	case HCC_CONTINUE:
		{
			getTokenAppend();
		}
		break;
	case HCC_GOTO:
		{
			getTokenAppend();
			if(token_type!=HCC_IDENTIFIER)
			{
				__tstring info = _T("after \'goto\'.");
				HccErrorManager::Error(HccErrorManager::errMissingIdentifier, info);
				Resync(HCC_SEMICOLON);
				return;
			}
			assert(function_ptr->getDeclDefinition().function.symbol_table_ptr!=0);
			Symbol* _label_ptr = 
						function_ptr->getDeclDefinition().function.symbol_table_ptr->find(token_ptr->String());


			if(_label_ptr==NULL)
			{
				//insert label in the local symbol table...
				_label_ptr = function_ptr->getDeclDefinition().function.symbol_table_ptr->insert(token_ptr->String());

				//marked as a proc block label
				_label_ptr->setType(HCC_IDENTIFIER);
				_label_ptr->getDeclDefinition().set_identifier_type(DECL_PROC_LABEL);
				_label_ptr->getDeclDefinition().set_identifier_scope(SCOPE_LOCAL);

			}else{
				//check it's declaration type...
				if(_label_ptr->getDeclDefinition().identifier_type()!=DECL_PROC_LABEL)
				{
					__tstring info = _T("; token \'");
					info += token_ptr->String();
					info += _T("\'.");
					//cannot use this identifier as a block destination label;
					HccErrorManager::Error(HccErrorManager::errInvalidProcLabel, info);
				}
			}

			//keep the label name as reference to confirm it was declared somewhere in the function
			ref_proc_labels.insert(_label_ptr->getName());

			assert(_label_ptr!=NULL);
			icode_ptr->append(_label_ptr);
			getTokenAppend();
		}
		break;
	case HCC_RETURN:
		{
			getTokenAppend();
			HCC_PARSE_TREE* tree_ptr = ParseExprList(function_ptr);
			TypeSpecifier* return_type = HccTypeChecker::ts_type_void_ptr;
			//if no type in expression, or not expression at all,
			//type is void
			if(tree_ptr!=NULL && tree_ptr->type_ptr!=NULL)
				return_type = tree_ptr->type_ptr;

			if(function_ptr!=NULL)
			{
				bool bTargetTypeIsPointer = function_ptr->getDeclDefinition().function.bReturnTypeIsPointer;
				if(bTargetTypeIsPointer && 
					return_type->getDataType()==HCC_INTEGER && 
					return_type->getDataTypeSize()==sizeof(int))
				{
					//do nothing for this case...
				}else{
					HccTypeChecker::CheckCompatibleAssignment(function_ptr->getDeclDefinition().function.return_type,
															  return_type,
															  HccErrorManager::errIncompatibleTypes,
															  _T("for expression in return of function."),
															  bTargetTypeIsPointer);
				}
			}
			delete tree_ptr;
		}
		break;
	};
}

void HCCParser::parseTryBlock(Symbol *function_ptr)
{
	//rule 1 for H++ statements: cannot be implemented outside a function scope
	if(function_ptr==NULL)
		HccErrorManager::Error(HccErrorManager::errStatementsMustBeInFunctionScope);

	getTokenAppend();//try
	if(token_type!=HCC_LBLOCK_KEY)	//{
	{
		__tstring info = _T(" after \'try\'.");
		HccErrorManager::Error(HccErrorManager::errMissingStartBlock,info);
		Resync(HCC_LBLOCK_KEY);
	}
	//{ statement-list }
	parseCompoundStatement(function_ptr);

	do{
		if(getTokenAppendIf(HCC_CATCH, HccErrorManager::errMissingCatchExceptBlock)==0){
			Resync(HCC_SEMICOLON);		
			return;
		}
		//(
		if(getTokenAppendIf(HCC_LPAREN, HccErrorManager::errMissingLeftParen)==0){
			Resync(HCC_RPAREN);		
		}
		//expr	
		ParseExprList(function_ptr);
		//)
		if(getTokenAppendIf(HCC_RPAREN, HccErrorManager::errMissingRightParen)==0){
			Resync(HCC_LBLOCK_KEY);		
		}

		if(token_type!=HCC_LBLOCK_KEY)	//{
		{
			__tstring info = _T(" after \'catch/except\'.");
			HccErrorManager::Error(HccErrorManager::errMissingStartBlock,info);
			Resync(HCC_LBLOCK_KEY);
			return;
		}
		//{ statement-list }
		parseCompoundStatement(function_ptr);
	}while(HCC_CATCH==token_type);
	//
	skipEmptyStatements();
}

bool HCCParser::IsUnarySign()
{
	return (token_type==HCC_MINUS_OP || token_type==HCC_PLUS_OP);
}

bool HCCParser::StatementRequiresSemicolon(HCC_TOKEN_TYPE stmt_type)
{
	return (stmt_type!=HCC_IF			&&
			stmt_type!=HCC_WHILE		&&
			stmt_type!=HCC_FOR			&&
			stmt_type!=HCC_SWITCH		&&
			stmt_type!=HCC_LBLOCK_KEY 	&&
			stmt_type!=HCC_TRY			&&
			stmt_type!=HCC_WITH			&&
			stmt_type!=HCC_NAMESPACE	&&
			stmt_type!=HCC_RBLOCK_KEY	&&
			stmt_type!=HCC_SEMICOLON	&&
			stmt_type!=HCC_GOTO			&&
			stmt_type!=HCC_DEBUGGER);	//&& stmt_type!=HCC_IDENTIFIER);
}

void HCCParser::parseWithStatement(Symbol *function_ptr)
{
	//rule 1 for H++ statements: cannot be implemented outside a function scope
	if(function_ptr==NULL)
		HccErrorManager::Error(HccErrorManager::errStatementsMustBeInFunctionScope);

	getTokenAppend(); //with
	//(
	if(getTokenAppendIf(HCC_LPAREN, HccErrorManager::errMissingLeftParen)==0){
		Resync(HCC_IDENTIFIER);		
	}
	Symbol* id_ptr = NULL;
	if(token_type==HCC_IDENTIFIER)
		id_ptr = getSymbolFromIdentifier(true, function_ptr);		

	icode_ptr->append(id_ptr);

	//identifier is null?
	if(id_ptr==NULL)
		Resync(HCC_RPAREN);		
	
	//)
	if(getTokenAppendIf(HCC_RPAREN, HccErrorManager::errMissingRightParen)==0){
		Resync(HCC_LBLOCK_KEY);		
	}

	if(getTokenAppendIf(HCC_LBLOCK_KEY, HccErrorManager::errMissingStartBlock)==0){
		Resync(HCC_PERIOD);		
	}

	HCC_PARSE_TREE* tree_ptr = NULL;
	while(1)
	{
		if(id_ptr!=NULL)
			tree_ptr = parseObjectInstanceMember(id_ptr->getTypeSpecifier().getBaseTypeSpec());
		else
			tree_ptr = ParseExprList(function_ptr);			
		
		delete tree_ptr;
		tree_ptr = NULL;

		// ;
		if(getTokenAppendIf(HCC_SEMICOLON, HccErrorManager::errMissingSemicolon)==0){
			HCC_TOKEN_TYPE tokens[] = {
				HCC_PERIOD,
				HCC_SEMICOLON,
				HCC_RBLOCK_KEY,
				//HCC_IDENTIFIER,
			};
			Resync(tokens, sizeof(tokens)/sizeof(tokens[0]));
		}
		if(token_type==HCC_LBLOCK_KEY || token_type==HCC_RBLOCK_KEY)
			break;
		if(token_type==HCC_EOF)
			break;
	}
	
	if(getTokenAppendIf(HCC_RBLOCK_KEY, HccErrorManager::errMissingEndBlock)==0){
		HCC_TOKEN_TYPE tokens[] = {
			HCC_SEMICOLON,
			HCC_RBLOCK_KEY,
			HCC_LBLOCK_KEY,
			HCC_IDENTIFIER,
		};
		Resync(tokens, sizeof(tokens)/sizeof(tokens[0]));		
	}
}

Symbol* HCCParser::insert_symbol(const TCHAR *symbol_name, DECLARATION_TYPE declType)
{
	Symbol* symbol_ptr = symbol_table_ptr->find(symbol_name);	
	if(symbol_ptr==NULL){
		symbol_ptr = symbol_table_ptr->insert(symbol_name);
		symbol_ptr->getDeclDefinition().set_identifier_type(declType);

		return symbol_ptr;
	}
	//if a symbol with this name already exists, then return null as an advise that
	//this symbol is already in the symbol table, or there is an identifier redefinition.
	return NULL;
}

Symbol* HCCParser::find_symbol(const TCHAR *symbol_name)
{
	Symbol* symbol_ptr = symbol_table_ptr->find(symbol_name);	
	if(symbol_ptr==NULL){
		__tstring info = "\'";
		info += symbol_name;
		info += "\'.";
		HccErrorManager::Error(HccErrorManager::errUndeclaredIdentifier, info);
		//enter it anyway...
		symbol_ptr = symbol_table_ptr->insert(symbol_name);
	}
	return symbol_ptr;
}

void HCCParser::CopyQuotedString(TCHAR *target, const TCHAR *quoted_source)
{
	int length = _tcslen(quoted_source) - 2;
	_tcsncpy(target, &quoted_source[1], length);
	target[length] = _T('\0');
}


//---------------------------------------------------------------
//
//
//	T H E   D E C L A R A T I O N   P A R S I N G   R E D U C T I O N   P R O C E D U R E S 
//
//
//---------------------------------------------------------------


//------------------------------------------------------------------
//
//	namespace <identifier> {
//
//		declaration-list new-declarator
//	}
//
//------------------------------------------------------------------
void HCCParser::parseNameSpace()
{
	getToken(); //namespace
	//identifier
	if(token_type==HCC_IDENTIFIER)
	{
		__tstring ns;
		if(current_namespace_ptr!=NULL){
			ns = current_namespace_ptr->getName();
			ns += _T("::");
		}
		ns += token_ptr->String();
		Symbol* namespace_ptr = symbol_table_ptr->find(ns);
		if(namespace_ptr==NULL)
		{
			namespace_ptr = symbol_table_ptr->insert(ns);
			namespace_ptr->getDeclDefinition().set_identifier_type(DECL_NAMESPACE);
			namespace_ptr->getTypeSpecifier().set_specifier(DSPEC_NAMESPACE);
		}
		//SET TO CURRENT
		prev_namespace_ptr		= current_namespace_ptr;
		current_namespace_ptr	= namespace_ptr;
		//put in the set of active namespaces...
		active_namespaces.insert(namespace_ptr);

		if(getTokenIf(HCC_IDENTIFIER, HccErrorManager::errMissingIdentifier)==0){
			Resync(HCC_LBLOCK_KEY);		
		}
		if(getTokenIf(HCC_LBLOCK_KEY, HccErrorManager::errMissingStartBlock)==0){
			Resync(HCC_SEMICOLON);		
		}
		ParseStatementList(HCC_RBLOCK_KEY);	
		if(getTokenIf(HCC_RBLOCK_KEY, HccErrorManager::errMissingEndBlock)==0){
			HCC_TOKEN_TYPE tokens[] = {
				HCC_LBLOCK_KEY,			
				HCC_NAMESPACE,			
				HCC_CLASS,
				HCC_ENUM,
				HCC_TYPENAME,
				HCC_IDENTIFIER,
			};
			Resync(tokens, sizeof(tokens)/sizeof(tokens[0]));		
		}
		//SET TO PREVIOUS
		current_namespace_ptr = prev_namespace_ptr;		
		prev_namespace_ptr = NULL;

		//CLEAR THE NAMESPACE DOMAIN
		if(namespace_ptr==current_namespace_ptr)
			current_namespace_ptr = NULL;

		//put in the set of active namespaces...
		active_namespaces.erase(namespace_ptr);
	}
}

//--------------------------------------------
//	using namespace <identifier>;
//
//	aliases:
//	
//	using <identifier1> = <identifier2>;
//
//--------------------------------------------
void HCCParser::parseUsingNamespace(Symbol* function_ptr)
{
	if(token_type!=HCC_USING){
		HccErrorManager::Error(HccErrorManager::errInvalidStatement, _T("missing \'using\'."));
		Resync(HCC_NAMESPACE);
	}
	else
		getToken();

	if(token_type==HCC_IDENTIFIER)
	{
		Symbol* alias_ptr = NULL;
		if(function_ptr==NULL)
		{
			alias_ptr = symbol_table_ptr->find(token_ptr->String());
			if(alias_ptr==NULL)
			{
				alias_ptr = symbol_table_ptr->insert(token_ptr->String());
				alias_ptr->getDeclDefinition().set_identifier_scope(SCOPE_GLOBAL);

			}else
				goto __IDENTIFIER_REDECLARED;
		}else{
			//alias_ptr = function_ptr->getTypeSpecifier().getSymbolTable()->find(token_ptr->String());
			alias_ptr = function_ptr->getDeclDefinition().function.symbol_table_ptr->find(token_ptr->String());
			if(alias_ptr==NULL)
			{
				//alias_ptr = function_ptr->getTypeSpecifier().getSymbolTable()->insert(token_ptr->String());
				alias_ptr = function_ptr->getDeclDefinition().function.symbol_table_ptr->insert(token_ptr->String());
				alias_ptr->getDeclDefinition().set_identifier_scope(SCOPE_LOCAL);
			}else
__IDENTIFIER_REDECLARED:
				HccErrorManager::Error(HccErrorManager::errRedeclaredIdentifier, _T(": alias name already used."));
		}
		assert(alias_ptr!=NULL);
		if(alias_ptr!=NULL)
			alias_ptr->getDeclDefinition().set_identifier_type(DECL_SYMBOL_USER_ALIAS);
		
		getToken();	//<id1>
		if(getTokenIf(HCC_ASSIGN_OP, HccErrorManager::errInvalidAssignment)!=0)
		{
			if(token_type==HCC_IDENTIFIER)
			{
				Symbol* id_ptr = getSymbolFromIdentifier(true, function_ptr);
				//now, the owner of this alias, is this identifier that must exist.
				alias_ptr->setOwner(id_ptr);
			}else
				goto __EXPECTED_IDENTIFIER_ERROR;
		}else
__EXPECTED_IDENTIFIER_ERROR:
			HccErrorManager::Error(HccErrorManager::errInvalidStatement, 
									_T(": H++ aliases must have an \'=\' followed by an identifier ")
									_T("as r-value in assignment"));
		
	}else if(token_type==HCC_NAMESPACE)
	{
		getToken();
		if(token_type==HCC_IDENTIFIER)
		{
			Symbol* namespace_ptr = getSymbolFromIdentifier(true, NULL);
			//register it
			if(namespace_ptr!=NULL)
				using_namespaces.insert(namespace_ptr);
		}else{
			HccErrorManager::Error(HccErrorManager::errInvalidStatement, _T("missing identifier after keyword \'namespace\'."));
			Resync(HCC_IDENTIFIER);
		}
	}else{
		HccErrorManager::Error(HccErrorManager::errInvalidStatement, _T("missing \'namespace\' or identifier."));
		Resync(HCC_IDENTIFIER);
	}
		
	Resync(HCC_SEMICOLON);
}


void HCCParser::parseDeclarations(Symbol *id_proc_ptr)
{	
	switch(token_type)
	{
		//data types
		case HCC_VOID:
			{
				//TODO: for functions/member functions only...
				HccErrorManager::Error(HccErrorManager::errUnimplementedFeature, _T(" (unknown type declaration)."));
			}
			break;
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
		//for locals/globals
		case HCC_AUTO:
		case HCC_REGISTER:
		case HCC_STATIC:				
		case HCC_VOLATILE:
		case HCC_EXTERN:
			{
				parseVariableDeclarations(id_proc_ptr, NULL);							//DONE
			}
			break;
		case HCC_CONST:
			{
			//
				parseConstantDefinitions(id_proc_ptr, current_namespace_ptr);			//DONE				
			}
			break;
		case HCC_ENUM:
			{
				Symbol* type_symbol_ptr = 
						parseEnumerationType(id_proc_ptr, current_namespace_ptr);		//DONE			
			}
			break;
		//for new types			
		case HCC_CLASS:
		case HCC_STRUCT:
			{
				while(token_type==HCC_CLASS || token_type==HCC_STRUCT){
					//This symbol belongs to the global symbol table
					Symbol* class_type_symbol_ptr = 
									parseClassStructTypes(current_namespace_ptr);		//DONE					
				}
			}
			break;
		case HCC_TYPENAME:
			{
				//when defining new sub-types like C++ typedef's
				Symbol* type_symbol_ptr = 
						parseTypeDefinitions(id_proc_ptr, current_namespace_ptr);		//DONE
			}
			break;			
		default:
			//unknown language declaration
			HccErrorManager::Error(HccErrorManager::errUnimplementedFeature, _T(" (unknown type declaration)."));
			break;
	};
}

TypeSpecifier* HCCParser::parseTypeSpec(Symbol* ns_class_ptr, Symbol* function_ptr, bool* pIsAbstractType, DataTypeModifier* pTypeModifier)
{

	if(pIsAbstractType)
		*pIsAbstractType = false;
	Symbol* type_ptr = NULL;
	TypeSpecifier* base_type_ptr = NULL;

	if(pTypeModifier)
		*pTypeModifier = HCC_SIGNED_TYPE; //by default, all scalar build-in types are signed integer
	//now, must be a type identifier...
	switch(token_type)
	{
	case HCC_VOID:
		base_type_ptr = HccTypeChecker::ts_type_void_ptr;
		break;
	case HCC_BOOL:
		base_type_ptr = HccTypeChecker::ts_type_boolean_ptr;		
		break;
	case HCC_CHAR:
		base_type_ptr = HccTypeChecker::ts_type_char_ptr;		
		break;
	case HCC_SHORT:
		base_type_ptr = HccTypeChecker::ts_type_short_ptr;		
		break;
	case HCC_LONG:
		base_type_ptr = HccTypeChecker::ts_type_long_ptr;		
		break;
	case HCC_INT:
		base_type_ptr = HccTypeChecker::ts_type_int_ptr;		
		break;
	case HCC_INT16:
		base_type_ptr = HccTypeChecker::ts_type_Int16_ptr;		
		break;
	case HCC_FLOAT:
		base_type_ptr = HccTypeChecker::ts_type_float_ptr;		
		break;
	case HCC_INT32:
		base_type_ptr = HccTypeChecker::ts_type_Int32_ptr;		
		break;
	case HCC_INT64:
		base_type_ptr = HccTypeChecker::ts_type_Int64_ptr;		
		break;
	case HCC_DOUBLE:
		base_type_ptr = HccTypeChecker::ts_type_double_ptr;		
		break;
	case HCC_UNSIGNED:
		{
		if(pTypeModifier)
			*pTypeModifier = HCC_UNSIGNED_TYPE; //if specified by user, this is now unsigned

			getToken();
			switch(token_type)
			{
				case HCC_INT:	
				case HCC_INT32:
					base_type_ptr = HccTypeChecker::ts_type_unsigned_ptr;
					getToken();
					break;
				case HCC_LONG:
					base_type_ptr = HccTypeChecker::ts_type_ulong_ptr;
					getToken();
					break;
				case HCC_SHORT:
				case HCC_INT16:
					base_type_ptr = HccTypeChecker::ts_type_ushort_ptr;
					getToken();
					break;
				case HCC_CHAR:
					base_type_ptr = HccTypeChecker::ts_type_uchar_ptr;
					getToken();
					break;
				case HCC_INT64:
					//NOTE: H++ allows signed Int64 only
					HccWarningManager::Warning(HccWarningManager::warnHppInt64IsSignedAlways);
					base_type_ptr = HccTypeChecker::ts_type_Int64_ptr;
					getToken();
					break;
				case HCC_VOID:
				case HCC_BOOL:
				case HCC_FLOAT:
				case HCC_DOUBLE:				
				{
					HccErrorManager::Error(HccErrorManager::errInvalidType);
					HccErrorManager::Error(HccErrorManager::errUnsignedUseWithIntegersOnly);
					getToken();					
				}
				break;
				default:
					base_type_ptr = HccTypeChecker::ts_type_unsigned_ptr;
				break;
			};

			return base_type_ptr;
		}
		break;
	case HCC_SIGNED:
		{	
			base_type_ptr = HccTypeChecker::ts_type_signed_ptr;
			getToken();
			switch(token_type)
			{				
				case HCC_INT:
				case HCC_INT32:
					//base_type_ptr = HccTypeChecker::ts_type_signed_ptr;
					getToken();
					break;
				case HCC_LONG:
					base_type_ptr = HccTypeChecker::ts_type_long_ptr;
					getToken();
					break;
				case HCC_SHORT:
				case HCC_INT16:
					base_type_ptr = HccTypeChecker::ts_type_short_ptr;
					getToken();
					break;
				case HCC_INT64:
					base_type_ptr = HccTypeChecker::ts_type_Int64_ptr;
					getToken();
					break;
				case HCC_CHAR:
					base_type_ptr = HccTypeChecker::ts_type_char_ptr;
					getToken();
					break;
				case HCC_VOID:
				case HCC_BOOL:
				case HCC_FLOAT:
				case HCC_DOUBLE:
				{
					HccErrorManager::Error(HccErrorManager::errInvalidType);
					HccErrorManager::Error(HccErrorManager::errSignedUseWithIntegersOnly);
					getToken();					
				}
				break;
			};
			return base_type_ptr;
		}
		break;
	case HCC_STRING:
		base_type_ptr = HccTypeChecker::ts_type_string_ptr;		
		break;
	case HCC_ENUM:
		type_ptr = parseEnumerationType(NULL, ns_class_ptr);
		if(type_ptr!=NULL)
			return &type_ptr->getTypeSpecifier();
		break;
	case HCC_CLASS:
	case HCC_STRUCT:
		type_ptr = parseClassStructTypes(ns_class_ptr);
		if(type_ptr!=NULL)
			return &type_ptr->getTypeSpecifier();
		break;
	case HCC_IDENTIFIER:	
		{
			//for user-defined types
			HCC_TOKEN_TYPE prev_token	= token_type;
			SYMBOL_TABLE::LPSYMBOL symbol_ptr = getSymbolFromIdentifier(true, function_ptr);
			if(symbol_ptr!=NULL && 
				(symbol_ptr->getDeclDefinition().identifier_type()==DECL_NEW_TYPE ||
				 symbol_ptr->getDeclDefinition().identifier_type()==DECL_NEW_ABSTRACT_TYPE)
				)
			{
				if(pIsAbstractType)
					*pIsAbstractType = (symbol_ptr->getDeclDefinition().identifier_type()==DECL_NEW_ABSTRACT_TYPE);
				DECLARE_SPEC_TYPE specType = symbol_ptr->getTypeSpecifier().specifier();
				//
				if(specType==DSPEC_CLASS || specType==DSPEC_ENUM || specType==DSPEC_SIMPLE)
					return &symbol_ptr->getTypeSpecifier();
			}
			//not using a build in type, or not a type at all?
			HccErrorManager::Error(HccErrorManager::errInvalidType);
			HccErrorManager::Error(HccErrorManager::errNotATypeIdentifier);
			//use an integer type by default...
			base_type_ptr = HccTypeChecker::ts_type_int_ptr;
			//
			return base_type_ptr;

		}
		break;
	default:
		//not using a build in type, or not a type at all?
		HccErrorManager::Error(HccErrorManager::errInvalidType);
		HccErrorManager::Error(HccErrorManager::errNotATypeIdentifier);
		//use an integer type by default...
		base_type_ptr = HccTypeChecker::ts_type_int_ptr;
		
		if(token_type!=HCC_IDENTIFIER && token_type!=HCC_SEMICOLON)
			getToken();
		
		return base_type_ptr;
		break;
	};

	getToken();
	return base_type_ptr;
}

extern GlobalVariables g_program_global_constants;
void HCCParser::parseConstantDefinitions(Symbol *id_ptr, Symbol *ns_class_ptr, MemberAccessType accessType)
{
	//skip the 'const' keyword
	getToken();
	//get the type specifier
	TypeSpecifier* base_type_ptr = parseTypeSpec();

	while(token_type==HCC_IDENTIFIER)
	{
		//'const' <type-specifier> identifier = const-expr, identifier = const-expr,...;
		Symbol* const_ptr = NULL;
		__tstring class_name;
		if(ns_class_ptr!=NULL)
		{
			//C L A S S   C O N S T A N T			
			if(ns_class_ptr!=NULL){
				class_name = ns_class_ptr->String();
				class_name += _T("::");
			}
		}
		//G L O B A L / C L A S S / N A M E S P A C E   C O N S T A N T 
		if(id_ptr==NULL){
			const_ptr = insert_symbol((class_name + token_ptr->String()).c_str(), DECL_CONSTANT);
			if(const_ptr!=NULL)
			{
				//for the help in code generation...
				g_program_global_constants.push_back(const_ptr);
			}
		}
		else
			const_ptr = id_ptr->getDeclDefinition().function.symbol_table_ptr->insert(token_ptr->String());
		
		if(const_ptr==NULL)
		{
			getToken();
			HccErrorManager::Error(HccErrorManager::errRedeclaredIdentifier, 
									_T("(cannot be a constant, because already was declared.)"));
			HCC_TOKEN_TYPE tokens[] = {
				HCC_IDENTIFIER,
				HCC_SEMICOLON,
				HCC_COMMA_OP,
			};
			Resync(tokens, sizeof(tokens)/sizeof(tokens[0]));
		}else{
			const_ptr->getDeclDefinition().set_identifier_type(DECL_CONSTANT);
			//the access modifier in case of a const data member...
			const_ptr->getDeclDefinition().set_member_access_type(accessType);
			//skip identifier
			getToken();
			//add it to the local constant definitions in this function...
			const_ptr->getTypeSpecifier().setBaseTypeSpec(base_type_ptr);
			if(id_ptr!=NULL)
				id_ptr->getDeclDefinition().function.locals.stack_constants.constants.push_back(const_ptr);

			//must be an equal token
			if(getTokenIf(HCC_ASSIGN_OP, HccErrorManager::errMissingConstantExpression)!=0)
			//the constant expression follows
			{
				/*the constant expression can be:
					
					   a math constant expression;
					   a string constant;
					   
				*/
				HCC_TOKEN_TYPE sign = HCC_TOKEN_ERROR;
				if(IsUnarySign()){
					sign = token_type;
					getToken();
				}

				switch(token_type)
				{
				case HCC_NUMBER:
					{
						if(token_ptr->dataType==HCC_INTEGER)
						{
							const_ptr->getDeclDefinition().constant.value.Integer = 
								(sign==HCC_MINUS_OP) ? -token_ptr->value.Integer :
														token_ptr->value.Integer;							
						}else{
							//floating-point
							const_ptr->getDeclDefinition().constant.value.Double = 
								(sign==HCC_MINUS_OP) ? -token_ptr->value.Double :
														token_ptr->value.Double;
						}
						//skip this
						getToken();
					}
					break;
				case HCC_CHARACTER:
				case HCC_STRING_LITERAL:
					{
						int length = _tcslen(token_ptr->String()) - 2;
						if(length==1){							
							const_ptr->getDeclDefinition().constant.value.Character = token_ptr->String()[1];
							//type changed to char...
							const_ptr->getTypeSpecifier().setBaseTypeSpec(HccTypeChecker::ts_type_char_ptr);
						}else{
							TCHAR* string_ptr = new TCHAR[length + 1];
							const_ptr->getDeclDefinition().constant.value.String = string_ptr;
							CopyQuotedString(string_ptr, token_ptr->String());
							//changed type to represent an array of characters...
							const_ptr->getTypeSpecifier().setStringSpec(length);
						}
						//skip this
						getToken();
					}
					break;
				case HCC_IDENTIFIER:
					{
						parseIdentifierConstant(const_ptr, sign, id_ptr);
					}
					break;
				};
			}
			//resync only to a semicolon, or to a comma operator...
			HCC_TOKEN_TYPE tokens[] = {
				HCC_SEMICOLON,
				HCC_COMMA_OP,
			};
			Resync(tokens, sizeof(tokens)/sizeof(tokens[0]));			
			//get the next identifier
			nextIdentifierFromComma();
		}
	}
	Resync(HCC_SEMICOLON);
}

void HCCParser::nextIdentifierFromComma(HCC_TOKEN_TYPE token_ref)
{
	//skip this comma, the next, must be an identifier
	if(token_type==HCC_COMMA_OP){
		do{
			getToken();
			HCC_TOKEN_TYPE tokens[] = {				
				HCC_IDENTIFIER,
				token_ref,
			};
			Resync(tokens, sizeof(tokens)/sizeof(tokens[0]));
			if(token_type==HCC_COMMA_OP)
				HccErrorManager::Error(HccErrorManager::errMissingIdentifier, _T(" before the \',\'."));
			//
		}while(token_type==HCC_COMMA_OP);
		if(token_type!=HCC_IDENTIFIER && token_type!=token_ref)
			HccErrorManager::Error(HccErrorManager::errMissingIdentifier, _T(" after the \',\'."));
		//
	}else if(token_type==HCC_IDENTIFIER){
		__tstring info = _T(" before identifier \'");
		info += token_ptr->String();
		info += _T("\'.");
		HccErrorManager::Error(HccErrorManager::errMissingComma, info);
		//no need to resync because of the loop
	}
}


void HCCParser::parseIdentifierConstant(Symbol *id_ptr, HCC_TOKEN_TYPE sign, Symbol *function_ptr)
{
	//use the current namespace...
	Symbol* id2_ptr = getSymbolFromIdentifier(true, function_ptr);
	//this identifier in the declaration, must be a constant
	if(id2_ptr==NULL || id2_ptr->getDeclDefinition().identifier_type()!=DECL_CONSTANT){
		HccErrorManager::Error(HccErrorManager::errInvalidConstant);
		HccErrorManager::Error(HccErrorManager::errMissingConstantExpression);

		return;
	}

	TypeSpecifier* base_type_ptr = id2_ptr->getTypeSpecifier().getBaseTypeSpec();
	if(base_type_ptr!=NULL)
	{
		if(base_type_ptr->getDataType()==HCC_INTEGER)
		{
			id_ptr->getDeclDefinition().constant.value.Integer = 
				(sign==HCC_MINUS_OP) ? -id2_ptr->getDeclDefinition().constant.value.Integer :
										id2_ptr->getDeclDefinition().constant.value.Integer;
		}else if(base_type_ptr->getDataType()==HCC_FLOATING_POINT)
		{
			id_ptr->getDeclDefinition().constant.value.Double = 
				(sign==HCC_MINUS_OP) ? -id2_ptr->getDeclDefinition().constant.value.Double :
										id2_ptr->getDeclDefinition().constant.value.Double;
		}else if(base_type_ptr->getDataType()==HCC_CHARACTER)
		{
			id_ptr->getDeclDefinition().constant.value.Character = 
				(sign==HCC_MINUS_OP) ? -id2_ptr->getDeclDefinition().constant.value.Character :
										id2_ptr->getDeclDefinition().constant.value.Character;			
		}else if(base_type_ptr->specifier()==DSPEC_ENUM)
		{
			id_ptr->getDeclDefinition().constant.value.Integer = 
				(sign==HCC_MINUS_OP) ? -id2_ptr->getDeclDefinition().constant.value.Integer :
										id2_ptr->getDeclDefinition().constant.value.Integer;
		}
		else if(base_type_ptr->getDataType()==HCC_BOOLEAN)
		{
			id_ptr->getDeclDefinition().constant.value.Integer = 
					id2_ptr->getDeclDefinition().constant.value.Integer;
		}
		else if(base_type_ptr->getDataType()==HCC_STRING_TYPE)
		{
			if(id2_ptr->getTypeSpecifier().specifier()==DSPEC_ARRAY)
			{
				//if has sign, or this array is not a string or character array, flag an error
				if(sign!=HCC_TOKEN_ERROR || 
					id2_ptr->getTypeSpecifier().array.pItemType!=HccTypeChecker::ts_type_char_ptr)
				{
					HccErrorManager::Error(HccErrorManager::errInvalidConstant);
				}
				id_ptr->getDeclDefinition().constant.value.String = 
						id2_ptr->getDeclDefinition().constant.value.String;
			}else
				HccErrorManager::Error(HccErrorManager::errInvalidConstant, _T(" (not a string constant or char array)."));
		}
	}else
	{
		//all declarations must have a base type
		//this was decided because it will ease the development.
		assert(0);
	}
}

//---------------------------------------------------------------
//
//	parseClassStructTypes(scope)	- parses classes and structures defined in a id_ptr scope
//								  in the first version,
//								  for simplicity, both types class and struct means the same thing for H++
//	like:
//
//		class id {
//			accessor-type data-member data-member-list 
//			accessor-type function-member function-member-list
//			};
//
//		struct id {
//			accessor-type data-member data-member-list 
//			accessor-type function-member function-member-list
//			};
//
//---------------------------------------------------------------
Symbol* HCCParser::parseClassStructTypes(Symbol* ns_class_ptr)
{
	Symbol* type_symbol_ptr = NULL;
	//skip the class/struct keyword...
	getToken();	
	Resync(HCC_IDENTIFIER);
	if(token_type==HCC_IDENTIFIER){
		__tstring class_name;
		if(ns_class_ptr!=NULL){
			class_name = ns_class_ptr->String();
			class_name += _T("::");
		}
		class_name += token_ptr->String();

		if((type_symbol_ptr = symbol_table_ptr->find(class_name.c_str()))!=NULL)
		{
			__tstring info = _T("\'");
			info += class_name.c_str();
			info += _T("\'.");
			HccErrorManager::Error(HccErrorManager::errRedeclaredIdentifier, info);
		}else
			type_symbol_ptr = insert_symbol(class_name.c_str(), DECL_NEW_TYPE);
	}
	getTokenIf(HCC_IDENTIFIER, HccErrorManager::errMissingIdentifier);
	//
	if(type_symbol_ptr!=NULL)
	{
		//BEGIN - ADDED - Jan 17, 2009
		//put in the set of active namespaces...
		Symbol* prev_ns_class_ptr = active_ns_class_ptr;
		active_ns_class_ptr = ns_class_ptr;
		//END - ADDED - Jan 17, 2009

		TypeSpecifier* classType = &type_symbol_ptr->getTypeSpecifier();
		classType->set_specifier(DSPEC_CLASS);		
		//has as its base type, itself!
		classType->setBaseTypeSpec(classType);
		//support when calling constructors...
		classType->setTypeName(type_symbol_ptr->getName());

		//inheritance?
		if(token_type==HCC_COLON)
		{
			getToken();
			if(token_type!=HCC_IDENTIFIER)
			{
				//Error: can only inherit from user defined types (class/struct)				
				HccErrorManager::Error(HccErrorManager::errMissingBaseType);
				Resync(HCC_LBLOCK_KEY);
			}else
			{
				Symbol* base_class_ptr = getSymbolFromIdentifier(true);	
				if(base_class_ptr!=NULL)
				{
					volatile DECLARATION_TYPE decl_type = 
												base_class_ptr->getDeclDefinition().identifier_type();

					if(decl_type==DECL_NEW_TYPE || decl_type==DECL_NEW_ABSTRACT_TYPE)
					{
						//for class and struct keywords (will be the same behavior for now)
						TypeSpecifier* base_type_spec_ptr = &base_class_ptr->getTypeSpecifier();
						while(base_type_spec_ptr->specifier()!=DSPEC_CLASS)					
						{
							base_type_spec_ptr = base_type_spec_ptr->getBaseTypeSpec();
							//at the top of the hierarchy?
							if(base_type_spec_ptr==NULL || 
								base_type_spec_ptr == base_type_spec_ptr->getBaseTypeSpec())
								break;
							//cannot inherit from a build in type...
							if(base_type_spec_ptr->specifier()==DSPEC_SIMPLE)
							{
								decl_type = DECL_BUILDIN_TYPE;
								goto __EVALUATE_DECL_TYPE;
							}
						}
						//if not inheriting from itself, go on...
						if(base_type_spec_ptr!=NULL && classType!=base_type_spec_ptr)
						{
							//set the base type spec!
							//has as its base type, itself!
							classType->setBaseTypeSpec(base_type_spec_ptr);
						}else{
							HccErrorManager::Error(HccErrorManager::errCannotInheritFromItSelf);
							Resync(HCC_LBLOCK_KEY);
						}

					}else
					{
__EVALUATE_DECL_TYPE:
						switch(decl_type)
						{
							case DECL_BUILDIN_TYPE:	//for all the compiler build-in types
								HccErrorManager::Error(HccErrorManager::errCannotInheritFromBuildInType);
								Resync(HCC_LBLOCK_KEY);
								break;
							case DECL_VARIABLE:
							case DECL_CONSTANT:
								HccErrorManager::Error(HccErrorManager::errCannotInheritFromConstOrVariable);
								Resync(HCC_LBLOCK_KEY);
								break;
							default:
								Resync(HCC_LBLOCK_KEY);
								break;
						};						
					}
					//for those who tries to do multiple inheritance...
					if(token_type==HCC_COMMA_OP)
					{
						HccErrorManager::Error(HccErrorManager::errMultipleInheritanceNotSupported);
						HCC_TOKEN_TYPE tokens[] = {
							HCC_LBLOCK_KEY,
							HCC_IDENTIFIER,
						};
						Resync(tokens, sizeof(tokens)/sizeof(tokens[0]));
					}else if(token_type!=HCC_LBLOCK_KEY)
					{
						Resync(HCC_LBLOCK_KEY);
					}
				}
			}
		}
		//

		//TODO: allow in the future forward declarations
		if(getTokenIf(HCC_LBLOCK_KEY, HccErrorManager::errMissingStartBlock)!=0)
		{
		//{			
			//the process is as follows:			
			//1. Get the vtbl ptr from the base class; if found, then we must take the vptr offset and save it;
			__tstring vtbl_ptr_symbol_name = CLASS_VPTR_VTBL_NAME;
			Symbol* base_class_vtbl_ptr = NULL;
			TypeSpecifier* baseTypeSpec = classType->getBaseTypeSpec();
			//this type is realy in inheritance?
			if(classType!=baseTypeSpec)
			{
				//if this base type is virtual/abstract, then, 
				//in the class's symbol table must exists a symbol to represent the vtbl ptr...
				assert(baseTypeSpec->getSymbolTable()!=NULL);
				base_class_vtbl_ptr = baseTypeSpec->getSymbolTable()->find(vtbl_ptr_symbol_name);
			}
			int vptr_offset = 0xFFFFFFFF;
			if(base_class_vtbl_ptr!=NULL)
				vptr_offset = base_class_vtbl_ptr->getDeclDefinition().user_data.offset;
			
			//2. Create the new class vtbl ptr, and assign as its offset, the saved offset from the base class,
			//if and only if, the base class if this one is derived, has a vtbl ptr.
			if(vptr_offset!=0xFFFFFFFF)
			{
				assert(base_class_vtbl_ptr!=NULL);
				//B E G I N - V T B L   C R E A T I O N   
				Symbol* vtbl_symbol_ptr = classType->getSymbolTable()->insert(vtbl_ptr_symbol_name);
				assert(vtbl_symbol_ptr!=NULL);

				if(vtbl_symbol_ptr!=NULL)
				{
					
					assert(vtbl_symbol_ptr!=NULL);
					//represents a memory address...
					vtbl_symbol_ptr->getTypeSpecifier().setBaseTypeSpec(HccTypeChecker::ts_type_int_ptr);
					vtbl_symbol_ptr->getDeclDefinition().set_identifier_type(DECL_VTBL_PTR);
					//
					//specify the offset for the vtbl ptr
					vtbl_symbol_ptr->getDeclDefinition().user_data.offset = vptr_offset;

					//|**|//					
					TypeDataMembers& data_members = classType->user_type_members.data_members;
					data_members.push_back(vtbl_symbol_ptr);
				}
				//E N D - V T B L   C R E A T I O N
				//3. Copy all members from base class's vtbl ptr, to the new vtbl ptr for the current class. 
				//add this virtual function to the vtbl...
				TypeDataMembers& vtbl_members = vtbl_symbol_ptr->getTypeSpecifier().user_type_members.function_members;
				//
				TypeDataMembers& base_vtbl_members = base_class_vtbl_ptr->getTypeSpecifier().user_type_members.function_members;
				TypeDataMembers::iterator it_base_member = base_vtbl_members.begin();
				while(it_base_member != base_vtbl_members.end())
				{
					Symbol* vmember_ptr = *it_base_member++;

					//copy one by one, in the same order as in base...
					vtbl_members.push_back(vmember_ptr);
				}
			}
			/*
				4. Determine the base class's object layout size, to take this parameter as a reference 
				   for the new class.
			*/
			int base_type_layout_size = getClassTypeLayoutSize(classType);

			//the data members...
			classType->user_type_members.layout_object_size += base_type_layout_size;
			//the current class data member offset is corrected here...
			long offset = base_type_layout_size;
			//
			while(token_type!=HCC_RBLOCK_KEY && token_type!=HCC_EOF && token_type!=HCC_TOKEN_ERROR)
			{
				Symbol* inner_type_ptr = NULL;
				active_class_ptr = type_symbol_ptr;
				if(token_type==HCC_CLASS || token_type==HCC_STRUCT){
					//I N N E R   C L A S S E S 
					classType->user_type_members.inner_types.push_back((inner_type_ptr = parseClassStructTypes(type_symbol_ptr)));
					//if found a member declaration based on this inner type...
					if(token_type==HCC_IDENTIFIER && inner_type_ptr!=NULL)
					{
						offset = parseClassDataMember(type_symbol_ptr, offset, &inner_type_ptr->getTypeSpecifier());
					}					
					skipEmptyStatements();
					goto _EXTRA_EMPTY_;

				}else if(token_type==HCC_ENUM){
					//I N N E R   E N U M  T Y P E S
					classType->user_type_members.inner_types.push_back((inner_type_ptr = parseEnumerationType(NULL, type_symbol_ptr)));
					//if found a member declaration based on this inner type...
					if(token_type==HCC_IDENTIFIER && inner_type_ptr!=NULL)
					{
						offset = parseClassDataMember(type_symbol_ptr, offset, &inner_type_ptr->getTypeSpecifier());
					}
					getTokenIf(HCC_SEMICOLON, HccErrorManager::errMissingSemicolon);	//;
					goto _EXTRA_EMPTY_;
				}else if(token_type==HCC_TYPENAME){
					//I N N E R   T Y P E N A M E S 
					classType->user_type_members.inner_types.push_back(parseTypeDefinitions(NULL, type_symbol_ptr));
					getTokenIf(HCC_SEMICOLON, HccErrorManager::errMissingSemicolon);	//;
					goto _EXTRA_EMPTY_;
				}else{
					//C L A S S   D A T A   M E M B E R S
					offset = parseClassDataMember(type_symbol_ptr, offset, NULL);
					skipEmptyStatements();
				}

_EXTRA_EMPTY_:
				//skip extra empty-statements
				while(token_type==HCC_SEMICOLON)
					getToken();
			}
			//set the object layout size
			if(offset < 1)
				offset = 1;
			classType->setDataType(HCC_CUSTOM_TYPE, HCC_NO_MODIFIER, offset);

			if(getTokenIf(HCC_RBLOCK_KEY, HccErrorManager::errMissingEndBlock)==0)
			{
				HCC_TOKEN_TYPE tokens[] = {
					HCC_RBLOCK_KEY,
					HCC_SEMICOLON,
				};
				Resync(tokens, sizeof(tokens)/sizeof(tokens[0]));
			}	
		//}
		}else
		{
			HCC_TOKEN_TYPE tokens[] = {
				HCC_SEMICOLON,
				HCC_STRUCT,
				HCC_CLASS,
			};
			Resync(tokens, sizeof(tokens)/sizeof(tokens[0]));			
		}
		//determine the virtual/abstract state of a class
		analyzeClassVirtualState(type_symbol_ptr);

		//BEGIN - ADDED - Jan 17, 2009
		active_class_ptr = NULL;
		active_ns_class_ptr = prev_ns_class_ptr; //ADDED - Jan 17, 2009
		//END - ADDED - Jan 17, 2009
	}else{
		Resync(HCC_RBLOCK_KEY);	
		if(token_type==HCC_RBLOCK_KEY)
			getToken();
		getTokenIf(HCC_SEMICOLON, HccErrorManager::errMissingSemicolon);	//;	
	}

	return type_symbol_ptr;
}

Symbol* HCCParser::parseTypeDefinitions(Symbol *id_proc_ptr, Symbol* ns_class_ptr)
{
	getToken(); //skip the 'typename' keyword
	TypeSpecifier* type_ptr = parseTypeSpec(ns_class_ptr);
	Symbol* typedef_symbol_ptr = NULL;
	if(token_type==HCC_IDENTIFIER)
	{
		__tstring class_name;
		if(ns_class_ptr!=NULL){
			class_name = ns_class_ptr->String();
			class_name += _T("::");
		}		
		class_name += token_ptr->String();

		typedef_symbol_ptr = insert_symbol(class_name.c_str(), DECL_NEW_TYPE);
		getToken(); //skip identifier
		if(typedef_symbol_ptr!=NULL)
		{
			//this will be treated as a user type!
			TypeSpecifier* typedef_spec = &typedef_symbol_ptr->getTypeSpecifier();
			//inherits the type-specifier type
			typedef_spec->set_specifier(type_ptr->specifier());
			typedef_spec->setBaseTypeSpec(type_ptr);
			//set data type info...
			typedef_spec->setDataType(type_ptr->getDataType(), type_ptr->getDataTypeModifier(), type_ptr->getDataTypeSize());
			//add the alias-name of the type its representing
			typedef_spec->setTypeName(class_name); //FIXED - Mar 05, 2009
			//if it was defined in a local stack, then add it!
			if(ns_class_ptr==NULL && id_proc_ptr!=NULL){
				//local scope
				typedef_symbol_ptr->getDeclDefinition().set_identifier_scope(SCOPE_LOCAL);

				id_proc_ptr->getDeclDefinition().function.locals.stack_def_types.user_def_types.push_back(typedef_symbol_ptr);
			}
		}
	}else{
		HccErrorManager::Error(HccErrorManager::errMissingIdentifier, _T(" ,invalid type definition."));
		Resync(HCC_SEMICOLON);
	}
	return typedef_symbol_ptr;
}

//---------------------------------------------------------------
//	parseEnumerationType() - parse an enumeration type like:
//
//	enum greek { alpha = 1, beta, gamma, delta, epsilon, pi, fi, omega};
//
//---------------------------------------------------------------
Symbol* HCCParser::parseEnumerationType(Symbol *id_proc_ptr, Symbol* ns_class_ptr)
{
	Symbol* enum_symbol_ptr = NULL;
	getToken(); //skip 'enum' keyword	
	Resync(HCC_IDENTIFIER);
	__tstring class_name;
	if(token_type==HCC_IDENTIFIER){		
		if(ns_class_ptr!=NULL){
			class_name = ns_class_ptr->String();
			class_name += _T("::");
		}				
		enum_symbol_ptr = insert_symbol((class_name + token_ptr->String()).c_str(), DECL_NEW_TYPE);
	}
	getTokenIf(HCC_IDENTIFIER, HccErrorManager::errMissingIdentifier);
	if(enum_symbol_ptr!=NULL)
	{
		volatile bool bValueSpecified = false;
		//if this type was declared in a function...
		if(ns_class_ptr==NULL && id_proc_ptr!=NULL){
			//local scope
			enum_symbol_ptr->getDeclDefinition().set_identifier_scope(SCOPE_LOCAL);

			id_proc_ptr->getDeclDefinition().function.locals.stack_def_types.user_def_types.push_back(enum_symbol_ptr);
		}
		//
		__int64 const_value = 0;
		TypeSpecifier* enumType = &enum_symbol_ptr->getTypeSpecifier();
		enumType->set_specifier(DSPEC_ENUM);
		//set as its base type, itself!
		enumType->setBaseTypeSpec(enumType);
		enumType->setDataType(HCC_INTEGER, HCC_SIGNED_TYPE, sizeof(int));
		enumType->setTypeName(enum_symbol_ptr->getName());
		
		if(getTokenIf(HCC_LBLOCK_KEY, HccErrorManager::errMissingStartBlock)!=0)
		{
			//parse enumeration constants...
			while(token_type==HCC_IDENTIFIER)
			{
				Symbol* enum_const_ptr = insert_symbol((class_name + token_ptr->String()).c_str()); //as constant
				//skip this identifier...
				getToken();
				if(enum_const_ptr!=NULL)
				{
					//add it to the enumeration type's list
					enumType->enumeration.values.push_back(enum_const_ptr);
					//the base type-spec
					enum_const_ptr->getTypeSpecifier().setBaseTypeSpec(enumType);					

					if(token_type==HCC_COMMA_OP || token_type==HCC_RBLOCK_KEY){
						enum_const_ptr->getDeclDefinition().constant.value.Integer = const_value++;						
						//
					}else if(token_type==HCC_ASSIGN_OP)
					{
						getToken(); //skip the '=' symbol
						//must be a constant expr (for now, just an integer value)
						if(token_type==HCC_NUMBER)
						{
							bValueSpecified = true;
							if(token_ptr->dataType==HCC_INTEGER){
								const_value = max(const_value, token_ptr->value.Integer);
								enum_const_ptr->getDeclDefinition().constant.value.Integer = const_value++;
								getToken();
							}else{								
								HccErrorManager::Error(HccErrorManager::errInvalidConstant, _T(", must be an integer value."));
								Resync(HCC_COMMA_OP);
							}

						}else{
							HccErrorManager::Error(HccErrorManager::errInvalidConstant, _T(", must be an integer value."));
							Resync(HCC_COMMA_OP);
						}
					}					
				}else{
					__tstring info = _T(": \'");
					info += token_ptr->String();
					info += _T("\'.");
					HccErrorManager::Error(HccErrorManager::errRedeclaredIdentifier, info);
					HCC_TOKEN_TYPE tokens[] = {
						HCC_RBLOCK_KEY,
						HCC_IDENTIFIER,
						HCC_SEMICOLON,
					};
					Resync(tokens, sizeof(tokens)/sizeof(tokens[0]));
				}
				//,
				//allows this type of decl: enum x { y, z, }; <--- note the last comma next to the ''.
				nextIdentifierFromComma(HCC_RBLOCK_KEY);
			}//while(identifier)...

			if(getTokenIf(HCC_RBLOCK_KEY, HccErrorManager::errMissingEndBlock)==0)
			{
				HCC_TOKEN_TYPE tokens[] = {
					HCC_RBLOCK_KEY,
					HCC_SEMICOLON,
				};
				Resync(tokens, sizeof(tokens)/sizeof(tokens[0]));
			}
			//set the max enumeration value
			enumType->enumeration.max_enum_const_value = (int)(bValueSpecified ? const_value - 1 : const_value);
		}
	}else{
		Resync(HCC_RBLOCK_KEY);	
		if(token_type==HCC_RBLOCK_KEY)
			getToken();
		getTokenIf(HCC_SEMICOLON, HccErrorManager::errMissingSemicolon);	//;	
	}

	return enum_symbol_ptr;
}

//---------------------------------------------------------------
//	parseArrayType() - parse an array declaration like : 
//
//	TODO: support array initializers
//
//	double array1[] = {3.1415927, 4.99, 1.14, 2.1421, 0.15};
//	int	   array2[] = {2, 4, 9, 0};	
//
//	int	   array3[4][5]; -->is seen like this:	array[4] of array[5] of int
//
//---------------------------------------------------------------
TypeSpecifier* HCCParser::parseArrayType(TypeSpecifier* id_type_spec, Symbol* function_ptr, TypeSpecifier* decl_final_type_ptr)
{
	if(id_type_spec==NULL)
		return 0;		
	if(getTokenIf(HCC_LBRACKET, HccErrorManager::errMissingLeftBracket)==0){
		Resync(HCC_LBRACKET);
		if(token_type==HCC_LBRACKET)
			getToken();
	}

	TypeSpecifier* arraySpec = id_type_spec;
	//the element type-spec
	TypeSpecifier* base_type_spec		= arraySpec->getBaseTypeSpec(),
				  *itemTypeSpec			= arraySpec->getBaseTypeSpec();

	if(decl_final_type_ptr!=NULL)
	{
		base_type_spec	= decl_final_type_ptr;
		itemTypeSpec	= decl_final_type_ptr;
	}
	//mark it as an array!
	arraySpec->set_specifier(DSPEC_ARRAY);
	//set as itself!
	arraySpec->setBaseTypeSpec(arraySpec);
	//array of type itemTypeSpec
	arraySpec->array.pItemType = itemTypeSpec;			
	//[
	if(token_type==HCC_IDENTIFIER)
	{		
		Symbol* const_ptr = getSymbolFromIdentifier(false, function_ptr);
		if(const_ptr!=NULL)
		{
			//a const identifier
			if(const_ptr->getDeclDefinition().identifier_type()==DECL_CONSTANT)
				arraySpec->array.item_count = (int)const_ptr->getDeclDefinition().constant.value.Integer;
			//H++ supports indices of type enum
			else if(const_ptr->getDeclDefinition().identifier_type()==DECL_NEW_TYPE &&
					const_ptr->getTypeSpecifier().specifier()==DSPEC_ENUM)
				arraySpec->array.item_count = const_ptr->getTypeSpecifier().enumeration.max_enum_const_value;
			else
				HccErrorManager::Error(HccErrorManager::errInvalidSubscriptType, _T(" ,must be an integer or constant integer value."));
		}else
			HccErrorManager::Error(HccErrorManager::errUndeclaredIdentifier, last_identifier.c_str());
	}else if(token_type==HCC_NUMBER){
		if(token_ptr->dataType!=HCC_INTEGER)
			HccErrorManager::Error(HccErrorManager::errInvalidSubscriptType, _T(" ,must be an integer or constant integer value."));
		else
			arraySpec->array.item_count = (int)token_ptr->value.Integer;
		//skip this constant
		getToken();
	}else if(token_type!=HCC_RBRACKET){
		HccErrorManager::Error(HccErrorManager::errInvalidSubscriptType, _T(" ,must be an integer or constant integer value."));
		Resync(HCC_COMMA_OP);
	}
	//next, must be a ']'	
	if(getTokenIf(HCC_RBRACKET, HccErrorManager::errMissingRightBracket)!=0)
	{
		if(token_type==HCC_LBRACKET)
		{	
			//when this array has more than one dimension,
			//a new type specifier of type DSPEC_ARRAY must be created
			//and set as the item type for the current array
			//avoiding Stack Overflow when calling getArraySizeFromType();
			//
			//the base type must be the type-specifier found for this declaration...
			itemTypeSpec = new TypeSpecifier(DSPEC_ARRAY);
			arraySpec->array.pItemType = itemTypeSpec;
			//set the previous type
			itemTypeSpec->setBaseTypeSpec(base_type_spec);
			parseArrayType(itemTypeSpec, function_ptr);
		}
	}

	if(arraySpec->specifier()==DSPEC_ARRAY)
	{
		if(MAX_ARRAY_DIMENSION < getArrayDimesionsFromType(arraySpec))
		{
			HccErrorManager::Error(HccErrorManager::errInvalidDeclaration, 
								   _T(", local arrays cannot exceed 2 dimensions; try creating classes to support the rest of the dimensions needed!"));
		}
	}
	
	return arraySpec;
}

//---------------------------------------------------------------
//	getArraySizeFromType() - get the user defined array size from an array type specifier
//
//		double my_array[10]; ---> getArraySizeFromType(&my_array->getTypeSpecifier())			== 80;
//		double my_array[10][10];  item_count * item_count * sizeof(double)						== 800;
//		double my_array[10][10][10];  item_count * item_count * item_count * sizeof(double)		== 8000;
//		string my_array[10];		item_count * sizeof(int)									== 40;
//---------------------------------------------------------------
long HCCParser::getArraySizeFromType(TypeSpecifier *array_ptr)
{
	if(array_ptr!=NULL)
	{
		if(array_ptr->specifier()==DSPEC_ARRAY){
			//this is a special case: for string types in arrays, we just specify the size of an address
			if(array_ptr->getDataType()==HCC_STRING_TYPE)
				return sizeof(int);
			//
			return array_ptr->array.item_count * getArraySizeFromType(array_ptr->array.pItemType);
		}else
			return array_ptr->getDataTypeSize();
	}
	return 0;
}

TypeSpecifier* HCCParser::getArrayScalarType(TypeSpecifier *array_ptr)
{
	if(array_ptr!=NULL)
	{
		if(array_ptr->specifier()==DSPEC_ARRAY){
			//this is a special case: for string types in arrays, we just specify the size of an address
			if(array_ptr->getDataType()==HCC_STRING_TYPE)
				return HccTypeChecker::ts_type_string_ptr;
			//
			return getArrayScalarType(array_ptr->array.pItemType);
		}else
			return array_ptr;
	}
	return 0;
}

short HCCParser::getArrayDimesionsFromType(TypeSpecifier *array_ptr)
{
	if(array_ptr!=NULL)
	{
		if(array_ptr->specifier()==DSPEC_ARRAY)
			return 1 + getArrayDimesionsFromType(array_ptr->array.pItemType);
	}
	return 0;
}



//---------------------------------------------------------------
//	parseVariableDeclarations() - parse a new declared variable
//
//---------------------------------------------------------------
long HCCParser::parseVariableDeclarations(Symbol *fn_ptr, TypeSpecifier* typeSpec)
{
	long offset  = parseVariableOrDataMemberDecl(fn_ptr, NULL, 
											  (fn_ptr!=NULL ? 
											   fn_ptr->getDeclDefinition().function.current_offset : 
											   global_variable_offset),
											  ACCESS_NONE, typeSpec);
	//
	if(fn_ptr==NULL)
		global_variable_offset = offset;

	return offset;
}

//---------------------------------------------------------------
//	parseClassDataMember(...) - parse class/struct data members.
//
//---------------------------------------------------------------
long HCCParser::parseClassDataMember(Symbol *class_ptr, long offset, TypeSpecifier* member_typeSpec)
{
	//private int x;
	//private double y;
	//protected float z;
	//public string name;
		
	//HCC_PRIVATE, HCC_PROTECTED, HCC_PUBLIC		
	MemberAccessType accessType = ACCESS_PRIVATE;
	if(token_type==HCC_PRIVATE)
		getToken();
	else if(token_type==HCC_PROTECTED){
		getToken();
		accessType = ACCESS_PROTECTED;
	}else if(token_type==HCC_PUBLIC){
		getToken();
		accessType = ACCESS_PUBLIC;
	}

	if(token_type==HCC_CONST)
		return parseClassConstantMember(class_ptr, offset, accessType);
	
	return parseVariableOrDataMemberDecl(NULL, class_ptr, offset, accessType, member_typeSpec);
}

long HCCParser::parseClassConstantMember(Symbol *class_ptr, long offset, MemberAccessType accessType)
{
	parseConstantDefinitions(NULL, class_ptr, accessType);
	return offset;
}

extern GlobalVariables g_program_global_variables;
extern bool bForcesVirtualDestructors;
//---------------------------------------------------------------
//
//	parseVariableOrDataMemberDecl(...) - parses function local variables,
//										 parses class type data members			
//
//				fn_ptr			: used when parsing local variables
//				class_type_ptr	: used when parsing class data members
//				offset			: when fn_ptr, the variable position 
//								  when class_type_ptr, the bytes offset in the object layout
//								  for variables and params: sequence position
//								  for data members: byte offset in class/struct
//---------------------------------------------------------------
long HCCParser::parseVariableOrDataMemberDecl(Symbol *fn_ptr, Symbol *class_ptr, long offset, 
											  MemberAccessType accessType,
											  TypeSpecifier* typeSpec)
{

	DECLARATION_TYPE variable_type = DECL_UNDEFINED;
	STORAGE_SPECIFIER_TYPE storage_spec = STG_AUTO;
	//for function members only
	//the void means this function whatever the return type, 
	//is a normal function
	HCC_TOKEN_TYPE member_type = HCC_VOID;
	//
	if(token_type==HCC_EXTERN)
	{
		member_type = token_type;
		getToken(); //skip it!
		//extern "lib" function ?
		if(class_ptr!=NULL)
		{
			//public extern "obj-file" static type-spec function(parameter-list);			
			//if extern, then must be static too.
			//"lib"
			if(token_type==HCC_STRING_LITERAL)
				extern_libs.insert(token_ptr->String());
			if(getTokenIf(HCC_STRING_LITERAL, HccErrorManager::errExpectedLibraryFileName)==0)
				Resync(HCC_STATIC);
		}
	}

	switch(token_type)
	{
		case HCC_AUTO:
			{
				storage_spec = STG_AUTO;
				getToken();
			}
			break;
		case HCC_REGISTER:
			{
				storage_spec = STG_REGISTER;
				getToken();
			}
			break;
		case HCC_STATIC:				
			{
				//if extern, is static by definition too.
				if(member_type == HCC_VOID)
					member_type		= HCC_STATIC;
				//
				storage_spec	= STG_STATIC;
				getToken();
			}
			break;
		case HCC_VOLATILE:
			{
				storage_spec = STG_VOLATILE;
				getToken();
			}
			break;
		case HCC_VIRTUAL:
			{
				member_type		= HCC_VIRTUAL;
				storage_spec	= STG_AUTO;
				getToken();
			}
			break;
		default:
			//no modifier found; uses auto by default
			break;
	};

	//V I R T U A L   A B S T R A C T   F U N C T I O N S 
	//if is declared a virtual abstract member function...
	if(class_ptr!=NULL && (token_type==HCC_ABSTRACT && member_type==HCC_VIRTUAL))
	{		
		member_type = HCC_ABSTRACT;
		getToken();
		bool bReturnTypeIsPointer = false;
		//get the type specifier
		TypeSpecifier* base_type_ptr = (typeSpec!=NULL) ? typeSpec : parseTypeSpec(class_ptr);
		//BEGIN - FIXED Mar 1, 2009
		if(token_type==HCC_XOR_OP)
		{
			getToken(); //^
			bReturnTypeIsPointer = true;
		}
		//END - FIXED Mar 1, 2009
		if(token_type==HCC_IDENTIFIER)
		{
			__tstring virtual_abstract = token_ptr->String();
			Symbol* abstract_id_ptr = class_ptr->getTypeSpecifier().getSymbolTable()->insert(virtual_abstract);

			if(abstract_id_ptr==NULL){
				HccErrorManager::Error(HccErrorManager::errRedeclaredIdentifier, token_ptr->String());
				abstract_id_ptr = class_ptr->getTypeSpecifier().getSymbolTable()->find(virtual_abstract);
				assert(abstract_id_ptr!=NULL);
			}else{
				//the storage...
				abstract_id_ptr->getDeclDefinition().set_storage_specifier(storage_spec);
				//the data member offset
				abstract_id_ptr->getDeclDefinition().user_data.offset = 0; //not used (vtbl offset is used instead!)
				//the access modifier
				abstract_id_ptr->getDeclDefinition().set_member_access_type(accessType);
				//the base type
				abstract_id_ptr->getTypeSpecifier().setBaseTypeSpec(base_type_ptr);
				//the return type is pointer
				abstract_id_ptr->getDeclDefinition().function.bReturnTypeIsPointer = bReturnTypeIsPointer;
			}
			//skip the identifier
			getTokenAppend();
			Resync(HCC_LPAREN);
			if(token_type==HCC_LPAREN) //must be a new member function...
				parseMemberFunction(class_ptr, base_type_ptr, abstract_id_ptr, offset, member_type);
			//
			return offset;
		}
	}
	
	//get the type specifier
	bool bIsAbstractType = false;
	DataTypeModifier typeModifier = HCC_SIGNED_TYPE;
	TypeSpecifier* base_type_ptr = (typeSpec!=NULL) ? typeSpec : parseTypeSpec(class_ptr, NULL, &bIsAbstractType, &typeModifier);

	//P O I N T E R S  |  D Y N A M I C   A R R A Y   D E F I N I T I O N 
	bool bIsArrayPointer = false;
	volatile short nDimensions = 0;
	if(token_type==HCC_XOR_OP)
	{
		//type-spec ^ identifier =  null | 
		//							new type-spec '(' param-list ')';

		getToken(); //skip the '^' symbol
		variable_type = DECL_POINTER_VARIABLE;
	}
	if(token_type==HCC_LBRACKET)
	{
		//type-spec [] identifier =  null | 
		//							 new type-spec [const-value];		
		while(token_type==HCC_LBRACKET)
		{
			getToken(); //skip the '[' symbol
			nDimensions++;
			getTokenIf(HCC_RBRACKET, HccErrorManager::errMissingRightBracket); // ']'
		}
		bIsArrayPointer = true;
		variable_type = DECL_POINTER_VARIABLE;
	}
	//C O N S T R U C T O R   D E F I N I T I O N
	//if a constructor is defined, parse it...
	if(class_ptr!=NULL && base_type_ptr==&class_ptr->getTypeSpecifier())
	{
		//we first asure we have a constructor, then validate it...
		if(member_type!=HCC_VOID && token_type==HCC_LPAREN)
		{
			//error
			HccErrorManager::Error(HccErrorManager::errConstructorWithStorageSpecifier);
			getToken();
		}
		//insert in the type's symbol table...
		assert(class_ptr!=NULL && class_ptr->getTypeSpecifier().getSymbolTable()!=0);

		//is a constructor...
		if(token_type==HCC_LPAREN)
		{
			parseClassConstructor(class_ptr);
			return offset;
		}
	}
	//P R O P E R T Y   D E F I N I T I O N S 
	//determine if there is defined a property...
	if(class_ptr!=NULL && (token_type==HCC_GET || token_type==HCC_PUT))
	{
		parseClassProperty(base_type_ptr, class_ptr, (variable_type == DECL_POINTER_VARIABLE));
		return offset;
	}
	volatile bool bIsConstructorCall = false;
	volatile long total_size = 0;
	//type-specifier identifier-list;
	while(token_type==HCC_IDENTIFIER)
	{
		//BEGIN - R E F A C T O R E D   C O D E Jan 2, 2008
		//check that the base type is not abstract, so we can flag an error otherwise...
		if(base_type_ptr->specifier()==DSPEC_CLASS && variable_type != DECL_POINTER_VARIABLE)
		{
			SYMBOL_TABLE::LPSYMBOL type_symbl_ptr = find_symbol(base_type_ptr->getTypeName().c_str());
			assert(type_symbl_ptr!=NULL);
			if(type_symbl_ptr!=NULL)
			{
				if(type_symbl_ptr->getDeclDefinition().identifier_type()==DECL_NEW_ABSTRACT_TYPE)
				{
					__tstring info = _T(" ");
					if(base_type_ptr!=NULL)
					{
						info = _T(", cannot instantiate abstract class \'");
						info += base_type_ptr->getTypeName();info += _T("\'.");
					}
					HccErrorManager::Error(HccErrorManager::errAbstractClassInstantiation, info);
				}
			}
		}
		//BEGIN - R E F A C T O R E D   C O D E Jan 2, 2008
		Symbol* id_ptr = NULL;
		//L O C A L   A N D   G L O B A L   V A R I A B L E S 
		if(fn_ptr!=NULL || (fn_ptr==NULL && class_ptr==NULL))
		{
			//always register a variable size multiple of sizeof(int)
			int var_size = base_type_ptr->getDataTypeSize();
			//the minimun variable size that H++ allows to have a good stack balance...
			if(var_size==0)
				var_size = sizeof(int);
			int modulus = var_size % sizeof(int);
			if(modulus > 0)
				var_size += sizeof(int) - modulus;
			//insert in the fn's symbol table...
			if(fn_ptr!=NULL && member_type!=HCC_STATIC)
			{
				assert(fn_ptr->getDeclDefinition().function.symbol_table_ptr!=0);
				id_ptr = fn_ptr->getDeclDefinition().function.symbol_table_ptr->insert(token_ptr->String());

				if(member_type==HCC_EXTERN)
				{
					//global variables can only be extern
					//local variables are not accepted!
					HccErrorManager::Error(HccErrorManager::errExternLocalVariableNotAllowed);
				}

				if(id_ptr==NULL){
					__tstring info = _T(": \'");
					info += token_ptr->String();
					info += _T("\'.");
					HccErrorManager::Error(HccErrorManager::errRedeclaredIdentifier, info);
					id_ptr = find_symbol(token_ptr->String());
					assert(id_ptr!=NULL);
				}
				//variable offset
				offset -= var_size;				
			}else{
				//use the current namespace...
				__tstring token_id;
				if(current_namespace_ptr!=NULL){
					token_id = current_namespace_ptr->getName();
					token_id += _T("::");
				}
				token_id += token_ptr->String();
				id_ptr = insert_symbol(token_id.c_str(), DECL_VARIABLE);

				if(id_ptr==NULL){
					__tstring info = _T(": \'");
					info += token_ptr->String();
					info += _T("\'.");
					HccErrorManager::Error(HccErrorManager::errRedeclaredIdentifier, info);
					id_ptr = find_symbol(token_id.c_str());
					assert(id_ptr!=NULL);
				}					
			}
						
			assert(id_ptr!=0);
			//
			id_ptr->getDeclDefinition().set_identifier_type(DECL_VARIABLE);
			//the storage...
			id_ptr->getDeclDefinition().set_storage_specifier(storage_spec);
			//the variable offset
			id_ptr->getDeclDefinition().user_data.offset = offset;
			//the base type
			id_ptr->getTypeSpecifier().setBaseTypeSpec(base_type_ptr);
			//the type modifier
			id_ptr->getTypeSpecifier().setDataTypeModifier(typeModifier);
			//
			//A D D   T O   T H E   F U N C T I O N ' S   S T A C K   L O C A L S 
			if(fn_ptr!=NULL && member_type!=HCC_STATIC){
				//local scope
				id_ptr->getDeclDefinition().set_identifier_scope(SCOPE_LOCAL);
				fn_ptr->getDeclDefinition().function.locals.stack_variables.variables.push_back(id_ptr);
			}else{
				id_ptr->getDeclDefinition().set_identifier_scope(SCOPE_GLOBAL);
				//register in the globals list (to maintain its order in the program)
				g_program_global_variables.push_back(id_ptr);
			}
			//skip the identifier
			getToken();
			//
			if(variable_type == DECL_POINTER_VARIABLE)
			{
				//correct the pointer variable offset to 32 bits
				offset += var_size;
				offset -= sizeof(int);	//pointers always has 32 bits of variable size
				//the variable offset
				id_ptr->getDeclDefinition().user_data.offset = offset;
				id_ptr->getDeclDefinition().set_identifier_type(variable_type);
				if(fn_ptr!=NULL && member_type!=HCC_STATIC)
				{
					//the next relative variable offset;			
					fn_ptr->getDeclDefinition().function.current_offset = offset;
					//add to the stack size, the size of a pointer variable...
					fn_ptr->getDeclDefinition().function.locals.stack_variables.total_variables_size += sizeof(int);
				}
				//
				if(false==bIsArrayPointer)
				{
					parsePointerVariableOrDataMember(id_ptr, base_type_ptr, fn_ptr, class_ptr, false);
					return offset;
				}else{
					parseArrayPointerVariableOrDataMember(id_ptr, base_type_ptr, nDimensions, fn_ptr, class_ptr, false);
					return offset;
				}
			}
			
			//if this variable is an array, parse it as an array...
			if(token_type==HCC_LBRACKET){				
				parseArrayType(&id_ptr->getTypeSpecifier(), fn_ptr);
				//keep track of the array's size
				int array_size = getArraySizeFromType(&id_ptr->getTypeSpecifier());
				if(array_size==0)
					HccErrorManager::Error(HccErrorManager::errInvalidDeclaration, _T("; arrays cannot have dimensions with value zero."));
				else{ 
					//always register an array size multiple of sizeof(int)
					modulus = array_size % sizeof(int);
					if(modulus > 0 )
						array_size += sizeof(int) - modulus;
					total_size += array_size;

					//now, the offset is different;
					offset += var_size;
					//new variable offset based on the previous offset + the array size in bytes
					offset -= array_size;

					//the variable offset
					id_ptr->getDeclDefinition().user_data.offset = offset;
				}
				//
				id_ptr->getTypeSpecifier().setDataType(HCC_CHAR_TYPE, HCC_NO_MODIFIER, array_size);
			}else if(token_type==HCC_LPAREN)
			{	
				//The only declaration for object instantiation actually, is a call to a constructor...				
				
				//O B J E C T   I N S T A N C E   L O A D
				icode_ptr->append(HCC_IDENTIFIER);
				icode_ptr->insert_line_marker();
				icode_ptr->append(id_ptr);				
				
				if(base_type_ptr!=NULL)
				{
					const __tstring& lpszType = base_type_ptr->getTypeName();
					assert(lpszType.length() > 0);
					if(lpszType.length() > 0)
					{
						//a call to a constructor for an object declaration...
						LPSYMBOL_TABLE symbl_tbl_ptr = base_type_ptr->getSymbolTable();
						Symbol* constructor_ptr = NULL;
						if(symbl_tbl_ptr!=NULL)
							constructor_ptr = symbl_tbl_ptr->find(lpszType);
						if(constructor_ptr!=NULL){
							
							//C O N S T R U C T O R   C A L L 
							bPrevTokenManuallyAdded = false;
							icode_ptr->append(token_type); //(
							getTokenAppend();	//next token						
							parseFunctionArgumentList(constructor_ptr, fn_ptr);

							Resync(HCC_RPAREN);
							getTokenAppendIf(HCC_RPAREN, 
											HccErrorManager::errMissingRightParen); //)
						}else{
							//when no constructor was defined, then we just take out the left and right parentesis,
							//because are useless for the code generation phase
							getToken();	//(
							getToken(); //)
						}
					}
				}

				//keep track of the variable's sizes
				total_size += var_size;
				
				icode_ptr->append(HCC_SEMICOLON);
				//flag a constructor call...
				bIsConstructorCall = true;
				//
			}else if(member_type!=HCC_STATIC)
			{
				//keep track of the variable's sizes
				total_size += var_size;
			}
			//the next relative variable offset;			
			if(fn_ptr!=NULL && member_type!=HCC_STATIC)
				fn_ptr->getDeclDefinition().function.current_offset = offset;
		}else{
			//insert in the type's symbol table...
			assert(class_ptr!=NULL && class_ptr->getTypeSpecifier().getSymbolTable()!=0);

			//if a destructor is specified, then apply rules
			if(_tcscmp(token_ptr->String(), HPP_CLASS_DESTRUCTOR)==0)
			{
				if(member_type!=HCC_VOID && member_type!=HCC_VIRTUAL)
				{
					//error: destructors cannot have storage specifier
					HccErrorManager::Error(HccErrorManager::errDestructorWithStorageSpecifier, _T(", static destructors are not supported."));	
				}
				if(base_type_ptr!=HccTypeChecker::ts_type_void_ptr)
					HccErrorManager::Error(HccErrorManager::errDestructorCannotReturnAValue, _T(", specify \'void\' for all destructors."));

				//BEGIN - FIXED Jan 5, 2009
				//if not a virtual destructor...
				if(member_type!=HCC_VIRTUAL && bForcesVirtualDestructors)
				{
					//F O R C E   A   V I R T U A L   D E S T R U C T O R
					member_type = HCC_VIRTUAL; 
					/*
					parseClassDestructor(class_ptr);
					return offset;
					*/
				}
				//END - FIXED Jan 5, 2009
			}
			//determine how to create the member name based on the storage type...
			__tstring member_name;
			if(member_type ==HCC_STATIC || member_type == HCC_EXTERN)
			{
				//this means that a static member must be public
				member_name = class_ptr->getName();
				member_name += _T("::");
				member_name += token_ptr->String();

				id_ptr = insert_symbol(member_name.c_str(), DECL_NEW_DATA_MEMBER);
			}else{
				member_name = token_ptr->String();
				id_ptr = class_ptr->getTypeSpecifier().getSymbolTable()->insert(member_name);
			}

			if(id_ptr==NULL){

				HccErrorManager::Error(HccErrorManager::errRedeclaredIdentifier, token_ptr->String());
				id_ptr = class_ptr->getTypeSpecifier().getSymbolTable()->find(member_name);
			}

			assert(id_ptr!=0);			
			//
			if(id_ptr!=NULL)
			{
				id_ptr->getDeclDefinition().set_identifier_type(DECL_NEW_DATA_MEMBER);
				//the storage...
				id_ptr->getDeclDefinition().set_storage_specifier(storage_spec);
				//the data member offset
				id_ptr->getDeclDefinition().user_data.offset = offset;			
				//the access modifier
				id_ptr->getDeclDefinition().set_member_access_type(accessType);
				//the base type
				id_ptr->getTypeSpecifier().setBaseTypeSpec(base_type_ptr);
				//the type modifier
				id_ptr->getTypeSpecifier().setDataTypeModifier(typeModifier);
			}
			//
			//skip the identifier
			getToken();

			if(variable_type == DECL_POINTER_VARIABLE)
			{
				//BEGIN - FIXED - Jan 1, 2008
				if(token_type==HCC_LPAREN)
				{
					//if this is a function which returns a pointer to an object or to a scalar value,
					//we must flag that this returned type is a pointer or 32-bits value
					//in reality, the decl is the one that is a pointer
					//that's why we cannot flag a type as a pointer
					//just the declarations.
					id_ptr->getDeclDefinition().function.bReturnTypeIsPointer = true;
				}else{
					if(id_ptr->getDeclDefinition().storage_specifier()==STG_STATIC)
					{
						id_ptr->getDeclDefinition().set_identifier_type(DECL_POINTER_VARIABLE);

						id_ptr->getDeclDefinition().set_identifier_scope(SCOPE_GLOBAL);
						//register in the globals list (to maintain its order in the program)
						g_program_global_variables.push_back(id_ptr);
					}else
					{
						id_ptr->getDeclDefinition().set_identifier_scope(SCOPE_LOCAL);
						//this data member is a pointer
						id_ptr->getDeclDefinition().user_data.bDataMemberIsPointer = true;
						//the variable offset
						id_ptr->getDeclDefinition().user_data.offset = offset;
						//
						offset += sizeof(int);	//pointers always has 32 bits of variable size
						class_ptr->getTypeSpecifier().user_type_members.layout_object_size += sizeof(int);
					}
					//END - FIXED - Jan 1, 2008
					//
					if(false==bIsArrayPointer)
					{
						parsePointerVariableOrDataMember(id_ptr, base_type_ptr, fn_ptr, class_ptr, false);
						return offset;
					}else{
						parseArrayPointerVariableOrDataMember(id_ptr, base_type_ptr, nDimensions, fn_ptr, class_ptr, false);
						return offset;
					}
				}
			}
			//BEGIN - FIXED Mar 04, 2009
			if(id_ptr==NULL){
				HCC_TOKEN_TYPE _tok[] = {
					HCC_SEMICOLON,
					HCC_PUBLIC,
					HCC_PROTECTED,
					HCC_PRIVATE,
					HCC_IDENTIFIER,
				};
				Resync(_tok, sizeof(_tok)/sizeof(HCC_TOKEN_TYPE));
				return offset;
			}
			//END - FIXED Mar 04, 2009
			if(token_type==HCC_LPAREN)
			{
				//the data member offset
				id_ptr->getDeclDefinition().user_data.offset = 0; //not used here
				//must be a new member function...				
				parseMemberFunction(class_ptr, base_type_ptr, id_ptr, offset, member_type);
				return offset;
			}else{
				if(member_type==HCC_VIRTUAL)
				{
					//cannot have virtual data members, because that concept doesn't exit!
					HccErrorManager::Error(HccErrorManager::errVirtualDataMemberNotAllowed);
				}
				if(member_type==HCC_EXTERN)
				{
					//global variables can only be extern
					//data members not accepted!
					HccErrorManager::Error(HccErrorManager::errExternDataMemberNotAllowed);
				}
				//if this variable is an array, parse it as an array...
				if(token_type==HCC_LBRACKET){				
					parseArrayType(&id_ptr->getTypeSpecifier(), NULL);
					//keep track of the array's size
					long array_size = getArraySizeFromType(&id_ptr->getTypeSpecifier());

					if(array_size==0)
						HccErrorManager::Error(HccErrorManager::errInvalidDeclaration, _T("; arrays cannot have dimensions with value zero."));
					else 
						total_size += array_size;
					//we set the size in bytes for whatever type-spec were used...
					id_ptr->getTypeSpecifier().setDataType(HCC_CHAR_TYPE, HCC_NO_MODIFIER, array_size);
					//the next data member offset;
					if(member_type!=HCC_STATIC)
						offset += array_size;
					//
				}else if(member_type!=HCC_STATIC){
					//keep track of the variable's sizes
					total_size += base_type_ptr->getDataTypeSize();
					//the next data member offset;
					offset += base_type_ptr->getDataTypeSize();
				}
				//
				if(member_type!=HCC_STATIC)
				{
					id_ptr->getDeclDefinition().set_identifier_scope(SCOPE_LOCAL);
					//A D D   T O   T H E   C L A S S ' S   D A T A   M E M B E R S  G R O U P 
					class_ptr->getTypeSpecifier().user_type_members.data_members.push_back(id_ptr);
				}else{
					id_ptr->getDeclDefinition().set_identifier_scope(SCOPE_GLOBAL);
					//register in the globals list (to maintain its order in the program)
					g_program_global_variables.push_back(id_ptr);
					//BEGIN - FIXED - Dec 28, 2008
					if(id_ptr->getDeclDefinition().identifier_type()==DECL_NEW_DATA_MEMBER)
					{
						//the global data is also a data member
						class_ptr->getTypeSpecifier().user_type_members.data_members.push_back(id_ptr);
					}
					//END - FIXED - Dec 28, 2008
				}
			}
		}
		//to avoid AV when destroying the symbol...
		TypeSpecifier* scalar_ptr = getArrayScalarType(&id_ptr->getTypeSpecifier());
		if(scalar_ptr==HccTypeChecker::ts_type_char_ptr)
			id_ptr->getDeclDefinition().constant.value.String = 0;
		
		if(token_type==HCC_ASSIGN_OP)
		{
			if(class_ptr!=NULL && fn_ptr==NULL)
			{
				//use a new icode for every class...
				icode_ptr = theICodeResponsible.getClassICode(&class_ptr->getTypeSpecifier());
				assert(icode_ptr!=NULL);
			}			
			//---------------------------------------------------------------------------
			icode_ptr->append(HCC_IDENTIFIER);
			icode_ptr->insert_line_marker();
			appendSymbolToICode(HCC_TOKEN_ERROR, id_ptr);

			getTokenAppend();
			//L O C A L  , G L O B A L S, A N D   D A T A   M E M B E R S   (V A R I A B L E S)
			{				
				HCC_PARSE_TREE* tree_ptr = ParseExprList(fn_ptr);

				if(tree_ptr!=NULL && tree_ptr->type_ptr!=NULL){
					//BEGIN - FIXED Dec 25, 2008
					TypeSpecifier* left_type_spec_ptr = id_ptr->getTypeSpecifier().getBaseTypeSpec();
					TypeSpecifier* right_type_spec_ptr = tree_ptr->type_ptr;

					TypeSpecifier* result_expr_type_ptr = 
										HccTypeChecker::GetResultTypeFromExprTypes(left_type_spec_ptr,
																				   right_type_spec_ptr);

					if(left_type_spec_ptr->getDataType()==HCC_INTEGER
						&&
						result_expr_type_ptr->getDataType()==HCC_FLOATING_POINT)
					{
						HccWarningManager::Warning(HccWarningManager::warnImplicitConversionPossibleLossOfData, 
													_T("Do explicit type conversion to avoid truncation."));
					}else
						HccTypeChecker::CheckCompatibleAssignment(left_type_spec_ptr,
																right_type_spec_ptr,
																HccErrorManager::errInvalidAssignment, 
																_T("(must have the same type as l-value)."));

					//
					/*
					HccTypeChecker::CheckCompatibleAssignment(id_ptr->getTypeSpecifier().getBaseTypeSpec(),
															  tree_ptr->type_ptr,
															  HccErrorManager::errInvalidAssignment, 
															  _T("(must have the same type as l-value)."));
					*/
					//END - FIXED Dec 25, 2008
				}else
					HccErrorManager::Error(HccErrorManager::errInvalidAssignment);
									
				delete tree_ptr;
			}
			//---------------------------------------------------------------------------
			if(class_ptr!=NULL  && fn_ptr==NULL)
			{
				//restore to the global icode object...	
				icode_ptr = global_icode_ptr;
				assert(icode_ptr!=NULL);
			}
		}else 
		{			
			if(token_type!=HCC_SEMICOLON && token_type!=HCC_COMMA_OP)
				HccErrorManager::Error(HccErrorManager::errInvalidAssignment, 
									_T("; expected \'=\' as the initial assignment operator."));
			else if(!bIsConstructorCall){
				//new code to behave the same way as when assignment is done
				icode_ptr->append(HCC_IDENTIFIER);
				icode_ptr->insert_line_marker();
				if(icode_ptr->get_last_token()==HCC_IDENTIFIER)
					appendSymbolToICode(HCC_TOKEN_ERROR, id_ptr);			
				icode_ptr->append(HCC_SEMICOLON);
			}
		}

		bIsConstructorCall = false;

		//resync only to a semicolon, to a comma operator...
		HCC_TOKEN_TYPE tokens[] = {
			HCC_SEMICOLON,
			HCC_COMMA_OP,
		};
		Resync(tokens, sizeof(tokens)/sizeof(tokens[0]));
		//get the next identifier, in the comma separated list...
		nextIdentifierFromComma();
	}

	if(fn_ptr!=NULL)
		fn_ptr->getDeclDefinition().function.locals.stack_variables.total_variables_size += total_size;
	else if(class_ptr!=NULL){
		class_ptr->getTypeSpecifier().user_type_members.layout_object_size += total_size;
		//must follow a semicolon...
		getTokenIf(HCC_SEMICOLON, HccErrorManager::errMissingSemicolon);
	}
	return offset;
}


bool HCCParser::IsTypeSpecifier()
{
	return (token_type==HCC_BOOL	|| token_type==HCC_CHAR		||
			token_type==HCC_SHORT	|| token_type==HCC_LONG		||
			token_type==HCC_INT		|| token_type==HCC_INT16	||
			token_type==HCC_FLOAT	|| token_type==HCC_INT32	||
			token_type==HCC_INT64	|| token_type==HCC_DOUBLE	||
			token_type==HCC_UNSIGNED || token_type==HCC_SIGNED);
}

//-----------------------------------------------------
//	basically, this procedure is intended to be used to initialize 
//	data members with a simple value from another identifier
//
//-----------------------------------------------------
void HCCParser::parseIdentifierVariable(Symbol *id_ptr, HCC_TOKEN_TYPE sign, Symbol *function_ptr)
{
	Symbol* id2_ptr = getSymbolFromIdentifier(false, function_ptr);
	//this identifier in the declaration, must be a constant
	if(id2_ptr->getDeclDefinition().identifier_type()!=DECL_CONSTANT && 
		id2_ptr->getDeclDefinition().identifier_type()!=DECL_VARIABLE &&
		id2_ptr->getDeclDefinition().identifier_type()!=DECL_NEW_DATA_MEMBER){

		HccErrorManager::Error(HccErrorManager::errInvalidAssignment);		
		return;
	}
	DataValue* init_value_ptr = NULL;
	if(id2_ptr->getDeclDefinition().identifier_type()==DECL_CONSTANT)
		init_value_ptr = &id2_ptr->getDeclDefinition().constant.value; 
	else
		init_value_ptr = &id2_ptr->getDeclDefinition().variable.init_value;

	TypeSpecifier* base_type_ptr = id2_ptr->getTypeSpecifier().getBaseTypeSpec();
	if(base_type_ptr!=NULL)
	{
		if(base_type_ptr->getDataType()==HCC_INTEGER)
		{
			id_ptr->getDeclDefinition().variable.init_value.Integer = 
				(sign==HCC_MINUS_OP) ? -init_value_ptr->Integer : init_value_ptr->Integer;
			//
		}else if(base_type_ptr->getDataType()==HCC_FLOATING_POINT)
		{
			id_ptr->getDeclDefinition().variable.init_value.Double = 
				(sign==HCC_MINUS_OP) ? -init_value_ptr->Double : init_value_ptr->Double;
			//
		}else if(base_type_ptr->getDataType()==HCC_CHARACTER)
		{
			id_ptr->getDeclDefinition().variable.init_value.Character = 
				(sign==HCC_MINUS_OP) ? -init_value_ptr->Character : init_value_ptr->Character;			
			//
		}else if(base_type_ptr->specifier()==DSPEC_ENUM)
		{
			id_ptr->getDeclDefinition().variable.init_value.Integer = 
				(sign==HCC_MINUS_OP) ? -init_value_ptr->Integer : init_value_ptr->Integer;
		}
		else if(base_type_ptr->getDataType()==HCC_STRING_TYPE)
		{
			if(id2_ptr->getTypeSpecifier().specifier()==DSPEC_ARRAY)
			{
				//if has sign, or this array is not a string or character array, flag an error
				if(sign!=HCC_TOKEN_ERROR || 
					id2_ptr->getTypeSpecifier().array.pItemType!=HccTypeChecker::ts_type_char_ptr)
				{
					HccErrorManager::Error(HccErrorManager::errInvalidConstant);
				}
				id_ptr->getDeclDefinition().variable.init_value.String = init_value_ptr->String;
			}else
				HccErrorManager::Error(HccErrorManager::errInvalidConstant, _T(" (not a string constant or char array)."));
		}
	}else
	{
		//all declarations must have a base type
		//this was decided because it will ease the development.
		assert(0);
	}	
}

Symbol* HCCParser::getNestedNameIdentifier(__tstring& token_id)
{
	//type1::type2::type_n <--- accessing this type requires going down the names
	//concatenated with '::'...
	while(token_type==HCC_DOUBLE_COLON)
	{
		token_id += _T("::");
		getToken();
		if(token_type==HCC_IDENTIFIER)
			token_id += token_ptr->String();
		//
		if(getTokenIf(HCC_IDENTIFIER, HccErrorManager::errUnexpectedToken)==0)
			getTokenIf(HCC_IDENTIFIER, HccErrorManager::errMissingIdentifier);
	}
	SYMBOL_TABLE::LPSYMBOL symbol_ptr = symbol_table_ptr->find(token_id);
	return symbol_ptr;
}

Symbol* HCCParser::getSymbolFromIdentifier(bool bAlertError, Symbol *function_ptr)
{	
	assert(token_type==HCC_IDENTIFIER);
	__tstring identifier	= token_ptr->String();
	__tstring token_id		= identifier;	
	last_identifier = identifier + _T("@");
	
	Symbol* symbol_ptr		= NULL;
	//skip the identifier...
	getToken();
	if(token_type==HCC_DOUBLE_COLON)
	{		
		symbol_ptr = getNestedNameIdentifier(token_id);
		if(symbol_ptr!=NULL)
			return symbol_ptr;
		else{
			identifier = token_id;
			last_identifier = identifier + _T("@");
		}
	}else{
		//L O C A L   S C O P E 
		//lookup first, in the local scope of the parameter function ptr...
		if(function_ptr!=NULL)
		{
			assert(function_ptr->getDeclDefinition().function.symbol_table_ptr!=0);
			symbol_ptr = function_ptr->getDeclDefinition().function.symbol_table_ptr->find(token_id);
			if(symbol_ptr!=NULL)
				return symbol_ptr;
		}
	}
	//C L A S S   S C O P E 
	//if this is an inner type
	if(active_class_ptr != NULL)
	{
		token_id = active_class_ptr->getName();
		token_id += _T("::");
		token_id += identifier;
		symbol_ptr = symbol_table_ptr->find(token_id);
		if(symbol_ptr!=NULL)
			return symbol_ptr;

		//lookup this symbol in the class/struct symbol table
		symbol_ptr = active_class_ptr->getTypeSpecifier().getSymbolTable()->find(identifier);
		if(symbol_ptr!=NULL)
			return symbol_ptr;
		//get it climbing through the class hierarchy...
		TypeSpecifier* base_type_spec_ptr = &active_class_ptr->getTypeSpecifier();
		while(symbol_ptr==NULL)
		{
			base_type_spec_ptr = base_type_spec_ptr->getBaseTypeSpec();
			symbol_ptr = base_type_spec_ptr->getSymbolTable()->find(identifier);
			//if climbed to high in the class hierarchy, then break
			if(base_type_spec_ptr == base_type_spec_ptr->getBaseTypeSpec())
				break;
		}
		if(symbol_ptr!=NULL)
			return symbol_ptr;

		//BEGIN - ADDED Jan 7, 2009
		if(active_ns_class_ptr!=NULL)
		{
			token_id = active_ns_class_ptr->getName();
			token_id += _T("::");
			token_id += identifier;
			symbol_ptr = symbol_table_ptr->find(token_id);
			if(symbol_ptr!=NULL)
				return symbol_ptr;			
		}
		//END - ADDED Jan 7, 2009
	}
	//There is a couple of rules that must be applied here, for the next cases:
	//if found in one of the active namespaces, and exists an identifier in the global scope,
	//which identifier to use?
	set<Symbol*> ambiguity;

	//A C T I V E  / D E C L A R E D   N A M E S P A C E S 
	//try using identifier, in the active namespaces
	set<Symbol*>::iterator it_ns_ptr = active_namespaces.begin();
	while(it_ns_ptr!= active_namespaces.end())
	{
		token_id = _T("");
		Symbol* namespace_ptr = *it_ns_ptr++;			
		//use the current namespace...
		if(namespace_ptr!=NULL){
			token_id = namespace_ptr->getName();
			token_id += _T("::");
		}
		token_id += identifier;
		symbol_ptr = symbol_table_ptr->find(token_id);
		//register to verify ambiguous identifiers at the end of the analysis...
		if(symbol_ptr!=NULL)
			ambiguity.insert(symbol_ptr);
	}

	//U S I N G   N A M E S P A C E  ( D I R E C T I V E S )
	//lookup in the current using namespaces...
	set<Symbol*>::iterator it_using_ptr = using_namespaces.begin();
	while(it_using_ptr != using_namespaces.end())
	{
		token_id = _T("");
		Symbol* namespace_ptr = *it_using_ptr++;			
		//use the current namespace...
		if(namespace_ptr!=NULL){
			token_id = namespace_ptr->getName();
			token_id += _T("::");
		}
		token_id += identifier;
		symbol_ptr = symbol_table_ptr->find(token_id);
		//register to verify ambiguous identifiers at the end of the analysis...
		if(symbol_ptr!=NULL)
			ambiguity.insert(symbol_ptr);
	}

	//G L O B A L   S C O P E 
	//lookup in the global scope/namespace...
	symbol_ptr = symbol_table_ptr->find(identifier);
	if(symbol_ptr!=NULL)
		ambiguity.insert(symbol_ptr);	

	//undeclared identifier
	if(ambiguity.size()==0)
	{
		if(bAlertError)
		{
			__tstring info = _T(": \'");
			info += identifier;
			info += _T("\'.");

			HccErrorManager::Error(HccErrorManager::errUndeclaredIdentifier, info);
		}
		return NULL;
	}

	set<Symbol*>::iterator it_ambiguous_id = ambiguity.begin();
	if(ambiguity.size() > 1)
	{
		__tstring which_to_choose = _T("; which do you want to use: ");		
		while(it_ambiguous_id != ambiguity.end())
		{
			symbol_ptr = *it_ambiguous_id++;

			which_to_choose += _T("\'");
			which_to_choose += symbol_ptr->getName();

			if(it_ambiguous_id != ambiguity.end())
				which_to_choose += _T("\' or ");
		}
		which_to_choose += _T("\' ?");

		HccErrorManager::Error(HccErrorManager::errFoundAmbiguousIdentifier, which_to_choose);
		return symbol_ptr;
	}	
	symbol_ptr = *it_ambiguous_id;
	//the found identifier is not ambiguous;
	return symbol_ptr;
}

LPHCC_PARSE_TREE HCCParser::parseObjectInstanceMember(TypeSpecifier *baseTypeSpec, Symbol *function_ptr)
{
	HCC_PARSE_TREE* tree_ptr = NULL;
	if(getTokenAppendIf(HCC_PERIOD, HccErrorManager::errMissingPeriod)==0){
		Resync(HCC_IDENTIFIER);
	}

	TypeSpecifier* typeSpec = NULL;
	TypeSpecifier* arrayItemTypeSpec = NULL;
	if(token_type==HCC_IDENTIFIER && baseTypeSpec!=NULL)
	{
		if(baseTypeSpec->getSymbolTable()==NULL)
		{
			getToken();
			HccErrorManager::Error(HccErrorManager::errNotAnObject);
			HccErrorManager::Error(HccErrorManager::errInvalidObjectMember);
			return tree_ptr;
		}
		Symbol* member_ptr = baseTypeSpec->getSymbolTable()->find(token_ptr->String());		
		if(member_ptr==NULL)
		{
			while(baseTypeSpec!=NULL && member_ptr ==NULL)
			{
				baseTypeSpec = baseTypeSpec->getBaseTypeSpec();
				if(baseTypeSpec!=NULL){
					//may be a typename type-spec new-type-spec
					member_ptr = baseTypeSpec->getSymbolTable()->find(token_ptr->String());
					//to avoid infinite loops looking in the same type-spec...
					if(baseTypeSpec == baseTypeSpec->getBaseTypeSpec())
						break;
				}
			}
			//if not found in the class instance, this could be a class constant
			//so, we can do nothing to solve this problem;
			//the programmer must use classx::const_member to access it!
		}
		//if found...
		if(member_ptr!=NULL)
		{
			//get the type spec...
			typeSpec = member_ptr->getTypeSpecifier().getBaseTypeSpec();

			icode_ptr->append(member_ptr);
			getTokenAppend();

			//if not a function, verify access rights for data members...
			if(token_type!=HCC_LPAREN)
			{
				//verify access rights...
				if(active_class_ptr==NULL || &active_class_ptr->getTypeSpecifier()!=baseTypeSpec){

					MEMBER_ACCESS_TYPE access_type = member_ptr->getDeclDefinition().member_access_type();
					if(access_type==ACCESS_PRIVATE)
						HccErrorManager::Error(HccErrorManager::errNoAccessToPrivateMember);
					if(access_type==ACCESS_PROTECTED)
						HccErrorManager::Error(HccErrorManager::errNoAccessToProtectedMember);
				}
			}
__ASSIGNMENT_EXPRESSION:
			//A S S I G N M E N T - E X P R E S S I O N S 
			switch(token_type)
			{
			case HCC_INCREMENT:		//postfix ++
			case HCC_DECREMENT:		//postfix --
			case HCC_ASSIGN_OP:			// =
			case HCC_INC_ASSIGN:		// +=
			case HCC_DEC_ASSIGN:		// -=
			case HCC_MUL_ASSIGN:		// *=
			case HCC_DIV_ASSIGN:		// /=
			case HCC_MOD_ASSIGN:		// %=
			case HCC_XOR_ASSIGN:		// ^=
			case HCC_BIT_OR_ASSIGN:		// |=
			case HCC_BIT_AND_ASSIGN:	// &=
				{
					DECLARATION_TYPE type = member_ptr->getDeclDefinition().identifier_type();
					if(type!=DECL_NEW_DATA_MEMBER 
						&& type!=DECL_WRITEONLY_PROPERTY
						&& type!=DECL_READWRITE_PROPERTY)
					{
						if(type==DECL_READONLY_PROPERTY)
							HccErrorManager::Error(HccErrorManager::errReadOnlyPropertyInvalidAssignment);
						else
							HccErrorManager::Error(HccErrorManager::errInvalidAssignment);
						Resync(HCC_SEMICOLON);
					}else{

						//for now, we only support simple assignments to write properties...
						if( (type==DECL_WRITEONLY_PROPERTY	||		//a write-only property
							 type==DECL_READWRITE_PROPERTY)		//a read-write property
							&&
							token_type!=HCC_ASSIGN_OP)
						{
							HccErrorManager::Error(HccErrorManager::errInvalidAssignment, _T(": writable properties can only be assigned through operator \'=\'."));
						}

						//the node in the syntax tree...
						tree_ptr = new HCC_PARSE_TREE(NULL, token_type);
						getTokenAppend();
						
						//this tree forms an assignment
						tree_ptr->left_ptr  = new HCC_PARSE_TREE(member_ptr, HCC_IDENTIFIER);
						tree_ptr->right_ptr = ParseExprList(function_ptr);

						//for array subscripting like in: array1[0].array2[0] = expr;
						if(arrayItemTypeSpec!=NULL && tree_ptr->left_ptr!=NULL)
						{
							tree_ptr->left_ptr->type_ptr = arrayItemTypeSpec;
						}

						TypeSpecifier* _assign_type_ptr = tree_ptr->right_ptr->type_ptr;
						//if this is a pointer property, and the right side is a dynamic array, that's ok
						if(
							 member_ptr->getDeclDefinition().function.bReturnTypeIsPointer &&
							(tree_ptr->right_ptr->type_ptr->specifier()==DSPEC_ARRAY
								&&
							 tree_ptr->right_ptr->type_ptr->array.bIsDynamicArray)
						   )
						{
							_assign_type_ptr = tree_ptr->right_ptr->type_ptr->array.pItemType;
						}

						HccTypeChecker::CheckCompatibleAssignment(tree_ptr->left_ptr->type_ptr,
																  _assign_type_ptr,
																  HccErrorManager::errInvalidAssignment, 
																  _T("(must have the same type as l-value)."),
																  member_ptr->getDeclDefinition().user_data.bDataMemberIsPointer);

						tree_ptr->type_ptr	= tree_ptr->left_ptr->type_ptr;
					}
				}
				break;
			case HCC_LBRACKET:	//[
					{
						//accessing an array element...
						TypeSpecifier* baseTypeSpec = member_ptr->getTypeSpecifier().getBaseTypeSpec();
						//
						arrayItemTypeSpec = parseArraySubscript(baseTypeSpec, function_ptr);						
						//if an assignment...
						goto __ASSIGNMENT_EXPRESSION;
					}
					break;
			case HCC_PERIOD:		// .
				{
					if(member_ptr->getDeclDefinition().identifier_type()!=DECL_NEW_DATA_MEMBER)
					{
						HccErrorManager::Error(HccErrorManager::errInvalidObjectMember);
						HccErrorManager::Error(HccErrorManager::errInvalidAssignment);
						Resync(HCC_SEMICOLON);
					}else
						tree_ptr = parseObjectInstanceMember(typeSpec, function_ptr);
				}
				break;
			case HCC_LPAREN:
				{
					tree_ptr = parseFunctionCall(member_ptr, function_ptr, baseTypeSpec);
				}
				break;
			default:

				//obj1.x1 = obj2.X1;
				tree_ptr = new HCC_PARSE_TREE(member_ptr);
				break;
			}
		}else{
			getToken();
			//member not found
			HccErrorManager::Error(HccErrorManager::errInvalidObjectMember);
			HccErrorManager::Error(HccErrorManager::errNotAnObject);
			Resync(HCC_SEMICOLON);
		}
	}else{
		HCC_TOKEN_TYPE tokens[] = {
			HCC_IDENTIFIER,
			HCC_SEMICOLON,
		};

		Resync(tokens, sizeof(tokens)/sizeof(tokens[0]));
	}
	return tree_ptr;
}

//-------------------------------------------------------------
//
//	array[const_expr1][const_expr2][const_expr3][...][const_expr_n]
//
//-------------------------------------------------------------
TypeSpecifier* HCCParser::parseArraySubscript(TypeSpecifier *baseTypeSpec, Symbol *function_ptr)
{
	TypeSpecifier* result_type_ptr = NULL;
	//using the dimensions, parse every subscript type...
	short dims = getArrayDimesionsFromType(baseTypeSpec);

	if(dims > MAX_ARRAY_DIMENSION)
	{
		HccErrorManager::Error(HccErrorManager::errTooManySubscripts);
		HCC_TOKEN_TYPE tokens[] = {
			HCC_PERIOD,
			HCC_SEMICOLON,			
		};
		Resync(tokens, sizeof(tokens)/sizeof(tokens[0]));
		return baseTypeSpec;
	}
	TypeSpecifier* arrayTypeSpec = baseTypeSpec;
	TypeSpecifier* itemTypeSpec = result_type_ptr = getArrayScalarType(baseTypeSpec);

	volatile short nSubscripts = 0;
	while(token_type==HCC_LBRACKET && arrayTypeSpec!=NULL)
	{
		//array[x][y][z][...max];
		if(getTokenAppendIf(HCC_LBRACKET, HccErrorManager::errMissingLeftBracket)==0)
		{
			HCC_TOKEN_TYPE tokens[] = {
				HCC_IDENTIFIER,
				HCC_RBRACKET,
				HCC_SEMICOLON,			
			};
			Resync(tokens, sizeof(tokens)/sizeof(tokens[0]));
		}	

		if(arrayTypeSpec->specifier()==DSPEC_ARRAY)
		{
			LPHCC_PARSE_TREE tree_ptr = ParseExprList(function_ptr);
			//
			if(tree_ptr!=NULL && tree_ptr->type_ptr!=NULL)
				HccTypeChecker::CheckCompatibleAssignment(HccTypeChecker::ts_type_int_ptr,
														  tree_ptr->type_ptr, 0,
														  _T(" ,must be an integer variable or constant integer value."));
			delete tree_ptr;
		}else{
			HccErrorManager::Error(HccErrorManager::errTooManySubscripts);			
			LPHCC_PARSE_TREE tree_ptr = ParseExprList(function_ptr);
			delete tree_ptr;
		}
	
		if(getTokenAppendIf(HCC_RBRACKET, HccErrorManager::errMissingLeftBracket)==0){
			Resync(HCC_RBRACKET);
			if(token_type==HCC_RBRACKET)
				getToken();
		}

		//get next subscript...
		result_type_ptr = arrayTypeSpec = arrayTypeSpec->array.pItemType;

		nSubscripts++;
	}

	if(dims==0){
		HccErrorManager::Error(HccErrorManager::errVariableIsNotArray);
		HCC_TOKEN_TYPE tokens[] = {
			HCC_RBRACKET,
			HCC_PERIOD,
			HCC_SEMICOLON,			
		};
		Resync(tokens, sizeof(tokens)/sizeof(tokens[0]));
	}

	//BEGIN - ADDED Jan 23, 2009
	if(nSubscripts > dims)
	{
		HccErrorManager::Error(HccErrorManager::errTooManySubscripts, 
								_T("Array dimensions and the specified subscripts does not match."));
	}
	//END - ADDED Jan 23, 2009

	if(token_type==HCC_PERIOD)
	{
		HCC_PARSE_TREE* tree_ptr = parseObjectInstanceMember(itemTypeSpec, function_ptr);
		if(tree_ptr!=NULL){
			result_type_ptr = tree_ptr->type_ptr;
		}
		delete tree_ptr;
	}
	return result_type_ptr;
}

void HCCParser::parseMemberFunction(Symbol *class_ptr, TypeSpecifier *returnTypeSpec, Symbol *function_ptr, long& offset, HCC_TOKEN_TYPE fnType)
{
	assert(class_ptr!=NULL);
	assert(function_ptr!=NULL);
	assert(returnTypeSpec!=NULL);
	
	if(class_ptr==NULL)
	{
		HccErrorManager::Error(HccErrorManager::errNoFunctionsOutSideOfClassDefinition);
		HCC_TOKEN_TYPE tokens[] = {
			HCC_RBRACKET,
			HCC_CLASS,
			HCC_STRUCT,
			HCC_IDENTIFIER,
			HCC_SEMICOLON,			
		};
		Resync(tokens, sizeof(tokens)/sizeof(tokens[0]));
		return;
	}

	//references to local labels
	ref_proc_labels.clear();

	volatile bool bEntryPointProspectFound = false;
	Symbol* vtbl_symbol_ptr = NULL;

	if(fnType==HCC_VIRTUAL || fnType==HCC_ABSTRACT)
	{
		if(fnType==HCC_VIRTUAL){
			function_ptr->getDeclDefinition().set_identifier_type(DECL_NEW_VIRTUAL_FUNC_MEMBER);
		}else
			function_ptr->getDeclDefinition().set_identifier_type(DECL_NEW_VIRTUAL_ABSTRACT_FUNC_MEMBER);

		//this is important for the code generation process...
		function_ptr->setOwner(class_ptr);
		//
		//in the class's symbol table must exists a symbol to represent the vtbl ptr...
		vtbl_symbol_ptr = getClassVtbl_PtrSymbol(class_ptr, offset);		
		//
	}else if(fnType==HCC_STATIC || fnType==HCC_EXTERN){
		function_ptr->getDeclDefinition().set_identifier_type(DECL_NEW_STATIC_CLASS_MEMBER);		
		if(fnType==HCC_EXTERN)
		{
			//this avoid the code generator asking for the implementation of this function.
			function_ptr->getDeclDefinition().set_is_external(true);
		}
		//not defined the entry point yet?
		if(fnType==HCC_STATIC && HCCParser::main_entry_point_ptr==NULL)
		{
			__tstring __main = _T("::main");
			const __tstring& __name = function_ptr->getName();
			int npos = __name.rfind(__main);
			if(npos!=__tstring::npos && __name.length()==(npos + __main.length()))
			{
				//if we found it, we must verify the param list definition
				//to aprove this symbol as the main entry point...
				bEntryPointProspectFound = true;
			}
		}
	}
	else{	//normal functions
		function_ptr->getDeclDefinition().set_identifier_type(DECL_NEW_FUNC_MEMBER);
		//this is important for the code generation process...
		function_ptr->setOwner(class_ptr);
	}

	DeclDefinition* fn_decl_ptr = &function_ptr->getDeclDefinition();
	//the return type...
	fn_decl_ptr->function.return_type = returnTypeSpec;

	assert(fn_decl_ptr->function.symbol_table_ptr==0);
	fn_decl_ptr->function.symbol_table_ptr = new SymbolTable<__tstring, Symbol>(_T("__member_function__"));
	
	//C L A S S   F U N C T I O N ' S   M E M B E R   L I S T
	class_ptr->getTypeSpecifier().user_type_members.function_members.push_back(function_ptr);	
	//parse: param-list
	parseFunctionParamList(function_ptr);
	//BEGIN - ADDED Jan 5, 2009
	if(function_ptr->getName()==HPP_CLASS_DESTRUCTOR)
	{
		if(function_ptr->getDeclDefinition().function.locals.stack_params.params.size() > 0)
			HccErrorManager::Error(HccErrorManager::errDestructorCannotHaveParamList);
	}
	//END - ADDED Jan 5, 2009
	getTokenIf(HCC_RPAREN, HccErrorManager::errMissingRightParen);

	if(fnType==HCC_EXTERN){
		getTokenIf(HCC_SEMICOLON, HccErrorManager::errMissingSemicolon);
		return;
	}
	
	if(fnType==HCC_ABSTRACT){
		//add this virtual function to the vtbl...
		assert(vtbl_symbol_ptr!=NULL);
		if(vtbl_symbol_ptr!=NULL)
		{
			TypeDataMembers& vtbl_members = vtbl_symbol_ptr->getTypeSpecifier().user_type_members.function_members;
			//the virtual offset in the vtable...
			function_ptr->getDeclDefinition().user_data.offset = (vtbl_members.size() * sizeof(int));
			vtbl_members.push_back(function_ptr);
		}
		getTokenIf(HCC_SEMICOLON, HccErrorManager::errMissingSemicolon);
		return;
	}

	//determine if this is a real H++ entry point...
	//H++  M A I N   E N T R Y   P O I N T 
	if(bEntryPointProspectFound)
	{
		vector<Symbol*>& param_list = fn_decl_ptr->function.locals.stack_params.params;
		
		//if this function has zero or two params exactly...
		if(param_list.size()==0)
			HCCParser::main_entry_point_ptr = function_ptr;		
		//
		else if(param_list.size()==2)
		{
			TypeSpecifier* fst_param_type = param_list[0]->getTypeSpecifier().getBaseTypeSpec();
			TypeSpecifier* snd_param_type = param_list[1]->getTypeSpecifier().getBaseTypeSpec();

			assert(fst_param_type!=NULL);
			assert(snd_param_type!=NULL);

			//three types of entry point signatures:
			/*
				public static void main(){}

				public static void main(int argc, string[] argv){}

				public static void main(int argc, char[][] argv){}
			*/

			if((fst_param_type==HccTypeChecker::ts_type_int_ptr		|| 
				fst_param_type==HccTypeChecker::ts_type_Int32_ptr	||
				fst_param_type==HccTypeChecker::ts_type_long_ptr)
				&& 
				(
				 snd_param_type->specifier()==DSPEC_ARRAY &&
					(
					  (snd_param_type->array.pItemType==HccTypeChecker::ts_type_string_ptr)
						||
						(snd_param_type->array.pItemType->specifier()==DSPEC_ARRAY 
										&&
						 snd_param_type->array.pItemType->array.pItemType==HccTypeChecker::ts_type_char_ptr)
					)
				)
			  )
			{
				//set it as the main entry point...
				HCCParser::main_entry_point_ptr = function_ptr;
			}else
				HccErrorManager::Error(HccErrorManager::errInvalidMainParamList);
		}else
			HccErrorManager::Error(HccErrorManager::errInvalidMainParamList);
	}


	//before parsing the statements inside this function, first, verify a number of syntax rules
	//defined to member functions in inheritance.
	if(&class_ptr->getTypeSpecifier()!=class_ptr->getTypeSpecifier().getBaseTypeSpec())
	{
		TypeSpecifier* this_class_ptr = &class_ptr->getTypeSpecifier();
		TypeSpecifier* base_class_ptr = class_ptr->getTypeSpecifier().getBaseTypeSpec();

		// O V E R R I D I N G   A N D   V I R T U A L   F U N C T I O N   M A T C H I N G 	
		//if this class has defined a non-virtual member function with the same name
		//of a virtual member function in the base class, it overrides it inmediately.
		Symbol* base_member_ptr = base_class_ptr->getSymbolTable()->find(function_ptr->getName());
		if(base_member_ptr!=NULL)
		{
			//B A S E   M E M B E R   H I D I N G 
			if(
				(base_member_ptr->getDeclDefinition().identifier_type()==DECL_NEW_VIRTUAL_FUNC_MEMBER ||
				base_member_ptr->getDeclDefinition().identifier_type()==DECL_NEW_VIRTUAL_ABSTRACT_FUNC_MEMBER)
				&&
				(function_ptr->getDeclDefinition().identifier_type()==DECL_NEW_FUNC_MEMBER || 
				function_ptr->getDeclDefinition().identifier_type()==DECL_NEW_VIRTUAL_FUNC_MEMBER)
				)
			{
				//overrides it only if both functions have the same return value,
				//and the same parameter list...
				if(returnTypeSpec!=base_member_ptr->getDeclDefinition().function.return_type)
					HccErrorManager::Error(HccErrorManager::errFunctionOverrideMustHaveSameReturnValue);
				else{
					bool retIsPointer		= function_ptr->getDeclDefinition().function.bReturnTypeIsPointer;
					bool baseRetIsPointer	= base_member_ptr->getDeclDefinition().function.bReturnTypeIsPointer;
					if(retIsPointer != baseRetIsPointer)
					{
						__tstring info = _T(", cannot convert from \'");
						info += returnTypeSpec->getTypeName();
						if(retIsPointer)
							info += _T("^\' pointer and \'");
						else
							info += _T("\' and \'");
						info += base_member_ptr->getDeclDefinition().function.return_type->getTypeName();	
						if(baseRetIsPointer)
							info += _T("^\' pointer.");
						else
							info += _T("\'.");

						HccErrorManager::Error(HccErrorManager::errFunctionOverrideMustHaveSameReturnValue,info);
					}
				}
				//
				int base_fn_param_size = 
					base_member_ptr->getDeclDefinition().function.locals.stack_params.params.size();
				int this_fn_param_size = 
					function_ptr->getDeclDefinition().function.locals.stack_params.params.size();
				if(base_fn_param_size==this_fn_param_size)
				{
					for(int pa = 0; pa < this_fn_param_size; pa++)
					{
						Symbol* param_fn_base_ptr = 
							base_member_ptr->getDeclDefinition().function.locals.stack_params.params[pa];
						assert(param_fn_base_ptr!=0);
						Symbol* param_fn_this_ptr = 
							function_ptr->getDeclDefinition().function.locals.stack_params.params[pa];
						assert(param_fn_this_ptr!=0);

						TypeSpecifier* param_base_type_ptr = 
							param_fn_base_ptr->getTypeSpecifier().getBaseTypeSpec();
						assert(param_base_type_ptr!=0);

						TypeSpecifier* param_this_fn_type_ptr = 
							param_fn_this_ptr->getTypeSpecifier().getBaseTypeSpec();
						assert(param_this_fn_type_ptr!=0);

						//TODO: type-checking system:
						//check the class hierarchy of both types to determine if
						//both types match in one point...
						if(param_base_type_ptr!=param_this_fn_type_ptr)
						{
							if(param_base_type_ptr->specifier()==DSPEC_ARRAY && 
								param_this_fn_type_ptr->specifier()==DSPEC_ARRAY)
							{
								TypeSpecifier* pBaseClassItemType	= param_base_type_ptr->array.pItemType;
								TypeSpecifier* pChildClassItemType	= param_this_fn_type_ptr->array.pItemType;

								if(pBaseClassItemType==pChildClassItemType)
									continue;

								param_base_type_ptr		= pBaseClassItemType;
								param_this_fn_type_ptr	= pChildClassItemType;
							}else{
								if(param_base_type_ptr->specifier()==DSPEC_ARRAY)
									param_base_type_ptr = param_base_type_ptr->array.pItemType;

								if(param_this_fn_type_ptr->specifier()==DSPEC_ARRAY)
									param_this_fn_type_ptr = param_this_fn_type_ptr->array.pItemType;
							}

							TCHAR buffer[512];
							_stprintf(buffer, _T(", expected type: \'%s\', found: \'%s\'."),
								param_base_type_ptr->getTypeName().c_str(),
								param_this_fn_type_ptr->getTypeName().c_str());

							HccErrorManager::Error(HccErrorManager::errFunctionOverrideParamTypesMismatch, buffer);
						}
					}
					//
					//add this virtual function to the vtbl...
					if(vtbl_symbol_ptr==NULL)
						vtbl_symbol_ptr = getClassVtbl_PtrSymbol(class_ptr, offset);
					assert(vtbl_symbol_ptr!=NULL);
					if(vtbl_symbol_ptr!=NULL)
					{
						TypeDataMembers& vtbl_members = vtbl_symbol_ptr->getTypeSpecifier().user_type_members.function_members;
						//to save the virtual function's offset
						int vfn_offset = 0;

						TypeDataMembers::iterator it_virtual_fn = vtbl_members.begin();
						while(it_virtual_fn != vtbl_members.end())
						{
							Symbol* fn_ptr = *it_virtual_fn;

							if(fn_ptr->getName()==function_ptr->getName())
							{
								vfn_offset = fn_ptr->getDeclDefinition().user_data.offset;				
								//here, we change the base implementation, for the new implementation found now
								function_ptr->getDeclDefinition().user_data.offset = vfn_offset;
								//overrides the identifier type...
								//to avoid loss of performance in the code generator...
								function_ptr->getDeclDefinition().set_identifier_type(DECL_NEW_VIRTUAL_FUNC_MEMBER);
								*it_virtual_fn = function_ptr;
								break;
							}

							it_virtual_fn++;
						}
						//if this new member function was not found by name, then add it to the class vptr vtbl array...
						if(it_virtual_fn ==vtbl_members.end())
						{
							//the virtual offset in the vtable...
							function_ptr->getDeclDefinition().user_data.offset = (vtbl_members.size() * sizeof(int));
							vtbl_members.push_back(function_ptr);
						}
					}
					//
					if(false==bWarningsDisabled)
						HccWarningManager::Warning(HccWarningManager::warnVirtualFunctionOverrideInSubClass);
				}else
					HccErrorManager::Error(HccErrorManager::errFunctionOverrideMustHaveSameParamList);
			}
		}else{
			//BEGIN - FIXED Jan 5, 2008
			goto __ADD_VIRTUAL_MEMBER_TO_VTBL_PTR;
			//END - FIXED Jan 5, 2008
		}
	}else{
__ADD_VIRTUAL_MEMBER_TO_VTBL_PTR:
		//for new virtual functions, add them to the vtbl ptr array...
		if(function_ptr->getDeclDefinition().identifier_type()==DECL_NEW_VIRTUAL_FUNC_MEMBER)
		{
			//add this virtual function to the vtbl...
			assert(vtbl_symbol_ptr!=NULL);
			if(vtbl_symbol_ptr!=NULL)
			{
				TypeDataMembers& vtbl_members = vtbl_symbol_ptr->getTypeSpecifier().user_type_members.function_members;
				//the virtual offset in the vtable...
				function_ptr->getDeclDefinition().user_data.offset = (vtbl_members.size() * sizeof(int));
				vtbl_members.push_back(function_ptr);
			}
		}
	}
	//
	//use a new icode for every function...
	icode_ptr = theICodeResponsible.getFunctionICode(function_ptr);
	assert(icode_ptr!=NULL);
	icode_ptr->set_source_buffer(lexer->sourceBuf());	

	icode_ptr->append(token_type); //{
	//the function body {}...
	getTokenAppend(); // next token from {
	HCC_TOKEN_TYPE last_stmt_here = ParseStatementList(HCC_RBLOCK_KEY, function_ptr); //{ statement-list }
	//
	if(last_stmt_here!=HCC_RETURN && returnTypeSpec!=HccTypeChecker::ts_type_void_ptr)
		HccErrorManager::Error(HccErrorManager::errFunctionMustReturnValue);

	icode_ptr->append(token_type); //}
	//
	//restore to the global icode object...	
	icode_ptr = global_icode_ptr;
	assert(icode_ptr!=NULL);

	if(getTokenAppendIf(HCC_RBLOCK_KEY, HccErrorManager::errMissingEndBlock)==0){ //}
		Resync(HCC_RBLOCK_KEY);
	}

	//check all local labels references, to determine if exists a label not declared...
	//to keep track of user-defined proc labels
	set<__tstring>& proc_labels = map_proc_labels[function_ptr];
	//to verify labels where defined
	set<__tstring>::iterator it_ref_label = ref_proc_labels.begin();
	while(it_ref_label!= ref_proc_labels.end())
	{
		__tstring& ref_label = (__tstring&)*it_ref_label++;

		set<__tstring>::iterator it_found = proc_labels.find(ref_label);
		if(it_found == proc_labels.end())
		{
			__tstring info = _T("-> label \'");
			info += ref_label;
			info += _T("\', not defined at: ");
			info += function_ptr->getCompleteName();
			info += _T(".");
			HccErrorManager::Error(HccErrorManager::errUndeclaredIdentifier, info);
		}
	}

	//check all defined labels, to determine if they are referenced by goto statements
	set<__tstring>::iterator it_fnlabel = proc_labels.begin();
	while(it_fnlabel != proc_labels.end())
	{
		__tstring& fn_label = (__tstring&)*it_fnlabel++;
		if(ref_proc_labels.size()==0)
			goto __UNREF_LABEL;
		else{
			it_ref_label = ref_proc_labels.find(fn_label);
			if(it_ref_label ==ref_proc_labels.end()){
__UNREF_LABEL:
			HccWarningManager::Warning(HccWarningManager::warnUnreferencedProcLabel, fn_label);
			}
		}

	}	
}

void HCCParser::parseFunctionParamList(Symbol *function_ptr)
{
	assert(function_ptr!=NULL);
	//skip '('
	getTokenIf(HCC_LPAREN, HccErrorManager::errMissingRightParen);
	volatile bool bParamByRef	= false,
				  bIsConst		= false;
	volatile long total_param_size = 0;
	volatile long param_offset = 8;
	while(token_type!=HCC_RPAREN)
	{
		//a empty param-list when using the empty-type 'void'
		if(token_type==HCC_VOID)
		{
			getToken();
			break;
		}
		//param-list :
		//[const] type-spec [ ref | ^ ] <identifier> = const-expr param-list |
		//<e>
		if((bIsConst = (token_type==HCC_CONST)))
			getToken();

		TypeSpecifier* type_spec_ptr = parseTypeSpec(NULL);

		bool bIsArrayParam = false;
		bool bIsPointerParam = false;
		short nDimensions = 0;
		short nItemCountIndex = 0;
		short nArrayItemCount[] = {0,0}; //up to 2 dimensions only
		bool bArrayHasFixedDimensions = false;

		if((bParamByRef = (token_type==HCC_REF)))
		{
			getToken();	//ref
		}else if(token_type==HCC_LBRACKET)
		{
			bIsArrayParam = true;
			while(token_type==HCC_LBRACKET)
			{
				getToken(); //the '[' symbol
				nDimensions++;
				//BEGIN - hlm Nov 11, 2011
				//I wanted to have a fixed-dimensions array parameters...				
				if(token_type==HCC_NUMBER){
					if(token_ptr->dataType!=HCC_INTEGER)
						HccErrorManager::Error(HccErrorManager::errInvalidSubscriptType, _T(" ,must be an integer or constant integer value."));
					else
						nArrayItemCount[nItemCountIndex++] = (int)token_ptr->value.Integer;
					//skip this constant
					getToken();
				}
				//END - hlm Nov 11, 2011
				getTokenIf(HCC_RBRACKET, HccErrorManager::errMissingRightBracket); // ']'
			}

			if(nItemCountIndex > 0)
			{
				bArrayHasFixedDimensions = true;
				nItemCountIndex = 0;
			}
		}else if(token_type==HCC_XOR_OP)
		{
			bIsPointerParam = true;
			getToken(); //^
		}

		//T H E   P A R A M E T E R   I D E N T I F I E R
		if(token_type==HCC_IDENTIFIER)
		{
			Symbol* param_ptr = 
				function_ptr->getDeclDefinition().function.symbol_table_ptr->insert(token_ptr->String());

			//if this param already exists in the function's symbol table...
			if(param_ptr==NULL){
				__tstring info = _T(", parameter : \'");
				info += token_ptr->String();
				info += _T("\'");
				HccErrorManager::Error(HccErrorManager::errRedeclaredIdentifier, info);
				Resync(HCC_COMMA_OP);				
			}else{
				//the param type...
				param_ptr->getTypeSpecifier().setBaseTypeSpec(type_spec_ptr);
				//set the param's offset...
				param_ptr->getDeclDefinition().user_data.offset = param_offset;
				//
				param_ptr->getDeclDefinition().set_identifier_scope(SCOPE_LOCAL);
				//to avoid AV when terminating...
				param_ptr->getDeclDefinition().constant.value.String = 0;

				if(bIsArrayParam) //only one-dim param is accepted for now!
				{
					if(bIsConst)
						param_ptr->getDeclDefinition().set_identifier_type(DECL_PARAM_CONST_ARRAY); //const ref | const ^; 
					else
						param_ptr->getDeclDefinition().set_identifier_type(DECL_PARAM_ARRAY); //ref | ^
					//
					TypeSpecifier* id_type_spec_ptr = &param_ptr->getTypeSpecifier();
					while(true)
					{
						TypeSpecifier* arraySpec = id_type_spec_ptr;
						//the element type-spec
						TypeSpecifier* base_type_spec		= arraySpec->getBaseTypeSpec(),
									  *itemTypeSpec			= arraySpec->getBaseTypeSpec();

						//mark it as an array!
						arraySpec->set_specifier(DSPEC_ARRAY);
						//set as itself!
						arraySpec->setBaseTypeSpec(arraySpec);
						//array of type itemTypeSpec
						arraySpec->array.pItemType = itemTypeSpec;
						//BEGIN - hlm Nov 11,2011
						if(bArrayHasFixedDimensions)
							arraySpec->array.item_count = nArrayItemCount[nItemCountIndex];
						else
							arraySpec->array.item_count = 0;
						//END - hlm Nov 11,2011
						if(--nDimensions > 0)
						{
							//when this array has more than one dimension,
							//a new type specifier of type DSPEC_ARRAY must be created
							//and set as the item type for the current array
							//avoiding Stack Overflow when calling getArraySizeFromType();
							//
							//the base type must be the type-specifier found for this declaration...
							itemTypeSpec = new TypeSpecifier(DSPEC_ARRAY);
							arraySpec->array.pItemType = itemTypeSpec;
							arraySpec->array.item_count = nArrayItemCount[++nItemCountIndex];
							//set the previous type
							itemTypeSpec->setBaseTypeSpec(base_type_spec);

							id_type_spec_ptr = itemTypeSpec;
							continue;
						}
						break;
					}
					//
					//acumulate the param type sizes
					total_param_size	+= sizeof(int);
					param_offset		+= sizeof(int);
					
				}else if(bIsPointerParam)
				{
					if(bIsConst)
						param_ptr->getDeclDefinition().set_identifier_type(DECL_PARAM_CONST_POINTER); //const ref | const ^; 
					else
						param_ptr->getDeclDefinition().set_identifier_type(DECL_PARAM_POINTER); //ref | ^

					//acumulate the param type sizes
					total_param_size	+= sizeof(int);
					param_offset		+= sizeof(int);
				}else if(bParamByRef){
					if(bIsConst)
						param_ptr->getDeclDefinition().set_identifier_type(DECL_PARAM_CONST_BYREF); //const ref | const ^; 
					else
						param_ptr->getDeclDefinition().set_identifier_type(DECL_PARAM_BYREF); //ref | ^

					//acumulate the param type sizes
					total_param_size	+= sizeof(int);
					param_offset		+= sizeof(int);
				}else{
					param_ptr->getDeclDefinition().set_identifier_type(DECL_PARAM_VALUE);
					//acumulate the param type sizes
					int param_size	= type_spec_ptr->getDataTypeSize();					
					int modulus		= param_size % sizeof(int);
					//size must be multiple of sizeof(int) int 32-bit CPUs
					if(modulus > 0)
						param_size += (sizeof(int) - modulus);
					total_param_size += param_size;
					param_offset += param_size;
				}
				//the function-param-list
				function_ptr->getDeclDefinition().function.locals.stack_params.params.push_back(param_ptr);
			}
			//skip it
			getToken();
		}else{
			HccErrorManager::Error(HccErrorManager::errMissingIdentifier, _T(" in a parameter declaration."));
			HCC_TOKEN_TYPE tokens[] = {
				HCC_RPAREN,
				HCC_COMMA_OP,							
			};
			Resync(tokens, sizeof(tokens)/sizeof(tokens[0]));
		}
		//next declaration from comma,
		//skip this comma, the next, must be an type/identifier
		if(token_type==HCC_COMMA_OP){
			do{
				getToken();
				/*
				HCC_TOKEN_TYPE tokens[] = {				
					HCC_COMMA_OP,
					HCC_RPAREN,
					HCC_IDENTIFIER,
				};
				Resync(tokens, sizeof(tokens)/sizeof(tokens[0]));
				*/
				if(token_type==HCC_COMMA_OP)					
					HccErrorManager::Error(HccErrorManager::errNotATypeIdentifier, _T(" after the \',\'."));				
				//
			}while(token_type==HCC_COMMA_OP);

			if(token_type==HCC_RPAREN)
				HccErrorManager::Error(HccErrorManager::errNotATypeIdentifier, _T(" after the \',\'."));
			//
		}else if(token_type!=HCC_RPAREN){
			__tstring info = _T(" before type specifier \'");
			info += token_ptr->String();
			info += _T("\'.");
			HccErrorManager::Error(HccErrorManager::errMissingComma, info);
			//no need to resync because of the loop
		}
	}
	function_ptr->getDeclDefinition().function.locals.stack_params.total_params_size = total_param_size;
}

void HCCParser::parseFunctionArgumentList(Symbol *called_function_ptr, Symbol *caller_function_ptr)
{
	//C A L L E D   F U N C T I O N 
	assert(called_function_ptr!=NULL);
	assert(called_function_ptr->getDeclDefinition().function.symbol_table_ptr!=NULL);

	//C A L L E R 
	if(caller_function_ptr!=NULL)
	{
		assert(caller_function_ptr->getDeclDefinition().function.symbol_table_ptr!=NULL);
	}

	//C A L L E D
	if(called_function_ptr!=NULL && called_function_ptr->getDeclDefinition().function.symbol_table_ptr!=NULL)
	{
		StackParams	&stack_params = called_function_ptr->getDeclDefinition().function.locals.stack_params;		
		if(stack_params.params.size()==0 && (token_type==HCC_RPAREN || token_type==HCC_VOID))
		{
			if(token_type==HCC_VOID)
				getToken();
		}else if(stack_params.params.size()==0 && token_type!=HCC_RPAREN){
			HccErrorManager::Error(HccErrorManager::errFunctionWithNoParams);
			Resync(HCC_RPAREN);			
		}else{
			long param_count = 0;			
			//the first argument...
			TypeSpecifier* argument_type = parseArgument(stack_params.params[param_count], caller_function_ptr);
			//strong type checking...			
			if(argument_type!=NULL)
			{
				//BEGIN - FIXED Dec 25, 2008
				__tstring param_name = _T(" for parameter: ");
				param_name += stack_params.params[param_count]->getName();

				TypeSpecifier* parameter_type = stack_params.params[param_count]->getTypeSpecifier().getBaseTypeSpec();
				//
				checkImplicitParamConversion(param_name, parameter_type, argument_type);
				//END - FIXED Dec 25, 2008
			}else{
				HccErrorManager::Error(HccErrorManager::errInvalidAssignment);
				
				HCC_TOKEN_TYPE next_arg_tokens[] = {HCC_COMMA_OP, HCC_RPAREN, HCC_SEMICOLON};
				Resync(next_arg_tokens, 
					   sizeof(next_arg_tokens)/sizeof(HCC_TOKEN_TYPE));
			}
			//the next param
			param_count++;
			while(token_type==HCC_COMMA_OP || token_type!=HCC_RPAREN)
			{				
				getTokenAppend();
				if((unsigned long)param_count < (unsigned long)stack_params.params.size())
					argument_type = parseArgument(stack_params.params[param_count], caller_function_ptr);
				//strong type checking...			
				if(argument_type!=NULL)
				{
					//BEGIN - CODE REMOVED Dec 25, 2008
					/*
					if((unsigned long)param_count < (unsigned long)stack_params.params.size())
					{
						__tstring param_name = _T(" for parameter: ");
						param_name += stack_params.params[param_count]->getName();
						HccTypeChecker::CheckCompatibleAssignment(
												stack_params.params[param_count]->getTypeSpecifier().getBaseTypeSpec(),
												argument_type, 
												HccErrorManager::errIncompatibleTypes,
												param_name);
					}
					*/
					if((unsigned long)param_count < (unsigned long)stack_params.params.size())
					{
						__tstring param_name = _T(" for parameter: ");
						param_name += stack_params.params[param_count]->getName();
						TypeSpecifier* parameter_type = stack_params.params[param_count]->getTypeSpecifier().getBaseTypeSpec();
						//
						checkImplicitParamConversion(param_name, parameter_type, argument_type);
					}
					//END - CODE REMOVED Dec 25, 2008
				}else{
					HccErrorManager::Error(HccErrorManager::errInvalidAssignment);

					HCC_TOKEN_TYPE next_arg_tokens[] = {HCC_COMMA_OP, HCC_RPAREN, HCC_SEMICOLON};
					Resync(next_arg_tokens, 
						   sizeof(next_arg_tokens)/sizeof(HCC_TOKEN_TYPE));
				}

				param_count++;
			}
			//different param count? flag error...
			if(param_count != stack_params.params.size())
			{
				HccErrorManager::Error(HccErrorManager::errWrongNumberOfArguments);
				TCHAR lpszParams[50];
				_stprintf(lpszParams, _T("%d parameters. Unknown function signature."), param_count);				
				HccErrorManager::Error(HccErrorManager::errFunctionDoesNotTakeXParams, lpszParams);
			}			
		}
	}
	//
}

LPHCC_PARSE_TREE HCCParser::parseFunctionCall(Symbol *called_function_ptr, Symbol *caller_function_ptr, TypeSpecifier* owner_type_ptr)
{
	LPHCC_PARSE_TREE parse_tree = NULL;
	assert(called_function_ptr!=NULL);

	//this should never happen
	if(called_function_ptr==NULL)
	{
		HccErrorManager::Error(HccErrorManager::errIdentifierNotAFunction, _T(", internal compiler error."));
		return NULL;
	}

	DECLARATION_TYPE called_decl_type = called_function_ptr->getDeclDefinition().identifier_type();


	if(	
	   called_decl_type==DECL_NEW_FUNC_MEMBER					||
	   called_decl_type==DECL_NEW_VIRTUAL_FUNC_MEMBER			||
	   called_decl_type==DECL_NEW_VIRTUAL_ABSTRACT_FUNC_MEMBER	||
	   called_decl_type==DECL_NEW_STATIC_CLASS_MEMBER			||
	   called_decl_type==DECL_OPERATOR_MIN						||		//the inline-min between two integer types
	   called_decl_type==DECL_OPERATOR_MAX						||		//the inline-max between two integer types
	   called_decl_type==DECL_OPERATOR_SIZEOF					//the inline-sizeof operator
	   )
	{
__PARSE_AS_FUNCTION_ANYWAY:
		if(owner_type_ptr==NULL)
		{
			DECLARATION_TYPE caller_decl_type = DECL_NEW_STATIC_CLASS_MEMBER;

			if(caller_function_ptr!=NULL)
					caller_decl_type = caller_function_ptr->getDeclDefinition().identifier_type();
			//
			if(caller_decl_type==DECL_NEW_STATIC_CLASS_MEMBER &&
			   (called_decl_type==DECL_NEW_FUNC_MEMBER			|| 
			    called_decl_type==DECL_NEW_VIRTUAL_FUNC_MEMBER	||
				called_decl_type==DECL_NEW_VIRTUAL_ABSTRACT_FUNC_MEMBER))
			{
				HccErrorManager::Error(HccErrorManager::errCannotCallInstanceMemberFromStatic);
			}
		}
		getTokenAppend();	//(
		//this is a call to a function...
		parseFunctionArgumentList(called_function_ptr, caller_function_ptr);
		//
		getTokenAppendIf(HCC_RPAREN, HccErrorManager::errMissingRightParen); //)

		parse_tree = new HCC_PARSE_TREE(called_function_ptr, HCC_IDENTIFIER);
		parse_tree->type_ptr = called_function_ptr->getDeclDefinition().function.return_type;

		//ADDED/FIXED Mar 01, 2009
		//if the return type is a pointer, then the final expression is also a pointer
		parse_tree->set_is_pointer_expr(called_function_ptr->getDeclDefinition().function.bReturnTypeIsPointer);

		MEMBER_ACCESS_TYPE access_type = called_function_ptr->getDeclDefinition().member_access_type();

		if(active_class_ptr!=NULL)
		{
			Symbol* member_function_ptr = 
				active_class_ptr->getTypeSpecifier().getSymbolTable()->find(called_function_ptr->getName());
			//if the requested member function is not in the active class's symbol table
			//this could be static a static function...
			if(member_function_ptr==NULL)
			{
				{
					TypeSpecifier* base_type_spec_ptr = &active_class_ptr->getTypeSpecifier();
					while(member_function_ptr==NULL)
					{
						base_type_spec_ptr = base_type_spec_ptr->getBaseTypeSpec();
						member_function_ptr = base_type_spec_ptr->getSymbolTable()->find(called_function_ptr->getName());
						//if climbed to high in the class hierarchy, then break
						if(base_type_spec_ptr == base_type_spec_ptr->getBaseTypeSpec())
							break;
					}
					if(access_type==ACCESS_PRIVATE){
						//cannot access a private member...
						if(&active_class_ptr->getTypeSpecifier()!=base_type_spec_ptr || member_function_ptr==NULL)
							HccErrorManager::Error(HccErrorManager::errNoAccessToPrivateMember);
					}else if(access_type==ACCESS_PROTECTED)
					{
						TypeSpecifier* class_type_ptr	= &active_class_ptr->getTypeSpecifier();
						while(class_type_ptr!=base_type_spec_ptr)
						{
							class_type_ptr = class_type_ptr->getBaseTypeSpec();
							if(class_type_ptr == class_type_ptr->getBaseTypeSpec())
								break;
						}
						//can only access protected members, in a class hierarchy...
						if(class_type_ptr!=base_type_spec_ptr  || member_function_ptr==NULL)
							HccErrorManager::Error(HccErrorManager::errNoAccessToProtectedMember);
					}else{
						//if is a public member, and is non-static, then we do nothing, because,
						//this procedure is called from parseObjectInstance doing a parseFunctionCall
						//so this member was verified its existence previously!
						//except, if this one is static, we do a possibly second confirmation of its existance...
						//find it in the globals...
						if(called_function_ptr->getDeclDefinition().identifier_type()==DECL_NEW_STATIC_CLASS_MEMBER)
							member_function_ptr = find_symbol(called_function_ptr->getName().c_str());
					}
				}
			}
		}		
	}else if(
			   (
			   called_function_ptr->getDeclDefinition().identifier_type()==DECL_BUILTIN_CONSOLE_WRITE	||
			   called_function_ptr->getDeclDefinition().identifier_type()==DECL_BUILTIN_CONSOLE_WRITELN
			   )
			 )
	{
		//for any new build-in function, we MUST add an ORing comparison with the decl definition 
		//of the function
		parse_tree = ParseCallWriteWriteLnBuiltInFunction(called_function_ptr, caller_function_ptr, false);

		return parse_tree;
		//
	}else{
		HccErrorManager::Error(HccErrorManager::errIdentifierNotAFunction);
		goto __PARSE_AS_FUNCTION_ANYWAY;
	}

	//BEGIN - ADDED Mar 1, 2009
	if(token_type==HCC_PERIOD)
	{
		TypeSpecifier* pItemType = parse_tree->type_ptr;
		delete parse_tree;
		parse_tree = parseObjectInstanceMember(pItemType, caller_function_ptr);
	}
	//END - ADDED Mar 1, 2009

	return parse_tree; //HCC_PARSE_TREE
}

TypeSpecifier* HCCParser::parseArgument(Symbol *parameter_ptr, Symbol *func_scope_ptr)
{
	DECLARATION_TYPE param_type = parameter_ptr->getDeclDefinition().identifier_type();
	volatile bool bParamByRef = 
		(param_type==DECL_PARAM_BYREF || param_type==DECL_PARAM_POINTER || param_type==DECL_PARAM_ARRAY); //ref	| ^ | []

	DECLARATION_TYPE arg_decl_type = DECL_VARIABLE;

	LPHCC_PARSE_TREE parse_tree = NULL;
	TypeSpecifier* argument_type = NULL;
	//the first argument...
	if(bParamByRef)
	{
		//BEGIN - FIXED Mar 1, 2009
		HCC_TOKEN_TYPE hcc_ptr_op = HCC_TOKEN_ERROR;
		if(param_type==DECL_PARAM_BYREF)		//R E F   P A R A M  O N L Y 
		{
			if(HCC_POINTER_DEREFERENCE==token_type)
			{
				hcc_ptr_op = HCC_POINTER_DEREFERENCE;
				getTokenAppend(); //*
			}
		}else if(HCC_POINTER_ADDRESSOF==token_type)		//BEGIN - P O I N T E R   P A R A M   O N L Y
		{
			hcc_ptr_op = HCC_POINTER_ADDRESSOF;
			getTokenAppend(); //skip the '&'
		}else if(HCC_DYNAMIC_CAST==token_type)		//for: function(dynamic_cast(pointer-expression));
		{
			HCC_PARSE_TREE* tree_ptr = ParseExprList(func_scope_ptr);
			if(tree_ptr!=NULL)
			{
				TypeSpecifier* pItemType = tree_ptr->type_ptr;
				delete tree_ptr;
				return pItemType;
			}
			return NULL;
		}
														//END - P O I N T E R   P A R A M   O N L Y
		//END - FIXED Mar 1, 2009
		if(token_type==HCC_IDENTIFIER)
		{
			Symbol* id_ptr = getSymbolFromIdentifier(true, func_scope_ptr);
			if(id_ptr!=NULL)
			{
				argument_type = id_ptr->getTypeSpecifier().getBaseTypeSpec();
				arg_decl_type = id_ptr->getDeclDefinition().identifier_type();

				if(param_type==DECL_PARAM_POINTER)
				{
					//BEGIN - FIXED Mar 1, 2009
					if(arg_decl_type==DECL_PARAM_VALUE		|| 
						arg_decl_type==DECL_VARIABLE		||					
						arg_decl_type==DECL_PARAM_BYREF		||					
						arg_decl_type==DECL_NEW_DATA_MEMBER	||					
						arg_decl_type==DECL_WRITEONLY_PROPERTY	|| arg_decl_type==DECL_READWRITE_PROPERTY)
					{
						if(hcc_ptr_op==HCC_POINTER_ADDRESSOF) //&identifier ?
						{
							//add pointer variable to icode buffer...
							appendSymbolToICode(HCC_TOKEN_ERROR, id_ptr);

						}else if(HCC_PERIOD==token_type)
						{
							//register in the icode...
							appendSymbolToICode(HCC_TOKEN_ERROR, id_ptr);
							//obj.func(); for now, it's the only expression supported
							//TODO: &obj.member1;
							TypeSpecifier* baseTypeSpec = id_ptr->getTypeSpecifier().getBaseTypeSpec();
							HCC_PARSE_TREE* tree_ptr = parseObjectInstanceMember(baseTypeSpec, func_scope_ptr);
							if(tree_ptr!=NULL)
							{
								if(false==tree_ptr->is_pointer_expr())
									HccErrorManager::Error(HccErrorManager::errInvalidPointerExpression, _T(", argument must be a pointer expression."));
								//
								argument_type = tree_ptr->type_ptr;
								delete tree_ptr;
							}else
								HccErrorManager::Error(HccErrorManager::errInvalidPointerExpression);
						}else if(DECL_NEW_DATA_MEMBER==arg_decl_type && 
								id_ptr->getDeclDefinition().user_data.bDataMemberIsPointer)
						{
							//add a data member pointer to the icode buffer...
							appendSymbolToICode(HCC_TOKEN_ERROR, id_ptr);
						}
						else
						{
							//error: must use '&' operator
							HccErrorManager::Error(HccErrorManager::errInvalidPointerExpression, 
												 _T(";pointer argument must use operator \'&\' with a variable or member,")
												 _T(" a true pointer is expected."));
						}
					}else if(arg_decl_type==DECL_POINTER_VARIABLE)
					{
						//add pointer variable to icode buffer...
						appendSymbolToICode(HCC_TOKEN_ERROR, id_ptr);
					}
					//END - FIXED Mar 1, 2009
					//
				}else if(param_type==DECL_PARAM_BYREF)
				{
					if(arg_decl_type!=DECL_VARIABLE				&& 						
						arg_decl_type!=DECL_PARAM_VALUE			&&
						arg_decl_type!=DECL_PARAM_BYREF			&& 						
						arg_decl_type!=DECL_NEW_DATA_MEMBER		&&						
						arg_decl_type!=DECL_WRITEONLY_PROPERTY	&&
						arg_decl_type!=DECL_READWRITE_PROPERTY	&&
						arg_decl_type!=DECL_PARAM_ARRAY)
					{
						if(arg_decl_type==DECL_POINTER_VARIABLE) //local dynamic arrays are pointer variables
						{
							if(HCC_LBRACKET==token_type)
								goto __ENTER_ARRAY_SUBSCRIPT;
							else if(HCC_POINTER_DEREFERENCE==hcc_ptr_op)
								goto __CONTINUE_PARAM_REF_OK;
						}
						HccErrorManager::Error(HccErrorManager::errInvalidReferenceParams, 
											_T("; cannot use this identifier as argument for this reference parameter."));
					}else if(arg_decl_type==DECL_PARAM_ARRAY)
					{
__ENTER_ARRAY_SUBSCRIPT:
						//register in the icode...
						appendSymbolToICode(HCC_TOKEN_ERROR, id_ptr);
						if(token_type==HCC_LBRACKET)	//[
						{
							//accessing an array element...
							TypeSpecifier* baseTypeSpec = id_ptr->getTypeSpecifier().getBaseTypeSpec();
							//
							argument_type = parseArraySubscript(baseTypeSpec, func_scope_ptr);
						}else{
							HccErrorManager::Error(HccErrorManager::errInvalidSubscriptType);
							HccErrorManager::Error(HccErrorManager::errInvalidReferenceParams, 
												_T(", expected a subscript array to pass as a reference argument."));
						}
__CONTINUE_PARAM_REF_OK: ; //to keep the compiler happy
					}else{
						//register in the icode...
						appendSymbolToICode(HCC_TOKEN_ERROR, id_ptr);
					}
					//
				}else if(param_type==DECL_PARAM_ARRAY 
						&& 
						(arg_decl_type==DECL_POINTER_VARIABLE	|| 
						 arg_decl_type==DECL_VARIABLE			||
						 arg_decl_type==DECL_PARAM_ARRAY
						 )
					  )
				{
					 //check argument type for array specifications like:
					// Dimensions, and Scalar Type...
					short arg_dim	= 0;
					TypeSpecifier* arg_type_ptr		= HccTypeChecker::ts_type_int_ptr;

					short param_dim	= getArrayDimesionsFromType(parameter_ptr->getTypeSpecifier().getBaseTypeSpec());
					TypeSpecifier* param_type_ptr	= 
										getArrayScalarType(parameter_ptr->getTypeSpecifier().getBaseTypeSpec());

					HCC_TOKEN_TYPE prev_token = HCC_TOKEN_ERROR;
					if(argument_type!=NULL && argument_type->specifier()==DSPEC_ARRAY)
					{
						arg_dim		= getArrayDimesionsFromType(argument_type);
						prev_token = token_type;							
						if((prev_token = token_type)==HCC_LBRACKET){
							//token_id, then id_ptr, then [, then expr, then ]...
							appendSymbolToICode(HCC_TOKEN_ERROR, id_ptr);
							arg_type_ptr= parseArraySubscript(argument_type, func_scope_ptr);
						}else{
							//token_id, then id_ptr;
							appendSymbolToICode(HCC_TOKEN_ERROR, id_ptr);
						}
						//get the scalar type to make a comparison...
						arg_type_ptr= getArrayScalarType(argument_type);
						//
						if(arg_type_ptr->getDataType()==HCC_STRING_TYPE)
						{
							arg_type_ptr = arg_type_ptr->array.pItemType;
						}
					}
					//compare the array dimensions between the formal and actual paramenter...
					if(prev_token!=HCC_LBRACKET && arg_dim!=param_dim)
					{
						//error: wrong dimension for this argument array
						//expected x found y;						
						TCHAR lpszInfoError[128];
						_stprintf(lpszInfoError, 
									_T("; expected %d, but array argument has %d dimensions."), 
									param_dim, 
									arg_dim);
						HccErrorManager::Error(HccErrorManager::errWrongArgumentArrayDimensions, lpszInfoError);
					}										
					if(arg_type_ptr!=param_type_ptr)
					{
						if(arg_type_ptr->getDataType()!=param_type_ptr->getDataType())
						{
							//error: wrong data type for this argument array
							//expected x found y;
							__tstring param_name = _T(" for array parameter: ");
							param_name += parameter_ptr->getName();
							HccTypeChecker::CheckCompatibleAssignment(
													param_type_ptr,
													arg_type_ptr,
													HccErrorManager::errIncompatibleTypes,
													param_name);							
						}
					}
					argument_type = parameter_ptr->getTypeSpecifier().getBaseTypeSpec();
				}
			}
		}else
			HccErrorManager::Error(HccErrorManager::errInvalidReferenceParams, 
									_T(";reference/pointer parameter must have a variable as its argument."));				
	}else{
		//param by value
		parse_tree = ParseExprList(func_scope_ptr);
		if(parse_tree!=NULL)
			argument_type = parse_tree->type_ptr;
	}
	//strong type checking...			
	if(argument_type!=NULL)
	{
		__tstring param_name = _T(" for parameter: ");
		param_name += parameter_ptr->getName();

		//BEGIN - FIXED Dec 25, 2008
		TypeSpecifier* parameter_type = parameter_ptr->getTypeSpecifier().getBaseTypeSpec();
		//
		checkImplicitParamConversion(param_name, parameter_type, argument_type);
		/*
		HccTypeChecker::CheckCompatibleAssignment(
								parameter_ptr->getTypeSpecifier().getBaseTypeSpec(),
								argument_type,
								HccErrorManager::errIncompatibleTypes,
								param_name);
		*/
		//END - FIXED Dec 25, 2008
	}else{
		HccErrorManager::Error(HccErrorManager::errInvalidAssignment);

		HCC_TOKEN_TYPE next_arg_tokens[] = {HCC_COMMA_OP, HCC_RPAREN};
		Resync(next_arg_tokens, 
			   sizeof(next_arg_tokens)/sizeof(HCC_TOKEN_TYPE));
	}

	delete parse_tree;

	return argument_type;
}

void HCCParser::parseClassConstructor(Symbol *class_ptr)
{
	//member overloading is not supported yet!
	//I have no time to implement it for now...
	Symbol* ctor_ptr = class_ptr->getTypeSpecifier().getSymbolTable()->find(class_ptr->getName());
	if(ctor_ptr!=NULL)
		HccErrorManager::Error(HccErrorManager::errMultipleConstructorOverloads);
	else{
		//parse the constructor...
		ctor_ptr = class_ptr->getTypeSpecifier().getSymbolTable()->insert(class_ptr->getName());
		assert(ctor_ptr!=NULL);
		if(ctor_ptr!=NULL)
		{
			ctor_ptr->getDeclDefinition().set_identifier_type(DECL_NEW_CLASS_CONSTRUCTOR);
			ctor_ptr->setOwner(class_ptr);

			DeclDefinition* ctor_decl_ptr = &ctor_ptr->getDeclDefinition();
			//the return type...
			ctor_decl_ptr->function.return_type = HccTypeChecker::ts_type_void_ptr;

			assert(ctor_decl_ptr->function.symbol_table_ptr==0);
			ctor_decl_ptr->function.symbol_table_ptr = new SymbolTable<__tstring, Symbol>(_T("__constructor__"));
		}
	}
	Resync(HCC_LPAREN);			
	parseFunctionParamList(ctor_ptr);	//(ctor-param-list param,...)

	Resync(HCC_RPAREN);
	getTokenIf(HCC_RPAREN, HccErrorManager::errMissingRightParen);

	//U S E R   C O D E 
	//use a new icode for every function...
	icode_ptr = theICodeResponsible.getFunctionICode(ctor_ptr);
	assert(icode_ptr!=NULL);
	icode_ptr->set_source_buffer(lexer->sourceBuf());	

	icode_ptr->append(token_type); //{
	//the function body {}...
	parseCompoundStatement(ctor_ptr);	//{ statement-list }	
	//
	//restore to the global icode object...	
	icode_ptr = global_icode_ptr;
	assert(icode_ptr!=NULL);
}

void HCCParser::parseClassDestructor(Symbol *class_ptr)
{
	//can have only one destructor for class
	Symbol* destructor = class_ptr->getTypeSpecifier().getSymbolTable()->find(token_ptr->String());
	if(destructor!=NULL)
		HccErrorManager::Error(HccErrorManager::errCannotHaveMultipleDestructors);
	else{
		destructor = class_ptr->getTypeSpecifier().getSymbolTable()->insert(token_ptr->String());
		assert(destructor!=NULL);
		if(destructor!=NULL)
		{
			destructor->getDeclDefinition().set_identifier_type(DECL_UNIQUE_DESTRUCTOR);
			destructor->setOwner(class_ptr);

			DeclDefinition* dest_decl_ptr = &destructor->getDeclDefinition();
			//the return type...
			dest_decl_ptr->function.return_type = HccTypeChecker::ts_type_void_ptr;

			assert(dest_decl_ptr->function.symbol_table_ptr==0);

			destructor->getDeclDefinition().function.symbol_table_ptr = 
												new SymbolTable<__tstring, Symbol>(_T("__destructor__"));

			//the return type...
			destructor->getDeclDefinition().function.return_type = HccTypeChecker::ts_type_void_ptr;
		}
	}
	getToken(); //the word destructor...
	Resync(HCC_LPAREN);
	getTokenIf(HCC_LPAREN, HccErrorManager::errMissingLeftParen);
	Resync(HCC_RPAREN);
	getTokenIf(HCC_RPAREN, HccErrorManager::errMissingRightParen);

	//U S E R   C O D E 
	//use a new icode for every function...
	icode_ptr = theICodeResponsible.getFunctionICode(destructor);
	assert(icode_ptr!=NULL);
	icode_ptr->set_source_buffer(lexer->sourceBuf());	

	icode_ptr->append(token_type); //{
	//the function body {}...
	parseCompoundStatement(destructor);	//{ statement-list }	
	//
	//restore to the global icode object...	
	icode_ptr = global_icode_ptr;
	assert(icode_ptr!=NULL);

}


void HCCParser::parseClassProperty(TypeSpecifier* property_type_ptr, Symbol *class_ptr, bool bIsTypePointer)
{
/*
	DECL_READONLY_PROPERTY,			//a read-only property
	DECL_WRITEONLY_PROPERTY,		//a write-only property
	DECL_READWRITE_PROPERTY,		//a read-write property


	public int get:Width()   //DECL_READONLY_PROPERTY
	{
	   return width;
	}
 
	public void put:Width(int value)  //DECL_WRITEONLY_PROPERTY
	{
		width = value;
	}
 

	both get | put == DECL_READWRITE_PROPERTY == (READONLY_PROPERTY | WRITEONLY_PROPERTY)
*/	
	assert(class_ptr !=NULL && class_ptr->getTypeSpecifier().getSymbolTable());
	SymbolTable<__tstring, Symbol>* class_symbl_tbl_ptr = class_ptr->getTypeSpecifier().getSymbolTable();
	if(class_symbl_tbl_ptr==NULL)
		return;

	if(token_type==HCC_GET)
	{
		getToken(); //get
		//
		Resync(HCC_COLON);
		getTokenIf(HCC_COLON, HccErrorManager::errMissingColon);
		//
		Resync(HCC_IDENTIFIER);
		if(token_type==HCC_IDENTIFIER)
		{
			Symbol* get_Property_ptr = NULL;
			volatile DECLARATION_TYPE type = DECL_READONLY_PROPERTY;
			Symbol* property_ptr = class_symbl_tbl_ptr->find(token_ptr->String());
			if(property_ptr!=NULL)
			{
				__tstring info = _T(": \'");
				info += token_ptr->String();
				info += _T("\'.");

				 type = property_ptr->getDeclDefinition().identifier_type();

				if(type==DECL_READONLY_PROPERTY || type==DECL_READWRITE_PROPERTY)
				{
					//property redefinition...
					HccErrorManager::Error(HccErrorManager::errPropertyRedefinition, info);
					goto PROPERTY_GET_IMPL;
				}else if(type!=DECL_WRITEONLY_PROPERTY)
				{
					//error : if something was defined using this name, we cannot define a property with it.
					HccErrorManager::Error(HccErrorManager::errUsedIdentifierInPropertyDefinition, info);
				}
			}else{
				//T H E   F R O N T   P R O P E R T Y 
				property_ptr = class_symbl_tbl_ptr->insert(token_ptr->String());
				property_ptr->getDeclDefinition().set_identifier_type(DECL_READONLY_PROPERTY);
				//the return type is defined here...
				property_ptr->getDeclDefinition().function.return_type = property_type_ptr;
				//every function/property must have a symbol table...
				property_ptr->getDeclDefinition().function.symbol_table_ptr = NULL;																	
				//set the property type...
				property_ptr->getTypeSpecifier().setBaseTypeSpec(property_type_ptr);
				//is pointer the resultant type for get accessor?
				property_ptr->getDeclDefinition().function.bReturnTypeIsPointer = bIsTypePointer;
			}
			//
PROPERTY_GET_IMPL:
			{
				get_Property_ptr = property_ptr->getDeclDefinition().function.property[_T("get")];
				//define this property as read-write property...
				if(type==DECL_WRITEONLY_PROPERTY)
					property_ptr->getDeclDefinition().set_identifier_type(DECL_READWRITE_PROPERTY);

				if(get_Property_ptr==NULL)
				{
					//T H E   R E A L   P R O P E R T Y   G E T 
					__tstring prop_get = _T("get::");
					prop_get += property_ptr->getName();
					//insert it in the same class's symbol table...
					get_Property_ptr = class_symbl_tbl_ptr->insert(prop_get);
					assert(get_Property_ptr!=NULL);
					//
					if(get_Property_ptr!=NULL)
					{
						get_Property_ptr->getDeclDefinition().set_identifier_type(DECL_READONLY_PROPERTY);
						//the return type is defined here...
						get_Property_ptr->getDeclDefinition().function.return_type = property_type_ptr;
						//is this accessor a pointer
						get_Property_ptr->getDeclDefinition().function.bReturnTypeIsPointer = bIsTypePointer;
						//every function/property must have a symbol table...
						get_Property_ptr->getDeclDefinition().function.symbol_table_ptr = 
																	new SymbolTable<__tstring, Symbol>(_T("__property_get__"));
						//set the property type...
						get_Property_ptr->getTypeSpecifier().setBaseTypeSpec(property_type_ptr);					

						//set the owner...
						get_Property_ptr->setOwner(class_ptr);
					}
					//register the real property, in the front property's map...
					property_ptr->getDeclDefinition().function.property[_T("get")] = get_Property_ptr;
				}

				getToken(); //identifier

				Resync(HCC_LPAREN);	//(
				//
				getTokenIf(HCC_LPAREN, HccErrorManager::errMissingLeftParen);
				//
				Resync(HCC_RPAREN);	//)
				//
				getTokenIf(HCC_RPAREN, HccErrorManager::errMissingRightParen);

				Resync(HCC_LBLOCK_KEY); //{

				assert(get_Property_ptr!=NULL);
				//use a new icode for every function...
				icode_ptr = theICodeResponsible.getFunctionICode(get_Property_ptr);
				assert(icode_ptr!=NULL);
				icode_ptr->set_source_buffer(lexer->sourceBuf());	

				icode_ptr->append(token_type); //{
				//the function body {}...
				getTokenAppend(); // {
				HCC_TOKEN_TYPE last_stmt_here = ParseStatementList(HCC_RBLOCK_KEY, get_Property_ptr); //{ statement-list }
				//
				if(last_stmt_here!=HCC_RETURN)
					HccErrorManager::Error(HccErrorManager::errFunctionMustReturnValue);

				icode_ptr->append(token_type); //}

				//restore to the global icode object...				
				icode_ptr = global_icode_ptr;
				assert(icode_ptr!=NULL);
				//
				if(getTokenAppendIf(HCC_RBLOCK_KEY, HccErrorManager::errMissingEndBlock)==0){ //}
					Resync(HCC_RBLOCK_KEY);
					return;
				}
			}
		}

	}else if(token_type==HCC_PUT)
	{
		getToken(); //put
		//
		Resync(HCC_COLON);
		getTokenIf(HCC_COLON, HccErrorManager::errMissingColon);
		//
		Resync(HCC_IDENTIFIER);
		if(token_type==HCC_IDENTIFIER)
		{
			Symbol* put_Property_ptr = NULL;
			volatile bool bPropPutAlready = false;
			volatile DECLARATION_TYPE type = DECL_WRITEONLY_PROPERTY;
			Symbol* property_ptr = class_symbl_tbl_ptr->find(token_ptr->String());
			if(property_ptr!=NULL)
			{
				__tstring info = _T(": \'");
				info += token_ptr->String();
				info += _T("\'.");

				type = property_ptr->getDeclDefinition().identifier_type();

				if(type==DECL_WRITEONLY_PROPERTY || type==DECL_READWRITE_PROPERTY)
				{
					bPropPutAlready = true;
					//property redefinition...
					HccErrorManager::Error(HccErrorManager::errPropertyRedefinition, info);
					goto PROPERTY_PUT_IMPL;
				}else if(type!=DECL_READONLY_PROPERTY)
				{
					//error : if something was defined using this name, we cannot define a property with it.
					HccErrorManager::Error(HccErrorManager::errUsedIdentifierInPropertyDefinition, info);
				}
			}else{
				property_ptr = class_symbl_tbl_ptr->insert(token_ptr->String());
				property_ptr->getDeclDefinition().set_identifier_type(DECL_WRITEONLY_PROPERTY);
				//the return type is defined here...
				property_ptr->getDeclDefinition().function.return_type = property_type_ptr;
				//every function/property must have a symbol table...
				property_ptr->getDeclDefinition().function.symbol_table_ptr = NULL;
				//set the property type...
				property_ptr->getTypeSpecifier().setBaseTypeSpec(property_type_ptr);
				//is pointer the resultant type for get accessor?
				property_ptr->getDeclDefinition().function.bReturnTypeIsPointer = bIsTypePointer;
			}
			//
PROPERTY_PUT_IMPL:
			{
				put_Property_ptr = property_ptr->getDeclDefinition().function.property[_T("put")];
				//a warning that flags that property puts should not return a value...
				if(property_type_ptr!=HccTypeChecker::ts_type_void_ptr)
					HccWarningManager::Warning(HccWarningManager::warnReturningValueFromPropertyPut, 
												_T("Use \'void\' as the return type."));

				//define this property as read-write property...
				if(type==DECL_READONLY_PROPERTY)
					property_ptr->getDeclDefinition().set_identifier_type(DECL_READWRITE_PROPERTY);

				if(put_Property_ptr==NULL)
				{
					//T H E   R E A L   P R O P E R T Y   G E T 
					__tstring prop_put = _T("put::");
					prop_put += property_ptr->getName();
					//insert it in the same class's symbol table...
					put_Property_ptr = class_symbl_tbl_ptr->insert(prop_put);
					assert(put_Property_ptr!=NULL);
					//
					if(put_Property_ptr!=NULL)
					{
						put_Property_ptr->getDeclDefinition().set_identifier_type(DECL_WRITEONLY_PROPERTY);
						//the return type is defined here...
						put_Property_ptr->getDeclDefinition().function.return_type = property_type_ptr;
						//is pointer the resultant type for get accessor?
						put_Property_ptr->getDeclDefinition().function.bReturnTypeIsPointer = bIsTypePointer;
						//every function/property must have a symbol table...
						put_Property_ptr->getDeclDefinition().function.symbol_table_ptr = 
																	new SymbolTable<__tstring, Symbol>(_T("__property_put__"));
						//set the property type...
						put_Property_ptr->getTypeSpecifier().setBaseTypeSpec(property_type_ptr);					

						//set the onwer...
						put_Property_ptr->setOwner(class_ptr);
					}
					//register the real property, in the front property's map...
					property_ptr->getDeclDefinition().function.property[_T("put")] = put_Property_ptr;
				}

				getToken(); //identifier

				assert(put_Property_ptr!=NULL);


				Resync(HCC_LPAREN);	//(
				//parse: param-list
				if(false==bPropPutAlready){
					parseFunctionParamList(put_Property_ptr);

					Symbol* param_ptr = 
								put_Property_ptr->getDeclDefinition().function.locals.stack_params.params[0];
					TypeSpecifier* param_type_ptr = param_ptr->getTypeSpecifier().getBaseTypeSpec();
					//the user type...				;
					property_ptr->getTypeSpecifier().setBaseTypeSpec(param_type_ptr);
				}
				else //skip the param definition...
					while(token_type!=HCC_RPAREN && token_type!=HCC_EOF)
						getToken();
				
				Resync(HCC_RPAREN); //)
				getTokenIf(HCC_RPAREN, HccErrorManager::errMissingRightParen);

				Resync(HCC_LBLOCK_KEY); //{

				//use a new icode for every function...
				icode_ptr = theICodeResponsible.getFunctionICode(put_Property_ptr);
				assert(icode_ptr!=NULL);
				icode_ptr->set_source_buffer(lexer->sourceBuf());	

				icode_ptr->append(token_type); //{
				//the function body {}...
				getTokenAppend(); // {
				HCC_TOKEN_TYPE last_stmt_here = ParseStatementList(HCC_RBLOCK_KEY, put_Property_ptr); //{ statement-list }
				//
				if(last_stmt_here!=HCC_RETURN && property_type_ptr!=HccTypeChecker::ts_type_void_ptr)
					HccErrorManager::Error(HccErrorManager::errFunctionMustReturnValue);

				icode_ptr->append(token_type); //}

				//restore to the global icode object...				
				icode_ptr = global_icode_ptr;
				assert(icode_ptr!=NULL);
				//
				if(getTokenAppendIf(HCC_RBLOCK_KEY, HccErrorManager::errMissingEndBlock)==0){ //}
					Resync(HCC_RBLOCK_KEY);
					return;
				}
			}
		}
	}else
		assert(0);
}

extern bool bShowListing;

//---------------------------------------------------------
//	parseImportStatement()	- parses the import statements 
//							  found in a hpp file.
//
//---------------------------------------------------------
// import samples:
//
//
//import "C:\HCC\HCC\TestSource\stdhpp\stdapi.hcc";
//
//import "lib\System.Testing.Test5";
//
//import "c:\Test\System.Test1";		//found in c:\Test\
//
//import "lib\System.Testing.Test2";	//found in Release\lib\
//
//import System.Testing.Test3;		//found in Release\
//
//import Test4;				//found in Release\
//
//import "TestSource\stdhpp\stdapi.hcc";	//C:\HCC\HCC\TestSource\stdhpp\
//
//---------------------------------------------------------

extern set<string> isearch_path;

__tstring toLower(const __tstring& src)
{
	__tstring result;
	__tstring::const_iterator it = src.begin();
	while(it!=src.end())
	{
		TCHAR chr = *it++;
		if((_istalpha(chr) && _istlower(chr)) || _istpunct(chr))
			result += chr;
		else
			result += _tolower(chr);
	}
	return result;
}

void HCCParser::parseImportStatement()
{
	__tstring imported_unit;

	lexer->m_bSilentEscapedString = true; //to avoid the lexer complaining with the escaped sequences

	HCC_TOKEN_TYPE tokens[] = {
						HCC_PERIOD,
						HCC_SEMICOLON,
	};
	Resync(HCC_IMPORT);
	if(getTokenIf(HCC_IMPORT, HccErrorManager::errUnexpectedToken)!=0)
	{
		__tstring final_path;
		if(token_type==HCC_STRING_LITERAL)
		{
			const TCHAR* lpszUnit = token_ptr->String();
			int len = _tcslen(lpszUnit);
			copy(lpszUnit + 1, lpszUnit + len - 1, inserter(imported_unit, imported_unit.begin()));
			getToken();
		}
		else{
			while(token_type==HCC_IDENTIFIER && token_type!=HCC_EOF)
			{
				imported_unit += token_ptr->String();
				getTokenIf(HCC_IDENTIFIER, HccErrorManager::errMissingIdentifier);
				if(token_type==HCC_SEMICOLON)
					break;
				if(token_type==HCC_IDENTIFIER)
				{
					__tstring info = _T(" before identifier: \'");
					info += token_ptr->String();
					info += _T("\'.");
					HccErrorManager::Error(HccErrorManager::errMissingPeriod, info);
				}else if(token_type==HCC_PERIOD){
					getToken();
					imported_unit += _T(".");
				}else{
					__tstring info = _T(" expected \'.\', found \'");
					info += token_ptr->String();
					info += _T("\'.");
					HccErrorManager::Error(HccErrorManager::errMissingPeriod, info);
					getToken();
					break;
				}
			}
		}
		//
#define MAX_IMPORT_FILE_PATH (MAX_PATH * 4)

		TCHAR* lpszFileName = NULL;		

		TCHAR lpszCompleteFilePath[MAX_IMPORT_FILE_PATH];
		::GetFullPathName(imported_unit.c_str(), MAX_IMPORT_FILE_PATH, lpszCompleteFilePath, &lpszFileName);

		
		TCHAR lpszFilePath[MAX_IMPORT_FILE_PATH];
		/*
DWORD SearchPath(
  LPCTSTR lpPath,		//[in] Pointer to a null-terminated string that specifies the path to be searched for the file
  LPCTSTR lpFileName,	//[in] Pointer to a null-terminated string that specifies the name of the file to search for. 
  LPCTSTR lpExtension,	//[in] Pointer to a null-terminated string that specifies an extension to be added to the file name when searching for the file. 
						//	   The first character of the file name extension must be a period (.). 
						//	   The extension is added only if the specified file name does not end with an extension.
  DWORD nBufferLength,	//[in] Size of the buffer that receives the valid path and file name, in TCHARs.
  LPTSTR lpBuffer,		//[out] Pointer to the buffer that receives the path and file name of the file found. 
  LPTSTR* lpFilePart	//[out] Pointer to the variable that receives the address (within lpBuffer) of the last component of 
						//	    the valid path and file name, which is the address of the character immediately following the final backslash (\) in the path. 
);
		*/
		DWORD dwLength = ::SearchPath(NULL, 
									  lpszCompleteFilePath, 
									  _T(".hpp"), 
									  MAX_IMPORT_FILE_PATH, 
									  lpszFilePath, 
									  &lpszFileName);

		if(dwLength==0)
		{	
			//BEGIN - ADDED - Jan 21, 2009
			set<string>::iterator it_search_path = isearch_path.begin();
			while(it_search_path != isearch_path.end())
			{
				string& the_search_path = (string&)*it_search_path;

				TCHAR lpszSearchPath[MAX_IMPORT_FILE_PATH];

#ifdef _UNICODE
				int nConvertedChars = 0;
				mbstowcs_s(&nConvertedChars, 
						   lpszSearchPath, 
						   the_search_path.c_str(), 
						   the_search_path.length());
#else
				//
				_tcsncpy(lpszSearchPath, the_search_path.c_str(), the_search_path.length());
				lpszSearchPath[the_search_path.length()] = _T('\0');
#endif //_UNICODE

				dwLength = ::SearchPath(lpszSearchPath, 
										imported_unit.c_str(), 
										_T(".hpp"), 
										MAX_IMPORT_FILE_PATH, 
										lpszFilePath, 
										&lpszFileName);

				if(dwLength > 0){
					lpszFilePath[dwLength] = _T('\0');
					imported_unit = lpszFilePath;
					break;
				}

				it_search_path++; //next user-specified search path...
			}
			//END - ADDED - Jan 21, 2009
			if(it_search_path == isearch_path.end())
			{
				cout << _T("{") 
					 << lexer->sourceBuf()->lineNumber() 
					 << _T("} ") 
					 << imported_unit << endl;
				HccErrorManager::AbortTranslation(HccErrorManager::abortSourceFileOpenFailed);
			}
		}else{
			lpszFilePath[dwLength] = _T('\0');
			imported_unit = lpszFilePath;
		}

		lexer->m_bSilentEscapedString = false; //to raise the lexer constrainst on escaped sequences

		imported_unit = toLower(imported_unit); //ADDED - Jan 21, 2009
		if(imported_transl_units.find(imported_unit)==imported_transl_units.end())
		{
			//before doing the parsing in this file, set as already imported to avoid
			//circular dependency and infinite unit import...
			imported_transl_units.insert(imported_unit);
			//		
			cout << _T("importing: ");
			source_buffer* unit = new source_buffer(imported_unit.c_str(), bShowListing);
			HCCParser parser(unit);
			//parse this compilation unit...
			parser.Parse();	
			//restore the original source buffer before continue...
			symbol_table_ptr->set_source_buffer(lexer->sourceBuf());
			//restore to the correct source file for this H++ parser instance...
			HccErrorManager::SetSourceFilePtr(lexer->sourceBuf());
			HccWarningManager::SetSourceFilePtr(lexer->sourceBuf());

			delete unit;

			if(token_type!=HCC_IMPORT)
			{
#ifdef _UNICODE
				int nConvertedChars = 0;
				mbstowcs_s(&nConvertedChars, 
						   lpszFilePath, 
						   lexer->sourceBuf()->sourceFile(), 
						   MAX_IMPORT_FILE_PATH);
#else
				_tcsncpy(lpszFilePath, lexer->sourceBuf()->sourceFile(), MAX_IMPORT_FILE_PATH);
				//
#endif //_UNICODE
				::GetFullPathName(lpszFilePath, //(const TCHAR*)_bstr_t(lexer->sourceBuf()->sourceFile()), 
									MAX_IMPORT_FILE_PATH, 
									lpszCompleteFilePath, 
									&lpszFileName);
				cout << _T(">") << lpszFileName << _T(":") << endl;
			}
		}
	}
	//
	lexer->m_bSilentEscapedString = false; //to raise the lexer constrainst on escaped sequences
}

bool HCCParser::IsAbstractClass(Symbol *class_ptr)
{
	assert(class_ptr!=NULL);
	return (class_ptr!=NULL) && (class_ptr->getDeclDefinition().identifier_type()==DECL_NEW_ABSTRACT_TYPE);
}

void HCCParser::analyzeClassVirtualState(Symbol *class_ptr)
{
	if(class_ptr!=NULL && class_ptr->getDeclDefinition().identifier_type()==DECL_NEW_TYPE)
	{
		//if this type is a class/struct, and this class has at least one virtual/virtual abstract function,
		//process its hierarchy to determine if this type will keep itself as an abstract class,
		//or the programmer/user can create instances from it.

		//1. create the stack of types
		stack<TypeSpecifier*> types_stack, class_stack;

		TypeSpecifier* type_ptr = &class_ptr->getTypeSpecifier();
		types_stack.push(type_ptr);

		while(type_ptr!=type_ptr->getBaseTypeSpec())
		{
			type_ptr = type_ptr->getBaseTypeSpec();
			types_stack.push(type_ptr);
		}

		//2. create a map to a boolean, with all virtual abstract functions symbols
		map<__tstring, bool> abstract_members;
		while(!types_stack.empty())
		{
			type_ptr = types_stack.top();
			types_stack.pop();
			//
			assert(type_ptr!=NULL);
			SymbolTable<__tstring, Symbol> *class_symbtbl_ptr = type_ptr->getSymbolTable();
			assert(class_symbtbl_ptr!=NULL);
			if(class_symbtbl_ptr!=NULL)
			{
				Symbol* vptr_ptr = class_symbtbl_ptr->find(CLASS_VPTR_VTBL_NAME);

				//if this type, has a symbol for the vptr, then we process it
				if(vptr_ptr!=NULL)
				{
					TypeDataMembers& vtbl_members = vptr_ptr->getTypeSpecifier().user_type_members.function_members;
					TypeDataMembers::iterator it = vtbl_members.begin();

					//3. insert in the map, all the virtual abstract members this class has...
					while(it != vtbl_members.end())
					{
						Symbol* virtual_fn_ptr = *it++;
						assert(virtual_fn_ptr!=NULL);
						DECLARATION_TYPE decl_type = virtual_fn_ptr->getDeclDefinition().identifier_type();
						if(decl_type==DECL_NEW_VIRTUAL_ABSTRACT_FUNC_MEMBER)
							abstract_members.insert(
												pair<__tstring, bool>(virtual_fn_ptr->getName(), true)
												);
					}
				}
			}
			//save it in another stack, to later walk up in the class hierarchy, to flag
			//every virtual member different from abstract...
			class_stack.push(type_ptr);
		}

		//4. Determine which classes in the hierarchy, implements the abstract members
		while(!class_stack.empty())
		{
			type_ptr = class_stack.top();
			class_stack.pop();
			//this time, use the function_members member to iterate all the function members of every class,			
			TypeDataMembers& fn_members = type_ptr->user_type_members.function_members;
			TypeDataMembers::iterator it_fn = fn_members.begin();
			//
			while(it_fn != fn_members.end())
			{
				Symbol* fn_ptr = *it_fn++;				
				DECLARATION_TYPE decl_member = fn_ptr->getDeclDefinition().identifier_type();
				if(decl_member!=DECL_NEW_VIRTUAL_ABSTRACT_FUNC_MEMBER)
				{
					map<__tstring, bool>::iterator in_map_it = abstract_members.find(fn_ptr->getName());
					//if found in map, set it to false to flag we found an implementation for that abstract member function
					if(in_map_it != abstract_members.end())
						in_map_it->second = false;
				}
			}
		}
		//5. if no more virtual abstract members exists in the map, this is an instantiable class.
		volatile bool bIsAbstract = false;
		map<__tstring, bool>::iterator it_abstract = abstract_members.begin();
		while(it_abstract != abstract_members.end())
		{
			if((bIsAbstract = (it_abstract->second)))
				break;
			it_abstract++;
		}
		if(bIsAbstract)
			class_ptr->getDeclDefinition().set_identifier_type(DECL_NEW_ABSTRACT_TYPE);
	}
}

void HCCParser::parsePointerVariableOrDataMember(Symbol *variable_ptr, 
												TypeSpecifier* base_type_ptr, 
												Symbol *fn_ptr, 
												Symbol *class_ptr,
												bool bAlreadyInICode)
{
	/*
	pointer-decl :: type-spec ^ identifier
 
	  new-decl     :: pointer-decl =  new  type-spec					|
						   pointer-decl =  new  type-spec(param-list)	|
						   pointer-decl =  new  type-spec[int|const]	|
						   pointer-decl =  null

 
	  destructor-decl :: destroy identifier  (calls System::Destroy::Pointer(addr, sizeof(type-spec)) ) |
							   destroy identifier  (calls System::Destroy::Array(addr, sizeof(array) * sizeof(type-spec)) ) |
							   destroy null (does nothing when a pointer is null)

	  dynamic_cast-decl :: pointer-decl = dynamic_cast(type-spec, pointer) |
						   pointer-decl = dynamic_cast(pointer) |
						   dynamic_cast(type-spec, pointer)
	*/
	if(token_type==HCC_ASSIGN_OP)
	{
		//------------------------------------------------------------------------
		if(class_ptr!=NULL && fn_ptr==NULL)
		{
			//use a new icode for every class...
			icode_ptr = theICodeResponsible.getClassICode(&class_ptr->getTypeSpecifier());
			assert(icode_ptr!=NULL);
		}
		if(false==bAlreadyInICode)		
		{
			//---------------------------------------------------------------------------
			appendSymbolToICode(HCC_IDENTIFIER, variable_ptr);
			//----------------------------------------------------------
		}
		getTokenAppend();	// '='
		{				
			if(token_type==HCC_NULL)
			{
				getTokenAppend(); // null
				variable_ptr->getDeclDefinition().variable.init_value.Integer = 0;
				Symbol* null_ptr = find_symbol(_T("null"));
				assert(!!null_ptr);
				icode_ptr->put_back();	//take off the semicolon;
				icode_ptr->append(null_ptr);
				icode_ptr->append(HCC_SEMICOLON);
				//
			}else if(token_type==HCC_NEW){
				//BEGIN - ADDED Jan 3, 2008
				TypeSpecifier* final_type_ptr = parseNewInstance(variable_ptr, base_type_ptr, class_ptr, fn_ptr);
				if(final_type_ptr!=base_type_ptr)
				{
					//used when upcasting/using dynamic_cast operator...
					//variable_ptr->getTypeSpecifier().setClassFinalType(final_type_ptr);
				}
				//END - ADDED Jan 3, 2008
			}else if(token_type==HCC_DYNAMIC_CAST){
				//BEGIN - ADDED Jan 4, 2008
				TypeSpecifier* final_type_ptr = parseDynamicCastOperator(base_type_ptr, class_ptr, fn_ptr);
				//END - ADDED Jan 4, 2008
			}else{
				if(variable_ptr->getTypeSpecifier().specifier()==DSPEC_ARRAY &&
					false==IsDynamicArray(&variable_ptr->getTypeSpecifier()))
				{
					HccErrorManager::Error(HccErrorManager::errCannotAssignToFullyDefinedArray, variable_ptr->getCompleteName());
				}

				TypeSpecifier* type_ptr = variable_ptr->getTypeSpecifier().getBaseTypeSpec(); 
				HCC_PARSE_TREE* tree_ptr = ParseExprList(fn_ptr);
				if(tree_ptr!=NULL)
				{
					if(tree_ptr->type_ptr!=NULL && type_ptr!=NULL)
						HccTypeChecker::CheckCompatibleAssignment(type_ptr,
																  tree_ptr->type_ptr,
																  HccErrorManager::errInvalidPointerExpression,
																  _T("; pointer assignment only support integer values for memory addresses."));
				}
				delete tree_ptr;
			}
		}
		//----------------------------------------------------------
		if(class_ptr!=NULL && fn_ptr==NULL)
		{
			//A D D   T O   T H E   C L A S S ' S   D A T A   M E M B E R S  G R O U P 
			class_ptr->getTypeSpecifier().user_type_members.data_members.push_back(variable_ptr);
			//restore to the global icode object...	
			icode_ptr = global_icode_ptr;
			assert(icode_ptr!=NULL);
		}
	}	
}

void HCCParser::parseArrayPointerVariableOrDataMember(Symbol *var_array_ptr, 
													  TypeSpecifier* base_type_ptr, 
													  short nDimensions,
													  Symbol *fn_ptr, 
													  Symbol *class_ptr, 
													  bool bAlreadyInICode)
{
	/*
	array-decl		::  type-spec [] identifier
 
	subscript-list	:: [integer-number]	subscript-list	| 
							subscript-list				| e
 
						array-decl  =  'new' type-spec subscript-list | 
						array-decl	=  'null'

	i.e.: 
	     type-spec [] identifier1 = new type-spec [x]; //this is the only declaration supported by H++

		 type-spec [][] identifier2 = new type-spec [x][y]; //not supported!

		identifer[2] = value;
 
	  destructor-decl :: destroy identifier  (calls System::Memory::Destroy::Pointer(addr, sizeof(type-spec)) ) |
							   destroy identifier  (calls System::Memory::Destroy::Array(addr, sizeof(array) * sizeof(type-spec)) ) |
							   destroy null (does nothing when a pointer is null)
	*/

	//BEGIN - ADDED Jan 23, 2009
	if(nDimensions != 1)
	{
		HccErrorManager::Error(HccErrorManager::errInvalidDynamicArrayDimension, 
							  _T(", dynamic arrays can have only one dimension, like in: int [] array = new int[n];"));
	}
	//END - ADDED Jan 23, 2009
	//
	if(token_type==HCC_ASSIGN_OP)
	{
		//------------------------------------------------------------------------
		if(class_ptr!=NULL && fn_ptr==NULL)
		{
			//use a new icode for every class...
			icode_ptr = theICodeResponsible.getClassICode(&class_ptr->getTypeSpecifier());
			assert(icode_ptr!=NULL);
		}			
		if(false==bAlreadyInICode)
		{
			//to avoid assigning to a completely defined array
			var_array_ptr->getTypeSpecifier().array.bIsDynamicArray = true;
			//---------------------------------------------------------------------------
			appendSymbolToICode(HCC_IDENTIFIER, var_array_ptr);
			//---------------------------------------------------------------------------
			//
		}
		getTokenAppend();	// '='
					
		if(token_type==HCC_NULL)
		{
			getTokenAppend(); // null
			var_array_ptr->getDeclDefinition().variable.init_value.Integer = 0;				
			Symbol* null_ptr = find_symbol(_T("null"));
			assert(!!null_ptr);
			icode_ptr->put_back();	//take off the semicolon;
			icode_ptr->append(null_ptr);
			icode_ptr->append(HCC_SEMICOLON);

			//only if this symbol was not defined as an array first, we transform it into an array
			if(var_array_ptr->getTypeSpecifier().specifier()!=DSPEC_ARRAY)
			{
				TypeSpecifier* id_type_spec_ptr = &var_array_ptr->getTypeSpecifier();
				while(true)
				{
					TypeSpecifier* arraySpec = id_type_spec_ptr;
					//the element type-spec
					TypeSpecifier* base_type_spec		= arraySpec->getBaseTypeSpec(),
								  *itemTypeSpec			= arraySpec->getBaseTypeSpec();

					//mark it as an array!
					arraySpec->set_specifier(DSPEC_ARRAY);
					//set as itself!
					arraySpec->setBaseTypeSpec(arraySpec);
					//array of type itemTypeSpec
					arraySpec->array.pItemType = itemTypeSpec;
					arraySpec->array.item_count = 0;

					if(--nDimensions > 0)
					{
						//when this array has more than one dimension,
						//a new type specifier of type DSPEC_ARRAY must be created
						//and set as the item type for the current array
						//avoiding Stack Overflow when calling getArraySizeFromType();
						//
						//the base type must be the type-specifier found for this declaration...
						itemTypeSpec = new TypeSpecifier(DSPEC_ARRAY);
						arraySpec->array.pItemType = itemTypeSpec;
						arraySpec->array.item_count = 0;
						//set the previous type
						itemTypeSpec->setBaseTypeSpec(base_type_spec);

						id_type_spec_ptr = itemTypeSpec;
						continue;
					}
					break;
				}
			}

			//short test_stack_overflow = getArrayDimesionsFromType(&var_array_ptr->getTypeSpecifier());

			//
		}else if(token_type==HCC_NEW)
		{
			//
			getTokenAppend(); // operator new 
			bool bIsAbstractType = false;
			TypeSpecifier* type_ptr = parseTypeSpec(class_ptr, fn_ptr, &bIsAbstractType);
			if(type_ptr==NULL || type_ptr!=base_type_ptr)
			{
				HccTypeChecker::CheckCompatibleAssignment(base_type_ptr, type_ptr,
														  HccErrorManager::errInvalidAssignment, 
														  _T("(arrays must have the same type for a correct definition)."));					
			}
			//add the symbol's type, only if is scalar type...
			if(type_ptr->is_scalar())
			{
				Symbol* scalar_ptr = find_symbol(type_ptr->getTypeName().c_str());
				assert(!!scalar_ptr);
				if(scalar_ptr!=NULL)
				{
					icode_ptr->append(HCC_IDENTIFIER);
					icode_ptr->append(scalar_ptr);
				}					
			}else{
				//BEGIN - FIXED Jan 7, 2009
				//this will be needed by the code generator...
				appendSymbolToICode(HCC_TOKEN_ERROR, var_array_ptr);
				//END - FIXED Jan 7, 2009
			}

			if(bIsAbstractType)
			{
				__tstring info = _T(" ");
				if(type_ptr!=NULL)
				{
					info = _T(", cannot instantiate abstract class \'");
					info += type_ptr->getTypeName();info += _T("\'.");
				}
				HccErrorManager::Error(HccErrorManager::errAbstractClassInstantiation, info);
			}

			//[
			if(token_type==HCC_LBRACKET)
			{
				long nNewDeclDimensions = 0;
				if(var_array_ptr->getTypeSpecifier().specifier()!=DSPEC_ARRAY)
				{
					//BEGIN - ADDED Jan 10, 2009
					if(type_ptr->is_scalar())
						icode_ptr->append(HCC_LBRACKET);
					//END - ADDED Jan 10, 2009

					//BEGIN - FIXED Jan 21, 2009
					parseDynamicallyAllocatedArrayType(&var_array_ptr->getTypeSpecifier(), fn_ptr, type_ptr);
					//END - FIXED Jan 21, 2009
					//
					nNewDeclDimensions = nDimensions;
				}else{
					//BEGIN - FIXED Feb 1, 2009
					TypeSpecifier* type_before_ptr = getArrayScalarType(&var_array_ptr->getTypeSpecifier());

					nNewDeclDimensions = getArrayDimesionsFromType(&var_array_ptr->getTypeSpecifier());
					
					if(type_ptr->is_scalar())
						icode_ptr->append(HCC_LBRACKET);

					parseDynamicallyAllocatedArrayType(&var_array_ptr->getTypeSpecifier(), fn_ptr, type_ptr);

					TypeSpecifier* type_after_ptr = getArrayScalarType(&var_array_ptr->getTypeSpecifier());

					if(
						(type_before_ptr->getDataType()==HCC_CUSTOM_TYPE && 
						 type_after_ptr->getDataType()==HCC_CUSTOM_TYPE)
						||
						(type_before_ptr->getDataTypeSize()!=type_after_ptr->getDataTypeSize())
					  )
					{
						if(type_before_ptr!=type_after_ptr)
						{
							__tstring info = _T(", expected array type: \'");
							info += type_before_ptr->getTypeName();
							info += _T("\', found: \'");
							info += type_after_ptr->getTypeName();
							info += _T("\'.");
							HccErrorManager::Error(HccErrorManager::errInvalidExpression, info);
						}
					}
					//END - FIXED Feb 1, 2009
				}
				//to avoid AV when destroying the symbol...
				TypeSpecifier* scalar_ptr = getArrayScalarType(&var_array_ptr->getTypeSpecifier());
				if(scalar_ptr==HccTypeChecker::ts_type_char_ptr)
					var_array_ptr->getDeclDefinition().constant.value.String = 0;

				//keep track of the array's size
				long nDynamicDimensions = getArrayDimesionsFromType(&var_array_ptr->getTypeSpecifier());
				if(nDynamicDimensions != nNewDeclDimensions)
				{
					TCHAR szInfo[100];
					_stprintf(szInfo, _T("; expected dimension: %d, actually defined: %d"), nDynamicDimensions, nNewDeclDimensions);
					HccErrorManager::Error(HccErrorManager::errInvalidDynamicArrayDimension, szInfo);
				}

				//
				/* For now, we will guarantee that default constructors will be called in an array of objects.


				//O B J E C T   C O N S T R U C T O R
				if(token_type==HCC_LPAREN)
				{	
					//The only declaration for object instantiation actually, is a call to a constructor...
					getTokenAppend();	//(
					const __tstring& lpszType = base_type_ptr->getTypeName();
					if(lpszType.length() > 0)
					{
						//a call to a constructor for an object declaration...
						Symbol* constructor_ptr = base_type_ptr->getSymbolTable()->find(lpszType);
						parseFunctionArgumentList(constructor_ptr, fn_ptr);					
					}

					Resync(HCC_RPAREN);
					getTokenAppendIf(HCC_RPAREN, HccErrorManager::errMissingRightParen); //)
				}
				*/

			}else{
				HccErrorManager::Error(HccErrorManager::errUnexpectedToken, _T(" ;expected \'[\' which denotes and array instantiation."));
			}					
		}else if(token_type==HCC_IDENTIFIER)
		{
			//BEGIN - ADDED Jan 7, 2009
			//maybe a var-array is being assigned a pointer to an array;
			Symbol* symbol_ptr = getSymbolFromIdentifier(true, fn_ptr);
			if(
				symbol_ptr!=NULL &&
				(
					symbol_ptr->getDeclDefinition().identifier_type()==DECL_POINTER_VARIABLE	||
					symbol_ptr->getDeclDefinition().identifier_type()==DECL_PARAM_ARRAY			||
					symbol_ptr->getDeclDefinition().identifier_type()==DECL_PARAM_CONST_ARRAY	||
					symbol_ptr->getDeclDefinition().identifier_type()==DECL_PARAM_POINTER		||
					symbol_ptr->getDeclDefinition().identifier_type()==DECL_PARAM_CONST_POINTER ||
				 (
				   symbol_ptr->getDeclDefinition().identifier_type()==DECL_NEW_DATA_MEMBER &&
				   symbol_ptr->getDeclDefinition().user_data.bDataMemberIsPointer
				 )
				)
			  )
			{
				//
				appendSymbolToICode(HCC_TOKEN_ERROR, symbol_ptr);
			}	
			//END - ADDED Jan 7, 2009
		}

		//------------------------------------------------------------------------
		if(class_ptr!=NULL && fn_ptr==NULL)
		{
			//A D D   T O   T H E   C L A S S ' S   D A T A   M E M B E R S  G R O U P 
			class_ptr->getTypeSpecifier().user_type_members.data_members.push_back(var_array_ptr);
			//restore to the global icode object...	
			icode_ptr = global_icode_ptr;
			assert(icode_ptr!=NULL);
		}
	}else if(false==bAlreadyInICode)
		{
			//to avoid assigning to a completely defined array
			var_array_ptr->getTypeSpecifier().array.bIsDynamicArray = true;
		}
}

LPHCC_PARSE_TREE HCCParser::ParseCallWriteWriteLnBuiltInFunction(Symbol *write_fn_ptr, 
																 Symbol* function_ptr, 
																 bool bAddWriteFnToICode)
{
	if(bAddWriteFnToICode) //to avoid expr == NULL when aliasing like : using printf = Console::WriteLn;
	{
		appendSymbolToICode(HCC_TOKEN_ERROR, write_fn_ptr);
	}
	LPHCC_PARSE_TREE tree_ptr = NULL;
	do{
		getTokenAppend();
		tree_ptr = ParseExprList(function_ptr);
		if(token_type==HCC_COMMA_OP)
			delete tree_ptr;
		//
	}while(token_type==HCC_COMMA_OP);

	getTokenAppend(); // )
	
	return tree_ptr;
}

Symbol* HCCParser::AppendNumber(LPHCC_TOKEN number_ptr)
{
	Symbol* symbol_ptr = NULL;
	assert(number_ptr->tokenType==HCC_NUMBER);
	if(number_ptr->tokenType!=HCC_NUMBER)
		return NULL;	

	//insert it into the symbol table, and set its data parameters...
	symbol_ptr = symbol_table_ptr->find(number_ptr->String());
	if(symbol_ptr==NULL){
		symbol_ptr = symbol_table_ptr->insert(number_ptr->String());
		//
		symbol_ptr->getDeclDefinition().set_identifier_type(DECL_CONSTANT);												
		symbol_ptr->setType(number_ptr->tokenType);

		if(number_ptr->dataType==HCC_FLOATING_POINT){
			//symbol_ptr->setValue(number_ptr->value.Double);
			symbol_ptr->getDeclDefinition().constant.value.Double = number_ptr->value.Double;

			symbol_ptr->getTypeSpecifier().setBaseTypeSpec(HccTypeChecker::ts_type_double_ptr);
		}
		else if(number_ptr->dataType==HCC_INTEGER){
			//symbol_ptr->setValue((double)number_ptr->value.Integer);
			symbol_ptr->getDeclDefinition().constant.value.Integer = number_ptr->value.Integer;
			/*
			HCC_SIGNED_TYPE		= 3,
			HCC_UNSIGNED_TYPE	= 4,
			*/
			TypeSpecifier* integer_type_ptr = HccTypeChecker::ts_type_int_ptr; //default

			//BEGIN - FIXED Mar 03, 2009 - ms-help://MS.MSDNQTR.2003FEB.1033/vclang/html/_langref_data_type_ranges.htm

			if(number_ptr->value.Integer >= SHRT_MIN && 
				number_ptr->value.Integer <= SHRT_MAX)
			{
				integer_type_ptr = HccTypeChecker::ts_type_Int16_ptr;
				//
			}else if(number_ptr->value.Integer >= 0 && 
					 number_ptr->value.Integer <= USHRT_MAX)
			{
				integer_type_ptr = HccTypeChecker::ts_type_ushort_ptr;
				//
			}else if(number_ptr->value.Integer >= INT_MIN && 
				     number_ptr->value.Integer <= INT_MAX)
			{
				integer_type_ptr = HccTypeChecker::ts_type_Int32_ptr;
				//
			}else if(number_ptr->value.Integer <= UINT_MAX)
			{
				integer_type_ptr = HccTypeChecker::ts_type_unsigned_ptr;
				//
			}else{
				/*for all other numbers in a range : 
						_I64_MIN    (-9223372036854775807i64 - 1)
						_I64_MAX      9223372036854775807i64
				*/
				integer_type_ptr = HccTypeChecker::ts_type_Int64_ptr;
			}
			/*
			if(number_ptr->value.Integer >= 0 && 
				number_ptr->value.Integer <= SHRT_MAX)
			{
				integer_type_ptr = HccTypeChecker::ts_type_Int16_ptr;
				//
			}else if(number_ptr->value.Integer < 0 && 
					number_ptr->value.Integer >= SHRT_MIN)
			{
				integer_type_ptr = HccTypeChecker::ts_type_Int16_ptr;
				//
			}else if(number_ptr->value.Integer >= 0 && 
				number_ptr->value.Integer <= INT_MAX)
			{
				integer_type_ptr = HccTypeChecker::ts_type_Int32_ptr;
				//
			}else if(number_ptr->value.Integer < 0 && 
					number_ptr->value.Integer >= INT_MIN)
			{
				integer_type_ptr = HccTypeChecker::ts_type_Int32_ptr;
				//
			}else if(number_ptr->value.Integer >= 0 && 
				number_ptr->value.Integer <= UINT_MAX)
			{
				integer_type_ptr = HccTypeChecker::ts_type_unsigned_ptr;
				//
			}else{
				//for all other numbers in a range : 
				//		_I64_MIN    (-9223372036854775807i64 - 1)
				//		_I64_MAX      9223372036854775807i64
				//
				integer_type_ptr = HccTypeChecker::ts_type_Int64_ptr;
			}
			*/
			//END - FIXED Mar 03, 2009
			//the resultant type...
			symbol_ptr->getTypeSpecifier().setBaseTypeSpec(integer_type_ptr);
		}
		else
			assert(!"Cannot operate with this data type yet!");
		
		symbol_ptr->setDataType(number_ptr->dataType);			
	}

	assert(symbol_ptr!=NULL);			
	//add to intermediate code...	
	icode_ptr->append(symbol_ptr);

	return symbol_ptr;
}

Symbol* HCCParser::getMainEntryPoint()
{
	return HCCParser::main_entry_point_ptr;
}

unsigned int HCCParser::getClassTypeLayoutSize(TypeSpecifier *type_class_ptr)
{
	volatile unsigned int layout_size = 0;

	assert(type_class_ptr!=NULL);
	if(type_class_ptr==NULL)
		return layout_size;
	
	TypeSpecifier* base_type_ptr = type_class_ptr->getBaseTypeSpec();

	if(type_class_ptr != base_type_ptr)
		layout_size = base_type_ptr->getDataTypeSize();
	//
	return layout_size;
}

void HCCParser::appendSymbolToICode(HCC_TOKEN_TYPE tokenType, Symbol *symbl_ptr)
{
	if(tokenType!=HCC_TOKEN_ERROR)
		icode_ptr->append(tokenType);
	//add to intermediate code...
	icode_ptr->append(symbl_ptr);

	//to avoid icode object desynchronization we set this flag...
	bPrevTokenManuallyAdded = true;
	getTokenAppend();
}

Symbol* HCCParser::getClassVtbl_PtrSymbol(Symbol *class_ptr, long& offset)
{
	__tstring vtbl_ptr_symbol_name = CLASS_VPTR_VTBL_NAME;
	Symbol* vtbl_symbol_ptr = class_ptr->getTypeSpecifier().getSymbolTable()->find(vtbl_ptr_symbol_name);
	if(vtbl_symbol_ptr==NULL)
	{
		vtbl_symbol_ptr = class_ptr->getTypeSpecifier().getSymbolTable()->insert(vtbl_ptr_symbol_name);
		assert(vtbl_symbol_ptr!=NULL);
		//represents a memory address...
		vtbl_symbol_ptr->getTypeSpecifier().setBaseTypeSpec(HccTypeChecker::ts_type_unsigned_ptr);
		vtbl_symbol_ptr->getDeclDefinition().set_identifier_type(DECL_VTBL_PTR);
		//specify the offset for the vtbl ptr
		vtbl_symbol_ptr->getDeclDefinition().user_data.offset = offset;
		offset += sizeof(int);
		TypeDataMembers& data_members = class_ptr->getTypeSpecifier().user_type_members.data_members;
		data_members.push_back(vtbl_symbol_ptr);
	}
	return vtbl_symbol_ptr;
}


//-----------------------------------------------------
//
// ptr-expr	::	ptr + expr-stmt(return:int) |
//				ptr - expr-stmt(return:int) |
//				ptr		
//
// ptr		::  &var | ptr-var
//
//
//
//-----------------------------------------------------
LPHCC_PARSE_TREE HCCParser::parsePointerExpression(Symbol* function_ptr, LPHCC_PARSE_TREE ptr_expr)
{
	LPHCC_PARSE_TREE left_value_ptr = NULL;
	if(ptr_expr==NULL)
	{
		assert(token_type==HCC_IDENTIFIER);
		//
		Symbol* pointer_ptr = getSymbolFromIdentifier(true, function_ptr);
#ifdef _DEBUG
		if(pointer_ptr!=NULL)
		{
			assert(pointer_ptr->getDeclDefinition().identifier_type()==DECL_POINTER_VARIABLE);
		}
#endif
		left_value_ptr = new HCC_PARSE_TREE(pointer_ptr, HCC_IDENTIFIER);
	}else{		
		if(ptr_expr->left_ptr==NULL)
		{			
			left_value_ptr = ptr_expr->right_ptr;
			ptr_expr->right_ptr = NULL;

			delete ptr_expr;
		}else
			left_value_ptr = ptr_expr;
	}

	LPHCC_PARSE_TREE ptr_expr_tree_ptr = NULL;

	switch(token_type)
	{
	case HCC_PLUS_OP:				// +
	case HCC_MINUS_OP:				// -
		{
			HCC_TOKEN_TYPE oper = token_type;
			ptr_expr_tree_ptr = new HCC_PARSE_TREE(NULL, oper);	//ptr -op- expr-stmt;

			getTokenAppend();	//skip operator
			LPHCC_PARSE_TREE tree_ptr = NULL;
			
			ptr_expr_tree_ptr->set_is_pointer_expr(true);
			ptr_expr_tree_ptr->left_ptr		= left_value_ptr;
			ptr_expr_tree_ptr->right_ptr	= tree_ptr = ParseExprList(function_ptr);
			
			if(tree_ptr!=NULL && tree_ptr->type_ptr!=NULL)
			{
				if(tree_ptr->type_ptr->getDataType()!=HCC_INTEGER || 
					tree_ptr->type_ptr->getDataTypeSize() > sizeof(int))
				{
					//: error: cannot construct a pointer expression because type in expression is not int
					HccErrorManager::Error(HccErrorManager::errInvalidPointerExpression, 
											_T("cannot construct a pointer expression from an expression with type different from integer."));
				}else{
					if(tree_ptr->is_pointer_expr() && oper==HCC_PLUS_OP)
					{
						//error: cannot add two pointers.
						HccErrorManager::Error(HccErrorManager::errCannotAddTwoPointers);
					}
				}
			}			
		}
		break;
	default:
		ptr_expr_tree_ptr = left_value_ptr;
		break;
	}

	Resync(HCC_SEMICOLON);

	return ptr_expr_tree_ptr;
}

LPHCC_PARSE_TREE HCCParser::parseSizeOfExpr(Symbol *sizeof_op_ptr, Symbol *function_ptr)
{
	appendSymbolToICode(HCC_TOKEN_ERROR, sizeof_op_ptr);

	LPHCC_PARSE_TREE tree_ptr = new HCC_PARSE_TREE(sizeof_op_ptr, HCC_SIZEOF);

	bool bMustTakeOffRParen = false;
	if(token_type==HCC_LPAREN){
		bMustTakeOffRParen = true;
		getTokenAppend();
	}
	//
	if(token_type==HCC_IDENTIFIER)
	{
		Symbol* id_ptr = getSymbolFromIdentifier(true, function_ptr);
		if(id_ptr!=NULL)
		{
			__tstring info = _T("\'");
			info += id_ptr->getName();
			info += _T("\'.");
			switch(id_ptr->getDeclDefinition().identifier_type())
			{
			case DECL_VOID:
			case DECL_VARIABLE:
			case DECL_CONSTANT:
			case DECL_PARAM_VALUE:		//param by value
			case DECL_PARAM_BYREF:		//param by reference
			case DECL_PARAM_CONST_BYREF://const param by reference


			case DECL_PARAM_ARRAY:			//one-dimension param array
			case DECL_PARAM_CONST_ARRAY:	//one-dimension const param array

			case DECL_POINTER_VARIABLE:
			case DECL_PARAM_POINTER:			//param pointer
			case DECL_PARAM_CONST_POINTER:		//const param pointer

			case DECL_BUILDIN_TYPE:				//for all the compiler build-in types
			case DECL_NEW_TYPE:					//for class and struct keywords (will be the same behavior for now)
			case DECL_NEW_ABSTRACT_TYPE:		//for abstract class and struct keywords
				break;
			default:
				HccErrorManager::Error(HccErrorManager::errUnexpectedToken, info);
				break;
			}

			tree_ptr->right_ptr = new HCC_PARSE_TREE(id_ptr, HCC_IDENTIFIER);

			appendSymbolToICode(HCC_TOKEN_ERROR, id_ptr);
		}
	}else{
		Symbol* type_ptr = find_symbol(token_ptr->String());
		switch(token_type)
		{
		case HCC_VOID:
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
			{
				//this case for all primitive types
				tree_ptr->right_ptr = new HCC_PARSE_TREE(NULL, token_type);
				tree_ptr->right_ptr->type_ptr = &type_ptr->getTypeSpecifier();

				appendSymbolToICode(HCC_IDENTIFIER, type_ptr);
				getTokenAppend();
			}
			break;
		default:
			HccErrorManager::Error(HccErrorManager::errInvalidExpression, _T(";expected identifier or type."));	
		}
	}
	//

	if(bMustTakeOffRParen)
		getTokenAppendIf(HCC_RPAREN, HccErrorManager::errMissingRightParen);
	else if(token_type==HCC_RPAREN){
		HccErrorManager::Error(HccErrorManager::errUnexpectedToken);
		getToken();
	}

	return tree_ptr;
}

void HCCParser::checkImplicitParamConversion(const __tstring& param_name, TypeSpecifier* parameter_type, TypeSpecifier* argument_type)
{
	TypeSpecifier* result_expr_type_ptr =
		HccTypeChecker::GetResultTypeFromExprTypes(parameter_type, argument_type);

	if(parameter_type->getDataType()==HCC_INTEGER
		&&
		result_expr_type_ptr->getDataType()==HCC_FLOATING_POINT)
	{
		HccWarningManager::Warning(HccWarningManager::warnImplicitConversionPossibleLossOfData, 
									param_name);
	}else{
		HccTypeChecker::CheckCompatibleAssignment(
								parameter_type,
								argument_type,
								HccErrorManager::errIncompatibleTypes,
								param_name);
	}
}


//--------------------------------------------------------------------
//
//	  new-decl     :: pointer-decl =  new  type-spec |
//						   pointer-decl =  new  type-spec(param-list) |
//						   pointer-decl =  new  type-spec subscript-expression |
//						   pointer-decl =  null
//	subscript-expression	::	[integer-expression] subscript-expression
//
//--------------------------------------------------------------------
TypeSpecifier* HCCParser::parseNewInstance(Symbol* var_pointer, TypeSpecifier* base_type_ptr, Symbol* class_ptr, Symbol* function_ptr, bool bCheckTypeCompatibility)
{
	assert(token_type==HCC_NEW);

	getTokenAppend(); // operator new 
	bool bIsAbstractType = false;
	TypeSpecifier* type_ptr = parseTypeSpec(class_ptr, function_ptr, &bIsAbstractType);
	if(type_ptr==NULL || (type_ptr!=base_type_ptr && bCheckTypeCompatibility))
	{
		HccTypeChecker::CheckCompatibleAssignment(base_type_ptr, 
												  type_ptr,
												  HccErrorManager::errInvalidAssignment, 
												 _T(", cannot instantiate to this unknown type."));
	}
	//now we set the base type as itself...
	base_type_ptr = type_ptr;

	if(bIsAbstractType)
	{
		__tstring info = _T(" ");
		if(type_ptr!=NULL)
		{
			info = _T(", cannot instantiate abstract class \'");
			info += type_ptr->getTypeName();info += _T("\'.");
		}
		HccErrorManager::Error(HccErrorManager::errAbstractClassInstantiation, info);
	}

	Symbol* ctor_ptr = NULL;
	const __tstring& lpszType = base_type_ptr->getTypeName();
	//add the symbol's type, only if is scalar type...
	if(type_ptr->is_scalar())
	{
		Symbol* scalar_ptr = find_symbol(lpszType.c_str());
		assert(!!scalar_ptr);
		if(scalar_ptr!=NULL)
		{
			icode_ptr->append(HCC_IDENTIFIER);
			icode_ptr->append(scalar_ptr);
		}					
	}else if(lpszType.length() > 0)
	{
		//a call to a constructor for an object declaration...
		ctor_ptr = base_type_ptr->getSymbolTable()->find(lpszType);
		if(ctor_ptr==NULL)
		{
			__tstring info = ", add a default constructor to class \'";
			info += lpszType;
			info += _T("\'.");
			HccErrorManager::Error(HccErrorManager::errMissingDefaultConstructorWithNewOp, info);
		}
	}
	//O B J E C T   C O N S T R U C T O R
	if(token_type==HCC_LPAREN)
	{
		if(ctor_ptr!=NULL)
		{
			appendSymbolToICode(HCC_TOKEN_ERROR, ctor_ptr);
			getTokenAppend();	//(
			parseFunctionArgumentList(ctor_ptr, function_ptr);
		}else
			getTokenAppend();	//(
		
		Resync(HCC_RPAREN);
		getTokenAppendIf(HCC_RPAREN, HccErrorManager::errMissingRightParen); //)
	}else if(token_type==HCC_LBRACKET)
	{
		//BEGIN - ADDED Jan 7, 2009
		if(var_pointer!=NULL)
		{
			//this will be needed by the code generator...
			icode_ptr->put_back(); // [
			appendSymbolToICode(HCC_IDENTIFIER, var_pointer);

			//  A declaration of this type:
			//  [accessor] Type-spec^ pointer = new Type-spec[integer-expression];
			
			//the pointer is converted to an array from this moment!
			//a dynamic array!
			var_pointer->getTypeSpecifier().array.bIsDynamicArray = true;

			//BEGIN - FIXED Jan 21, 2009
			parseDynamicallyAllocatedArrayType(&var_pointer->getTypeSpecifier(), function_ptr, type_ptr);
			//END - FIXED Jan 21, 2009

			//to avoid AV when destroying the symbol...
			TypeSpecifier* scalar_ptr = getArrayScalarType(&var_pointer->getTypeSpecifier());
			if(scalar_ptr==HccTypeChecker::ts_type_char_ptr)
				var_pointer->getDeclDefinition().constant.value.String = 0;

			long nDynamicDimensions = getArrayDimesionsFromType(&var_pointer->getTypeSpecifier());
			//BEGIN - ADDED Jan 23, 2009
			if(nDynamicDimensions != 1)
			{
				HccErrorManager::Error(HccErrorManager::errInvalidDynamicArrayDimension, 
									  _T(", dynamic arrays can have only one dimension; like in: int [] array = new int[n];"));
			}
			//END - ADDED Jan 23, 2009
		}else
			HccErrorManager::Error(HccErrorManager::errInvalidExpression);
		//END - ADDED Jan 7, 2009
	}
	icode_ptr->append(HCC_SEMICOLON);

	return base_type_ptr;
}


//-----------------------------------------------------------------
//
//	  dynamic_cast-decl :: pointer-decl = dynamic_cast(type-spec, pointer) |
//						   pointer-decl = dynamic_cast(pointer) |
//						   dynamic_cast(type-spec, pointer)		|
//						   dynamic_cast(type-spec, &array[subscript-expression]) |
//						   dynamic_cast(&array[subscript-expression])
//
//-----------------------------------------------------------------
TypeSpecifier* HCCParser::parseDynamicCastOperator(TypeSpecifier* decl_type_ptr, Symbol* class_ptr, Symbol* function_ptr)
{
	assert(token_type==HCC_DYNAMIC_CAST);
	if(token_type!=HCC_DYNAMIC_CAST)
		return NULL;

	TypeSpecifier* final_type_ptr = decl_type_ptr;
	getTokenAppend(); //dynamic_cast
	getTokenAppendIf(HCC_LPAREN, HccErrorManager::errMissingLeftParen); //(

	//BEGIN - ADDED Jan 26, 2009
	bool bUsingAddressOf = false;
	if(token_type==HCC_POINTER_ADDRESSOF)
	{
		getTokenAppend();
		bUsingAddressOf = true;
	}
	//END - ADDED Jan 26, 2009

	bool bIdentifierInICode = false;

	if(token_type==HCC_IDENTIFIER)
	{
		bool bIsAbstractType = false;
		TypeSpecifier* result_type_ptr = NULL;
		//TypeSpecifier* result_type_ptr = parseTypeSpec(class_ptr, function_ptr, &bIsAbstractType);
		SYMBOL_TABLE::LPSYMBOL symbol_ptr = getSymbolFromIdentifier(true, function_ptr);
		if(symbol_ptr!=NULL)
		{
			DECLARE_SPEC_TYPE specType = symbol_ptr->getTypeSpecifier().specifier();
			if(
				(symbol_ptr->getDeclDefinition().identifier_type()==DECL_NEW_TYPE			||
				 symbol_ptr->getDeclDefinition().identifier_type()==DECL_NEW_ABSTRACT_TYPE	||
				 symbol_ptr->getDeclDefinition().identifier_type()==DECL_BUILDIN_TYPE)
				)
			{
				//
				bIsAbstractType = (symbol_ptr->getDeclDefinition().identifier_type()==DECL_NEW_ABSTRACT_TYPE);
				if(specType==DSPEC_CLASS || specType==DSPEC_ENUM || specType==DSPEC_SIMPLE)
					result_type_ptr = &symbol_ptr->getTypeSpecifier();
			}
		}

		//found type like in: dynamic_cast(class-type, object-instance)
		if(result_type_ptr!=NULL)
		{
			//dynamic_cast not supported on scalar types
			if(result_type_ptr->is_scalar())
			{
				HccErrorManager::Error(HccErrorManager::errDynamicCastOnScalarType);
			}
			//abstract types are dangerous!
			if(bIsAbstractType)
			{
				HccWarningManager::Warning(HccWarningManager::warnAbstractTypeUsingDynamicCast);
			}

			//BEGIN - ADDED Jan 26, 2009
			if(bUsingAddressOf)
			{
				HccErrorManager::Error(HccErrorManager::errInvalidPointerExpression, _T(", cannot use operator \'&\' on a type."));
			}
			//END - ADDED Jan 26, 2009
			//if has a declation type like in: class-type^ pointer = dynamic_cast(class-type, object-instance);
			if(decl_type_ptr!=NULL)
			{
				if(decl_type_ptr!=result_type_ptr)
				{
					HccTypeChecker::CheckCompatibleAssignment(decl_type_ptr,
															  result_type_ptr,
															  HccErrorManager::errInvalidAssignment,
															  _T(", declared-type must be base class or the type used in dynamic_cast."));

					final_type_ptr = result_type_ptr;
				}
			}
			//
			if(token_type==HCC_COMMA_OP){
				appendSymbolToICode(HCC_TOKEN_ERROR, symbol_ptr); //the class-type symbol first
				getTokenAppendIf(HCC_COMMA_OP, HccErrorManager::errMissingComma); //,
			}else
				HccErrorManager::Error(HccErrorManager::errMissingComma, 
									  _T(", when using dynamic_cast with types like in : dynamic_cast(class-type, object-instance);"));

			//
			//BEGIN - ADDED Jan 26, 2009
			if(token_type==HCC_POINTER_ADDRESSOF)
			{
				getTokenAppend();
				bUsingAddressOf = true;
			}
			//END - ADDED Jan 26, 2009
			if(token_type==HCC_IDENTIFIER)
			{
				symbol_ptr = getSymbolFromIdentifier(true, function_ptr);
				if(symbol_ptr!=NULL)
				{
					//the pointer type
					TypeSpecifier* ptr_type_ptr = symbol_ptr->getTypeSpecifier().getBaseTypeSpec();

					//BEGIN - ADDED/FIXED Feb 22, 2009
					if(HCC_LBRACKET==token_type)	//[
					{
						appendSymbolToICode(HCC_TOKEN_ERROR, symbol_ptr);
						//to flag that the symbol is already in the icode buffer...
						bIdentifierInICode = true;
						//accessing an array element...
						TypeSpecifier* baseTypeSpec = symbol_ptr->getTypeSpecifier().getBaseTypeSpec();
						//
						ptr_type_ptr = parseArraySubscript(baseTypeSpec, function_ptr);
						//
					}
					//END - ADDED/FIXED Feb 22, 2009

					if(result_type_ptr!=ptr_type_ptr)
					{
						HccTypeChecker::CheckCompatibleAssignment(ptr_type_ptr,
																  result_type_ptr,
																  HccErrorManager::errInvalidType,
																  _T(", pointer-type must be base class or the type used to declare the pointer itself."));
					}
					final_type_ptr = result_type_ptr;
				}
			}
			else
				HccErrorManager::Error(HccErrorManager::errExpectedClassTypeOrVariable, 
									  _T(", like in dynamic_cast(Type, instance_variable) | dynamic_cast(instance_variable)."));
			//
			goto __PARSE_OBJECT_INSTANCE_VARIABLE;				
			//
		}else{
__PARSE_OBJECT_INSTANCE_VARIABLE:
			//
			if(symbol_ptr!=NULL)
			{
				DeclarationType declType = symbol_ptr->getDeclDefinition().identifier_type();

				if(
					declType==DECL_POINTER_VARIABLE
					|| (declType==DECL_NEW_DATA_MEMBER && symbol_ptr->getDeclDefinition().user_data.bDataMemberIsPointer)
					|| declType==DECL_PARAM_POINTER
					|| declType==DECL_PARAM_CONST_POINTER
				  )
				{
					if(false==bIdentifierInICode)
					{
						appendSymbolToICode(HCC_TOKEN_ERROR, symbol_ptr);
					}
				}else if(
						bUsingAddressOf &&
						(declType==DECL_VARIABLE
						|| declType==DECL_PARAM_VALUE
						|| declType==DECL_PARAM_BYREF
						|| declType==DECL_PARAM_CONST_BYREF
						|| declType==DECL_PARAM_ARRAY
						|| declType==DECL_PARAM_CONST_ARRAY)
						)
				{
					if(false==bIdentifierInICode)
					{
						appendSymbolToICode(HCC_TOKEN_ERROR, symbol_ptr);
					}
				}
				else
					HccErrorManager::Error(HccErrorManager::errExpectedPointerVariable);
				
				TypeSpecifier* ptr_type_ptr = symbol_ptr->getTypeSpecifier().getBaseTypeSpec();
				//
				//BEGIN - ADDED/FIXED Feb 22, 2009
				if(HCC_LBRACKET==token_type)	//[
				{
					//accessing an array element...
					TypeSpecifier* baseTypeSpec = symbol_ptr->getTypeSpecifier().getBaseTypeSpec();
					//
					ptr_type_ptr = parseArraySubscript(baseTypeSpec, function_ptr);
					//
					if(NULL==ptr_type_ptr)
						ptr_type_ptr = HccTypeChecker::ts_type_int_ptr;
				}
				//END - ADDED/FIXED Feb 22, 2009
				//
				
				if(decl_type_ptr!=NULL)
				{
					if(decl_type_ptr!=result_type_ptr)
					{
						HccTypeChecker::CheckCompatibleAssignment(ptr_type_ptr,
																  decl_type_ptr,
																  HccErrorManager::errInvalidAssignment,
																  _T(", declared-type must be base class or the type used in dynamic_cast."));

						final_type_ptr = result_type_ptr;
					}
				}
				if(final_type_ptr==NULL)
					final_type_ptr = ptr_type_ptr;
			}
		}
		//
	}else
		HccErrorManager::Error(HccErrorManager::errExpectedClassTypeOrVariable, 
							  _T(", like in dynamic_cast(Type, instance_variable) | dynamic_cast(instance_variable)."));

	getTokenAppendIf(HCC_RPAREN, HccErrorManager::errMissingRightParen); //)

	return final_type_ptr;
}

bool HCCParser::IsDynamicArray(TypeSpecifier* array_type_ptr)
{
	return array_type_ptr->array.bIsDynamicArray;
}

//---------------------------------------------------------------
//  parseDynamicallyAllocatedArrayType()
//
//  This function parses a dynamically allocated array
//  Dynamic arrays size is only known in runtime; so the only way to know how many objects
//  must be destroyed when calling the destroy operator, is reading a prefixed array size in the memory
//  before attempting to call the destructors.
//
//---------------------------------------------------------------
TypeSpecifier* HCCParser::parseDynamicallyAllocatedArrayType(TypeSpecifier* id_type_spec, 
															 Symbol* function_ptr, 
															 TypeSpecifier* decl_final_type_ptr)
{
	if(id_type_spec==NULL)
		return 0;

	if(getTokenAppendIf(HCC_LBRACKET, HccErrorManager::errMissingLeftBracket)==0){
		Resync(HCC_LBRACKET);
		if(token_type==HCC_LBRACKET)
			getToken();
	}

	TypeSpecifier* arraySpec = id_type_spec;
	//the element type-spec
	TypeSpecifier* base_type_spec		= arraySpec->getBaseTypeSpec(),
				  *itemTypeSpec			= arraySpec->getBaseTypeSpec();

	if(decl_final_type_ptr!=NULL)
	{
		base_type_spec	= decl_final_type_ptr;
		itemTypeSpec	= decl_final_type_ptr;
	}
	//mark it as an array!
	arraySpec->set_specifier(DSPEC_ARRAY);
	//set as itself!
	arraySpec->setBaseTypeSpec(arraySpec);
	//array of type itemTypeSpec
	arraySpec->array.pItemType = itemTypeSpec;
	//for dynamic arrays, we don't know the size until runtime
	arraySpec->array.item_count = 0;
	//[
	if(token_type==HCC_IDENTIFIER || token_type==HCC_NUMBER)
	{
		//
		LPHCC_PARSE_TREE tree_ptr = ParseExprList(function_ptr);
		//
		if(tree_ptr!=NULL && tree_ptr->type_ptr!=NULL)
		{
			HccTypeChecker::CheckCompatibleAssignment(HccTypeChecker::ts_type_int_ptr,
													  tree_ptr->type_ptr, 0,
													  _T(" ,must be an integer value or expression."));
		}else 
			HccErrorManager::Error(HccErrorManager::errInvalidExpression, _T(", expression is not acceptable; must be an integer expression."));
		//
		delete tree_ptr;
		//
	}else if(token_type!=HCC_RBRACKET){
		HccErrorManager::Error(HccErrorManager::errInvalidSubscriptType, _T(" ,must be an integer value or expression."));
		Resync(HCC_SEMICOLON);
	}
	//next, must be a ']'	
	if(getTokenAppendIf(HCC_RBRACKET, HccErrorManager::errMissingRightBracket)!=0)
	{
		if(token_type==HCC_LBRACKET)
		{
			//when this array has more than one dimension,
			//a new type specifier of type DSPEC_ARRAY must be created
			//and set as the item type for the current array
			//avoiding Stack Overflow when calling getArraySizeFromType();
			//
			//the base type must be the type-specifier found for this declaration...
			itemTypeSpec = new TypeSpecifier(DSPEC_ARRAY);
			arraySpec->array.pItemType = itemTypeSpec;
			//set the previous type
			itemTypeSpec->setBaseTypeSpec(base_type_spec);
			parseDynamicallyAllocatedArrayType(itemTypeSpec, function_ptr);
		}
	}	
	
	return arraySpec;
}
