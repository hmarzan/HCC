#include "errors.h"
#include "corecommon.h"
#include <iostream>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;	//this is private/unique by file
#endif


using namespace std;

/*
	enum AbortCode
	{
		abortInvalidCommandLineArgs	= -1,
		abortSourceFileOpenFailed	= -2,
		abortICodeFileOpenFailed	= -3, //NOTE: not used for this moment
		abortAssemblyFileOpenFailed	= -4,

		abortTooManySyntaxErrors	= -5,

		//RunTime Errors
		abortStackOverflow			= -6,		
		abortDivisionByZero			= -7,
		abortAccessViolation		= -8,

		abortCodeSegmentOverflow	= -9,
		abortNestingTooDeep			= -10,
		abortUnimplementedFeature	= -11,		
	};

*/

__tstring abortMessage[] = {
			_T(""),
			_T("Invalid command line arguments."),
			_T("Failed to open source file."),
			_T("Source program entry point not found."),

			_T("Failed to open assembly file."),
			_T("Too many syntax errors."),
			//Run time Errors
			_T("Stack overflow. The Stack Frame was corrupted before ending."),
			_T("Division by Zero."),
			_T("Access Violation (reading | writing) address."),

			_T("Code segment overflow."),
			_T("Nesting too deep."),
			_T("Unimplemented feature requested."),
};


/*
	enum LexerError
	{
		errNOError,
		errUnexpectedEndOfFile,
		errUnrecognizable,
		errTooManySyntaxErrors,
		errInvalidNumber,
		errInvalidHexNumber,
		errInvalidFraction,
		errInvalidExponent,
		errFloatingPointOutOfRange,
		errIntegerOutOfRange,
		errTooManyDigits,

		errMissingRightParen,
		errMissingLeftParen,
		errMissingStartBlock,
		errMissingEndBlock,

		errMissingLeftBracket,
		errMissingRightBracket,
		errInvalidExpression,
		errInvalidAssignment,

		errUndeclaredIdentifier,
		errRedeclaredIdentifier,

		errStackOverflow,
		errInvalidStatement,
		errUnexpectedToken,
		errMissingSemicolon,
		errMissingComma,
		errMissingDo,
		errMissingWhile,
		errMissingCaseInSwitch,
		errInvalidForControlVariable,

		errInvalidConstant,
		errMissingColon,
		errMissingDoubleColon,
		errMissingEqual,
		errMissingOperator,
		errInvalidType,
		errNotATypeIdentifier,
		errInvalidSubscriptType,
		errIncompatibleTypes, 

		errCannotAddTwoPointers,
		errMissingPeriod,
		errMissingArrowOp,
		errTooManySubscripts,
		errInvalidField,
		errNestingLoopTooDeep,
		errMissingMain,
		errAlreadyImportedLib,
		errMissingPrototype,
		errWrongNumberOfArguments,
		errInvalidReferenceParams,
		errNotAnObject,
		errNotAStruct,
		errCodeSegmentOverflow,
		errUnimplementedFeature,
		errMissingConstantExpression,
		errMissingCatchExceptBlock,
		errLabelRedefined,		
		errVariableIsNotArray,
		errBitShiftingOperandsMustBeIntegers,
		errMustBeIntegerOperands,
		errMustBeBooleanExpression,
		errNoAccessToPrivateMember,
		errNoAccessToProtectedMember,
		errIdentifierNotAFunction,
		errFunctionWithNoParams,
		errFunctionDoesNotTakeXParams,
		errVirtualDataMemberNotAllowed,
		errExternDataMemberNotAllowed,
		errExpectedLibraryFileName,
		errUnsignedUseWithIntegersOnly,
		errSignedUseWithIntegersOnly,
	};

*/

__tstring errorMessages[] = {
	_T("No Errors found "),
	_T("Unexpected end of file."),	
	_T("Unrecognizable input "),
	_T("Too many syntax errors "),
	_T("Invalid number "),
	_T("Invalid Hex number "),
	_T("Invalid fraction "),
	_T("Invalid exponent "),
	_T("Floating point value out of range "),
	_T("Integral type out of range."),
	_T("Number with too many digits."),
	_T("Identifier exceeds the max token size:"),
	_T("Missing \')\' "),
	_T("Missing \'(\' "),
	_T("Missing \'{\' "),
	_T("Missing \'}\' "),
	_T("Missing \'[\' "),
	_T("Missing \']\' "),
	_T("Invalid expression "),
	_T("Invalid assignment "),
	_T("Invalid declaration "),
	_T("Undeclared identifier "),
	_T("Identifier redeclared "),
	_T("Stack overflow "),
	_T("Invalid statement "),
	_T("Unexpected token "),
	_T("Missing \';\' "),
	_T("Missing \',\' "),
	_T("Missing \'do\' "),
	_T("Missing \'while\' "),
	_T("Missing \'case\' in switch statement "),
	_T("Missing for control variable."),
	_T("Invalid constant "),
	_T("Missing \':\' "),
	_T("Missing \'::\' "),
	_T("Missign \'==\'"),
	_T("Missing operator "),
	_T("Invalid type "),
	_T("Not a type identifier "),
	_T("Invalid subscript or index type"),
	_T("Subscript ouf of range, or out of array boundaries."), //errSubscriptOutOfRange
	_T("Incompatible types "),
	_T("Cannot add two pointers."),
	_T("Missing \'.\' "),
	_T("Missing \'->\' "),
	_T("Too many subscripts "),
	_T("Invalid object instance member, or member not found."),
	_T("Nesting loop too deep "),
	_T("Missing \'main\' entry point function."),
	_T("This library is already imported "),
	_T("Missing prototype for function "),
	_T("Wrong number of arguments "),
	_T("Invalid reference parameter "),
	_T("This identifier is not an object "),
	_T("This identifier is not a structure "),
	_T("Code segment overflow."),
	_T("Unimplemented feature "),
	_T("Missing constant expression."),
	_T("Missing identifier "),
	_T("Missing catch/except block."),
	_T("Label redefined "),
	_T("Variable token is not an array (array has no dimensions)."),
	_T("Bit shifting operands must be integer values (>> | <<)."),
	_T("div/mod(%)/and(&)/xor(^) operators must have integer operands."),
	_T("Must be a boolean expression."),
	_T("Cannot access private member from this object."),
	_T("Cannot access protected member from this object."),
	_T("Identifier is not a function, or was not defined (undeclared function identifier)."),
	_T("Function does not receive parameters."),
	_T("Function does not take "), //n paramters
	_T("Cannot define a \'virtual\' data member, only member functions. Such concept does not exist."),
	_T("Cannot define an \'extern\' data member. Only global variables can have this modifier."),		//errExternDataMemberNotAllowed
	_T("Cannot define an \'extern\' local variable. Only global variables can have this modifier."),	//errExternLocalVariableNotAllowed
	_T("Expected a library string name in double quotation, for the function to be imported."),
	_T("unsigned keyword can only be used with integer types (char, int, short, long)."),
	_T("signed keyword can only be used with integer types (char, int, short, long)."),
	_T("Missing base type in class definition."),				//errMissingBaseType
	_T("Cannot inherit a new type from Build-In types."),		//errCannotInheritFromBuildInType,
	_T("Cannot inherit a new type from a constant/variable."),	//errCannotInheritFromConstOrVariable,
	_T("Multiple Inheritance is not supported."), //errMultipleInheritanceNotSupported
	_T("Cannot inherit a new type from itself."), //errCannotInheritFromItSelf
	_T("Member function must return a value. Incomplete definition."),
	_T("Functions can only be defined in the boundaries of a class definition."), //errNoFunctionsOutSideOfClassDefinition
	_T("Statements must dwell in the scope of a function only."), //errStatementsMustBeInFunctionScope
	_T("Function override in sub-class must have the same return value."),	//errFunctionOverrideMustHaveSameReturnValue,
	_T("Function override in sub-class must have the same parameter list."),	//errFunctionOverrideMustHaveSameParamList,
	_T("Parameter type mismatch in new class\'s function override."),	//errFunctionOverrideParamTypesMismatch,
	_T("Identifier must be a l-value in an assignment expression."),
	_T("Cannot have a storage specifier in a constructor definition (avoid: \'static\', \'extern\', etc.)."), //errConstructorWithStorageSpecifier
	_T("Cannot have multiple constructor overloads in this current compiler version."),
	_T("Destructor cannot return a value."),
	_T("Cannot have a storage specifier in a destructor definition (avoid: \'static\', \'extern\', etc.)."), //errDestructorWithStorageSpecifier
	_T("Already found a destructor for this class."), //errCannotHaveMultipleDestructors
	_T("Property redefinition; symbol already redeclared"), //errPropertyRedefinition
	_T("Cannot define a property with this Identifier; it's already used in other declarations."), //errUsedIdentifierInPropertyDefinition
	_T("Cannot assign a value or expression to a read-only property."), //errReadOnlyPropertyInvalidAssignment
	_T("Cannot create an object instance from an abstract class (has virtual abstract members) "), //errAbstractClassInstantiation
	_T("Cannot call a virtual abstract member function. This member must have an implementation in a derived sub-class."), //errAbstractClassFunctionMemberCall
	_T("Invalid dynamic array dimensions or not compatibles "), //errInvalidDynamicArrayDimension
	_T("Invalid use of operator \'destroy\'; can only be applied to dynamically allocated pointers and arrays."),
	_T("Invalid pointer expression "),
	_T("Invalid \'main\' or entry point function parameter list."), //errInvalidMainParamList
	_T("Identifier is ambiguous in the current scope "), //errFoundAmbiguousIdentifier
	_T("Member function must have an object. Cannot call instance members from a static function without an object."), //errCannotCallInstanceMemberFromStatic
	_T("Too many \'default\' blocks in this \'switch\' statement."), //errTooManySwitchDefaultBlocks,
	_T("\'default\' switch-block must be the last one in a \'switch\' statement."), //errDefaultBlockMustBeLastInSwitch,
	_T("Invalid procedure label. Cannot use identifier as block destination "), //errInvalidProcLabel
	_T("Invalid variable type used with this operator"), //errWrongTypeForThisOperator
	_T("Wrong array dimension for this argument"), //errWrongArgumentArrayDimensions
	_T("Class needs at least default constructor when creating objects using operator new "), //errMissingDefaultConstructorWithNewOp
	_T("Expected type or instance variable when type-casting using operator dynamic_cast "), //errExpectedClassTypeOrVariable
	_T("Cannot apply operator dynamic_cast on scalar types."), //errDynamicCastOnScalarType
	_T("Expected a pointer variable in dynamic_cast operator "), //errExpectedPointerVariable
	_T("Destructors cannot have a parameter list."), //errDestructorCannotHaveParamList
	_T("Cannot assign a pointer or pointer array to a fully defined array like "), //errCannotAssignToFullyDefinedArray
};

/*
	enum RunTimeError
	{
		rteNone,
		rteStackOverflow,
		rteValueOutOfRange,
		rteInvalidCaseValue,
		rteDivisionByZero,
		rteInvalidFunctionArgument,
		rteInvalidUserInput,
		rteUnimplementedFeature,
	};
*/

__tstring runtimeErrorMessages[] = {
	_T("No runtime errors."),
	_T("Runtime stack overflow."),
	_T("Value is out of range."),
	_T("Invalid \'case\' expression value."),
	_T("Division by zero."),
	_T("Invalid function argument."),
	_T("Invalid user input."),
	_T("Unimplemented runtime feature.")
};

//global error counter
int HccErrorManager::m_nGlobalErrorCount		= 0;
//errors by specific unit
map<__tstring, int> HccErrorManager::m_mapErrorCount;
int HccErrorManager::m_nMaxSyntaxErrors = 10;
bool HccErrorManager::m_bShowErrorArrowFlag = true;

source_buffer* HccErrorManager::source_file_ptr = NULL;

void HccErrorManager::AbortTranslation(AbortCode ac)
{
	cout << _T("FATAL COMPILER ERROR:\n")
		 << abortMessage[-ac] 
		 << _T(" Translation aborted.")
		 << endl;
	exit(ac);
}

HccErrorManager::HccErrorManager()
{

}

/*
	enum HccWarning
	{
		warnVirtualFunctionOverrideInSubClass,
	};
*/

source_buffer* HccWarningManager::source_file_ptr = NULL;
unsigned int HccWarningManager::nWarningsCount = 0;

__tstring warningMessage[] = {
	_T("virtual function override in new derived class. Virtual member stripped from derived class's Vtable."),
	_T("a property put should not return a value; ignoring the return expression."), //warnReturningValueFromPropertyPut
	_T("Unreferenced label: "), //warnUnreferencedProcLabel
	_T("Implicit conversion from floating point to integer, possible loss of data "), //warnImplicitConversionPossibleLossOfData
	_T("Using dynamic_cast with abstract classes, undefined behavior."), //warnAbstractTypeUsingDynamicCast
	_T("Must use destroy [] identifier to call object destructors in array of objects."), //warnMustUseBracketsForArrayObjectsDtorCall
	_T("H++ allows signed Int64 only."), //warnHppInt64IsSignedAlways
};