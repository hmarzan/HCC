// HCCParser.h: interface for the HCCParser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HCCPARSER_H__1508B548_4DEF_4D21_9FE8_6CE5F519E1C7__INCLUDED_)
#define AFX_HCCPARSER_H__1508B548_4DEF_4D21_9FE8_6CE5F519E1C7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/*
********************************************************************************************************
*																									   *
*																									   *
*	MODULE			: <hccparser.h>																	   *	
*																									   *
*	DESCRIPTION		: HCC Compiler Parser class (token retrieval and syntax analysis for HC)		   *
*																									   *
*	AUTHOR			: Harold L. Marzan																   *	
*																									   *
*	LAST-MODIFIED	: (unknown)																		   *	
*																									   *
********************************************************************************************************
*/

#include "hcclexer.h"
#include "source_unit_compactor.h"
#include "HCCLib\coresymbols.h"
#include "HCCLib\coreicode.h"

#pragma warning(disable:4503)
#include <set>
#include <map>

using namespace std;

extern SYMBOL_TABLE g_symbol_table;
extern icode_generator g_icode_gen;

class HCCParser  
{
	HCCLexer* lexer;
	LPHCC_TOKEN token_ptr;
	HCC_TOKEN_TYPE token_type;
	LPHCC_TOKEN getToken(void);
	LPHCC_TOKEN getTokenAppend(void);
	//conditional get token...
	LPHCC_TOKEN getTokenIf(HCC_TOKEN_TYPE token, ParserError error);
	LPHCC_TOKEN getTokenAppendIf(HCC_TOKEN_TYPE token, ParserError error);
	source_unit_compactor* unit_compactor_ptr;	
	LPSYMBOL_TABLE symbol_table_ptr;
	static bool bEnableXREF;
	static bool m_bParserInit;
	icode_generator* icode_ptr;		//used to control the intermediate code generation when parsing
	icode_generator* global_icode_ptr;	//holds the global icode ptr (icode_ptr holds it too by default)	
	//for the treatment of namespaces
	Symbol* current_namespace_ptr;
	Symbol* prev_namespace_ptr;
	//to support the lookup of symbols based on different namespaces
	set<Symbol*> active_namespaces;
	Symbol* active_class_ptr;
	Symbol* active_ns_class_ptr;
	__tstring last_identifier;
	set<Symbol*> using_namespaces;
	set<__tstring> extern_libs;
	//the main entry point...
	static Symbol* main_entry_point_ptr;
	//
	volatile bool bPrevTokenManuallyAdded;
	volatile bool is_ternary_expression;

	//to keep track of user-defined proc labels
	map<Symbol*, set<__tstring> > map_proc_labels;
	//to verify labels where defined
	set<__tstring> ref_proc_labels;
public:		
	static Symbol* getMainEntryPoint();
	void TestLexer();
	void Init();
	void CompactUnit();
	void Parse();
	HCCParser(source_buffer* source_ptr) : 
					lexer(new HCCLexer(source_ptr)), 
					token_ptr(0), token_type(HCC_EOF),
					unit_compactor_ptr(new source_unit_compactor()),
					symbol_table_ptr(&g_symbol_table),
					icode_ptr(&g_icode_gen),
					global_icode_ptr(&g_icode_gen),					
					global_variable_offset(0),
					current_namespace_ptr(0),
					prev_namespace_ptr(0),
					active_class_ptr(0),	//the active class
					active_ns_class_ptr(0), //the active namespace
					bPrevTokenManuallyAdded(false),
					is_ternary_expression(false)
					
					{Init();HccErrorManager::SetSourceFilePtr(source_ptr);HccWarningManager::SetSourceFilePtr(source_ptr);}
	virtual ~HCCParser() 
		{delete lexer;delete unit_compactor_ptr;}
	static void EnableCrossReference(bool bXREF = true)
		{bEnableXREF = bXREF;}
	static bool IsXREFEnabled()
		{return bEnableXREF;}
private:
	void appendSymbolToICode(HCC_TOKEN_TYPE tokenType, Symbol *symbl_ptr);
	//E X P R E S S I O N   P A R S E R   P R O C E D U R E S 
	LPHCC_PARSE_TREE ParseFactor(Symbol *function_ptr = NULL);
	LPHCC_PARSE_TREE ParseTerm(Symbol *function_ptr = NULL);
	LPHCC_PARSE_TREE ParseExprList(Symbol *function_ptr = NULL);
	LPHCC_PARSE_TREE ParseRelationalExprList(Symbol *function_ptr);
	LPHCC_PARSE_TREE ParseExpr(Symbol *function_ptr = NULL);
	LPHCC_PARSE_TREE parseCommaExprList(Symbol *function_ptr = NULL);
private:
	LPHCC_PARSE_TREE parseSizeOfExpr(Symbol* sizeof_op_ptr, Symbol* function_ptr);
	LPHCC_PARSE_TREE parsePointerExpression(Symbol* function_ptr, LPHCC_PARSE_TREE ptr_expr);
	Symbol* getClassVtbl_PtrSymbol(Symbol* class_ptr, long& offset);
	unsigned int getClassTypeLayoutSize(TypeSpecifier* type_class_ptr);
	Symbol* AppendNumber(LPHCC_TOKEN number_ptr);
	LPHCC_PARSE_TREE ParseCallWriteWriteLnBuiltInFunction(Symbol* write_fn_ptr, Symbol* function_ptr, bool bAddWriteFnToICode = true);
	void parseArrayPointerVariableOrDataMember(Symbol* var_array_ptr, TypeSpecifier* base_type_ptr, short nDimensions, Symbol *fn_ptr, Symbol *class_ptr, bool bAlreadyInICode);
	void parsePointerVariableOrDataMember(Symbol* variable_ptr, TypeSpecifier* base_type_ptr, Symbol *fn_ptr, Symbol *class_ptr, bool bAlreadyInICode);
	void analyzeClassVirtualState(Symbol* class_ptr);
	inline bool IsAbstractClass(Symbol* class_ptr);
	//F U N C T I O N   S P E C I F I C
	TypeSpecifier* parseArgument(Symbol* parameter_ptr, Symbol* func_scope_ptr);	
	LPHCC_PARSE_TREE parseFunctionCall(Symbol* called_function_ptr, Symbol* caller_function_ptr, TypeSpecifier* owner_type_ptr = NULL);
	void parseFunctionArgumentList(Symbol *called_function_ptr, Symbol *caller_function_ptr);
	void parseFunctionParamList(Symbol *function_ptr);
	LPHCC_PARSE_TREE parseObjectInstanceMember(TypeSpecifier* baseTypeSpec, Symbol *function_ptr = NULL);

	//D E C L A R A T I O N   P A R S E R   P R O C E D U R E S 
	Symbol* getSymbolFromIdentifier(bool bAlertError = false, Symbol *function_ptr = NULL);
	Symbol* getNestedNameIdentifier(__tstring& token_id);

	long global_variable_offset;
	void parseIdentifierVariable(Symbol *id_ptr, HCC_TOKEN_TYPE sign, Symbol *function_ptr = NULL);	
	bool IsTypeSpecifier();
	//
	void CopyQuotedString(TCHAR* target, const TCHAR* quoted_source);
	Symbol* find_symbol(const TCHAR* symbol_name);
	Symbol* insert_symbol(const TCHAR* symbol_name, DECLARATION_TYPE declType = DECL_CONSTANT);
	//Declarations parsing procedures	
	void nextIdentifierFromComma(HCC_TOKEN_TYPE token_ref = HCC_EOF);
	//for H++ declarations
	TypeSpecifier* parseTypeSpec(Symbol* ns_class_ptr = NULL, Symbol* function_ptr = NULL, bool *pIsAbstractType = NULL, DataTypeModifier* pTypeModifier = NULL);
	//for typedefs using keyword 'typename'
	Symbol* parseTypeDefinitions(Symbol *id_proc_ptr, Symbol* ns_class_ptr = NULL);
	//for enumerations
	Symbol* parseEnumerationType(Symbol *id_proc_ptr, Symbol* ns_class_ptr = NULL);
	//for classes/structs
	Symbol* parseClassStructTypes(Symbol* ns_class_ptr = NULL);

	//C L A S S   S P E C I F I C 
	long parseVariableOrDataMemberDecl(Symbol* fn_ptr, Symbol* class_ptr, long offset, 
										MemberAccessType accessType = ACCESS_PRIVATE, 
										TypeSpecifier* typeSpec = NULL);	
	void parseMemberFunction(Symbol* class_ptr, TypeSpecifier* returnTypeSpec, Symbol* function_ptr, long& offset, HCC_TOKEN_TYPE fnType = HCC_VOID);
	long parseClassDataMember(Symbol *class_ptr, long offset, TypeSpecifier* member_typeSpec);
	long parseClassConstantMember(Symbol *class_ptr, long offset, MemberAccessType accessType);
	void parseClassDestructor(Symbol* class_ptr);
	void parseClassConstructor(Symbol *class_ptr);
	void parseClassProperty(TypeSpecifier* property_type_ptr,Symbol *class_ptr, bool bIsTypePointer);


	//for /global/local variables
	long parseVariableDeclarations(Symbol* fn_ptr, TypeSpecifier* typeSpec);
	//for local arrays
	TypeSpecifier* parseArraySubscript(TypeSpecifier* baseTypeSpec, Symbol *function_ptr = NULL);
	short getArrayDimesionsFromType(TypeSpecifier *array_ptr);
	TypeSpecifier* getArrayScalarType(TypeSpecifier* array_ptr);
	long getArraySizeFromType(TypeSpecifier* array_ptr);
	TypeSpecifier* parseArrayType(TypeSpecifier* id_type_spec, 
								  Symbol* function_ptr, 
								  TypeSpecifier* decl_final_type_ptr = NULL);
	
	//for dynamic arrays (heap based)
	TypeSpecifier* parseDynamicallyAllocatedArrayType(TypeSpecifier* id_type_spec, 
													  Symbol* function_ptr, 
													  TypeSpecifier* decl_final_type_ptr = NULL);


	void parseIdentifierConstant(Symbol *id_ptr, HCC_TOKEN_TYPE sign, Symbol *function_ptr = NULL);
	void parseConstantDefinitions(Symbol *id_ptr, Symbol *ns_class_ptr, MemberAccessType accessType = ACCESS_NONE);
	void parseDeclarations(Symbol *id_proc_ptr);
	void parseNameSpace();

	//S T A T E M E N T   P A R S E R   P R O C E D U R E S 
	HCC_TOKEN_TYPE ParseStatementList(HCC_TOKEN_TYPE terminator, Symbol *function_ptr = NULL);	
	HCC_TOKEN_TYPE ParseStatement(Symbol *function_ptr = NULL);
	HCC_TOKEN_TYPE parseCompoundStatement(Symbol *function_ptr);
	//Statements parsing procedures
	void parseUsingNamespace(Symbol* function_ptr);
	void parseWithStatement(Symbol *function_ptr);
	bool StatementRequiresSemicolon(HCC_TOKEN_TYPE stmt_type);
	void Resync(HCC_TOKEN_TYPE *tokens, int sz);
	bool IsUnarySign();
	void parseTryBlock(Symbol *function_ptr);
	void parseJumpStatement(Symbol *function_ptr);
	bool IsJumpStatement();
	void skipEmptyStatements();
	bool IsNewStatement();
	void Resync(HCC_TOKEN_TYPE token);	
	void parseCaseBranch(Symbol *function_ptr);
	void parseSwitch(Symbol *function_ptr);
	HCC_TOKEN_TYPE parseIfStatement(Symbol *function_ptr);
	void parseFor(Symbol *function_ptr);
	void parseWhile(Symbol *function_ptr);
	void parseDoWhile(Symbol *function_ptr);
	void parseImportStatement();	
	void checkImplicitParamConversion(const __tstring& param_name, TypeSpecifier* parameter_type, TypeSpecifier* argument_type);
	TypeSpecifier* parseNewInstance(Symbol* var_pointer, TypeSpecifier* base_type_ptr, Symbol* class_ptr, Symbol* function_ptr, bool bCheckTypeCompatibility = true);
	TypeSpecifier* parseDynamicCastOperator(TypeSpecifier* decl_type_ptr, Symbol* class_ptr, Symbol* function_ptr);
	bool IsDynamicArray(TypeSpecifier* array_type_ptr);
};

#endif // !defined(AFX_HCCPARSER_H__1508B548_4DEF_4D21_9FE8_6CE5F519E1C7__INCLUDED_)
