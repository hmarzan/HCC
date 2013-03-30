#include "coresymbols.h"

#include "errors.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;	//this is private/unique by file
#endif

SYMBOL_TABLE g_symbol_table(_T("__all_hpp_program_identifiers"));

//to keep the order in which the user/programmer declared the global variables
//to avoid unordered variable initialization. 
GlobalVariables g_program_global_variables;

//treat const definitions as global variables too.
GlobalConstants g_program_global_constants;

int SYMBOL_TABLE::__asm_label_index = 0;

void Symbol::print_var_of_member_info(source_unit_listing& unit_lister)
{
	assert(ptypeSpecifier!=NULL);
	unit_lister << getTypeSpecifier()
				<< _T(" data type: ");

	getDataTypeStr(unit_lister) 
		<< (
			(getDeclDefinition().identifier_type()==DECL_NEW_DATA_MEMBER) ?  			
			_T(", member: ") :
			_T(", variable: ") 
			)
		<< String()		
		<< _endl;
}

void Symbol::print_type_info(source_unit_listing& unit_lister)
{
	assert(ptypeSpecifier!=NULL);
	unit_lister << getTypeSpecifier();
}

void Symbol::print_namespace_info(source_unit_listing& unit_lister)
{
	unit_lister << _T("namespace: ") << getName();
}

void Symbol::print_constant_info(source_unit_listing& unit_lister)
{
	TypeSpecifier* baseTypeSpec = ptypeSpecifier->getBaseTypeSpec();
	if(baseTypeSpec!=NULL && baseTypeSpec==ptypeSpecifier)
		unit_lister << *baseTypeSpec;
	else
		unit_lister << getTypeSpecifier();	

	if(baseTypeSpec!=NULL)
	{
		if(baseTypeSpec->getDataType()==HCC_INTEGER || 
			baseTypeSpec->specifier()==DSPEC_ENUM)
		{
			unit_lister << _T("integer:\t") << (int)pdeclDefinition->constant.value.Integer;
		}
		//
		else if(baseTypeSpec->getDataType()==HCC_FLOATING_POINT)
			unit_lister << _T("double:\t") << pdeclDefinition->constant.value.Double;
		//
		else if(baseTypeSpec->getDataType()==HCC_CHAR_TYPE)
			unit_lister << _T("char:\t") << pdeclDefinition->constant.value.Character;		
		//
		else if(baseTypeSpec->getDataType()==HCC_STRING_TYPE || 
				baseTypeSpec->specifier()==DSPEC_ARRAY)
		{
			unit_lister << _T("string:\t") << pdeclDefinition->constant.value.String;
		}

	}else
	{
		if(ptypeSpecifier->getDataType()==HCC_INTEGER || 
			ptypeSpecifier->specifier()==DSPEC_ENUM)
		{
			unit_lister << _T("integer:\t") << (int)pdeclDefinition->constant.value.Integer;
		}
		//
		else if(ptypeSpecifier->getDataType()==HCC_FLOATING_POINT)
			unit_lister << _T("double:\t") << pdeclDefinition->constant.value.Double;
		//
		else if(ptypeSpecifier->getDataType()==HCC_CHAR_TYPE)
			unit_lister << _T("char:\t") << pdeclDefinition->constant.value.Character;		
		//
		else if(ptypeSpecifier->getDataType()==HCC_STRING_TYPE || 
				ptypeSpecifier->specifier()==DSPEC_ARRAY)
		{
			unit_lister << _T("string:\t") << pdeclDefinition->constant.value.String;
		}
	}
	unit_lister << _endl;
}

void Symbol::print_identifier(source_unit_listing& unit_lister)
{
	assert(pdeclDefinition!=NULL);

	unit_lister << getDeclDefinition();
		switch(getDeclDefinition().identifier_type())
		{
		
		case DECL_CONSTANT:	print_constant_info(unit_lister);		break;	//for all constant declarations
		case DECL_NEW_TYPE:	print_type_info(unit_lister);			break;	//for class and struct keywords (will be the same behavior for now)		
		//
		case DECL_VARIABLE:													//for all variables		
		case DECL_NEW_DATA_MEMBER:											//an attribute in a class/struct		
			print_var_of_member_info(unit_lister);
			break;
	/*
	DECL_NEW_FUNC_MEMBER:											//a function in a class/struct
	DECL_PARAM_VALUE,		//param by value
	DECL_PARAM_BYREF,		//param by reference
	*/
		};

}

TypeSpecifier::_arrays::~_arrays()
{
	if(pItemType!=NULL && 
		pItemType->specifier()==DSPEC_ARRAY && 
		pItemType!=HccTypeChecker::ts_type_string_ptr)
	{
		delete pItemType;
	}
}

void TypeSpecifier::setStringSpec(long string_length)
{	
	setDataType(HCC_STRING_TYPE);
	setTypeName(HccTypeChecker::ts_type_string_ptr->getTypeName());
	specType = DSPEC_ARRAY;
	array.item_count = string_length;	
	array.pItemType = HccTypeChecker::ts_type_char_ptr;
	//because this is an array-spec, its base is itself too!
	setBaseTypeSpec(HccTypeChecker::ts_type_string_ptr);
	data_type_size = string_length * HccTypeChecker::ts_type_char_ptr->getDataTypeSize();
}

source_unit_listing& TypeSpecifier::printTypeSpecInfo(source_unit_listing& unit_lister, VERBOSITY_OUTPUT_TYPE outType)
{
	switch(specifier())
	{
	case DSPEC_SIMPLE:	//the primitive | build-in data types		
		//do nothing for scalar types!
		break;
	case DSPEC_ARRAY:
		printArrayTypeSpecInfo(this, unit_lister, outType);
		break;
	case DSPEC_CLASS:
		printClassTypeSpecInfo(unit_lister, outType);
		break;
	case DSPEC_ENUM:
		printEnumTypeSpecInfo(unit_lister, outType);
		break;
	case DSPEC_NAMESPACE:
		//TODO
		break;
	};	
	return unit_lister;
}


//-------------------------------------------------------
//	printEnumTypeSpecInfo	-	prints info about the enumeration type and its constants.
//
//-------------------------------------------------------
source_unit_listing& TypeSpecifier::printEnumTypeSpecInfo(source_unit_listing& unit_lister, VERBOSITY_OUTPUT_TYPE outType)
{
	//outType
	if(outType==voMinimal)
		return unit_lister;
	//
	if(enumeration.values.size() > 0)
	{
		unit_lister << _T("begin\t- enumeration list: ") 
					<< _endl
					<< _T("{")
					<< _endl;

		EnumConstants::iterator it = enumeration.values.begin();
		while(it != enumeration.values.end())
		{
			Symbol* symbol_ptr = *it++;
			unit_lister << symbol_ptr->String()
						<< _T("\t\t= ")
						<< (int)symbol_ptr->getDeclDefinition().constant.value.Integer
						<< _endl;
		}
		unit_lister << _T("};")
					<< _endl
					<< _T("end\t- enumeration list: ")
					<< _endl
					<< _endl;
	}
	//
	return unit_lister;
}

//-------------------------------------------------------
//	printArrayTypeSpecInfo  - prints recursively, the array type info
//
//-------------------------------------------------------
source_unit_listing& TypeSpecifier::printArrayTypeSpecInfo(TypeSpecifier *_typeSpecifier, source_unit_listing& unit_lister, VERBOSITY_OUTPUT_TYPE outType)
{
	//this member can be called recursively specially for
	//multidimensional arrays
	if(outType==voMinimal)
		return unit_lister;

	if(_typeSpecifier->array.pItemType!=NULL && 
		_typeSpecifier->array.pItemType->specifier()==DSPEC_ARRAY)
		unit_lister << _T("multi-dimensional ");

	unit_lister << _T(" of size = ")
				<< _typeSpecifier->array.item_count
				<< _T(" elements, of type: ");
	
	if(_typeSpecifier->array.pItemType!=NULL && 
		_typeSpecifier->array.pItemType->specifier()==DSPEC_ARRAY){
		unit_lister << _T("array\n, with specs :") 
					<< _endl;
		printArrayTypeSpecInfo(_typeSpecifier->array.pItemType, unit_lister, outType);
	}else{
		Symbol::getDataTypeStr(unit_lister, _typeSpecifier->array.pItemType->getDataType())
					<< _endl
					<< _endl;
	}
	//
	return unit_lister;
}

//-------------------------------------------------------
//	printClassTypeSpecInfo	- prints the class/struct type info and its data members/function members
//
//-------------------------------------------------------
source_unit_listing& TypeSpecifier::printClassTypeSpecInfo(source_unit_listing& unit_lister, VERBOSITY_OUTPUT_TYPE outType)
{
	if(outType==voMinimal)
		return unit_lister;

	unit_lister << _T("begin\t- data members (offset :: name): ")
				<< _T("{")
				<< _endl;
	TypeDataMembers::iterator it = user_type_members.data_members.begin();
	while(it != user_type_members.data_members.end())
	{
		Symbol* symbol_ptr = *it++;
		unit_lister << symbol_ptr->getDeclDefinition().user_data.offset
					<< _T("\t== ")
					<< symbol_ptr->String()
					<< _endl;
	}
	unit_lister << _T("};")
				<< _endl
				<< _T("end\t- data members: ")
				<< _endl
				<< _endl;
	
	return unit_lister;
}

void HccTypeChecker::CheckRelOpOperands(const TypeSpecifier *type1, const TypeSpecifier *type2)
{
//	type1 = type1->getBaseTypeSpec();
//	type2 = type2->getBaseTypeSpec();
	assert(type1!=0);
	assert(type2!=0);

	//BEGIN - hlm March 20, 2011 
		if(type1==NULL || type2==NULL)
			return; //should send a warning about this issue of a undeclared variable (type == NULL).
	//END - hlm March 20, 2011 
	//for identical scalar types, and for enumration types
	if(type1->getDataType()==type2->getDataType() &&
				(type1->is_scalar() || type1->specifier()==DSPEC_ENUM))
		return;

	//integer and real operands...
	if( (type1->getDataType()==HCC_INTEGER && type2->getDataType()==HCC_FLOATING_POINT) ||
		(type1->getDataType()==HCC_FLOATING_POINT && type2->getDataType()==HCC_INTEGER)
		)
		return;

	//two strings of the same length...
	if(
		(type1->specifier() == DSPEC_ARRAY && 
			type2->specifier() == DSPEC_ARRAY) 
		&&
		(type1->array.pItemType == HccTypeChecker::ts_type_char_ptr &&
			type2->array.pItemType == HccTypeChecker::ts_type_char_ptr)
		&&
		(type1->array.item_count == type2->array.item_count)
		)
		return;

	//BEGIN - FIXED Jan 5, 2009
	if(type1->specifier()==DSPEC_CLASS && 
		type2->specifier()==DSPEC_CLASS)
	{
		if(type1==type2)
			return;

		if(type1->getDataTypeSize()==type2->getDataTypeSize())
			return;
	}
	//END - FIXED Jan 5, 2009

	//BEGIN - FIXED Jan 10, 2009
	if(type1->specifier()==DSPEC_ARRAY && type1->array.bIsDynamicArray &&
		type2->getDataType()==HCC_INTEGER && type2->getDataTypeSize()==sizeof(int))
			return;
	else if(type2->specifier()==DSPEC_ARRAY && type2->array.bIsDynamicArray && 
		   type1->getDataType()==HCC_INTEGER && type1->getDataTypeSize()==sizeof(int))
			return;

	//END - FIXED Jan 10, 2009

	HccErrorManager::Error(HccErrorManager::errIncompatibleTypes, _T(" comparison of these types is ilegal."));	
}

void HccTypeChecker::CheckIntegerOrReal(const TypeSpecifier *type1, const TypeSpecifier *type2)
{
	assert(type1!=0);

	if(type1->getDataType() != HCC_INTEGER && type1->getDataType() != HCC_FLOATING_POINT)
	{
		HccErrorManager::Error(HccErrorManager::errIncompatibleTypes, _T(" comparison of these types is ilegal."));	
		return;
	}

	assert(type2!=0);
	if(type2->getDataType() != HCC_INTEGER && type2->getDataType() != HCC_FLOATING_POINT)
	{
		HccErrorManager::Error(HccErrorManager::errIncompatibleTypes, _T(" comparison of these types is ilegal."));			
	}
}

void HccTypeChecker::CheckBoolean(const TypeSpecifier *type1, const TypeSpecifier *type2)
{
	assert(type1!=0);
	assert(type2!=0);

	if(type1!=HccTypeChecker::ts_type_boolean_ptr || type2!=HccTypeChecker::ts_type_boolean_ptr)
		HccErrorManager::Error(HccErrorManager::errIncompatibleTypes);

}

void HccTypeChecker::CheckBoolean(const TypeSpecifier *typeSpec)
{
	assert(typeSpec!=0);

	if(typeSpec!=HccTypeChecker::ts_type_boolean_ptr)
		HccErrorManager::Error(HccErrorManager::errMustBeBooleanExpression);
}


void HccTypeChecker::CheckCompatibleAssignment(const TypeSpecifier* pTargetType, 
												const TypeSpecifier* pValueType, 
												int error,
												const __tstring& errInfo, bool bTargetTypeIsPointer)
{
	if(pValueType==pTargetType)
		return;

	// t1   t1 tx  t2  ---->  tx = (t1==t2)
	// |    |  |   |
	//int   a  =   4;

	//are type compatibles
	if((pTargetType->is_scalar() && pValueType->is_scalar()) && 
		pTargetType->getDataType()==pValueType->getDataType())
		return;

	//   t1  t1  tx t2  ---> tx = (t1(double) && t2(int))
	//   |   |   |  | 
	//double v1  =  15;
	if(pTargetType->getDataType()==HCC_FLOATING_POINT && 
		pValueType->getDataType()==HCC_INTEGER)
		return;

	//NOTE: this code will be removed when new techniques be learned to handle strings
	//two strings of the same length...
	if(
		(pTargetType->specifier() == DSPEC_ARRAY && 
			pValueType->specifier() == DSPEC_ARRAY) 
		&&
		(pTargetType->array.pItemType == HccTypeChecker::ts_type_char_ptr &&
			pValueType->array.pItemType == HccTypeChecker::ts_type_char_ptr)
		
		)
		return;

	//BEGIN - ADDED Jan 7 2009
	if(bTargetTypeIsPointer)
	{
		if(pValueType->specifier() == DSPEC_ARRAY)
		{
			//Type [] arrayOfType = new Type[100];
			//Type^ obj = arrayOfType;
			if(pTargetType==pValueType->array.pItemType)
				return;

			//now, evaluate to this type
			pValueType = pValueType->array.pItemType;
		}else if(pValueType->specifier()==DSPEC_SIMPLE)
		{
			//BEGIN - ADDED Jan 7 2009
			if(pValueType->getDataType()==HCC_INTEGER && pValueType->getDataTypeSize()==sizeof(int))
				return;
			//END - ADDED Jan 7 2009
		}
	}
	//END - ADDED Jan 7 2009
	if(pValueType->getBaseTypeSpec()==pTargetType)
		return;

	//BEGIN - FIXED Jan 4, 2008
	TypeSpecifier* base_type_ptr = pValueType->getBaseTypeSpec();
	//if base and type are the same, we got to the top of the hierarchy
	while(base_type_ptr!=NULL && 
		  base_type_ptr!=base_type_ptr->getBaseTypeSpec())
	{
		base_type_ptr = base_type_ptr->getBaseTypeSpec(); //get the next base type in the hierarchy...
		if(base_type_ptr==pTargetType)
			return; //the type was found!
	}
	//END - FIXED Jan 4, 2008

	__tstring finalErrorInfo = _T(": cannot convert from \'");
	if(pValueType!=NULL){

		finalErrorInfo += const_cast<TypeSpecifier*>(pValueType)->getTypeName();
		if(pValueType->getDataType()!=HCC_STRING_TYPE)
		{
			if(pValueType->specifier() == DSPEC_ARRAY && pValueType->array.pItemType!=NULL){
				pValueType = pValueType->array.pItemType;
				finalErrorInfo += const_cast<TypeSpecifier*>(pValueType)->getTypeName();
				finalErrorInfo += _T("[]");
			}
		}
	}
	else
		finalErrorInfo += ts_type_int_ptr->getTypeName();

	finalErrorInfo += _T("\' to \'");
	if(pTargetType!=NULL){
		finalErrorInfo += const_cast<TypeSpecifier*>(pTargetType)->getTypeName();
		if(pTargetType->getDataType()!=HCC_STRING_TYPE)
		{
			if(pTargetType->specifier() == DSPEC_ARRAY && pTargetType->array.pItemType!=NULL){
				pTargetType = pTargetType->array.pItemType;
				finalErrorInfo += const_cast<TypeSpecifier*>(pTargetType)->getTypeName();
				finalErrorInfo += _T("[]");
			}
		}
	}
	else
		finalErrorInfo += ts_type_int_ptr->getTypeName();
	finalErrorInfo += _T("\' ");
	finalErrorInfo += errInfo;

	HccErrorManager::Error(HccErrorManager::HccError(error), finalErrorInfo);
}

bool HccTypeChecker::IntegerOperands(const TypeSpecifier *type1, const TypeSpecifier *type2)
{
	assert(type1!=0);
	assert(type2!=0);
	if(type1->getDataType()==HCC_INTEGER && (type2->getDataType()==HCC_INTEGER || type2->getDataType()==HCC_BOOLEAN))
		return true;
	if(type1->getDataType()==HCC_BOOLEAN && (type2->getDataType()==HCC_BOOLEAN || type2->getDataType()==HCC_INTEGER))
		return true;

	return false;
}

bool HccTypeChecker::FloatingPointOperands(const TypeSpecifier *type1, const TypeSpecifier *type2)
{
	assert(type1!=0);
	assert(type2!=0);

	return (
		    (type1->getDataType()==HCC_FLOATING_POINT && type2->getDataType()==HCC_FLOATING_POINT)
			||
			(type1->getDataType()==HCC_FLOATING_POINT && type2->getDataType()==HCC_INTEGER)
			||
			(type1->getDataType()==HCC_INTEGER && type2->getDataType()==HCC_FLOATING_POINT)
			);
}

TypeSpecifier* HccTypeChecker::GetResultTypeFromExprTypes(TypeSpecifier *type1, TypeSpecifier *type2)
{
	TypeSpecifier* resultTypeSpec = NULL;
	if(type1==type2)
		return type1;
	else if(IntegerOperands(type1, type2))
		return HccTypeChecker::ts_type_Int64_ptr;
	else if(FloatingPointOperands(type1, type2))
		return HccTypeChecker::ts_type_double_ptr;
	else if(
		(type1->specifier() == DSPEC_ARRAY && 
			type2->specifier() == DSPEC_ARRAY) 
		&&
		(type1->array.pItemType == HccTypeChecker::ts_type_char_ptr &&
			type2->array.pItemType == HccTypeChecker::ts_type_char_ptr)
		)
		return type1;
	else
		return HccTypeChecker::ts_type_int_ptr;
}
