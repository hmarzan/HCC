// HCCExecutor.cpp: implementation of the HCCExecutor class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HCCExecutor.h"
#include "HCCLib\errors.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
HCCExecutor::~HCCExecutor()
{}

LPHCC_TOKEN HCCExecutor::getToken()
{	
	delete token_ptr;
	token_ptr	= icode_ptr->getToken();
	token_type	= token_ptr->tokenType;
	symbol_ptr	= icode_ptr->symbol();
	return 0;
}

//--------------------------------------------------------
// Run		- Executes every statement serially
//			  and evaluates each expression.
//--------------------------------------------------------
void HCCExecutor::Run()
{
	//It's understood that the code in the icode is only correct code
	//so, we don't validate here anything but execute every statement
	//and evaluate every expression.
	//
	if(HccErrorManager::errorCount() > 0)
		return;
	//reset the cursor pointer from the icode object...
	icode_ptr->reset();
	//ask for the first token
	getToken();
	ExecuteStatementList(HCC_EOF);
	delete token_ptr;
	cout << _T("Execution completed!")
		 << endl
		 << setw(6)
		 << m_nStatementCount
		 << _T(" statements executed.")
		 << endl;
}

void HCCExecutor::ExecuteStatement()
{
	if(token_type!=HCC_LBLOCK_KEY)
		m_nStatementCount++;

	switch(token_type)
	{
		case HCC_IDENTIFIER:
		case HCC_NUMBER:
		case HCC_INCREMENT:
		case HCC_DECREMENT:
		{
			ExecuteExprList();
		}
		break;
	case HCC_LBLOCK_KEY:	ExecuteCompound();	break;
	case HCC_DO:			ExecuteDoWhile();	break;
	case HCC_WHILE:			ExecuteWhile();		break;
	case HCC_FOR:			ExecuteForStmt();	break;
		//
	case HCC_IF:
	case HCC_SWITCH:
	
	case HCC_WITH:
		HccErrorManager::RuntimeError(HccErrorManager::rteUnimplementedFeature);
		break;
	};
}

void HCCExecutor::ExecuteAssignment(Symbol* sy_variable)
{
	volatile bool bIsOutput = sy_variable->String()==_T("output");
	//determine the type of assignment operator if any...		
	HCC_TOKEN_TYPE oper = token_type;
	switch(oper)
	{
	case HCC_INCREMENT:
		{
			getToken();
			// ++ | --
			double dvalue = sy_variable->getValue(); 
			run_stack.push(dvalue);			
			//set the value to the Symbol's variable	
			dvalue++;
			sy_variable->setValue(dvalue);
		}
		break;
	case HCC_DECREMENT:
		{
			getToken();
			double dvalue = sy_variable->getValue(); 
			run_stack.push(dvalue);			
			//set the value to the Symbol's variable					
			dvalue--;
			sy_variable->setValue(dvalue);
		}
		break;

	case HCC_ASSIGN_OP:			// =
		{			
			//skip the assignment operator/semicolon...
			getToken();
			//parse an expression...
			ExecuteExprList();
			//set the value to the Symbol's variable and pop it off the runtime stack...						
			sy_variable->setValue(run_stack.top());
			run_stack.pop();
			goto SHOW_ASSIGNMENT_RESULTS;			
		}
		break;
	case HCC_INC_ASSIGN:		// +=
		{
			//skip the assignment operator/semicolon...
			getToken();
			//parse an expression...
			ExecuteExprList();
			//set the value to the Symbol's variable and pop it off the runtime stack...			
			double value = sy_variable->getValue();
			value += run_stack.top();
			sy_variable->setValue(value);
			run_stack.pop();
			goto SHOW_ASSIGNMENT_RESULTS;			
		}
		break;
	case HCC_DEC_ASSIGN:		// -=
		{
			//skip the assignment operator/semicolon...
			getToken();
			//parse an expression...
			ExecuteExprList();
			//set the value to the Symbol's variable and pop it off the runtime stack...			
			double value = sy_variable->getValue();
			value -= run_stack.top();
			sy_variable->setValue(value);
			run_stack.pop();
			goto SHOW_ASSIGNMENT_RESULTS;			
		}
		break;
	case HCC_MUL_ASSIGN:		// *=
		{
			//skip the assignment operator/semicolon...
			getToken();
			//parse an expression...
			ExecuteExprList();
			//set the value to the Symbol's variable and pop it off the runtime stack...			
			double value = sy_variable->getValue();
			value *= run_stack.top();
			sy_variable->setValue(value);
			run_stack.pop();
			goto SHOW_ASSIGNMENT_RESULTS;
		}
		break;
	case HCC_DIV_ASSIGN:		// /=
		{
			//skip the assignment operator/semicolon...
			getToken();
			//parse an expression...
			ExecuteExprList();
			//set the value to the Symbol's variable and pop it off the runtime stack...			
			double value = sy_variable->getValue();
			value /= run_stack.top();
			sy_variable->setValue(value);
			run_stack.pop();
			goto SHOW_ASSIGNMENT_RESULTS;
		}
		break;
	case HCC_MOD_ASSIGN:		//%=
		{
			//skip the assignment operator/semicolon...
			getToken();
			//parse an expression...
			ExecuteExprList();
			//set the value to the Symbol's variable and pop it off the runtime stack...			
			__int64 ivalue = sy_variable->getValue();
			ivalue %= (__int64)run_stack.top();
			sy_variable->setValue(ivalue);
			run_stack.pop();
			goto SHOW_ASSIGNMENT_RESULTS;
		}
		break;
	case HCC_XOR_ASSIGN:		//^=
		{
			//skip the assignment operator/semicolon...
			getToken();
			//parse an expression...
			ExecuteExprList();
			//set the value to the Symbol's variable and pop it off the runtime stack...
			__int64 ivalue = sy_variable->getValue();
			ivalue ^= (__int64)run_stack.top();
			sy_variable->setValue(ivalue);
			run_stack.pop();
			goto SHOW_ASSIGNMENT_RESULTS;
		}
		break;
	case HCC_BIT_OR_ASSIGN:		// |=
		{
			//skip the assignment operator/semicolon...
			getToken();
			//parse an expression...
			ExecuteExprList();
			//set the value to the Symbol's variable and pop it off the runtime stack...
			__int64 ivalue = sy_variable->getValue();
			ivalue |= (__int64)run_stack.top();
			sy_variable->setValue(ivalue);
			run_stack.pop();
			goto SHOW_ASSIGNMENT_RESULTS;
		}
		break;
	case HCC_BIT_AND_ASSIGN:	// &=
		{
			//skip the assignment operator/semicolon...
			getToken();
			//parse an expression...
			ExecuteExprList();
			//set the value to the Symbol's variable and pop it off the runtime stack...
			__int64 ivalue = sy_variable->getValue();
			ivalue &= (__int64)run_stack.top();
			sy_variable->setValue(ivalue);
			run_stack.pop();
			goto SHOW_ASSIGNMENT_RESULTS;
		}
		break;
	
	default:
		{
			run_stack.push(sy_variable->getValue());
		}
		break;
	};
SHOW_ASSIGNMENT_RESULTS:
	//output the debugging results...
	if(bIsOutput)	
		listing << _T("[")
				<< icode_generator::currentLineNumber()
				<< _T("]\t")
				<< _T(">>") 				
				<< sy_variable->getName()
				<< _T(" == ")
				<< sy_variable->getValue()				
				<< _T(";")
				<< _endl;
}

void HCCExecutor::ExecuteExprList()
{
	//simple expression
	double right_operand = 0.0;
	double left_operand	 = 0.0;
	int nResult = 0;
	ExecuteExpr();
	HCC_TOKEN_TYPE oper = token_type;
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
	case HCC_AND_OP:			// &&
	case HCC_OR_OP:				// || 
	case HCC_TERNARY_OP:		//?
	case HCC_COMMA_OP:			// ','
		do{
			if(token_type==HCC_TERNARY_OP)
				goto PARSE_TERNARY_OPERATOR;
			oper = token_type; //used at the end of this evaluation...
			getToken();			
			//simple expression
			if(oper==HCC_AND_OP			|| oper==HCC_OR_OP		|| 
				oper==HCC_BIT_AND_OP	|| oper==HCC_BIT_OR_OP	|| 
				oper==HCC_XOR_OP		|| oper==HCC_COMMA_OP)
				ExecuteExprList();
			else
				ExecuteExpr();			

			if(oper!=HCC_COMMA_OP)
			{
				right_operand	= run_stack.top();
				run_stack.pop();
				left_operand	= run_stack.top();
				run_stack.pop();
			}else{
				if(token_type==HCC_COMMA_OP)
				{
					run_stack.pop();
					if(run_stack.size() > 0) run_stack.pop();
				}
			}
			nResult = 0; //here we have an integer value as a result of these operands
			switch(oper)
			{
				case HCC_LESS_OP:			//<
					nResult = (left_operand < right_operand);	
					run_stack.push(nResult);
					break;
				case HCC_LESS_EQ_OP:		//<=
					nResult = (left_operand <= right_operand);					
					run_stack.push(nResult);
					break;
				case HCC_GREATER_OP:		//>
					nResult = (left_operand > right_operand);					
					run_stack.push(nResult);
					break;
				case HCC_GTER_EQ_OP:		//>=
					nResult = (left_operand >= right_operand);					
					run_stack.push(nResult);
					break;
					//L E S S   P R E C E D E N C E 
				case HCC_EQUAL_OP:			//==
					nResult = (left_operand == right_operand);					
					run_stack.push(nResult);
					break;
				case HCC_NOT_EQ_OP:			//!=
					nResult = (left_operand != right_operand);					
					run_stack.push(nResult);
					break;
					//B I T W I S E   P R E C E D E N C E 
				case HCC_BIT_AND_OP:		// &
					nResult = ((int)left_operand & (int)right_operand);					
					run_stack.push(nResult);
					break;
				case HCC_XOR_OP:			// ^
					nResult = ((int)left_operand ^ (int)right_operand);					
					run_stack.push(nResult);
					break;
				case HCC_BIT_OR_OP:			// |
					nResult = ((int)left_operand | (int)right_operand);					
					run_stack.push(nResult);
					break;
				case HCC_AND_OP:			// &&
					nResult = (left_operand!=0.0 && right_operand!=0.0);					
					run_stack.push(nResult);
					break;
				case HCC_OR_OP:
					nResult = (left_operand!=0.0 || right_operand!=0.0);					
					run_stack.push(nResult);
					break;
			};						
			
			if(token_type==HCC_TERNARY_OP)
			{
PARSE_TERNARY_OPERATOR:
				getToken();
				// expr1 ? expr2 : expr3
				bool leftVal = run_stack.top() > 0;
				run_stack.pop();
				if(leftVal){
					ExecuteExprList(); //expr2
					if(token_type==HCC_COLON){
						getToken();
						while(token_type!=HCC_SEMICOLON && 
							  token_type!=HCC_EOF)
							  getToken();
						if(token_type==HCC_EOF)
							HccErrorManager::Error(HccErrorManager::errUnexpectedEndOfFile);
					}
				}
				else{
					while(token_type!=HCC_COLON && 
						  token_type!=HCC_EOF)
						  getToken();				
					if(token_type==HCC_COLON){
						getToken();
						ExecuteExprList(); //expr3
					}
				}
			};
			//continue evaluation if the current operator is (and | or)...
			//note: this loop is critical and necessary because of parentesis-less relational expression
			//like a > 1 || b==0 && c < a;
		}while(token_type==HCC_AND_OP		|| 
			   token_type==HCC_OR_OP		|| 
			   token_type==HCC_BIT_AND_OP	|| 
			   token_type==HCC_BIT_OR_OP	|| 
			   token_type==HCC_XOR_OP		||
			   token_type==HCC_COMMA_OP);
		break;
	};
}

void HCCExecutor::ExecuteExpr()
{

	HCC_TOKEN_TYPE unaryOp = HCC_PLUS_OP;

	if(token_type==HCC_MINUS_OP || 
		token_type==HCC_PLUS_OP){
		unaryOp = token_type;
		//skip this sign token
		getToken();
	}
	//left operator
	ExecuteTerm();
	
	if(unaryOp == HCC_MINUS_OP  && !run_stack.empty()){		
		double value = run_stack.top();
		run_stack.pop();
		run_stack.push(-value);		
	}
	
	for(;;)
		switch(token_type)
	{
	case HCC_PLUS_OP:		//+
	case HCC_MINUS_OP:		//-
	case HCC_LEFT_SHIFT_OP:			// <<
	case HCC_RIGHT_SHIFT_OP:		// >>			
		{
			HCC_TOKEN_TYPE oper = token_type;
			//skip this operator
			getToken();
			//the right operator
			ExecuteTerm();			
			double dright = run_stack.top();
			run_stack.pop();

			double dleft = run_stack.top();
			run_stack.pop();
			//put result back to the runtime stack...
			if(oper==HCC_PLUS_OP)
				run_stack.push(dleft + dright);			
			else if(oper==HCC_MINUS_OP)
				run_stack.push(dleft - dright);
			else if(oper==HCC_LEFT_SHIFT_OP)
				run_stack.push((__int64)dleft << (__int64)dright);
			else if(oper==HCC_RIGHT_SHIFT_OP)
				run_stack.push((__int64)dleft >> (__int64)dright);
		}
		break;
	default:
		return;
		break;
	};
}

void HCCExecutor::ExecuteTerm()
{	
	//the left factor
	ExecuteFactor();
	for(;;)
		switch(token_type)
	{
	case HCC_MUL_OP:			//*
		{
			//skip this operator
			getToken();
			//the right factor
			ExecuteFactor();			
			double dright = run_stack.top();
			run_stack.pop();

			double dleft = run_stack.top();
			run_stack.pop();

			//put result back to the runtime stack...	
			run_stack.push(dleft * dright);			
		}
		break;
	case HCC_DIV_OP:			// /	
		{
			//skip this operator
			getToken();
			//the right factor
			ExecuteFactor();			
			double dright = run_stack.top();
			run_stack.pop();

			double dleft = run_stack.top();
			run_stack.pop();

			//put result back to the runtime stack...	
			if(dright!=0.0)
				run_stack.push(dleft / dright);
			else
				HccErrorManager::RuntimeError(HccErrorManager::rteDivisionByZero);
				
		}
		break;
	case HCC_DIV:			// Integer Division	
		{
			//skip this operator
			getToken();
			//the right factor
			ExecuteFactor();			
			__int64 iright = (__int64)run_stack.top();
			run_stack.pop();

			__int64 ileft = (__int64)run_stack.top();
			run_stack.pop();

			//put result back to the runtime stack...	
			if(iright!=0)
				run_stack.push(double(ileft / iright));
			else
				HccErrorManager::RuntimeError(HccErrorManager::rteDivisionByZero);
		}
		break;
	case HCC_MOD_OP:			//%
		{
			//skip this operator
			getToken();
			//the right factor
			ExecuteFactor();			
			__int64 iright = (__int64)run_stack.top();
			run_stack.pop();

			__int64 ileft = (__int64)run_stack.top();
			run_stack.pop();

			//put result back to the runtime stack...	
			//modulus is only define for integer types; that's why the cast...
			if(iright!=0)
				run_stack.push(double(ileft % iright));
			else
				HccErrorManager::RuntimeError(HccErrorManager::rteDivisionByZero);
		}
		break;
	default:
		return;
		break;
	};
}

SYMBOL_TABLE::LPSYMBOL HCCExecutor::ExecuteFactor()
{
	SYMBOL_TABLE::LPSYMBOL sy_variable = NULL;
	switch(token_type)
	{
		case HCC_NUMBER:
		{		
			if(token_ptr->dataType==HCC_FLOATING_POINT)
				run_stack.push(token_ptr->value.Double);
			else if(token_ptr->dataType==HCC_INTEGER)
				run_stack.push((double)token_ptr->value.Integer);
			else
				assert(!"Cannot operate with this data type yet!");
			//skip this number
			getToken();
		}
		break;
		case HCC_IDENTIFIER:
		{
			sy_variable = symbol_table_ptr->find(token_ptr->String());			
			if(sy_variable!=NULL){
				if(sy_variable!=sy_input_ptr){
					getToken();
					ExecuteAssignment(sy_variable);					
				}else{					
					cout << _T(">> At ") 
						<< icode_generator::currentLineNumber()
						 << _T(" input? :");
					
					double value = 0.0;
					/*This code always fails in Release Builds!
					cin >> value;					
					if(!cin)
						HccErrorManager::RuntimeError(HccErrorManager::rteInvalidUserInput);
					*/
					char line[100 + 1];
					fgets(line, 100, stdin);
					line[100] = '\0';
					value = atof(line);
					if(ferror(stdin))
						HccErrorManager::RuntimeError(HccErrorManager::rteInvalidUserInput);
					sy_variable->setValue(value);
					run_stack.push(value);
					//skip this identifier...
					getToken();
				}
			}			
			else{
				//should never get here!
				assert(0);
			}
		}
		break;
		case HCC_LPAREN:
		{
			//skip this '('
			getToken();			
			ExecuteExprList();
			//skip this ')'
			getToken();			
		}
		break;
		case HCC_STRING_LITERAL:
		{
			//do nothing yet!
		}
		break;
		//P R E F I X   O P E R A T O R S  (T H E   H I G H E S T   P R E C E D E N C E )
		case HCC_NOT_OP:		//!  : !1;   == -1
		case HCC_INCREMENT:		//++ : ++id; == (id = id + 1)
		case HCC_DECREMENT:		//-- : --id; == (id = id - 1)
		case HCC_COMPL_OP:		//~  : ~0x0; == 0xFFFFFFFF;

		{
			//these operators are only allowed in integer identifiers
			HCC_TOKEN_TYPE oper = token_type;
			getToken();
			sy_variable = ExecuteFactor();
			double value = 0;
			if(oper==HCC_NOT_OP)
				value = (run_stack.top()!=0.0 ? 0.0 : 1.0);
			else if(oper==HCC_INCREMENT){
				value = (run_stack.top() + 1);
				if(sy_variable!=NULL)
					sy_variable->setValue(value);
			}else if(oper==HCC_DECREMENT){
				value = (run_stack.top() - 1);
				if(sy_variable!=NULL)
					sy_variable->setValue(value);
			}else if(oper==HCC_COMPL_OP){
				value = ~(int)run_stack.top();
			}		
			run_stack.pop();
			run_stack.push(value);			
		}
		break;
		default:
			//should never get here...
			break;
	};
	
	return sy_variable;
}

void HCCExecutor::ExecuteStatementList(HCC_TOKEN_TYPE terminator)
{
	do{
		ExecuteStatement();
		//skip this token
		while(token_type==HCC_SEMICOLON)
			getToken();
		//execute the next statement...
	}while(token_type!=terminator);
}

void HCCExecutor::ExecuteDoWhile()
{
	int atLoopStart = get_pos();//get_pos();
	do{
		getToken(); //do
		if(token_type==HCC_LBLOCK_KEY)
			ExecuteCompound(); // do{ statement-list }while(expr);
		else{
			ExecuteStatement(); // do statement; while(expr);
			getToken(); //the ';'
		}

		assert(token_type==HCC_WHILE);
		getToken(); //while
		ExecuteExprList();
		//evaluate the expression result...
		if(run_stack.top() != 0.0)
			set_pos(atLoopStart);
		run_stack.pop();
	}while(atLoopStart == get_pos());
}

void HCCExecutor::ExecuteCompound()
{
	getToken(); //{
	ExecuteStatementList(HCC_RBLOCK_KEY);
	getToken(); //}
}


void HCCExecutor::ExecuteWhile()
{	
	int atLoopStart = get_pos();
	assert(token_type==HCC_WHILE);
	while(atLoopStart==get_pos())
	{		
		getToken();	//while
		ExecuteExprList();
		if(run_stack.top() != 0.0)
		{
			run_stack.pop();
			//eval expr
			if(token_type==HCC_LBLOCK_KEY)
				ExecuteCompound();
			else{
				ExecuteStatement();
				getToken(); //the ';'
			}
			set_pos(atLoopStart);
		}else{
			run_stack.pop();
			if(token_type==HCC_LBLOCK_KEY)
			{
				getToken(); // '{'
				while(token_type!=HCC_RBLOCK_KEY)
					getToken();
				getToken(); // '}'
			}else{
				while(token_type!=HCC_SEMICOLON)
					getToken();
			}
		}		
	}
}

void HCCExecutor::ExecuteForStmt()
{	
	assert(token_type==HCC_FOR);
	getToken(); //for
	getToken(); // '('
	ExecuteExprList(); //expr1
	assert(token_type==HCC_SEMICOLON);	
	int atLoopStart = get_pos();	
	volatile bool bInfinite = false;
	while(atLoopStart == get_pos())
	{		
		getToken(); // ';'
		bInfinite = (token_type==HCC_SEMICOLON);
		ExecuteExprList();		
		//the 3rd expression location...
		int atExpr3Start = get_pos();
		getToken(); // ';'
		//skip the third expression (this one is evaluated after processing the <statement-list> )
		while(token_type!=HCC_RPAREN)
			getToken();
		getToken(); // ')'
		if(bInfinite || run_stack.top() != 0.0)
		{
			if(!bInfinite)
				run_stack.pop();
			//eval expr
			if(token_type==HCC_LBLOCK_KEY)
				ExecuteCompound();
			else{
				ExecuteStatement();
				getToken(); //the ';'
			}
			//evaluate the 3rd expression...
			set_pos(atExpr3Start);
			getToken(); // ';'
			ExecuteExprList();
			getToken(); // ')'
			//set location to evaluate the 2nd expression of this loop...
			set_pos(atLoopStart);
		}else{
			run_stack.pop();
			if(token_type==HCC_LBLOCK_KEY)
			{
				getToken(); // '{'
				while(token_type!=HCC_RBLOCK_KEY)
					getToken();
				getToken(); // '}'
			}else{
				while(token_type!=HCC_SEMICOLON)
					getToken();
			}			
		}
	}
}
