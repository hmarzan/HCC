// HCCCodeGenerator.h: interface for the HCCCodeGenerator class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HCCCODEGENERATOR_H__94130FF8_55CD_453E_83AD_A7A38795F21A__INCLUDED_)
#define AFX_HCCCODEGENERATOR_H__94130FF8_55CD_453E_83AD_A7A38795F21A__INCLUDED_

/*
********************************************************************************************************
*																									   *
*																									   *
*	MODULE			: <HCCCodeGenerator.h>															   *	
*																									   *
*	DESCRIPTION		: HCC Compiler x86 Assembly Code Generator class								   *
*					  Default Code-Optimization														   *
*																									   *
*	AUTHOR			: Harold L. Marzan																   *	
*																									   *
*	LAST-MODIFIED	: (unknown)																		   *	
*																									   *
********************************************************************************************************
*/
#include "assembly_source_unit.h"

#include "HCCLib\corecommon.h"
#include "HCCLib\coresymbols.h"
#include "HCCLib\errors.h"
#include "HCCLib\coreicode.h"

#include <stack>
#include <queue>

using namespace std;

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


//--------------------------------------------------------------
//  x86 Registers
//
//
//--------------------------------------------------------------

typedef enum __tagx86_REGISTER
{
	//32-bit registers
	EAX,	// Accumulator for operands and results data
	EBX,	// Pointer to data in the DS segment
	ECX,	// Counter for string and loop operations
	EDX,	// I/O pointer

	ESI,	// Pointer to data in the segment pointed to by the DS register; source pointer for string operations.
	EDI,	// Pointer to data (or destination) in the segment pointed to by the ES register; destination pointer for string operations.
	ESP,	// Stack pointer (in the SS segment)
	EBP,	// Pointer to data on the stack (in the SS segment)

	//16-bit registers
	AX, BX, CX, DX, 	
	SI, DI, SP, BP,
	
	//8-bit registers
	AL, BL, CL, DL,
	AH,	BH,	CH,	DH,

	//segment registers
	CS, DS, SS, ES,
	FS, GS

} x86_REGISTER, *LPx86_REGISTER;


//--------------------------------------------------------------
// x86 CPU 5.1 GENERAL-PURPOSE INSTRUCTIONS
//
//--------------------------------------------------------------

typedef enum __tagx86_INSTRUCTION
{

	//5.1.1 DATA TRANSFER INSTRUCTIONS

	MOV,		//Move data between general-purpose registers; move data between memory and general-purpose or segment registers; move immediates to general-purpose registers 
	CMOVE,
	CMOVZ,		//Conditional move if equal/Conditional move if zero
	CMOVNE,
	CMOVNZ,		//Conditional move if not equal/Conditional move if not zero
	CMOVA,
	CMOVNBE,	//Conditional move if above/Conditional move if not below or equal
	CMOVAE,
	CMOVNB,		//Conditional move if above or equal/Conditional move if not below
	CMOVB,
	CMOVNAE,	//Conditional move if below/Conditional move if not above or equal
	CMOVBE,
	CMOVNA,		//Conditional move if below or equal/Conditional move if not above
	CMOVG,
	CMOVNLE,	//Conditional move if greater/Conditional move if not less or equal
	CMOVGE,
	CMOVNL,		//Conditional move if greater or equal/Conditional move if not less	

	CMOVL,
	CMOVNGE,	// Conditional move if less/Conditional move if not greater or equal
	CMOVLE,
	CMOVNG,		// Conditional move if less or equal/Conditional move if not greater
	CMOVC,		// Conditional move if carry
	CMOVNC,		// Conditional move if not carry
	CMOVO,		// Conditional move if overflow
	CMOVNO,		// Conditional move if not overflow
	CMOVS,		// Conditional move if sign (negative)
	CMOVNS,		// Conditional move if not sign (non-negative)
	CMOVP,
	CMOVPE,		// Conditional move if parity/Conditional move if parity even
	CMOVNP,
	CMOVPO,		// Conditional move if not parity/Conditional move if parity odd
	XCHG,		// Exchange
	BSWAP,		// Byte swap
	XADD,		// Exchange and add
	CMPXCHG,	// Compare and exchange
	CMPXCHG8B,	// Compare and exchange 8 bytes
	PUSH,		// Push onto stack
	POP,		// Pop off of stack
	PUSHA,
	PUSHAD,		// Push general-purpose registers onto stack
	POPA,
	POPAD,		// Pop general-purpose registers from stack
	CWD,
	CDQ,		// Convert word to doubleword/Convert doubleword to quadword
	CBW,
	CWDE,		// Convert byte to word/Convert word to doubleword in EAX register
	MOVSX,		// Move and sign extend
	MOVZX,		// Move and zero extend


	//5.1.2 BINARY ARITHMETIC INSTRUCTIONS

	ADD,		// Integer add
	ADC,		// Add with carry
	SUB,		// Subtract
	SBB,		// Subtract with borrow

	IMUL,		// Signed multiply
	MUL,		// Unsigned multiply
	IDIV,		// Signed divide
	DIV,		// Unsigned divide
	INC,		// Increment
	DEC,		// Decrement
	NEG,		// Negate
	CMP,		// Compare

	//5.1.3 DECIMAL ARITHMETIC INSTRUCTIONS

	DAA,		// Decimal adjust after addition
	DAS,		// Decimal adjust after subtraction
	AAA,		// ASCII adjust after addition
	AAS,		// ASCII adjust after subtraction
	AAM,		// ASCII adjust after multiplication
	AAD,		// ASCII adjust before division

	//5.1.4 LOGICAL INSTRUCTIONS

	AND,		// Perform bitwise logical AND
	OR,			// Perform bitwise logical OR
	XOR,		// Perform bitwise logical exclusive OR
	NOT,		// Perform bitwise logical NOT

	//5.1.5 SHIFT AND ROTATE INSTRUCTIONS

	SAR,		// Shift arithmetic right
	SHR,		// Shift logical right
	SAL,
	SHL,		// Shift arithmetic left/Shift logical left
	SHRD,		// Shift right double
	SHLD,		// Shift left double
	ROR,		// Rotate right
	ROL,		// Rotate left
	RCR,		// Rotate through carry right
	RCL,		// Rotate through carry left

	//5.1.6 BIT AND BYTE INSTRUCTIONS

	BT,			// Bit test
	BTS,		// Bit test and set
	BTR,		// Bit test and reset
	BTC,		// Bit test and complement
	BSF,		// Bit scan forward
	BSR,		// Bit scan reverse
	SETE,
	SETZ,		// Set byte if equal/Set byte if zero
	SETNE,
	SETNZ,		// Set byte if not equal/Set byte if not zero
	SETA,
	SETNBE,		// Set byte if above/Set byte if not below or equal
	SETAE,
	SETNB,
	SETNC,		// Set byte if above or equal/Set byte if not below/Set byte if not carry
	SETB,
	SETNAE,
	SETC,		// Set byte if below/Set byte if not above or equal/Set byte if carry
	SETBE,
	SETNA,		// Set byte if below or equal/Set byte if not above
	SETG,
	SETNLE,		// Set byte if greater/Set byte if not less or equal
	SETGE,
	SETNL,		// Set byte if greater or equal/Set byte if not less

	SETL,
	SETNGE,		// Set byte if less/Set byte if not greater or equal
	SETLE,
	SETNG,		// Set byte if less or equal/Set byte if not greater
	SETS,		// Set byte if sign (negative)
	SETNS,		// Set byte if not sign (non-negative)
	SETO,		// Set byte if overflow
	SETNO,		// Set byte if not overflow
	SETPE,
	SETP,		// Set byte if parity even/Set byte if parity
	SETPO,
	SETNP,		// Set byte if parity odd/Set byte if not parity
	TEST,		// Logical compare


	//5.1.7 CONTROL TRANSFER INSTRUCTIONS

	JMP,		// Jump
	JE,
	JZ,			// Jump if equal/Jump if zero
	JNE,
	JNZ,		// Jump if not equal/Jump if not zero
	JA,
	JNBE,		// Jump if above/Jump if not below or equal
	JAE,
	JNB,		// Jump if above or equal/Jump if not below
	JB,
	JNAE,		// Jump if below/Jump if not above or equal
	JBE,
	JNA,		// Jump if below or equal/Jump if not above
	JG,
	JNLE,		// Jump if greater/Jump if not less or equal
	JGE,
	JNL,		// Jump if greater or equal/Jump if not less
	JL,
	JNGE,		// Jump if less/Jump if not greater or equal
	JLE,
	JNG,		// Jump if less or equal/Jump if not greater

	JC,			// Jump if carry
	JNC,		// Jump if not carry
	JO,			// Jump if overflow
	JNO,		// Jump if not overflow
	JS,			// Jump if sign (negative)
	JNS,		// Jump if not sign (non-negative)
	JPO,
	JNP,		// Jump if parity odd/Jump if not parity

	JPE,
	JP,			// Jump if parity even/Jump if parity
	JCXZ,
	JECXZ,		// Jump register CX zero/Jump register ECX zero
	LOOP,		// Loop with ECX counter
	LOOPZ,
	LOOPE,		// Loop with ECX and zero/Loop with ECX and equal
	LOOPNZ,
	LOOPNE,		// Loop with ECX and not zero/Loop with ECX and not equal
	CALL,		// Call procedure
	RET,		// Return
	RETN,		// Return pop n params from stack
	IRET,		// Return from interrupt
	INT_,		// Software interrupt
	INTO,		// Interrupt on overflow
	BOUND,		// Detect value out of range

	//5.1.8 STRING INSTRUCTIONS

	MOVS,
	MOVSB,		// Move string/Move byte string
	MOVSW,		// Move string/Move word string
	MOVSD,		// Move string/Move doubleword string
	CMPS,
	CMPSB,		// Compare string/Compare byte string
	CMPSW,		// Compare string/Compare word string
	CMPSD,		// Compare string/Compare doubleword string
	SCAS,
	SCASB,		// Scan string/Scan byte string
	SCASW,		// Scan string/Scan word string
	SCASD,		// Scan string/Scan doubleword string
	LODS,
	LODSB,		// Load string/Load byte string
	LODSW,		// Load string/Load word string
	LODSD,		// Load string/Load doubleword string
	STOS,
	STOSB,		// Store string/Store byte string
	STOSW,		// Store string/Store word string
	STOSD,		// Store string/Store doubleword string

	REP_MOVS,
	REP_MOVSB,		// Move string/Move byte string
	REP_MOVSW,		// Move string/Move word string
	REP_MOVSD,		// Move string/Move doubleword string
	REP_CMPS,
	REP_CMPSB,		// Compare string/Compare byte string
	REP_CMPSW,		// Compare string/Compare word string
	REP_CMPSD,		// Compare string/Compare doubleword string
	REP_SCAS,
	REP_SCASB,		// Scan string/Scan byte string
	REP_SCASW,		// Scan string/Scan word string
	REP_SCASD,		// Scan string/Scan doubleword string
	REP_LODS,
	REP_LODSB,		// Load string/Load byte string
	REP_LODSW,		// Load string/Load word string
	REP_LODSD,		// Load string/Load doubleword string
	REP_STOS,
	REP_STOSB,		// Store string/Store byte string
	REP_STOSW,		// Store string/Store word string
	REP_STOSD,		// Store string/Store doubleword string

	REPE_MOVS,
	REPE_MOVSB,		// Move string/Move byte string
	REPE_MOVSW,		// Move string/Move word string
	REPE_MOVSD,		// Move string/Move doubleword string
	REPE_CMPS,
	REPE_CMPSB,		// Compare string/Compare byte string
	REPE_CMPSW,		// Compare string/Compare word string
	REPE_CMPSD,		// Compare string/Compare doubleword string
	REPE_SCAS,
	REPE_SCASB,		// Scan string/Scan byte string
	REPE_SCASW,		// Scan string/Scan word string
	REPE_SCASD,		// Scan string/Scan doubleword string
	REPE_LODS,
	REPE_LODSB,		// Load string/Load byte string
	REPE_LODSW,		// Load string/Load word string
	REPE_LODSD,		// Load string/Load doubleword string
	REPE_STOS,
	REPE_STOSB,		// Store string/Store byte string
	REPE_STOSW,		// Store string/Store word string
	REPE_STOSD,		// Store string/Store doubleword string


	REPNE_MOVS,
	REPNE_MOVSB,		// Move string/Move byte string
	REPNE_MOVSW,		// Move string/Move word string
	REPNE_MOVSD,		// Move string/Move doubleword string
	REPNE_CMPS,
	REPNE_CMPSB,		// Compare string/Compare byte string
	REPNE_CMPSW,		// Compare string/Compare word string
	REPNE_CMPSD,		// Compare string/Compare doubleword string
	REPNE_SCAS,
	REPNE_SCASB,		// Scan string/Scan byte string
	REPNE_SCASW,		// Scan string/Scan word string
	REPNE_SCASD,		// Scan string/Scan doubleword string
	REPNE_LODS,
	REPNE_LODSB,		// Load string/Load byte string
	REPNE_LODSW,		// Load string/Load word string
	REPNE_LODSD,		// Load string/Load doubleword string
	REPNE_STOS,
	REPNE_STOSB,		// Store string/Store byte string
	REPNE_STOSW,		// Store string/Store word string
	REPNE_STOSD,		// Store string/Store doubleword string

	REP,		//Repeat while ECX not zero
	REPE,
	REPZ,		// Repeat while equal/Repeat while zero
	REPNE,
	REPNZ,		// Repeat while not equal/Repeat while not zero

	//5.1.9 I/O INSTRUCTIONS

	IN,			// Read from a port
	OUT,		// Write to a port
	INS,
	INSB,		// Input string from port/Input byte string from port
	INSW,		// Input string from port/Input word string from port
	INSD,		// Input string from port/Input doubleword string from port
	OUTS,
	OUTSB,		// Output string to port/Output byte string to port
	OUTSW,		// Output string to port/Output word string to port
	OUTSD,		// Output string to port/Output doubleword string to port


	//5.1.10 ENTER AND LEAVE INSTRUCTIONS

	ENTER,		// High-level procedure entry
	LEAVE,		// High-level procedure exit


	//5.1.11 FLAG CONTROL (EFLAG) INSTRUCTIONS
	
	STC,		// Set carry flag
	CLC,		// Clear the carry flag
	CMC,		// Complement the carry flag
	CLD,		// Clear the direction flag
	STD,		// Set direction flag
	LAHF,		// Load flags into AH register

	SAHF,		// Store AH register into flags
	PUSHF,
	PUSHFD,		// Push EFLAGS onto stack
	POPF,
	POPFD,		// Pop EFLAGS from stack
	STI,		// Set interrupt flag
	CLI,		// Clear the interrupt flag


	//5.1.12 SEGMENT REGISTER INSTRUCTIONS

	LDS,		// Load far pointer using DS
	LES,		// Load far pointer using ES
	LFS,		// Load far pointer using FS
	LGS,		// Load far pointer using GS
	LSS,		// Load far pointer using SS


	//5.1.13 MISCELLANEOUS INSTRUCTIONS

	LEA,		// Load effective address
	NOP,		// No operation
	UD2,		// Undefined instruction
	XLAT,
	XLATB,		// Table lookup translation
	CPUID,		// Processor Identification

}x86_INSTRUCTION, *LPx86_INSTRUCTION;

//--------------------------------------------------------------
// X87 FPU INSTRUCTIONS
//
//--------------------------------------------------------------

typedef enum __tagx87FPU_INSTRUCTION
{
	//5.2.1 X87 FPU DATA TRANSFER INSTRUCTIONS

	FLD,		// Load floating-point value
	FST,		// Store floating-point value
	FSTP,		// Store floating-point value and pop
	FILD,		// Load integer
	FIST,		// Store integer
	FISTP,		// Store integer and pop
	FBLD,		// Load BCD
	FBSTP,		// Store BCD and pop
	FXCH,		// Exchange registers
	FCMOVE,		// Floating-point conditional move if equal
	FCMOVNE,	// Floating-point conditional move if not equal
	FCMOVB,		// Floating-point conditional move if below
	FCMOVBE,	// Floating-point conditional move if below or equal
	FCMOVNB,	// Floating-point conditional move if not below
	FCMOVNBE,	// Floating-point conditional move if not below or equal
	FCMOVU,		// Floating-point conditional move if unordered
	FCMOVNU,	// Floating-point conditional move if not unordered


	//5.2.2 X87 FPU BASIC ARITHMETIC INSTRUCTIONS

	FADD,		// Add floating-point
	FADDP,		// Add floating-point and pop
	FIADD,		// Add integer
	FSUB,		// Subtract floating-point
	FSUBP,		// Subtract floating-point and pop

	FISUB,		// Subtract integer
	FSUBR,		// Subtract floating-point reverse
	FSUBRP,		// Subtract floating-point reverse and pop
	FISUBR,		// Subtract integer reverse
	FMUL,		// Multiply floating-point
	FMULP,		// Multiply floating-point and pop
	FIMUL,		// Multiply integer
	FDIV,		// Divide floating-point
	FDIVP,		// Divide floating-point and pop
	FIDIV,		// Divide integer
	FDIVR,		// Divide floating-point reverse
	FDIVRP,		// Divide floating-point reverse and pop
	FIDIVR,		// Divide integer reverse
	FPREM,		// Partial remainder
	FPREM1,		// IEEE Partial remainder
	FABS,		// Absolute value
	FCHS,		// Change sign
	FRNDINT,	// Round to integer
	FSCALE,		// Scale by power of two
	FSQRT,		// Square root
	FXTRACT,	// Extract exponent and significand

	//5.2.3 X87 FPU COMPARISON INSTRUCTIONS

	FCOM,		// Compare floating-point
	FCOMP,		// Compare floating-point and pop
	FCOMPP,		// Compare floating-point and pop twice
	FUCOM,		// Unordered compare floating-point
	FUCOMP,		// Unordered compare floating-point and pop
	FUCOMPP,	// Unordered compare floating-point and pop twice
	FICOM,		// Compare integer

	FICOMP,		// Compare integer and pop
	FCOMI,		// Compare floating-point and set EFLAGS
	FUCOMI,		// Unordered compare floating-point and set EFLAGS
	FCOMIP,		// Compare floating-point, set EFLAGS, and pop
	FUCOMIP,	// Unordered compare floating-point, set EFLAGS, and pop
	FTST,		// Test floating-point (compare with 0.0)
	FXAM,		// Examine floating-point


	//5.2.4 X87 FPU TRANSCENDENTAL INSTRUCTIONS

	FSIN,		// Sine
	FCOS,		// Cosine
	FSINCOS,	// Sine and cosine
	FPTAN,		// Partial tangent
	FPATAN,		// Partial arctangent
	F2XM1,		// 2^x - 1
	FYL2X,		// y * log2x
	FYL2XP1,	// y * log2(x+1)

	//5.2.5 X87 FPU LOAD CONSTANTS INSTRUCTIONS

	FLD1,		// Load +1.0
	FLDZ,		// Load +0.0
	FLDPI,		// Load pi
	FLDL2E,		// Load log2e
	FLDLN2,		// Load loge2
	FLDL2T,		// Load log210
	FLDLG2,		// Load log102

	//5.2.6 X87 FPU CONTROL INSTRUCTIONS

	FINCSTP,	// Increment FPU register stack pointer
	FDECSTP,	// Decrement FPU register stack pointer
	FFREE,		// Free floating-point register
	FINIT,		// Initialize FPU after checking error conditions
	FNINIT,		// Initialize FPU without checking error conditions
	FCLEX,		// Clear floating-point exception flags after checking for error conditions
	FNCLEX,		// Clear floating-point exception flags without checking for error conditions
	FSTCW,		// Store FPU control word after checking error conditions
	FNSTCW,		// Store FPU control word without checking error 	condition,		//s
	FLDCW,		// Load FPU control word
	FSTENV,		// Store FPU environment after checking error conditions
	FNSTENV,	// Store FPU environment without checking error 	condition,		//s
	FLDENV,		// Load FPU environment
	FSAVE,		// Save FPU state after checking error conditions
	FNSAVE,		// Save FPU state without checking error conditions
	FRSTOR,		// Restore FPU state
	FSTSW,		// Store FPU status word after checking error conditions
	FNSTSW,		// Store FPU status word without checking error conditions
	WAIT,
	FWAIT,		// Wait for FPU
	FNOP,		// FPU no operation


}x87FPU_INSTRUCTION, *LPx87FPU_INSTRUCTION;

typedef enum x87FPU_REGISTER
{
	st, st0, //is the same register both
	st1, st2, 
	st3, st4, 
	st5, st6, 
	st7
}x87FPU_REGISTER, *LPx87FPU_REGISTER;

//------------------------------------------------------------------
//
// C O D E   G E N E R A T O R   H E L P E R   C L A S S E S 
//
//------------------------------------------------------------------

typedef enum __tagASM_PTR_OPERATOR
{
	NO_OP_PTR,
	BYTE_PTR,
	WORD_PTR,
	DWORD_PTR_,
	QWORD_PTR,
}ASM_PTR_OPERATOR, *LPASM_PTR_OPERATOR;

typedef enum __tagVAR_OPERATOR
{
	VAR_OFFSET,
	VAR_SIZEOF,
	VAR_LENGTH_OF,
}VAR_OPERATOR, *LPVAR_OPERATOR;

typedef enum __tagASSUME_NAME
{
	ASSUME_NOTHING,
	ASSUME_ERROR,
}ASSUME_NAME, *LPASSUME_NAME;

extern const TCHAR* asm_ptr_oper_name[];
extern const TCHAR* var_operator_name[];
extern const TCHAR* x86Regs[];
extern const TCHAR* x86Instr[];
extern const TCHAR* x87FPURegs[];
extern const TCHAR* x87FPUInstr[];

class CPUInstruction;
class FPUInstruction;
class ASSUME;


template<ASM_PTR_OPERATOR ptr_op = DWORD_PTR_>
class asm_operator_ptr
{
	
	asm_operator_ptr& operator=(const asm_operator_ptr&);
public:
	asm_operator_ptr(){}
	asm_operator_ptr(const asm_operator_ptr& ot)
	{
		output = ot.output;
	}	
	//operators
	asm_operator_ptr<ptr_op>& operator()(x86_REGISTER reg)					// BYTE PTR [EAX]
	{
		TCHAR szText[60];
		_stprintf(szText, _T("%s[%s]"), 
							asm_ptr_oper_name[ptr_op],
							x86Regs[reg]);
		output = szText;
		return *this;
	}
	asm_operator_ptr<ptr_op>& operator()(const __tstring& identifier)		// DWORD PTR GlobalVarXXXX;
	{
		output = asm_ptr_oper_name[ptr_op];
		output += identifier;
		return *this;
	}

	asm_operator_ptr<ptr_op>& operator()(x86_REGISTER reg1, TCHAR op, x86_REGISTER reg2)	//DWORD PTR [EAX+EDX]
	{
		TCHAR szText[60];
		_stprintf(szText, _T("%s[%s%c%s]"), 
							asm_ptr_oper_name[ptr_op],
							x86Regs[reg1],
							op,
							x86Regs[reg2]);

		output = szText;
		return *this;
	}
	asm_operator_ptr<ptr_op>&  operator()(x86_REGISTER reg, 
										TCHAR op, 
										int nInmediate)								//QWORD PTR [ESP+4]
	{
		TCHAR szText[100];
		_stprintf(szText, _T("%s[%s%c%04Xh]"),
							asm_ptr_oper_name[ptr_op],
							x86Regs[reg],
							op,
							nInmediate);

		output = szText;
		return *this;
	}

	asm_operator_ptr<ptr_op>&  operator()(x86_REGISTER reg, 
										int nInmediate)								//WORD PTR [ESP-4]
	{
		TCHAR szText[50];
		if(	reg!=CS	&& 
			reg!=DS && 
			reg!=FS && 
			reg!=SS)
			_stprintf(szText, _T("%s[%s%+d]"),
								asm_ptr_oper_name[ptr_op],
								x86Regs[reg],
								nInmediate);
		else
			_stprintf(szText, _T("%s %s:[%d]"),
								asm_ptr_oper_name[ptr_op],
								x86Regs[reg],
								nInmediate);
		output = szText;
		return *this;
	}


	asm_operator_ptr<ptr_op>&  operator()(x86_REGISTER reg1, 
										TCHAR op1, 
										x86_REGISTER reg2, 
										TCHAR op2,
										int nInmediate)								//DWORD PTR [EAX+EDX*4]
	{
		TCHAR szText[100];
		_stprintf(szText, _T("%s[%s%c%s%c%04Xh]"),
							asm_ptr_oper_name[ptr_op],
							x86Regs[reg1],
							op1,
							x86Regs[reg2],
							op2,
							nInmediate);
		output = szText;
		return *this;
	}

	asm_operator_ptr<ptr_op>& operator()(x86_REGISTER reg1, TCHAR op1, int nInmediate1, 
															TCHAR op2, int nInmediate2)	//DWORD PTR [EAX+010h*4]
	{
		TCHAR szText[100];
		_stprintf(szText, _T("%s[%s%c%04Xh%c%d]"), 
							asm_ptr_oper_name[ptr_op],
							x86Regs[reg1],
							op1,
							nInmediate1,
							op2,
							nInmediate2);

		output = szText;
		return *this;
	}

	asm_operator_ptr<ptr_op>& operator()(x86_REGISTER reg, TCHAR op, const __tstring& identifier)	//DWORD PTR [EAX+global]
	{
		TCHAR szText[1024];
		_stprintf(szText, _T("%s[%s%c%s]"), 
							asm_ptr_oper_name[ptr_op],
							x86Regs[reg],
							op,
							identifier.c_str());

		output = szText;
		return *this;
	}

	asm_operator_ptr<ptr_op>& operator()(x86_REGISTER reg, TCHAR op1, int nInmediate1, 
															TCHAR op2, const __tstring& identifier)	//DWORD PTR [EAX*8+global]
	{
		TCHAR szText[1024];
		_stprintf(szText, _T("%s[%s%c%d%c%s]"), 
							asm_ptr_oper_name[ptr_op],
							x86Regs[reg],
							op1,
							nInmediate1,
							op2,
							identifier.c_str());

		output = szText;
		return *this;
	}

	const __tstring& operator()(){return output;}
private:
	__tstring output;
};

extern SYMBOL_TABLE g_symbol_table;
extern icode_generator g_icode_gen;

typedef struct __tagHPP_STATEMENT_INFO
{
	int stmt_start_block;
	int stmt_end_block;
	HCC_TOKEN_TYPE type;

	__tagHPP_STATEMENT_INFO() : stmt_start_block(0),
								stmt_end_block(0),
								type(HCC_TOKEN_ERROR)
	{
	}

}HPP_STATEMENT_INFO, *LPHPP_STATEMENT_INFO;


typedef struct __tagCONST_VALUE_INFO
{
	TypeSpecifier* type_ptr;
	long eaxVal;
	long edxVal;
	__tagCONST_VALUE_INFO() : type_ptr(0), eaxVal(0), edxVal(0) {}

	__tagCONST_VALUE_INFO(TypeSpecifier* _type): type_ptr(_type), eaxVal(0), edxVal(0) {}
}CONST_VALUE_INFO, *LPCONST_VALUE_INFO;

class HCCCodeGenerator  
{
	assembly_source_unit* asm_unit_ptr;
	SYMBOL_TABLE::LPSYMBOL symbol_ptr;
	HCC_TOKEN_TYPE token_type;
	LPHCC_TOKEN token_ptr;
	icode_generator* icode_ptr;
	LPSYMBOL_TABLE symbol_table_ptr;
	LPHCC_TOKEN getToken();
	inline assembly_source_unit& unit();
	SYMBOL_TABLE::LPSYMBOL the_entry_point_ptr;
	int m_nICodePos;
	vector<Symbol*> obj_instance_in_scope;
	vector<Symbol*> obj_instance_in_globals;
	__tstring hpp_source_file;
	volatile bool is_operand_in_stack;
	TypeSpecifier* stack_operand_type;
	volatile bool is_floating_point_in_fpu;
	volatile bool emitting_ternary_expr;
	//to keep the symbols from expressions like : id = expr;
	stack<Symbol*> stack_of_ids;

	stack<HPP_STATEMENT_INFO> stack_of_stmts;
	//To optimize the const assignment expressions of type: id = number; //Dec 27, 2008
	CONST_VALUE_INFO lastConstValueInfo;
	bool bRequiresAddressOf;
public:	
	void setHppSourceFile(const char* hpp_file_string);
	virtual bool Generate(Symbol* entry_point_ptr = NULL);
	HCCCodeGenerator(const char* output_asm_file) : asm_unit_ptr(new assembly_source_unit(output_asm_file)), 
													symbol_table_ptr(&g_symbol_table),
													icode_ptr(&g_icode_gen),
													the_entry_point_ptr(0),
													m_nICodePos(0),
													token_ptr(0),
													is_operand_in_stack(false),
													is_floating_point_in_fpu(false),
													emitting_ternary_expr(false),
													bRequiresAddressOf(false)
	{
		stack_operand_type = HccTypeChecker::ts_type_void_ptr;
	}
	virtual ~HCCCodeGenerator();

private:
	__tstring CreateVariableLabel(Symbol* variable_ptr);
	__tstring getSymbolLabel(Symbol* symbl_ptr);
	__tstring CreateLabel(const TCHAR* prefix, unsigned int index);


	friend class CPUInstruction;
	friend class FPUInstruction;

	friend class ASSUME;

	inline void RestoreICodePos();
	inline void SaveICodePos();
private:
	void emit_SizeOf(Symbol* caller_function_ptr);
	void emit_InlineMax(Symbol *caller_function_ptr);
	void emit_InlineMin(Symbol *caller_function_ptr);
	void EmitPropertyPutCall(TypeSpecifier *target_type, Symbol* object_ptr, Symbol* function_ptr, bool bIsAddressOnStack = false);
	void emitDebuggerCall();
	//C O D E   G E N E R A T O R   P R O C E D U R E S 
	void emitWithStatement(Symbol* function_ptr);
	void emitTryBlock(Symbol* function_ptr);
	void emitCaseBranch(queue<__tstring>& branch_table,
						queue<__tstring>& block_stmt,
						Symbol *function_ptr,
						const __tstring& __end_switch_stmt_block,
						int* def_switch_block_ptr = NULL);
	void emitSwitch(Symbol* function_ptr);
	void emitFor(Symbol* function_ptr);
	void emitDoWhile(Symbol* function_ptr);
	void emitWhile(Symbol* function_ptr);
	void emitIfStatement(Symbol* function_ptr);

	//E X P R E S S I O N S   E M I T S
	bool IsTypeQualifiedForDefaultConstructor(TypeSpecifier& typeSpec);
	//BEGIN - REDUCE COMPILER CODE/OPTIMIZE/AVOID REDUNDANCY - Dec 20, 2008
	TypeSpecifier* EmitLogicalOpAssignExpr(Symbol *target_ptr, TypeSpecifier *source_type, HCC_TOKEN_TYPE _operator, Symbol *function_ptr, bool bIsAddressOnStack, TypeSpecifier* array_item_type);
	/*
	TypeSpecifier* EmitAndAssignExpr(Symbol *target_ptr, TypeSpecifier *source_type, HCC_TOKEN_TYPE _operator, Symbol *function_ptr, bool bIsAddressOnStack, TypeSpecifier* array_item_type = NULL);
	TypeSpecifier* EmitOrAssignExpr(Symbol *target_ptr, TypeSpecifier *source_type, HCC_TOKEN_TYPE _operator, Symbol *function_ptr, bool bIsAddressOnStack, TypeSpecifier* array_item_type = NULL);
	TypeSpecifier* EmitXorAssignExpr(Symbol *target_ptr, TypeSpecifier* source_type, HCC_TOKEN_TYPE _operator, Symbol* function_ptr, bool bIsAddressOnStack, TypeSpecifier* array_item_type = NULL);
	*/
	//END - REDUCE COMPILER CODE/OPTIMIZE/AVOID REDUNDANCY - Dec 20, 2008

	TypeSpecifier* EmitModAssignExpr(Symbol *target_ptr, TypeSpecifier* source_type, HCC_TOKEN_TYPE _operator, Symbol* function_ptr, bool bIsAddressOnStack, TypeSpecifier* array_item_type = NULL);
	void changeOperandSign(TypeSpecifier* operand1_type, HCC_TOKEN_TYPE unaryOp);
	TypeSpecifier* EmitDivAssignExpr(Symbol *target_ptr, TypeSpecifier* source_type, HCC_TOKEN_TYPE _operator, Symbol *function_ptr, bool bIsAddressOnStack, TypeSpecifier* array_item_type = NULL);
	TypeSpecifier* EmitMultAssignExpr(Symbol *target_ptr, TypeSpecifier* source_type, HCC_TOKEN_TYPE _operator, Symbol *function_ptr, bool bIsAddressOnStack, TypeSpecifier* array_item_type = NULL);
	TypeSpecifier* EmitPlusAssignExpr(Symbol *target_ptr, TypeSpecifier* source_type, HCC_TOKEN_TYPE _operator, Symbol* function_ptr, bool bIsAddressOnStack, TypeSpecifier* array_item_type = NULL);	
	TypeSpecifier* EmitMinusAssignExpr(Symbol *target_ptr, TypeSpecifier* source_type, HCC_TOKEN_TYPE _operator, Symbol* function_ptr, bool bIsAddressOnStack, TypeSpecifier* array_item_type = NULL);

	Symbol* CreateDefaultClassConstructor(Symbol* class_ptr);
	void EmitClassConstructor(Symbol* constructor_ptr, Symbol* onwer_class_ptr);
	void EmitPopOperand(TypeSpecifier* stack_operand_type);
	void emitJumpStatement(Symbol* function_ptr);
	void EmitObjectDataMembersInitForClasses();
	void EmitInitObjectDataMembersFunction(TypeSpecifier *class_type_ptr);
	void emitProgramGlobalsInit();
	void emitMainHPPRT_Startup(Symbol *entry_point_ptr);

	TypeSpecifier* EmitDestructorCall(Symbol* symbol_ptr, Symbol* function_ptr, TypeSpecifier* dyn_type_ptr = NULL, bool* pbHasDestructor = NULL);
	TypeSpecifier* EmitObjectConstructorCall(TypeSpecifier* class_type_ptr, Symbol* function_ptr, Symbol* object_ptr, x86_REGISTER x86RegThisPointer = ECX);
	__tstring getLabelFromName(const __tstring& raw_name);
	void EmitClassVtables();	
	
	long getArrayDimensionsFromType(TypeSpecifier *array_ptr);
	TypeSpecifier* getArrayScalarType(TypeSpecifier *array_ptr);
	long getArraySizeFromType(TypeSpecifier *array_ptr);
	TypeSpecifier* EmitNewDynamicAllocation(Symbol *target_ptr, Symbol* function_ptr, bool bIsAddressOnStack, bool bIsPropertyPut);

	TypeSpecifier* emitCallWriteWriteLnBuiltInFunction(Symbol *write_fn_ptr, Symbol* function_ptr);
	TypeSpecifier* emitObjectInstanceMember(Symbol* object_ptr, TypeSpecifier *arrayTypeSpec, Symbol* function_ptr);
	TypeSpecifier* EmitPrefixAssignmentExpr(Symbol *function_ptr);
	TypeSpecifier* EmitPostFixAssignmentExpr(Symbol* target_ptr, Symbol* function_ptr, bool bArraySubscript = false);
	long getArrayCountFromType(TypeSpecifier *array_ptr);
	TypeSpecifier* emitArraySubscript(Symbol* array_var_syptr, TypeSpecifier* type_ptr, TypeSpecifier* pItemType, Symbol* function_ptr);
	TypeSpecifier* EmitAssignmentExpr(Symbol* target_ptr, 
									  HCC_TOKEN_TYPE _operator, 
									  Symbol* function_ptr = NULL,
									  bool bIsAddressOnStack = false,
									  bool bIsPropertyPut = false,
									  Symbol* object_ptr = NULL,
									  TypeSpecifier* array_item_type = NULL);

	TypeSpecifier* emitCommaExprList(Symbol *function_ptr);
	TypeSpecifier* EmitExprList(Symbol *function_ptr);
	TypeSpecifier* EmitRelationalExprList(Symbol *function_ptr);
	TypeSpecifier* EmitExpression(Symbol *function_ptr);
	TypeSpecifier* EmitTerm(Symbol *function_ptr);
	TypeSpecifier* EmitFactor(Symbol *function_ptr);

	//S T A T E M E N T S   E M I T S
	HCC_TOKEN_TYPE emitCompoundStatement(Symbol *function_ptr);
	HCC_TOKEN_TYPE EmitStatement(Symbol *function_ptr);
	HCC_TOKEN_TYPE EmitStatementList(HCC_TOKEN_TYPE terminator, Symbol *function_ptr);

	void EmitDataSegment();
	void EmitGlobalConstants();

	void EmitLoadObjectInstancePointer_This(Symbol* function_ptr);
	void EmitAddressFromAddressOf(Symbol *symbl_ptr, Symbol* function_ptr, bool bEmitAssignment = false, TypeSpecifier *target_type = NULL, x86_INSTRUCTION __xInst = MOV, x86_REGISTER __AReg = EAX, x86_REGISTER __ARegHigh = EDX, bool bSetThisPointer = false);
	void EmitLoadValueAddressOf(Symbol* target_ptr, Symbol* function_ptr, bool bSetThisPointer = false);
	void EmitPushOperand(TypeSpecifier* operand_type_ptr);

	void EmitActualParameterList(Symbol* callee_fn_ptr, Symbol* caller_fn_ptr);
	void EmitFunctionCall(Symbol* callee_fn_ptr, Symbol* caller_fn_ptr, Symbol* object_ptr, x86_REGISTER x86RegThisPointer = ECX);
	void EmitProgramProcedures();
	void EmitFunctionBody(Symbol *function_ptr);
	void EmitFunction(Symbol* function_ptr);
	void EmitCodeSegment();
	void EmitGlobalVariables();
	void EmitProgramEpilog();
	void EmitProgramProlog(const __tstring& min_cpu, int stack_size, const __tstring& mem_model, const __tstring& calling_conv);
	void EmitPromoteToFloatingPoint(TypeSpecifier* type_ptr, bool bSaveOntoStack = true);
	void EmitPushAddressOf(Symbol *symbl_ptr, Symbol* function_ptr);
	void EmitPushStringLiteral(Symbol* string_ptr);
	void EmitFloatingPointLoad(Symbol *number_ptr, bool bDestroyTempVariable = true);
	void EmitPushNumber(Symbol *number_ptr);
	void EmitLoadNumber(const CONST_VALUE_INFO& valueInfo);
	void EmitConstNumberValue(Symbol *number_ptr, LPCONST_VALUE_INFO lpConstValue); //Dec 27, 2008
	void EmitLoadValue(Symbol* symbl_ptr);
	void EmitStatementLabel(int index);
	
	void EmitMainEntryPoint(Symbol* entry_point_ptr);
	void EmitFunctionEpilog(int stack_params_size = 0);
	void EmitFunctionProlog(int stack_variables_size = 0);	

	//C O M M E N T   R E D U C T I O N   P R O C E D U R E S 
	void EmitComment(const __tstring &comment);
	void EmitNewLineComment(int line_number, const __tstring &comment);
	void EmitNewLineComment(const __tstring& comment);
	
	void EmitFunctionComment(Symbol* function_ptr);
	void EmitFunctionParameterListComment(Symbol* function_ptr);
	void EmitStatementComment(Symbol* function_ptr);

	void EmitObjectInstanceComment(void);	
	void EmitFactorComment(void);
	void EmitTermComment(void);
	void EmitExprComment(void);
	void EmitExprListComment();
	void EmitCommaExprListComment(void);

	void EmitFunctionLocalVariablesComment(Symbol* function_ptr);
	void EmitVariableDeclComment(void);
	void EmitConstantComment(Symbol *symbl_const_ptr);

	void EmitSEHStatementLabel(int index);
private:
	void emitPushInteger(TypeSpecifier* source_type, bool bLoadInFPU = true);
	void emitAssignInt64ToAddress(bool bIsLocal = false, TCHAR op = _T('-'), int offset = sizeof(int), x86_INSTRUCTION __xInst = MOV, x86_REGISTER __regHigh = EDX, x86_REGISTER __regLow = EAX);
	void emitLoadInt64ToRegisters(bool bIsLocal = false, TCHAR op = _T('-'), int offset = sizeof(int), x86_REGISTER __regHigh = EDX, x86_REGISTER __regLow = EAX);
	TypeSpecifier* emitDynamicCastOperator(Symbol* target_ptr, 
										   Symbol* function_ptr, 
										   bool bIsAddressOnStack = false,
										   bool bIsPropertyPut = false,
										   Symbol* object_ptr = NULL,
										   Symbol** pDynamicClassSymbol = NULL,
										   Symbol** pObjectVarSymbol = NULL);

protected:
	asm_operator_ptr<QWORD_PTR> qword_ptr;
	asm_operator_ptr<DWORD_PTR_> dword_ptr;	
	asm_operator_ptr<WORD_PTR>	word_ptr;
	asm_operator_ptr<BYTE_PTR>	byte_ptr;
	asm_operator_ptr<NO_OP_PTR> no_op_ptr;
private:
	void EmitMultipleDestructorCall(TypeSpecifier* array_type_ptr);
	void EmitMultipleConstructorCall(TypeSpecifier* array_type_ptr);
public:
	void emitArrayAddressLoadFromSubscript(TypeSpecifier* pItemType, x86_INSTRUCTION x86Inst = LEA, x86_REGISTER regLow = EAX, x86_REGISTER regHigh = EDX);
	TypeSpecifier* emitArraySubscriptReadorWrite(int type_size, TypeSpecifier* arrayItemTypeSpec, Symbol* function_ptr);
	void emitArrayAddressLoadGlobalFromSubscript(Symbol* array_var_syptr, int type_size, x86_INSTRUCTION x86Inst = LEA, x86_REGISTER regLow = EAX, x86_REGISTER regHigh = EDX);
private:
	TypeSpecifier* emitAddressOf(Symbol* function_ptr, TypeSpecifier* pItemType = NULL, Symbol** ppResultantSymbol = NULL);
public:
	x86_REGISTER getReg16From(x86_REGISTER reg);
	x86_REGISTER getReg8From(x86_REGISTER reg);
private:
	bool emitVirtualMemberFunctionCall(Symbol* object_ptr, Symbol* member_ptr, TypeSpecifier* baseTypeSpec, Symbol* function_ptr, bool bPushParamsOntoStack = true);
};



class CPUInstruction
{
	CPUInstruction();
	CPUInstruction(const CPUInstruction&);
	CPUInstruction& operator=(const CPUInstruction&);
public:
	CPUInstruction(HCCCodeGenerator* codegen, x86_INSTRUCTION instr) : 
										codegen_ptr(codegen), 
										x86_instr(instr) {}

	inline void set_new_comment(const __tstring& new_comment)
	{
		comment = _T(" ;");
		comment += new_comment;
	}
	inline void clear_comment()
	{
		comment = _T(" ");
	}
public:
	void operator()()							//like : CDQ ; a comment
	{
		codegen_ptr->unit() << ctab << x86Instr[x86_instr]
							<< ctab
							<< comment
							<< endl;
		clear_comment();
	}

	void operator()(const TCHAR ch)				//like : ? ; a comment
	{
		codegen_ptr->unit() << ctab << x86Instr[x86_instr]
							<< ctab
							<< _T('\'')
							<< ch
							<< _T('\'')
							<< comment
							<< endl;
		clear_comment();
	}

	void operator()(x86_REGISTER reg, const TCHAR* Identifier)	//like : LEA EAX, GlobalVariable; a comment
	{
		codegen_ptr->unit() << ctab << x86Instr[x86_instr]
							<< ctab
							<< x86Regs[reg]
							<< comma
							<< Identifier
							<< comment
							<< endl;
		clear_comment();
	}


	void operator()(x86_REGISTER reg, VAR_OPERATOR var_op, const __tstring& Global)	//like : MOV EAX, OFFSET GlobalVariable; a comment
	{
		codegen_ptr->unit() << ctab << x86Instr[x86_instr]
							<< ctab
							<< x86Regs[reg]
							<< comma
							<< var_operator_name[var_op]
							<< Global
							<< comment
							<< endl;
		clear_comment();
	}

	void operator()(VAR_OPERATOR var_op, const __tstring& Global)	//like : JZ OFFSET GlobalVariable; a comment
	{
		codegen_ptr->unit() << ctab << x86Instr[x86_instr]
							<< ctab
							<< var_operator_name[var_op]
							<< Global
							<< comment
							<< endl;
		clear_comment();
	}

	void operator()(x86_REGISTER reg)			//like : INC ECX ; a comment
	{
		codegen_ptr->unit() << ctab << x86Instr[x86_instr]
							<< ctab
							<< x86Regs[reg] 
							<< comment
							<< endl;
		clear_comment();
	}

	void operator()(long nInmediate)			//like : PUSH 0Ah ; a comment
	{
		codegen_ptr->unit() << ctab << x86Instr[x86_instr]
							<< ctab
							<< hex
							<< nInmediate
							<< comment
							<< dec
							<< endl;
		clear_comment();
	}
	void operator()(int nInmediate)			//like : PUSH 0Ah ; a comment
	{
		codegen_ptr->unit() << ctab << x86Instr[x86_instr]
							<< ctab
							<< hex
							<< nInmediate
							<< comment
							<< dec
							<< endl;
		clear_comment();
	}


	void operator()(x86_REGISTER dest, x86_REGISTER src)	//like : MOV EAX, EDX ; a comment
	{
		codegen_ptr->unit() << ctab << x86Instr[x86_instr]
							<< ctab
							<< x86Regs[dest] 
							<< comma
							<< x86Regs[src]
							<< comment
							<< endl;
		clear_comment();
	}

	void operator()(x86_REGISTER reg1, x86_REGISTER reg2, int nInmediate)	//like : SHLD EAX, EDX, 1 ; a comment
	{
		codegen_ptr->unit() << ctab << x86Instr[x86_instr]
							<< ctab
							<< x86Regs[reg1] 
							<< comma
							<< x86Regs[reg2]
							<< comma
							<< nInmediate
							<< comment
							<< endl;
		clear_comment();
	}


	void operator()(x86_REGISTER dest, int nInmediate)	//like : MOV EAX, hex(value)	; a comment
	{
		codegen_ptr->unit() << ctab << x86Instr[x86_instr]
							<< ctab
							<< x86Regs[dest] 
							<< comma
							<< hex
							<< nInmediate
							<< comment
							<< dec
							<< endl;
		clear_comment();
	}

	void operator()(x86_REGISTER dest, long nInmediate)	//like : MOV EAX, hex(value)	; a comment
	{
		codegen_ptr->unit() << ctab << x86Instr[x86_instr]
							<< ctab
							<< x86Regs[dest] 
							<< comma
							<< hex
							<< nInmediate
							<< comment
							<< dec
							<< endl;
		clear_comment();
	}


	template<typename __assembly_ptr_op>
	void operator()(x86_REGISTER reg, __assembly_ptr_op& ptr_operator)	//like : MOV EAX, DWORD PTR [...]
	{
		codegen_ptr->unit() << ctab << x86Instr[x86_instr]
							<< ctab
							<< x86Regs[reg] 
							<< comma
							<< ptr_operator()							
							<< comment
							<< endl;
		clear_comment();
	}

	template<typename __assembly_ptr_op>
	void operator()(__assembly_ptr_op& ptr_operator, x86_REGISTER reg)	//like : MOV DWORD PTR [...], EAX
	{
		codegen_ptr->unit() << ctab << x86Instr[x86_instr]
							<< ctab							
							<< ptr_operator()							
							<< comma							
							<< x86Regs[reg]
							<< comment
							<< endl;
		clear_comment();
	}

	template<typename __assembly_ptr_op>
	void operator()(__assembly_ptr_op& ptr_operator, int nInmediate)	//like : MOV DWORD PTR [...], EAX
	{
		codegen_ptr->unit() << ctab << x86Instr[x86_instr]
							<< ctab
							<< ptr_operator()
							<< comma							
							<< nInmediate
							<< comment
							<< endl;
		clear_comment();
	}

	template<typename __assembly_ptr_op>
	void operator()(__assembly_ptr_op& ptr_operator)	//like : PUSH DWORD PTR [...]	; a comment
	{
		codegen_ptr->unit() << ctab << x86Instr[x86_instr]
							<< ctab
							<< ptr_operator()
							<< comment
							<< endl;
		clear_comment();
	}

	template<typename __assembly_ptr_op>
	void operator()(__assembly_ptr_op& ptr_operator, VAR_OPERATOR var_op, __tstring identifier)	//like : MOV DWORD PTR [EAX], OFFSET identifier	; a comment
	{
		codegen_ptr->unit() << ctab << x86Instr[x86_instr]
							<< ctab
							<< ptr_operator()
							<< comma
							<< var_operator_name[var_op]
							<< space
							<< identifier
							<< comment
							<< endl;
		clear_comment();
	}

private:	
	HCCCodeGenerator* codegen_ptr;
	x86_INSTRUCTION x86_instr;
	__tstring comment;
};

class FPUInstruction
{
	FPUInstruction(const FPUInstruction&);
	FPUInstruction& operator=(const FPUInstruction&);
public:
	FPUInstruction(HCCCodeGenerator* codegen, x87FPU_INSTRUCTION fpu_reg) : codegen_ptr(codegen), 
																		 x87FPU_instr(fpu_reg) {}

	inline void set_new_comment(const __tstring& new_comment)
	{
		comment = _T(" ;");
		comment += new_comment;
	}
	inline void clear_comment()
	{
		comment = _T("");
	}

	void operator()()								//like: FLDZ
	{
		codegen_ptr->unit() << ctab << x87FPUInstr[x87FPU_instr]
							<< comment
							<< endl;

		clear_comment();
	}
	void operator()(x87FPU_REGISTER fpu_reg)					//like: ?
	{
		codegen_ptr->unit() << ctab << x87FPUInstr[x87FPU_instr]
							<< ctab
							<< x87FPURegs[fpu_reg]
							<< comment
							<< endl;

		clear_comment();
	}

	void operator()(x87FPU_REGISTER fpu_reg1, x87FPU_REGISTER fpu_reg2)	//like: FMUL st, st(i)
	{
		codegen_ptr->unit() << ctab << x87FPUInstr[x87FPU_instr]
							<< ctab
							<< x87FPURegs[fpu_reg1]
							<< comma
							<< x87FPURegs[fpu_reg2]
							<< comment
							<< endl;

		clear_comment();
	}


	template<typename __assembly_ptr_op>
	void operator()(__assembly_ptr_op& ptr_operator)	//like : FLD QWORD PTR [EBP-...]	; a comment
	{
		codegen_ptr->unit() << ctab << x87FPUInstr[x87FPU_instr]
							<< ctab
							<< ptr_operator()
							<< comment
							<< endl;
		clear_comment();
	}

private:
	HCCCodeGenerator* codegen_ptr;	
	x87FPU_INSTRUCTION x87FPU_instr;
	__tstring comment;
};

class ASSUME
{
	ASSUME();
	ASSUME(const ASSUME&);
	ASSUME& operator=(const ASSUME&);
public:
	ASSUME(HCCCodeGenerator* codegen_ptr, x86_REGISTER reg = FS, ASSUME_NAME what = ASSUME_NOTHING){
		codegen_ptr->unit() << ctab << _T("assume ") 
							<< x86Regs[reg] << _T(":")
							<< ((what == ASSUME_NOTHING) ? _T("nothing") : _T("error")) << _T('\n');
	}
};

#endif // !defined(AFX_HCCCODEGENERATOR_H__94130FF8_55CD_453E_83AD_A7A38795F21A__INCLUDED_)
