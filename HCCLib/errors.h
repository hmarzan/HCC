#ifndef __HCC_SOURCE_ERRORS_h__
#define __HCC_SOURCE_ERRORS_h__
/*
********************************************************************************************************
*																									   *
*																									   *
*	MODULE			: <errors.h>																	   *	
*																									   *
*	DESCRIPTION		: HCC Compiler error classes (all source unit error types are defined here.)	   *
*					  All HCC errors are defined here.												   *
*																									   *
*	AUTHOR			: Harold L. Marzan																   *	
*																									   *
*	LAST-MODIFIED	: (unknown)																		   *	
*																									   *
********************************************************************************************************
*/

#include "corelisting.h"
#include "coreicode.h"

#include <map>

using namespace std;

extern __tstring errorMessages[];
extern __tstring runtimeErrorMessages[];

class HccErrorManager
{
	static int m_nGlobalErrorCount;
	static map<__tstring, int> m_mapErrorCount;
	static int m_nMaxSyntaxErrors;
	static bool m_bShowErrorArrowFlag;
	static source_buffer* source_file_ptr;
public:
	HccErrorManager();

	//C O M P I L E R   E R R O R S 
	enum AbortCode
	{
		abortInvalidCommandLineArgs	= -1,

		abortSourceFileOpenFailed	= -2,

		abortEntryPointNotFound		= -3,

		abortAssemblyFileOpenFailed	= -4,

		abortTooManySyntaxErrors	= -5,
		//RunTime Errors
		abortRuntimeError			= -6,
		abortStackOverflow			= -7,		
		abortDivisionByZero			= -8,
		abortAccessViolation		= -9,

		abortCodeSegmentOverflow	= -10,
		abortNestingTooDeep			= -11,
		abortUnimplementedFeature	= -12,		
	};

	enum HccError 
	{
		errNOError,
		errUnexpectedEndOfFile,
		errUnrecognizable,
		errTooManySyntaxErrors,

		//LexerErrors
		errInvalidNumber,
		errInvalidHexNumber,
		errInvalidFraction,
		errInvalidExponent,
		errFloatingPointOutOfRange,
		errIntegerOutOfRange,
		errTooManyDigits,
		errTokenExceedsMaxSize,

		//ParserErrors
		errMissingRightParen,
		errMissingLeftParen,
		errMissingStartBlock,
		errMissingEndBlock,
		errMissingLeftBracket,
		errMissingRightBracket,
		errInvalidExpression,
		errInvalidAssignment,
		errInvalidDeclaration,

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
		errSubscriptOutOfRange,
		errIncompatibleTypes,
		errCannotAddTwoPointers,
		errMissingPeriod,
		errMissingArrowOp,
		errTooManySubscripts,
		errInvalidObjectMember,
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
		errMissingIdentifier,
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
		errExternLocalVariableNotAllowed,
		errExpectedLibraryFileName,		
		errUnsignedUseWithIntegersOnly,
		errSignedUseWithIntegersOnly,
		errMissingBaseType,
		errCannotInheritFromBuildInType,
		errCannotInheritFromConstOrVariable,
		errMultipleInheritanceNotSupported,
		errCannotInheritFromItSelf,
		errFunctionMustReturnValue,
		errNoFunctionsOutSideOfClassDefinition,
		errStatementsMustBeInFunctionScope,
		errFunctionOverrideMustHaveSameReturnValue,
		errFunctionOverrideMustHaveSameParamList,
		errFunctionOverrideParamTypesMismatch,
		errMustBe_LValue_Identifier,
		errConstructorWithStorageSpecifier,
		errMultipleConstructorOverloads,
		errDestructorCannotReturnAValue,
		errDestructorWithStorageSpecifier,
		errCannotHaveMultipleDestructors,
		errPropertyRedefinition,
		errUsedIdentifierInPropertyDefinition,
		errReadOnlyPropertyInvalidAssignment,
		errAbstractClassInstantiation,
		errAbstractClassFunctionMemberCall,
		errInvalidDynamicArrayDimension,
		errCanOnlyDestroyPointerOrArrayTypes,
		errInvalidPointerExpression,
		errInvalidMainParamList,
		errFoundAmbiguousIdentifier,
		errCannotCallInstanceMemberFromStatic,
		errToManySwitchDefaultBlocks,
		errDefaultBlockMustBeLastInSwitch,
		errInvalidProcLabel,
		errWrongTypeForThisOperator,
		errWrongArgumentArrayDimensions,
		errMissingDefaultConstructorWithNewOp,
		errExpectedClassTypeOrVariable,
		errDynamicCastOnScalarType,
		errExpectedPointerVariable,
		errDestructorCannotHaveParamList,
		errCannotAssignToFullyDefinedArray,
	};

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

	//error count by unit
	static const int errorCount()	
	{if(source_file_ptr!=NULL)
		return m_mapErrorCount[source_file_ptr->sourceFile()];
		return 0;
	}
	//global compilation error count
	static const int globalErrorCount()
	{return m_nGlobalErrorCount;}
	static void IncErrorCount()	
	{m_nGlobalErrorCount++; 
		if(source_file_ptr!=NULL)
			m_mapErrorCount[source_file_ptr->sourceFile()]++;
	}
	static void AbortTranslation(AbortCode ac);

	static void ShowArrowOnErrors(void)
		{m_bShowErrorArrowFlag = true;}
	static void HideArrowOnErrors(void)
		{m_bShowErrorArrowFlag = false;}


	static void Error(HccError error, const __tstring& information = _T(""))
	{
		IncErrorCount();		
		int errorArrowOffset = 8;
		TCHAR text[1024];
		if(m_bShowErrorArrowFlag){
			int errorPos = errorArrowOffset + source_buffer::inputPosition() - 1;			
			_stprintf(text, _T("%*s^"), errorPos, " ");
			listing << text << _T('\n');

			//print the error message...
			_stprintf(text, _T("error: %s"), errorMessages[error].c_str());
			listing << text << information << _T('\n');

		}else{
			if(source_file_ptr!=NULL)
			{
				//print the error message...
				_stprintf(text, _T("%s{%ld} : error: %s"), source_file_ptr->sourceFile(),
														   source_file_ptr->lineNumber(),														   
														   errorMessages[error].c_str());
				listing << text << information << _T('\n');
			}
		}

		if(m_nGlobalErrorCount > m_nMaxSyntaxErrors)		
			AbortTranslation(abortTooManySyntaxErrors);
	}

	static void RuntimeError(RunTimeError error)
	{
		cout << endl
			 << _T("Runtime error: ")
			 << runtimeErrorMessages[error]
			 << endl;
		exit(abortRuntimeError);
	}

	//to be used for error line and file highlighting
	static void SetSourceFilePtr(source_buffer* __source_file_ptr)
	{
		source_file_ptr = __source_file_ptr;
	}
};

typedef HccErrorManager::HccError ParserError;


extern __tstring warningMessage[];

extern bool bWarningsDisabled;

class HccWarningManager
{
	static source_buffer* source_file_ptr;
	static unsigned int nWarningsCount;
public:
	enum HccWarning
	{
		warnVirtualFunctionOverrideInSubClass,
		warnReturningValueFromPropertyPut,
		warnUnreferencedProcLabel,
		warnImplicitConversionPossibleLossOfData,
		warnAbstractTypeUsingDynamicCast,
		warnMustUseBracketsForArrayObjectsDtorCall,
		warnHppInt64IsSignedAlways,
	};
	static void Warning(HccWarning warning, __tstring resolution = _T(""))
	{
		//at this time, I've found this warning:warnVirtualFunctionOverrideInSubClass very annoying!
		if(false==bWarningsDisabled && warning!=warnVirtualFunctionOverrideInSubClass)
		{
			if(source_file_ptr!=NULL)
			{				
				HccWarningManager::nWarningsCount++;
				TCHAR text[512 + 128];
				_stprintf(text, _T("%s{%ld} : warning: %s"), source_file_ptr->sourceFile(),
															 source_file_ptr->lineNumber(),
															 warningMessage[warning].c_str());
				listing << text << resolution << _T('\n');		
			}
		}
	}
	static unsigned int getWarningsCount(void)
		{return HccWarningManager::nWarningsCount;}

	//to be used for warning line and file highlighting
	static void SetSourceFilePtr(source_buffer* __source_file_ptr)
	{
		source_file_ptr = __source_file_ptr;
	}
};

#endif //__HCC_SOURCE_ERRORS_h__