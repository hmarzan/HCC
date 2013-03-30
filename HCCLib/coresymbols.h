#ifndef __HCC_CORE_SYMBOLS_h__
#define __HCC_CORE_SYMBOLS_h__

/*
********************************************************************************************************
*																									   *
*																									   *
*	MODULE			: <coresymbols.h>																   *	
*																									   *
*	DESCRIPTION		: HCC Compiler/Parser symbol table (symbols, symbol table, listing)				   *
*																									   *
*	AUTHOR			: Harold L. Marzan																   *	
*																									   *
*	LAST-MODIFIED	: (unknown)																		   *	
*																									   *
********************************************************************************************************
*/
#pragma warning(disable:4786 4355) //warning C4355: 'this' : used in base member initializer list

#include <map>
#include <list>
#include <vector>
#include <iostream>
#include <iomanip>
#include "corecommon.h"
#include "corelisting.h"
#include <cassert>


using namespace std;

//forward declaration
class Symbol;

typedef enum __tagMEMBER_ACCESS_TYPE
{
	ACCESS_NONE,
	ACCESS_PUBLIC,
	ACCESS_PROTECTED,
	ACCESS_PRIVATE,
}MEMBER_ACCESS_TYPE, *LPMEMBER_ACCESS_TYPE;

typedef HCC_IDENTIFIER_SCOPE_TYPE	IdentifierScopeType;	//local, global, heap
typedef STORAGE_SPECIFIER_TYPE		StorageSpecifierType;	//auto, register, static, extern
typedef DECLARATION_TYPE			DeclarationType;		//variable, constant, new type, new function, etc.
typedef DECLARE_SPEC_TYPE			DeclareSpecType;		//simple, array, class, enum
typedef HCC_FUNC_TYPE				FunctionType;			//user-defined, new, destroy, static_cast(),
typedef MEMBER_ACCESS_TYPE			MemberAccessType;		//private, public, protected
/*
  decl-specifier:
		type-specifier			|
		storage-class-specifier |
		typedef

   type-specifier:
		simple-type-specifier	|
		class-specifier			|
		enum-specifier

  simple-type-specifier:
		primitive-type type-name	|
		custom-type		type-name	|

  type-name:
		class-name	|
		enum-name	|
		typedef-name
*/

typedef struct __tagSTACK_PARAMS_TYPE
{
	vector<Symbol*> params; //this mantains all the function params, and has already the param count in it
	int total_params_size;		//total byte size of params

	__tagSTACK_PARAMS_TYPE() : total_params_size(0){}

}STACK_PARAMS_TYPE, *LPSTACK_PARAMS_TYPE;

typedef struct __tagSTACK_LOCAL_VARIABLES_TYPE
{
	vector<Symbol*> variables;
	int total_variables_size;		//total byte size of local variables

	__tagSTACK_LOCAL_VARIABLES_TYPE() : total_variables_size(0){}
}STACK_LOCAL_VARIABLES_TYPE, *LPSTACK_LOCAL_VARIABLES_TYPE;

typedef struct __tagSTACK_DEFINED_USER_TYPES_TYPE
{
	list<Symbol*> user_def_types;
}STACK_DEFINED_USER_TYPES_TYPE, *LPSTACK_DEFINED_USER_TYPES_TYPE;

typedef struct __tagSTACK_LOCAL_CONSTANTS_TYPE
{
	list<Symbol*> constants;
}STACK_LOCAL_CONSTANTS_TYPE, *LPSTACK_LOCAL_CONSTANTS_TYPE;


//shortcout-typedefs for the above struct types!
typedef STACK_PARAMS_TYPE				StackParams;
typedef STACK_LOCAL_VARIABLES_TYPE		StackVariables;
typedef STACK_DEFINED_USER_TYPES_TYPE	StackDefinedTypes;
typedef STACK_LOCAL_CONSTANTS_TYPE		StackConstants;

typedef vector<Symbol*>					EnumConstants;
typedef vector<Symbol*>					TypeDataMembers;
typedef vector<Symbol*>					ArrayInitConstants;
typedef vector<Symbol*>					GlobalVariables;
typedef vector<Symbol*>					GlobalConstants;

typedef enum __tagVERBOSITY_OUTPUT_TYPE
{
	voVerbose,
	voMinimal,

}VERBOSITY_OUTPUT_TYPE, *LPVERBOSITY_OUTPUT_TYPE;

template<typename _Key, 
		 typename _SymbolType>
class SymbolTable
{
	map<_Key, _SymbolType*> symbol_tbl;
	__tstring the_name;
	SymbolTable();
	SymbolTable(const SymbolTable&);
	SymbolTable& operator=(const SymbolTable&);
	source_buffer* source_ptr;
public:
	typedef _SymbolType* LPSYMBOL;
	typedef typename map<_Key, _SymbolType*>::iterator iterator;
	SymbolTable(const __tstring& _name, source_buffer* _source_ptr = 0) : the_name(_name.c_str()), 
																		  source_ptr(_source_ptr) {}
	~SymbolTable(){clear();}
	void clear(){
		map<_Key, _SymbolType*>::iterator it_symb = symbol_tbl.begin();
		while(it_symb != symbol_tbl.end())
		{
			_SymbolType* ptr = it_symb->second;
			try{
				delete ptr;
			}catch(...){} //FIXED - Mar 07, 2009 - AV for first symbol
			it_symb++;
		}

		symbol_tbl.clear();
	}
	const long size() const
		{return symbol_tbl.size();}
	LPSYMBOL insert(const _Key& _K)
	{
		pair<map<_Key, _SymbolType*>::iterator, bool> _symbol_inserted =
																symbol_tbl.insert(pair<_Key, _SymbolType*>(_K, NULL));		
		LPSYMBOL symbol_ptr = NULL;
		if(_symbol_inserted.second){			
			symbol_ptr = _symbol_inserted.first->second = new _SymbolType(_K);
			list<__uint>& line_numbers = symbol_ptr->getLineNumberList();
			if(source_ptr!=NULL)				
				line_numbers.push_back(source_ptr->lineNumber());						
		}else{
			if(source_ptr!=NULL){
				symbol_ptr = _symbol_inserted.first->second;
				list<__uint>& line_numbers = symbol_ptr->getLineNumberList();
				if(line_numbers.size() > 0 && line_numbers.back()!=source_ptr->lineNumber())
					line_numbers.push_back(source_ptr->lineNumber());				
			}
		}
		return symbol_ptr;
	}

	LPSYMBOL find(const _Key& _K)
	{
		map<_Key, _SymbolType*>::iterator it_symb = symbol_tbl.find(_K);
		if(it_symb != symbol_tbl.end()){
			LPSYMBOL symbol_ptr = it_symb->second;
			if(source_ptr!=NULL){
				list<__uint>& line_numbers = symbol_ptr->getLineNumberList();			
				if(line_numbers.size() > 0 && line_numbers.back()!=source_ptr->lineNumber())
					line_numbers.push_back(source_ptr->lineNumber());
			}
			return symbol_ptr;
		}
		return 0;
	}
	bool erase(const _Key& _K)
	{
		map<_Key, _SymbolType*>::iterator it_symb = symbol_tbl.find(_K);
		if(it_symb != symbol_tbl.end())
			return (symbol_tbl.erase(it_symb->first) > 0);
		return false;
	}

	bool replace_symbol(_SymbolType* old_symbol, _SymbolType* new_symbol)
	{
		map<_Key, _SymbolType*>::iterator it_symb = symbol_tbl.find(old_symbol->getName());
		if(it_symb != symbol_tbl.end())
		{
			delete it_symb->second;
			it_symb->second = new_symbol;
			return true;
		}
		return false;		
	}

	LPSYMBOL check_insert(const _Key& _K)
	{
		LPSYMBOL symbol_ptr = find(_K);
		if(symbol_ptr==NULL)
			return insert(_K);
		//flag redefinition...
		HccErrorManager::Error(HccErrorManager::errIdentifierRedeclared, _K);
	}

	friend ostream& operator<<(ostream& ostr, SymbolTable<_Key, _SymbolType>& sytbl)
	{
		map<_Key, _SymbolType*>::iterator it_symb = sytbl.symbol_tbl.begin();
		while(it_symb != sytbl.symbol_tbl.end())
		{			
			ostr << _T("Symbol: ")
				 << *it_symb->second << endl;
			it_symb++;
		}
		return ostr;
	}
	friend source_unit_listing& operator<<(source_unit_listing& unit_lister, 
											SymbolTable<_Key, _SymbolType>& sytbl)
	{
		map<_Key, _SymbolType*>::iterator it_symb = sytbl.symbol_tbl.begin();
		while(it_symb != sytbl.symbol_tbl.end())
		{
			if(it_symb->second->getType()!=HCC_KEYWORD)
			{
				unit_lister << _T("Symbol: ")
					 << *it_symb->second 
					 << _endl;
			}
			it_symb++;
		}
		return unit_lister;
	}

	const __tstring& name() const
		{return the_name;}
	void set_source_buffer(source_buffer* _source_ptr = 0)
		{source_ptr = _source_ptr;}
public:
	static int __asm_label_index;
public:
	iterator begin(){ 
		return symbol_tbl.begin();
	}
	iterator end(){
		return symbol_tbl.end();
	}
};

//forward declaration
class TypeSpecifier;

/*
  This class gives information about how an identifier was defined in the source,
  and allows to hold the data that represents it.
  This info is later used by the Back End to generate code.
*/
class DeclDefinition
{
	StorageSpecifierType	stgType;	//auto, register, static, extern	
	DeclarationType			declType;	//undefined, variable, constant, new type, new function, etc.
	IdentifierScopeType		scopeType;	//local, global, heap
	MemberAccessType		accessType;	//public, protected, private
	Symbol* symbol_ptr;
	bool is_external_symbol;
public:
	DeclDefinition(Symbol* _symbol_ptr,
					DeclarationType			__declType	= DECL_VARIABLE, 
					StorageSpecifierType	__stgType	= STG_AUTO,
					IdentifierScopeType		__scopeType = SCOPE_GLOBAL,
					MemberAccessType		__accessType = ACCESS_NONE) : 
													//
													symbol_ptr(_symbol_ptr),
													declType(__declType), 
													stgType(__stgType),
													scopeType(__scopeType),
													accessType(__accessType),
													is_external_symbol(false) {}
	~DeclDefinition(){}

public:
	const StorageSpecifierType storage_specifier() const
		{return stgType;}
	const DeclarationType identifier_type() const
		{return declType;}
	const IdentifierScopeType identifier_scope() const
		{return scopeType;}
	const MemberAccessType	member_access_type() const
		{return accessType;}

	void set_storage_specifier(const StorageSpecifierType& __stgType)
		{stgType = __stgType;}
	void set_identifier_type(const DeclarationType& __declType)
		{declType = __declType;}
	void set_identifier_scope(const IdentifierScopeType& __scopeType)
		{scopeType = __scopeType;}
	void set_member_access_type(const MemberAccessType& __accessType)
		{accessType = __accessType;}

	void set_is_external(bool is_extern = false)
		{is_external_symbol = is_extern;}
	bool symbol_is_external() const
		{return is_external_symbol;}
	
	//for constant declarations
	class constant_type
	{
		constant_type(const constant_type&);
		constant_type& operator=(const constant_type&);
	public:
		constant_type(){}
		~constant_type(){}
	public:
		DataValueType value;
	} constant;

	class variable_type
	{
		variable_type(const variable_type&);
		variable_type& operator=(const variable_type&);
	public:
		variable_type(){}
		~variable_type(){}
	public:
		DataValueType init_value;
	} variable;

	//for function declarations
	class function_type
	{		
		function_type(const function_type&);
		function_type& operator=(const function_type&);
	public:
		function_type() :symbol_table_ptr(0), 
						 current_offset(0), 
						 return_type(0), 
						 fnType(HCC_USER_DEFINED),
						 bReturnTypeIsPointer(false) {}
		~function_type()
			{delete symbol_table_ptr;}
	public:
		FunctionType	fnType;			//user-defined, new, destroy, static_cast(), other...
		TypeSpecifier* return_type;
		bool bReturnTypeIsPointer;
		struct _locals
		{
			StackParams			stack_params;	//function parameters
			StackVariables		stack_variables;//function local variables
			StackDefinedTypes	stack_def_types;//function user-defined types
			StackConstants		stack_constants;//function local constants
		} locals;

		//LOCAL_IDS_TYPE locals; //will not be used!
		//a local symbol table
		SymbolTable<__tstring, Symbol>* symbol_table_ptr;
		//the current offset to assign to the next variable...
		long current_offset;
		//used when a property get/put/both is defined to hold the symbol of the real property function...
		//map<__tstring, Symbol*> property;
		class property_type
		{
			property_type(const property_type&);
			property_type& operator=(const property_type&);
			Symbol* get_ptr;
			Symbol* put_ptr;
			Symbol* not_a_property_accessor;
		public:
			property_type() : get_ptr(0), put_ptr(0), not_a_property_accessor(0) {}

			Symbol*& operator[](const __tstring& name)
			{
				if(name==_T("get"))
					return get_ptr;
				else if(name==_T("put"))
					return put_ptr;
				else
					assert(0);
				return not_a_property_accessor;
			}

		}property;
	} function;		

	//for variables/data members
	class user_data_storage
	{
		user_data_storage(const user_data_storage&);
		user_data_storage& operator=(const user_data_storage&);
	public:
		user_data_storage() : offset(0), bDataMemberIsPointer(false) {}
		~user_data_storage(){}
	public:
		bool bDataMemberIsPointer; //to flag whether this data member is a pointer to another type
		long offset;			//used when determining one of these cases:
		//						for variables and params: sequence position
		//						for data members: byte offset in class/struct

	} user_data;

	friend source_unit_listing& operator<<(source_unit_listing& unit_lister, DeclDefinition& declDefn)
	{
		unit_lister << _T("Decl-Definition:");
		switch(declDefn.identifier_type())
		{
		
		case DECL_CONSTANT:			unit_lister << _T("Defined Constant:\t");	break;	//for all constant declarations
		case DECL_NEW_TYPE:			unit_lister << _T("New Type/Class/Struct:\t");	break;	//for class and struct keywords (will be the same behavior for now)		
		case DECL_VARIABLE:			unit_lister << _T("Defined Variable:\t");	break;	//for all variables
		case DECL_NEW_FUNC_MEMBER:	unit_lister << _T("Function:\t");			break;	//a function in a class/struct
		case DECL_NEW_DATA_MEMBER:	unit_lister << _T("Data member:\t");		break;	//an attribute in a class/struct
		case DECL_NAMESPACE:		unit_lister << _T("Namespace:\t");			break;
	/*
	DECL_NEW_FUNC, //TODO: allow in the future only member functions
	DECL_PARAM_VALUE,		//param by value
	DECL_PARAM_BYREF,		//param by reference
	DECL_USING_NAMESPACE,
	*/
		};

		if(declDefn.identifier_type()==DECL_CONSTANT || 
			declDefn.identifier_type()==DECL_VARIABLE ||
			declDefn.identifier_type()==DECL_NEW_TYPE)
		{
			unit_lister << _T("Scope:\t");
			if((declDefn.identifier_scope() & SCOPE_GLOBAL)==SCOPE_GLOBAL)
				unit_lister << _T("global");
			else if((declDefn.identifier_scope() & SCOPE_LOCAL)==SCOPE_LOCAL)
				unit_lister << _T("local") << _endl;
			
			if((declDefn.identifier_scope() & SCOPE_HEAP)==SCOPE_HEAP)
				unit_lister << _T(",(on heap)");
			unit_lister << _endl;
		}
		return unit_lister;
	}

};

class TypeSpecifier
{
	//the number of refs from other symbols (NOT USED)
	long refCount;
protected:
	DataType				dataType;	//bool, int, double, char, string, others
	DataTypeModifier		dataTypeModifier;
	DeclareSpecType			specType;	//simple, array, class, enum	
	TypeSpecifier* baseTypeSpec;		//the base type-spec for enums, and other similar types
	int data_type_size;
	SymbolTable<__tstring, Symbol>* symbol_table_ptr;
	__tstring type_name;
	TypeSpecifier* finalTypeSpec; //used in upcasting/dynamic_cast operator
public:
	TypeSpecifier(DeclareSpecType __specType = DSPEC_SIMPLE) :
								baseTypeSpec(0),								
								dataType(HCC_INTEGER),
								dataTypeModifier(HCC_NO_MODIFIER),
								data_type_size(0),
								refCount(0),
								specType(__specType),
								symbol_table_ptr(0),
								finalTypeSpec(0)
								{
									if(__specType==DSPEC_CLASS)
										symbol_table_ptr = 
													new SymbolTable<__tstring, Symbol>(_T("__class_definition__"));
								}
	~TypeSpecifier()
		{delete symbol_table_ptr;}

	SymbolTable<__tstring, Symbol>* getSymbolTable()
		{return symbol_table_ptr;}

	void setDataType(const DataType& _dataType)
		{dataType = _dataType;}
	void setDataTypeModifier(const DataTypeModifier& _dataTypeModifier)
		{dataTypeModifier = _dataTypeModifier;}

	void setDataType(const DataType& _dataType, const DataTypeModifier& _dataTypeModifier, int bytes)
		{
			dataType			= _dataType; 
			dataTypeModifier	= _dataTypeModifier;
			data_type_size		= bytes;
		}
	const DataType getDataType(void) const
		{return dataType;}
	const DataTypeModifier getDataTypeModifier() const
		{return dataTypeModifier;}
	const int getDataTypeSize() const
		{return data_type_size;}

	const DeclareSpecType	specifier() const
		{return specType;}

	void set_specifier(const DeclareSpecType& __specType)
		{specType = __specType;
			if(symbol_table_ptr==0 && __specType==DSPEC_CLASS)
				symbol_table_ptr = new SymbolTable<__tstring, Symbol>(_T("__class_definition__"));
		}

	bool is_scalar() const
		{return (specType!=DSPEC_ARRAY && specType!=DSPEC_CLASS);}
	/*
	TypeSpecifier* classFinalType()
		{return finalTypeSpec;}
	void setClassFinalType(TypeSpecifier* final_type)
		{finalTypeSpec = final_type;}
	*/
	friend source_unit_listing& operator<<(source_unit_listing& unit_lister, TypeSpecifier& typeSpec)
	{
		unit_lister << _T("Type-Specifier:");
		switch(typeSpec.specifier())
		{
		case DSPEC_SIMPLE:	//the primitive | build-in data types
			unit_lister << _T("scalar,") << _endl;
			break;
		case DSPEC_ARRAY:
			unit_lister << _T("array,") << _endl;
			break;
		case DSPEC_CLASS:
			unit_lister << _T("class,") 
						<< _endl;
			break;
		case DSPEC_ENUM:
			unit_lister << _T("enumeration,") 
						<< _endl;
			break;
		case DSPEC_NAMESPACE:
			unit_lister << _T("namespace, ") 
						<< _endl;
			break;
		};
		//print extended info		
		return typeSpec.printTypeSpecInfo(unit_lister, voVerbose);
	}
	//BEGIN - EXTENDED TYPE SPECIFIER INFO MEMBERS
	struct _enums
	{
		EnumConstants values;
		int max_enum_const_value; //the max value in the enumeration decl.
	} enumeration;

	struct _arrays
	{	//ie.: double my_array[1000];
		TypeSpecifier* pItemType;
		ArrayInitConstants init_consts; //ie.: double my_array[] = {2.1412, 3.1415, 1.12e-10, 9.99};
		int item_count;
		bool bIsDynamicArray;

		_arrays() : pItemType(0), item_count(0), bIsDynamicArray(false) {}
		~_arrays();
			/*
			{if(pItemType!=NULL && pItemType->specifier()==DSPEC_ARRAY)
				delete pItemType;
			}
		*/
	} array;

	struct _type_members
	{
		//the total size in bytes that occupies this object
		long layout_object_size;
		TypeDataMembers data_members;
		TypeDataMembers inner_types;
		TypeDataMembers function_members;

		_type_members() : layout_object_size(0){}
		~_type_members(){}
	} user_type_members;
	//END - EXTENDED TYPE SPECIFIER INFO MEMBERS

	public:
		//G E N E R A L 
		source_unit_listing& printTypeSpecInfo(source_unit_listing& unit_lister, VERBOSITY_OUTPUT_TYPE outType);
		//T Y P E - S P E C I F I C 
		source_unit_listing& printEnumTypeSpecInfo(source_unit_listing& unit_lister, VERBOSITY_OUTPUT_TYPE outType);
		source_unit_listing& printArrayTypeSpecInfo(TypeSpecifier *_typeSpecifier, 
													source_unit_listing& unit_lister, VERBOSITY_OUTPUT_TYPE outType);
		source_unit_listing& printClassTypeSpecInfo(source_unit_listing& unit_lister, VERBOSITY_OUTPUT_TYPE outType);
/*
	enum-decl:
		enum <identifier>
		{
			constant-expression-list constant-expression
		};

	constant-expression-list:
		constant-expression, | constant-assignment, | empty
		

	class-decl:
		class <identifier>
		{
			access-modifier ':' :
				data-member-declaration-list;
				function-member-declaration-list;
				function-member-definition-list;
		};

		access-modifier ':'
			private		|
			protected	|
			public

		data-member-declaration-list:
				declaration-list

		function-member-declaration-list:
				function-declaration | empty
		function-member-definition-list:
				function-definition | empty
*/
	TypeSpecifier* getBaseTypeSpec() const
		{return baseTypeSpec;}
	void setBaseTypeSpec(TypeSpecifier* __typeSpec)
		{baseTypeSpec = __typeSpec;}
	void AddRef()
		{refCount++;}
	long Release()
		{return --refCount;}
	const __tstring& getTypeName()
	{
		return type_name;
	}
	void setTypeName(const __tstring& name)
		{type_name = name;}
public:
	void setStringSpec(long string_length);
};

class __tagHCC_PARSE_TREE;


#define UNSIGNED_CHAR_NAME	_T("__@@UNSIGNED_CHAR@@__")
#define UNSIGNED_SHORT_NAME	_T("__@@UNSIGNED_SHORT@@__")
#define UNSIGNED_LONG_NAME	_T("__@@UNSIGNED_LONG@@__")

class HccTypeChecker
{
public:
	static bool FloatingPointOperands(const TypeSpecifier* type1, const TypeSpecifier* type2);
	static bool IntegerOperands(const TypeSpecifier* type1, const TypeSpecifier* type2);
	static void CheckCompatibleAssignment(const TypeSpecifier* pTargetType, 
										  const TypeSpecifier* pValueType, 
										  int error,
										  const __tstring& errInfo,
										  bool bTargetTypeIsPointer = false);
	static void CheckBoolean(const TypeSpecifier *typeSpec);
	static void CheckBoolean(const TypeSpecifier* type1, const TypeSpecifier* type2);
	static void CheckIntegerOrReal(const TypeSpecifier* type1, const TypeSpecifier* type2);
	static void CheckRelOpOperands(const TypeSpecifier* type1, const TypeSpecifier* type2);

	static	TypeSpecifier* ts_type_void_ptr;

	static	TypeSpecifier* ts_type_boolean_ptr;
	static	TypeSpecifier* ts_type_char_ptr;

	static	TypeSpecifier* ts_type_short_ptr;
	static	TypeSpecifier* ts_type_long_ptr;
	static	TypeSpecifier* ts_type_int_ptr;
	static	TypeSpecifier* ts_type_Int16_ptr;
	static	TypeSpecifier* ts_type_Int32_ptr;
	static	TypeSpecifier* ts_type_Int64_ptr;
	static	TypeSpecifier* ts_type_unsigned_ptr;
	static	TypeSpecifier* ts_type_signed_ptr;

	static	TypeSpecifier* ts_type_float_ptr;
	static	TypeSpecifier* ts_type_double_ptr;
	
	//U N S I G N E D   I N T E G E R S 
	static	TypeSpecifier* ts_type_uchar_ptr;
	static	TypeSpecifier* ts_type_ushort_ptr;
	static	TypeSpecifier* ts_type_ulong_ptr;

	static	TypeSpecifier* ts_type_string_ptr;
	static TypeSpecifier* GetResultTypeFromExprTypes(TypeSpecifier* type1, TypeSpecifier* type2);

};

class Symbol
{
	__tstring name;
	HccTokenType tokenType;
	
	list<__uint> line_numbers_list;
	double value;

	int		level;			//nesting level;
	int		label_index;	//index for code level;
	
	DeclDefinition * pdeclDefinition;
	TypeSpecifier * ptypeSpecifier;
	Symbol* class_owner_ptr;
public:
	Symbol(const __tstring _name) : 
							name(_name), //this copy MUST be from a temp stack copied param and NOT a reference
										 //to avoid AV when destroying the symbol, and removing from memory, the string
							value(0), 
							tokenType(HCC_IDENTIFIER),
							pdeclDefinition(new DeclDefinition(this)),
							ptypeSpecifier(new TypeSpecifier()),
							class_owner_ptr(0)

							{
								label_index = SymbolTable<__tstring, Symbol>::__asm_label_index++;
							}
	~Symbol(){
		if(ptypeSpecifier->specifier()==DSPEC_ARRAY && 
			ptypeSpecifier->array.pItemType==HccTypeChecker::ts_type_char_ptr){
			//this string is pointed in the DeclDefinition instance...
			delete [] pdeclDefinition->constant.value.String;
			pdeclDefinition->constant.value.String = NULL;
		}
		//
		delete pdeclDefinition;
		delete ptypeSpecifier;
		//
	}

	void setOwner(Symbol* owner_ptr)
		{class_owner_ptr = owner_ptr;}

	Symbol* getOwner() const
		{return class_owner_ptr;}

	
	//
	void setDataType(const DataType& _dataType)
		{ptypeSpecifier->setDataType(_dataType);}
	const DataType getDataType(void) const
		{return (DataType)ptypeSpecifier->getDataType();}

	void setType(const HccTokenType& _tokenType)
		{tokenType = _tokenType;}
	const HccTokenType getType(void) const
		{return tokenType;}

	void setScopeType(const IdentifierScopeType& _scopeType)
		{assert(pdeclDefinition!=NULL);
		 pdeclDefinition->set_identifier_scope(_scopeType);}

	const IdentifierScopeType getScopeType(void) const
		{assert(pdeclDefinition!=NULL);
		 return pdeclDefinition->identifier_scope();}

	//these members setter/getter for symbol value, will be removed in the future
	//they will be replaced by the TypeSpecifier extended type info members
	void setValue(const double& _value)
		{ value = _value;}
	const double getValue()
		{return value;}

	list<__uint>& getLineNumberList()
		{return line_numbers_list;}

	const __tstring& getName() const
		{return name;}
	const __tstring getCompleteName()
		{
			__tstring final_name;
			if(class_owner_ptr!=NULL)
			{
				//NOTE: for member functions only!
				//this is necessary to solve the use of a complete name for the 
				//code generation process...
				//a function in a class || //a virtual function in a class || //a property in a class
				DECLARATION_TYPE func_decl_type = getDeclDefinition().identifier_type();
				if(
					func_decl_type==DECL_NEW_FUNC_MEMBER			||
					func_decl_type==DECL_NEW_VIRTUAL_FUNC_MEMBER	||
					func_decl_type==DECL_READONLY_PROPERTY			||			//a read-only property
					func_decl_type==DECL_WRITEONLY_PROPERTY			||			//a write-only property
					func_decl_type==DECL_READWRITE_PROPERTY			||			//a read-write property					
					func_decl_type==DECL_UNIQUE_DESTRUCTOR						//the destructor
				  )
				{
					final_name = class_owner_ptr->getName();
					final_name += _T("::");
				}
			}
			/*
			   for constructors, the name of the class will suffice.
			*/
			final_name += name;
			return final_name;
		}
	const __tstring& String() const
		{return name;}

	//if this identifier is : constant, variable, function, a new type, 
	DeclDefinition& getDeclDefinition()
		{return *pdeclDefinition;}
	//if this identifier is : a simple specifier, an array, a class/struct type, an enum type
	TypeSpecifier& getTypeSpecifier()
		{return *ptypeSpecifier;}

	const int& getLabelIndex() const	//used in the code generation process
		{return label_index;}
private:
	//the symbol printing private members
	void print_var_of_member_info(source_unit_listing& unit_lister);
	void print_constant_info(source_unit_listing& unit_lister);
	void print_type_info(source_unit_listing& unit_lister);
	void print_namespace_info(source_unit_listing& unit_lister);
	void print_identifier(source_unit_listing& unit_lister);

	//P R I N T   T H E   D A T A   T Y P E   S I Z E 
	source_unit_listing& getDataTypeStr(source_unit_listing& unit_lister)
		{	
			TypeSpecifier* baseTypeSpec = ptypeSpecifier->getBaseTypeSpec();
			if(baseTypeSpec!=NULL)
			{
				 if(baseTypeSpec->specifier()==DSPEC_CLASS)
					return getDataTypeStr(unit_lister, HCC_CUSTOM_TYPE);
				 else if(baseTypeSpec->specifier()==DSPEC_ENUM)
					return getDataTypeStr(unit_lister, HCC_INTEGER);
				 else
					 return getDataTypeStr(unit_lister, baseTypeSpec->getDataType());
			}
			return getDataTypeStr(unit_lister, ptypeSpecifier->getDataType());
		}
public:
	static source_unit_listing& getDataTypeStr(source_unit_listing& unit_lister, DataType dataType)
	{
		switch(dataType)
		{
		case HCC_UNKNOWN_TYPE:
			unit_lister << _T("<unknown>, size: <unknown>");			break;
		case HCC_CHAR_TYPE:
			unit_lister << _T("char, size: ") 
						<< sizeof(TCHAR) 
						<< _T(" bytes");					
			break;
		case HCC_INTEGER:
			unit_lister << _T("integer, size: ")
						<< sizeof(int)
						<< _T(" bytes");								break;
		case HCC_FLOATING_POINT:
			unit_lister << _T("floating point, size: ")
						<< sizeof(float)
						<< _T("/")
						<< sizeof(double)
						<< _T(" bytes");								break;
		case HCC_STRING:
			unit_lister << _T("string literal, size: ") 
						<< _T("user-defined size ")
						<< _T(" bytes");	
			break;
		case HCC_BOOLEAN:
			unit_lister << _T("boolean, size: ")
						<< sizeof(bool)
						<< _T(" byte");									break;
		case HCC_CUSTOM_TYPE:
			unit_lister << _T("user-defined size: ");					break;
		default:
			assert(0);
		};
		return unit_lister;
	}
	
public:
	friend ostream& operator<<(ostream& ostr, Symbol& symbol)
	{
		assert(symbol.pdeclDefinition!=NULL);
		//TODO : convert types to string types for this output...
		ostr << _T("Symbol: ")
			 << symbol.name << endl			 
			 << _T(" Type: ")		<< symbol.tokenType			 
			 << _T(" Value: ")		<< symbol.value
			 << endl;

			 //TODO: symbol.print_identifier(ostr);

		ostr << _T("XREF^:");			 
		const int maxLNPerLine = 10;
		int LNCount = 1;
		list<__uint>::iterator it = symbol.line_numbers_list.begin();
		while(it!= symbol.line_numbers_list.end()){
			ostr << setw(4) << *it++ 
				 << _T(" ");
			if((LNCount++%maxLNPerLine)==0){
				ostr << endl;
				LNCount = 1;
			}			
		}
		ostr << endl;
		return ostr;
	}

	friend source_unit_listing& operator<<(source_unit_listing& unit_lister, Symbol& symbol)
	{
		assert(symbol.pdeclDefinition!=NULL);
		unit_lister 
			 << symbol.name << _endl
			 << _T("Type: ") 
			 << (int)symbol.tokenType			 //TODO: print friendly name
			 << _endl;

		symbol.print_identifier(unit_lister);

		unit_lister << _T("XREF^: Line Numbers:");			 
		const int maxLNPerLine = 10;
		int LNCount = 1;
		list<__uint>::iterator it = symbol.line_numbers_list.begin();
		while(it!= symbol.line_numbers_list.end()){
			unit_lister << *it++ 
						<< _T(" ");
			if((LNCount++%maxLNPerLine)==0){
				unit_lister << _endl;
				LNCount = 1;
			}			
		}
		unit_lister << _endl;
		return unit_lister;
	}	
};

typedef class __tagHCC_PARSE_TREE
{
public:
	__tagHCC_PARSE_TREE(Symbol*	__symbol, 
		HCC_TOKEN_TYPE __token_op = HCC_TOKEN_ERROR) : left_ptr(0), 
													   right_ptr(0),
													   type_ptr(0),
													   symbol_ptr(__symbol), 
													   token_op(__token_op),
													   expr1_ptr(0),
													   expr2_ptr(0), 
													   expr3_ptr(0),
													   _is_pointer_expr(false){
		if(symbol_ptr!=NULL){
			type_ptr = symbol_ptr->getTypeSpecifier().getBaseTypeSpec();
			assert(type_ptr!=NULL);

			_is_pointer_expr = (symbol_ptr->getDeclDefinition().identifier_type()==DECL_POINTER_VARIABLE);
		}
	}
	~__tagHCC_PARSE_TREE(){
		delete left_ptr;
		delete right_ptr;
		//for ternary operator only
	    delete expr1_ptr;
	    delete expr2_ptr;
	    delete expr3_ptr;

	}

public:
	__tagHCC_PARSE_TREE* left_ptr;		//left operand
	__tagHCC_PARSE_TREE* right_ptr;		//right operand

	//for expr in statements
	__tagHCC_PARSE_TREE* expr1_ptr;
	__tagHCC_PARSE_TREE* expr2_ptr;
	__tagHCC_PARSE_TREE* expr3_ptr;

	HCC_TOKEN_TYPE		 token_op;		//Operand/Operator (HCC_NUMBER, HCC_IDENTIFIER, HCC_CHARACTER, HCC_STRING_LITERAL, HCC_X_OP's)
	Symbol*				 symbol_ptr;	//Operand's symbol
	TypeSpecifier*		 type_ptr;		//Expr type, 

private:
	volatile bool		 _is_pointer_expr;

public:
	const bool is_pointer_expr() const
		{return _is_pointer_expr;}
	void set_is_pointer_expr(bool value)
		{_is_pointer_expr = value;}

}HCC_PARSE_TREE, *LPHCC_PARSE_TREE;


typedef SymbolTable<__tstring, Symbol> SYMBOL_TABLE;
typedef SymbolTable<__tstring, Symbol>* LPSYMBOL_TABLE;

extern SYMBOL_TABLE g_symbol_table;

#endif //__HCC_CORE_SYMBOLS_h__