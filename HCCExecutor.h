// HCCExecutor.h: interface for the HCCExecutor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HCCEXECUTOR_H__38801D7F_E87A_4DB0_BFF9_4D52E3CBBE98__INCLUDED_)
#define AFX_HCCEXECUTOR_H__38801D7F_E87A_4DB0_BFF9_4D52E3CBBE98__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/*
********************************************************************************************************
*																									   *
*																									   *
*	MODULE			: <hccexecutor.h>																   *	
*																									   *
*	DESCRIPTION		: HCC Interpreter class (Statement and Expression Executor/Evaluator for HC)	   *
*																									   *
*	AUTHOR			: Harold L. Marzan																   *	
*																									   *
*	LAST-MODIFIED	: (unknown)																		   *	
*																									   *
********************************************************************************************************
*/

#include "HCCLib\corecommon.h"
#include "HCCLib\coresymbols.h"
#include "HCCLib\coreicode.h"
#include <cassert>


extern SYMBOL_TABLE g_symbol_table;
extern icode_generator g_icode_gen;


class HCCExecutor  
{
	SYMBOL_TABLE::LPSYMBOL symbol_ptr;
	HCC_TOKEN_TYPE token_type;
	LPHCC_TOKEN token_ptr;
	icode_generator* icode_ptr;
	LPSYMBOL_TABLE symbol_table_ptr;
protected:
	LPHCC_TOKEN getToken();
	void set_pos(int offset)
		{icode_ptr->set_pos(offset);}
	const int get_pos() const
		{return icode_ptr->get_pos();}
public:
	void Run(void);
	HCCExecutor() : symbol_table_ptr(&g_symbol_table), 
					icode_ptr(&g_icode_gen), 
					m_nStatementCount(0),
					symbol_ptr(0),
					token_ptr(0){
		sy_input_ptr		= symbol_table_ptr->find(_T("input"));
		sy_output_ptr	= symbol_table_ptr->find(_T("output"));
		assert(sy_input_ptr!=0);
		assert(sy_output_ptr!=0);
	}
	virtual ~HCCExecutor();
private:
	void ExecuteForStmt();
	void ExecuteWhile();
	void ExecuteCompound();
	void ExecuteDoWhile();
	void ExecuteStatementList(HCC_TOKEN_TYPE terminator);
	SYMBOL_TABLE::LPSYMBOL ExecuteFactor();
	void ExecuteTerm();
	void ExecuteExprList();
	void ExecuteExpr();
	void ExecuteAssignment(Symbol* sy_variable);	
	void ExecuteStatement();
	RUNTIME_STACK run_stack;
	//these are special symbols already in the symbol table...
	SYMBOL_TABLE::LPSYMBOL sy_input_ptr;
	SYMBOL_TABLE::LPSYMBOL sy_output_ptr;
	//number of statements executed
	long m_nStatementCount;
};

#endif // !defined(AFX_HCCEXECUTOR_H__38801D7F_E87A_4DB0_BFF9_4D52E3CBBE98__INCLUDED_)
