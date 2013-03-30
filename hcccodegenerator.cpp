// HCCCodeGenerator.cpp: implementation of the HCCCodeGenerator class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HCCCodeGenerator.h"


#include <stack>
#include <algorithm>
#include <queue>

FLOATING_POINT_CONVERSION fp_conv;

//The conversion routines
#include "HCCLib\hpplib.h"
#include "HCCLib\errors.h"

#include "assembly_source_unit.h"
#include "HCCIntermediate.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;	//this is private/unique by file
#endif


extern TCHAR* symbolStrings[];
extern ICodeResponsible theICodeResponsible;

#define __OSVER		_T("__osver")
#define __WINMINOR	_T("__winminor")
#define __WINMAJOR	_T("__winmajor")
#define __WINVER	_T("__winver")
#define __COMPUTER	_T("__computer")

#define ___ARGV		_T("___argv")
#define ___ARGC		_T("___argc")


#define __plus		_T('+')
#define __minus		_T('-')
#define __mult		_T('*')

#define MIN_CPU_REQUIREMENTS		_T(".486P")	//x87 FPU support
#define PROTECTED_MODE_MODEL		_T("flat")
#define STDCALL_CALLING_CONVENTION	_T("stdcall")
#define QWORD_OFFSET_LOW _T("+4")	//for Floating Point values
#define QWORD_OFFSET_HIGH QWORD_OFFSET_LOW	//for Int64 is inverted

#define STMT_LABEL_PREFIX		_T("$L")

#define STRING_LABEL_PREFIX		_T("$S")
#define CHAR_LABEL_PREFIX		_T("$C")
#define FLOATING_LABEL_PREFIX	_T("$F")
#define INTEGER_LABEL_PREFIX	_T("$N")
#define EXPRESSION_LABEL_PREFIX	_T("$E")

#define HPP_CLASS_DESTRUCTOR _T("Destructor")

#define CLASS_VPTR_VTBL_NAME	_T("@@class_vptr_vtbl@@__")
#define CLASS_VTBL_VPTR_PREFIX	CLASS_VPTR_VTBL_NAME

#define STRUCTURED_EXCEPTION_HANDLING_PREFIX _T("__except_handlerXXX")

#define __PURE_VIRTUAL_CALL			_T("__purecall")

#define FLOATING_POINT_FROM_INT		_T("FloatingPoint_FromInt")
#define FLOATING_POINT_FROM_INT64	_T("FloatingPoint_FromInt64")

#define FLOATING_POINT_TO_INT		_T("FloatingPoint_ToInt")
#define FLOATING_POINT_TO_INT64	_T("FloatingPoint_ToInt64")

#define FLOATING_POINT_COMPARE		_T("FloatingPoint_Compare")
#define FLOATING_POINT_COMPARE_INT	_T("FloatingPoint_CompareToInteger")

#define SYSTEM_HPP_RUNTIME_INIT		_T("__System_Hpp_Runtime_Init")
#define SYSTEM_EXIT					_T("System_Exit")
#define SYSTEM_MEMORY_OPER_DESTROY	_T("System_Memory_Destroy")
#define SYSTEM_MEMORY_OPER_NEW		_T("System_Memory_New")
#define SYSTEM_COMMAND_LINE_ARGV	_T("System_GetCommandLineArgv")
#define SYSTEM_MEMORY_PROCESSHEAP	_T("System_Memory_GetProcessHeap")
#define SYSTEM_MEMORY_FREE			_T("System_Memory_Free")
//to avoid name resolution to label, we use the name directly for the comparison
#define SYSTEM_BREAKPOINT			_T("System::Debug::BreakPoint")

#define CONSOLE_WRITE_STRING 			_T("Console_WriteString")
#define CONSOLE_WRITE_INT64  			_T("Console_WriteInteger64")
#define CONSOLE_WRITE_FLOATING_POINT 	_T("Console_WriteDouble")
#define CONSOLE_WRITE_INTEGER			_T("Console_WriteInteger")
#define CONSOLE_WRITE_CHARACTER			_T("Console_WriteChar")
#define CONSOLE_WRITE_BOOLEAN			_T("Console_WriteBoolean")
#define CONSOLE_WRITE_CRLF				_T("Console_WriteCrLf")

#define USER_GLOBALS_INIT				_T("__User_Globals_Init")
#define __MEMBER_INIT					_T("__member_Init@")

#define STRING_HANDLING_LENGTH			_T("StringHandling_StringLength")
#define STRING_HANDLING_NCOMPARE		_T("StringHandling_StringCompare")

extern bool bPatternInitStackVariables;
extern bool bSourceAnnotation;
//to identify the data member init procedure; we are assuming that this is an invalid address,
//so we can use it as the flag, to know where to look for the this pointer...
#define DATA_MEMBERS_INIT_PROC_ADDRESS reinterpret_cast<Symbol*>(0xCCCCDEF4)


const TCHAR* x86Regs[] = {
	//32-bit registers
	_T("EAX"),	// Accumulator for operands and results data
	_T("EBX"),	// Pointer to data in the DS segment
	_T("ECX"),	// Counter for string and loop operations
	_T("EDX"),	// I/O pointer

	_T("ESI"),	// Pointer to data in the segment pointed to by the DS register; source pointer for string operations.
	_T("EDI"),	// Pointer to data (or destination) in the segment pointed to by the ES register; destination pointer for string operations.
	_T("ESP"),	// Stack pointer (in the SS segment)
	_T("EBP"),	// Pointer to data on the stack (in the SS segment)

	//16-bit registers
	_T("AX"), _T("BX"), _T("CX"), _T("DX"),
	_T("SI"), _T("DI"), _T("SP"), _T("BP"),

	//8-bit registers
	_T("AL"), _T("BL"), _T("CL"), _T("DL"),
	_T("AH"), _T("BH"), _T("CH"), _T("DH"),

	//segment registers
	_T("CS"), _T("DS"), _T("SS"), _T("ES"),
	_T("FS"), _T("GS"),
};

const TCHAR* x86Instr[] = {

	_T("mov"),	
	_T("cmove"),
	_T("cmovz"),
	_T("cmovne"),
	_T("cmovnz"),
	_T("cmova"),
	_T("cmovnbe"),	
	_T("cmovae"),
	_T("cmovnb"),
	_T("cmovb"),
	_T("cmovnae"),
	_T("cmovbe"),
	_T("cmovna"),
	_T("cmovg"),
	_T("cmovnle"),
	_T("cmovge"),
	_T("cmovnl"), 
	_T("cmovl"),
	_T("cmovnge"),	
	_T("cmovle"),
	_T("cmovng"),
	_T("cmovc"),	
	_T("cmovnc"),
	_T("cmovo"),
	_T("cmovno"),
	_T("cmovs"),
	_T("cmovns"),
	_T("cmovp"),
	_T("cmovpe"),
	_T("cmovnp"),
	_T("cmovpo"),
	_T("xchg"),
	_T("bswap"),
	_T("xadd"),
	_T("cmpxchg"),
	_T("cmpxchg8b"),
	_T("push"),
	_T("pop"),	
	_T("pusha"),
	_T("pushad"),
	_T("popa"),
	_T("popad"),
	_T("cwd"),
	_T("cdq"),
	_T("cbw"),
	_T("cwde"),
	_T("movsx"),
	_T("movzx"),
	_T("add"),
	_T("adc"),
	_T("sub"),
	_T("sbb"), 
	_T("imul"),
	_T("mul"),
	_T("idiv"),
	_T("div"),
	_T("inc"),
	_T("dec"),
	_T("neg"),
	_T("cmp"),
	_T("daa"),		// decimal adjust after addition
	_T("das"),		// decimal adjust after subtraction
	_T("aaa"),		// ascii adjust after addition
	_T("aas"),		// ascii adjust after subtraction
	_T("aam"),		// ascii adjust after multiplication
	_T("aad"),		// ascii adjust before division
	_T("and"),		// perform bitwise logical and
	_T("or"),			// perform bitwise logical or
	_T("xor"),		// perform bitwise logical exclusive or
	_T("not"),		// perform bitwise logical not
	_T("sar"),		// shift arithmetic right
	_T("shr"),		// shift logical right
	_T("sal"),
	_T("shl"),		// shift arithmetic left/shift logical left
	_T("shrd"),		// shift right double
	_T("shld"),		// shift left double
	_T("ror"),		// rotate right
	_T("rol"),		// rotate left
	_T("rcr"),		// rotate through carry right
	_T("rcl"),		// rotate through carry left
	_T("bt"),			// bit test
	_T("bts"),		// bit test and set
	_T("btr"),		// bit test and reset
	_T("btc"),		// bit test and complement
	_T("bsf"),		// bit scan forward
	_T("bsr"),		// bit scan reverse
	_T("sete"),
	_T("setz"),		// set byte if equal/set byte if zero
	_T("setne"),
	_T("setnz"),		// set byte if not equal/set byte if not zero
	_T("seta"),
	_T("setnbe"),		// set byte if above/set byte if not below or equal
	_T("setae"),
	_T("setnb"),
	_T("setnc"),
	_T("setb"),
	_T("setnae"),
	_T("setc"),
	_T("setbe"),
	_T("setna"),		// set byte if below or equal/set byte if not above
	_T("setg"),
	_T("setnle"),		// set byte if greater/set byte if not less or equal
	_T("setge"),
	_T("setnl"),		// set byte if greater or equal/set byte if not less

	_T("setl"),
	_T("setnge"),		// set byte if less/set byte if not greater or equal
	_T("setle"),
	_T("setng"),		// set byte if less or equal/set byte if not greater
	_T("sets"),		// set byte if sign (negative)
	_T("setns"),		// set byte if not sign (non-negative)
	_T("seto"),		// set byte if overflow
	_T("setno"),		// set byte if not overflow
	_T("setpe"),
	_T("setp"),		// set byte if parity even/set byte if parity
	_T("setpo"),
	_T("setnp"),		// set byte if parity odd/set byte if not parity
	_T("test"),		// logical compare
	_T("jmp"),		// jump
	_T("je"),
	_T("jz"),			// jump if equal/jump if zero
	_T("jne"),
	_T("jnz"),		// jump if not equal/jump if not zero
	_T("ja"),
	_T("jnbe"),		// jump if above/jump if not below or equal
	_T("jae"),
	_T("jnb"),		// jump if above or equal/jump if not below
	_T("jb"),
	_T("jnae"),		// jump if below/jump if not above or equal
	_T("jbe"),
	_T("jna"),		// jump if below or equal/jump if not above
	_T("jg"),
	_T("jnle"),		// jump if greater/jump if not less or equal
	_T("jge"),
	_T("jnl"),		// jump if greater or equal/jump if not less
	_T("jl"),
	_T("jnge"),		// jump if less/jump if not greater or equal
	_T("jle"),
	_T("jng"),		// jump if less or equal/jump if not greater
	_T("jc"),			// jump if carry
	_T("jnc"),		// jump if not carry
	_T("jo"),			// jump if overflow
	_T("jno"),		// jump if not overflow
	_T("js"),			// jump if sign (negative)
	_T("jns"),		// jump if not sign (non-negative)
	_T("jpo"),
	_T("jnp"),		// jump if parity odd/jump if not parity

	_T("jpe"),
	_T("jp"),			// jump if parity even/jump if parity
	_T("jcxz"),
	_T("jecxz"),		// jump register cx zero/jump register ecx zero
	_T("loop"),		// loop with ecx counter
	_T("loopz"),
	_T("loope"),
	_T("loopnz"),
	_T("loopne"),
	_T("call"),		// call procedure
	_T("ret"),		// return
	_T("retn"),		// return
	_T("iret"),		// return from interrupt
	_T("int"),		// software interrupt
	_T("into"),		// interrupt on overflow
	_T("bound"),		// detect value out of range
	_T("movs"),
	_T("movsb"),		// move string/move byte string
	_T("movsw"),		// move string/move word string
	_T("movsd"),		// move string/move doubleword string
	_T("cmps"),
	_T("cmpsb"),		// compare string/compare byte string
	_T("cmpsw"),		// compare string/compare word string
	_T("cmpsd"),		// compare string/compare doubleword string
	_T("scas"),
	_T("scasb"),		// scan string/scan byte string
	_T("scasw"),		// scan string/scan word string
	_T("scasd"),		// scan string/scan doubleword string
	_T("lods"),
	_T("lodsb"),		// load string/load byte string
	_T("lodsw"),		// load string/load word string
	_T("lodsd"),		// load string/load doubleword string
	_T("stos"),
	_T("stosb"),		// store string/store byte string
	_T("stosw"),		// store string/store word string
	_T("stosd"),		// store string/store doubleword string

	_T("rep movs"),
	_T("rep movsb"),		// move string/move byte string
	_T("rep movsw"),		// move string/move word string
	_T("rep movsd"),		// move string/move doubleword string
	_T("rep cmps"),
	_T("rep cmpsb"),		// compare string/compare byte string
	_T("rep cmpsw"),		// compare string/compare word string
	_T("rep cmpsd"),		// compare string/compare doubleword string
	_T("rep scas"),
	_T("rep scasb"),		// scan string/scan byte string
	_T("rep scasw"),		// scan string/scan word string
	_T("rep scasd"),		// scan string/scan doubleword string
	_T("rep lods"),
	_T("rep lodsb"),		// load string/load byte string
	_T("rep lodsw"),		// load string/load word string
	_T("rep lodsd"),		// load string/load doubleword string
	_T("rep stos"),
	_T("rep stosb"),		// store string/store byte string
	_T("rep stosw"),		// store string/store word string
	_T("rep stosd"),		// store string/store doubleword string

	_T("repe movs"),
	_T("repe movsb"),		// move string/move byte string
	_T("repe movsw"),		// move string/move word string
	_T("repe movsd"),		// move string/move doubleword string
	_T("repe cmps"),
	_T("repe cmpsb"),		// compare string/compare byte string
	_T("repe cmpsw"),		// compare string/compare word string
	_T("repe cmpsd"),		
	_T("repe scas"),
	_T("repe scasb"),		// scan string/scan byte string
	_T("repe scasw"),		// scan string/scan word string
	_T("repe scasd"),		// scan string/scan doubleword string
	_T("repe lods"),
	_T("repe lodsb"),		// load string/load byte string
	_T("repe lodsw"),		// load string/load word string
	_T("repe lodsd"),		// load string/load doubleword string
	_T("repe stos"),
	_T("repe stosb"),		// store string/store byte string
	_T("repe stosw"),		// store string/store word string
	_T("repe stosd"),		// store string/store doubleword string

	_T("repne movs"),
	_T("repne movsb"),		// move string/move byte string
	_T("repne movsw"),		// move string/move word string
	_T("repne movsd"),		// move string/move doubleword string
	_T("repne cmps"),
	_T("repne cmpsb"),		// compare string/compare byte string
	_T("repne cmpsw"),		// compare string/compare word string
	_T("repne cmpsd"),		
	_T("repne scas"),
	_T("repne scasb"),		// scan string/scan byte string
	_T("repne scasw"),		// scan string/scan word string
	_T("repne scasd"),		// scan string/scan doubleword string
	_T("repne lods"),
	_T("repne lodsb"),		// load string/load byte string
	_T("repne lodsw"),		// load string/load word string
	_T("repne lodsd"),		// load string/load doubleword string
	_T("repne stos"),
	_T("repne stosb"),		// store string/store byte string
	_T("repne stosw"),		// store string/store word string
	_T("repne stosd"),		// store string/store doubleword string


	_T("rep"),		//repeat while ecx not zero
	_T("repe"),
	_T("repz"),		// repeat while equal/repeat while zero
	_T("repne"),
	_T("repnz"),		// repeat while not equal/repeat while not zero
	_T("in"),			// read from a port
	_T("out"),		// write to a port
	_T("ins"),
	_T("insb"),		// input string from port/input byte string from port
	_T("insw"),		// input string from port/input word string from port
	_T("insd"),		// input string from port/input doubleword string from port
	_T("outs"),
	_T("outsb"),		// output string to port/output byte string to port
	_T("outsw"),		// output string to port/output word string to port
	_T("outsd"),
	_T("enter"),		// high-level procedure entry
	_T("leave"),		// high-level procedure exit
	_T("stc"),		// set carry flag
	_T("clc"),		// clear the carry flag
	_T("cmc"),		// complement the carry flag
	_T("cld"),		// clear the direction flag
	_T("std"),		// set direction flag
	_T("lahf"),		// load flags into ah register
	_T("sahf"),		// store ah register into flags
	_T("pushf"),
	_T("pushfd"),		// push eflags onto stack
	_T("popf"),
	_T("popfd"),		// pop eflags from stack
	_T("sti"),		// set interrupt flag
	_T("cli"),		// clear the interrupt flag
	_T("lds"),		// load far pointer using ds
	_T("les"),		// load far pointer using es
	_T("lfs"),		// load far pointer using fs
	_T("lgs"),		// load far pointer using gs
	_T("lss"),		// load far pointer using ss
	_T("lea"),		// load effective address
	_T("nop"),		// no operation
	_T("ud2"),		// undefined instruction
	_T("xlat"),
	_T("xlatb"),
	_T("cpuid"),

};

const TCHAR* x87FPUInstr[] = {
	//5.2.1 X87 FPU DATA TRANSFER INSTRUCTIONS

	_T("fld"),		// load floating-point value
	_T("fst"),		// store floating-point value
	_T("fstp"),		// store floating-point value and pop
	_T("fild"),		// load integer
	_T("fist"),		// store integer
	_T("fistp"),		// store integer and pop
	_T("fbld"),		// load bcd
	_T("fbstp"),		// store bcd and pop
	_T("fxch"),		// exchange registers
	_T("fcmove"),		// floating-point conditional move if equal
	_T("fcmovne"),	// floating-point conditional move if not equal
	_T("fcmovb"),		// floating-point conditional move if below
	_T("fcmovbe"),	// floating-point conditional move if below or equal
	_T("fcmovnb"),	// floating-point conditional move if not below
	_T("fcmovnbe"),	// floating-point conditional move if not below or equal
	_T("fcmovu"),		// floating-point conditional move if unordered
	_T("fcmovnu"),	// floating-point conditional move if not unordered


	//5.2.2 X87 FPU BASIC ARITHMETIC INSTRUCTIONS

	_T("fadd"),		// add floating-point
	_T("faddp"),		// add floating-point and pop
	_T("fiadd"),		// add integer
	_T("fsub"),		// subtract floating-point
	_T("fsubp"),		// subtract floating-point and pop

	_T("fisub"),		// subtract integer
	_T("fsubr"),		// subtract floating-point reverse
	_T("fsubrp"),		// subtract floating-point reverse and pop
	_T("fisubr"),		// subtract integer reverse
	_T("fmul"),		// multiply floating-point
	_T("fmulp"),		// multiply floating-point and pop
	_T("fimul"),		// multiply integer
	_T("fdiv"),		// divide floating-point
	_T("fdivp"),		// divide floating-point and pop
	_T("fidiv"),		// divide integer
	_T("fdivr"),		// divide floating-point reverse
	_T("fdivrp"),		// divide floating-point reverse and pop
	_T("fidivr"),		// divide integer reverse
	_T("fprem"),		// partial remainder
	_T("fprem1"),		// ieee partial remainder
	_T("fabs"),		// absolute value
	_T("fchs"),		// change sign
	_T("frndint"),	// round to integer
	_T("fscale"),		// scale by power of two
	_T("fsqrt"),		// square root
	_T("fxtract"),	// extract exponent and significand

	//5.2.3 X87 FPU COMPARISON INSTRUCTIONS

	_T("fcom"),		// compare floating-point
	_T("fcomp"),		// compare floating-point and pop
	_T("fcompp"),		// compare floating-point and pop twice
	_T("fucom"),		// unordered compare floating-point
	_T("fucomp"),		// unordered compare floating-point and pop
	_T("fucompp"),	// unordered compare floating-point and pop twice
	_T("ficom"),		// compare integer

	_T("ficomp"),		// compare integer and pop
	_T("fcomi"),		// compare floating-point and set eflags
	_T("fucomi"),		// unordered compare floating-point and set eflags
	_T("fcomip"),		// compare floating-point, set eflags, and pop
	_T("fucomip"),	// unordered compare floating-point, set eflags, and pop
	_T("ftst"),		// test floating-point (compare with 0.0)
	_T("fxam"),		// examine floating-point


	//5.2.4 X87 FPU TRANSCENDENTAL INSTRUCTIONS

	_T("fsin"),		// sine
	_T("fcos"),		// cosine
	_T("fsincos"),	// sine and cosine
	_T("fptan"),		// partial tangent
	_T("fpatan"),		// partial arctangent
	_T("f2xm1"),		// 2^x - 1
	_T("fyl2x"),		// y * log2x
	_T("fyl2xp1"),	// y * log2(x+1)

	//5.2.5 X87 FPU LOAD CONSTANTS INSTRUCTIONS

	_T("fld1"),		// load +1.0
	_T("fldz"),		// load +0.0
	_T("fldpi"),		// load pi
	_T("fldl2e"),		// load log2e
	_T("fldln2"),		// load loge2
	_T("fldl2t"),		// load log210
	_T("fldlg2"),		// load log102

	//5.2.6 X87 FPU CONTROL INSTRUCTIONS

	_T("fincstp"),	// increment fpu register stack pointer
	_T("fdecstp"),	// decrement fpu register stack pointer
	_T("ffree"),		// free floating-point register
	_T("finit"),		// initialize fpu after checking error conditions
	_T("fninit"),		// initialize fpu without checking error conditions
	_T("fclex"),		// clear floating-point exception flags after checking for error conditions
	_T("fnclex"),		// clear floating-point exception flags without checking for error conditions
	_T("fstcw"),		// store fpu control word after checking error conditions
	_T("fnstcw"),		// store fpu control word without checking error 	condition,			
	_T("fldcw"),		// load fpu control word
	_T("fstenv"),		// store fpu environment after checking error conditions
	_T("fnstenv"),	// store fpu environment without checking error 	condition,
	_T("fldenv"),		// load fpu environment
	_T("fsave"),		// save fpu state after checking error conditions
	_T("fnsave"),		// save fpu state without checking error conditions
	_T("frstor"),		// restore fpu state
	_T("fstsw"),		// store fpu status word after checking error conditions
	_T("fnstsw"),		// store fpu status word without checking error conditions
	_T("wait"),
	_T("fwait"),		// wait for fpu
	_T("fnop"),		// fpu no operation

};

const TCHAR* x87FPURegs[] = {
	_T("st"), _T("st(0)"), 
	_T("st(1)"), _T("st(2)"), 
	_T("st(3)"), _T("st(4)"), 
	_T("st(5)"), _T("st(6)"), 
	_T("st(7)"),
};

const TCHAR* asm_ptr_oper_name[] = {
	_T(" "),
	_T("byte ptr "),
	_T("word ptr "),
	_T("dword ptr "),
	_T("qword ptr "),
};

const TCHAR* var_operator_name[] = {
	_T("OFFSET "),
	_T("SIZEOF "),
	_T("LENGTHOF "),
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
HCCCodeGenerator::~HCCCodeGenerator()
{
	delete asm_unit_ptr;
}

assembly_source_unit& HCCCodeGenerator::unit()
{
	assert(!!asm_unit_ptr);
	return *asm_unit_ptr;
}

//-----------------------------------------------------
//	Emit Generic Labels : ie.: $E0005, $L0001.
//
//-----------------------------------------------------
__tstring HCCCodeGenerator::CreateLabel(const TCHAR *prefix, unsigned int index)
{
	__tstring label;
	TCHAR szLabel[50];

	_stprintf(szLabel, _T("%s%06d"), prefix, index);
	label = szLabel;
	return label;
}

__tstring HCCCodeGenerator::CreateVariableLabel(Symbol *variable_ptr)
{
	if(variable_ptr==NULL)
		return _T("");
	__tstring label;

	TCHAR szLabel[MAX_TOKEN_SIZE + 10];
	_stprintf(szLabel, _T("%s%06d"), getSymbolLabel(variable_ptr) , variable_ptr->getLabelIndex());
	label = szLabel;
	return label;
}


__tstring HCCCodeGenerator::getSymbolLabel(Symbol *symbl_ptr)
{
	assert(symbl_ptr!=0);
	__tstring label;
	if(symbl_ptr!=0)
	{
		const TCHAR* colon_string = _T("::");
		label = symbl_ptr->getName();
		int npos = label.find(colon_string);
		while(npos!=__tstring::npos)
		{
			label.replace(npos, 2, _T("_"));
			npos = label.find(colon_string, npos);
		}
	}
	return label;
}

__tstring HCCCodeGenerator::getLabelFromName(const __tstring &raw_name)
{
	const TCHAR* colon_string = _T("::");
	__tstring label = raw_name;

	int npos = label.find(colon_string);
	while(npos!=__tstring::npos)
	{
		label.replace(npos, 2, _T("_"));
		npos = label.find(colon_string, npos);
	}
	return label;
}


LPHCC_TOKEN HCCCodeGenerator::getToken()
{	
	delete token_ptr;
	token_ptr	= icode_ptr->getToken();
	token_type	= token_ptr->tokenType;
	symbol_ptr	= icode_ptr->symbol();
	return 0;
}

void HCCCodeGenerator::EmitNewLineComment(const __tstring &comment)
{
	unit() << endl << _T(";") << comment;
}

void HCCCodeGenerator::EmitNewLineComment(int line_number, const __tstring &comment)
{
	unit() << endl << _T(";{") << dec << line_number << _T("}: ") << comment;
}

void HCCCodeGenerator::EmitComment(const __tstring &comment)
{
	unit() << _T(";") << comment;
}

extern int g_program_stack_size;

bool HCCCodeGenerator::Generate(Symbol *entry_point_ptr)
{
	//this process cannot support any kind of errors
	if(HccErrorManager::errorCount() > 0)
		return false;

	EmitProgramProlog(MIN_CPU_REQUIREMENTS, 
					  g_program_stack_size, 
					  PROTECTED_MODE_MODEL, 
					  STDCALL_CALLING_CONVENTION);

	the_entry_point_ptr = entry_point_ptr;
	
	//C O D E   S E G M E N T 
	EmitCodeSegment();	
	//1. Generate all Procedures for all Classes...
	EmitProgramProcedures();
	//2. Generate all member initialization functions for all Classes...
	EmitObjectDataMembersInitForClasses();
	//3. Generate the global expressions init function...
	//TODO: fix bugs in point 3.
	//emitProgramGlobalsInit();
	if(entry_point_ptr!=0)
	{
		//4. Generate the main Entry Point	
		EmitMainEntryPoint(entry_point_ptr);
		//5. Generate the H++ runtime main startup...
		emitMainHPPRT_Startup(entry_point_ptr);
	}

	//6. emits class vtbls, and flags the program entry point...
	EmitProgramEpilog();

	unit().flush();
	unit().close();

	//Ok! No errors, so we can continue assemblying the resultant code file...
	return true;
	//return false;
}

void HCCCodeGenerator::EmitConstantComment(Symbol *symbl_const_ptr)
{	
	/*
		definition: const type-spec <identifier> = const-value;
	*/
	assert(symbl_const_ptr!=NULL && symbl_const_ptr->getDeclDefinition().identifier_type()==DECL_CONSTANT);
	if(symbl_const_ptr!=NULL && 
		symbl_const_ptr->getDeclDefinition().identifier_type()==DECL_CONSTANT)
	{
		unit() << endl << _T(";")
			   << _T("const ")
			   << symbl_const_ptr->getTypeSpecifier().getBaseTypeSpec()->getTypeName()
			   << space			    
			   << symbl_const_ptr->getName()
			   << _T(" = ");

		switch(symbl_const_ptr->getTypeSpecifier().getBaseTypeSpec()->getDataType())
		{
		case HCC_CHAR_TYPE:
			unit() << symbl_const_ptr->getDeclDefinition().constant.value.Character;
			break;
		case HCC_INTEGER:
			unit() << symbl_const_ptr->getDeclDefinition().constant.value.Integer;
			break;
		case HCC_FLOATING_POINT:
			unit() << symbl_const_ptr->getDeclDefinition().constant.value.Double;
			break;
		case HCC_STRING_TYPE:
			unit() << symbl_const_ptr->getDeclDefinition().constant.value.String;
		case HCC_BOOLEAN:
			unit() << (symbl_const_ptr->getDeclDefinition().constant.value.Boolean ? _T("true") : _T("false"));
			break;
		default:
			unit() << _T("(unknown)");
		};
		unit() << _T(";");		
	}
}

void HCCCodeGenerator::EmitVariableDeclComment()
{
	Symbol *symbl_variable_ptr = symbol_ptr;
	assert(symbl_variable_ptr!=NULL && symbl_variable_ptr->getDeclDefinition().identifier_type()==DECL_VARIABLE);
	if(symbl_variable_ptr!=NULL && 
		symbl_variable_ptr->getDeclDefinition().identifier_type()==DECL_VARIABLE)
	{
		while(token_type==HCC_IDENTIFIER)
		{
			unit() << endl << _T(";")
				   << symbl_variable_ptr->getTypeSpecifier().getBaseTypeSpec()->getTypeName()
				   << space
				   << symbl_variable_ptr->getName();
			getToken();		//skip token_ptr ==identifier_ptr;
			if(token_type==HCC_ASSIGN_OP)
			{
				unit() << _T(" = ");
				getToken(); //=
				EmitExprListComment();
			}			
			//next comma
			if(token_type==HCC_COMMA_OP || token_type==HCC_SEMICOLON)
				getToken();
			unit() << _T(";");
		}		
	}
}

void HCCCodeGenerator::EmitFunctionComment(Symbol *function_ptr)
{
	assert(function_ptr!=NULL);
	if(function_ptr!=NULL)
	{		
		TypeSpecifier* result_type_ptr = function_ptr->getDeclDefinition().function.return_type;
		__tstring result_type = result_type_ptr->getTypeName();
		if(result_type_ptr->getBaseTypeSpec()!=NULL)
			result_type = result_type_ptr->getBaseTypeSpec()->getTypeName();

		if(result_type==UNSIGNED_SHORT_NAME)
			result_type = _T("unsigned short");
		else if(result_type==UNSIGNED_LONG_NAME)
			result_type = _T("unsigned long ");
		else if(result_type==UNSIGNED_CHAR_NAME)
			result_type = _T("unsigned char ");

		unit() << endl << _T(";") 
			   << result_type
			   << space
			   << function_ptr->getName()
			   <<_T("(");		
		EmitFunctionParameterListComment(function_ptr);
		unit() << _T(");");
	}
}


void HCCCodeGenerator::EmitFunctionParameterListComment(Symbol *function_ptr)
{
	/* 
		definition: type-space <identifier> [,] param-list
	*/
	assert(function_ptr!=NULL);
	if(function_ptr!=NULL)
	{		
		vector<Symbol*>& param_list = function_ptr->getDeclDefinition().function.locals.stack_params.params;
		vector<Symbol*>::iterator it_param = param_list.begin();
		if(it_param==param_list.end())
			unit() << _T("void");		
		//
		else while(it_param != param_list.end())
		{
			Symbol* param_ptr = *it_param++;
			assert(param_ptr!=NULL);
			if(param_ptr!=NULL)
			{
				if(param_ptr->getTypeSpecifier().specifier()!=DSPEC_ARRAY)
				{
					const __tstring& param_type_name = param_ptr->getTypeSpecifier().getBaseTypeSpec()->getTypeName();
						if(param_type_name==UNSIGNED_SHORT_NAME)
							unit() << _T("unsigned short ");
						else if(param_type_name==UNSIGNED_LONG_NAME)
							unit() << _T("unsigned long ");
						else if(param_type_name==UNSIGNED_CHAR_NAME)
							unit() << _T("unsigned char ");
						else unit() << param_type_name
									<< space;
				}

				switch(param_ptr->getDeclDefinition().identifier_type())
				{
				case DECL_PARAM_BYREF:
					unit() << _T("ref ");
					break;
				case DECL_PARAM_CONST_BYREF:
					unit() << _T("const ref ");
					break;					
				case DECL_PARAM_POINTER:
					unit() << _T("^ ");
					break;
				case DECL_PARAM_CONST_POINTER:
					unit() << _T("const ^ ");
					break;
				case DECL_PARAM_ARRAY:
					{
						TypeSpecifier* array_type_ptr = param_ptr->getTypeSpecifier().getBaseTypeSpec(),
									   *scalar_type_ptr = getArrayScalarType(array_type_ptr);

						const __tstring& scalar_type_name = scalar_type_ptr->getTypeName();
						if(scalar_type_name==UNSIGNED_SHORT_NAME)
							unit() << _T("unsigned short ");
						else if(scalar_type_name==UNSIGNED_LONG_NAME)
							unit() << _T("unsigned long ");
						else if(scalar_type_name==UNSIGNED_CHAR_NAME)
							unit() << _T("unsigned char ");
						else unit() << scalar_type_name
									<< space;

						unit() << scalar_type_name 
							   << space;
					
						short nDims = (short)getArrayDimensionsFromType(array_type_ptr);

						if(scalar_type_ptr==HccTypeChecker::ts_type_string_ptr) 
							nDims--;

						while(nDims > 0)
						{
							unit() << _T("[]");
							nDims--;
						}
						unit() << space;
					}
					break;
				case DECL_PARAM_CONST_ARRAY:
					unit() << _T(" const []");
					break;
				}					
				unit() << param_ptr->getName();
			}
			if(it_param != param_list.end())
				unit() << comma << space;			// ','
		}		
	}
}

void HCCCodeGenerator::EmitFunctionLocalVariablesComment(Symbol *function_ptr)
{
	assert(function_ptr!=NULL);
	if(function_ptr!=NULL)
	{
		//L O C A L   C O N S T A N T S 
		list<Symbol*>& consts = function_ptr->getDeclDefinition().function.locals.stack_constants.constants;
		list<Symbol*>::iterator it_const = consts.begin();
		while(it_const != consts.end())
		{
			Symbol* const_ptr = *it_const++;
			assert(const_ptr!=NULL);
			if(const_ptr!=NULL)
				EmitConstantComment(const_ptr);

		}
		icode_ptr = theICodeResponsible.getFunctionICode(function_ptr, false);
		assert(icode_ptr!=NULL);
		if(icode_ptr!=NULL)
		{
			//save icode state...
			getToken();
			while(token_type==HCC_IDENTIFIER)
				EmitVariableDeclComment();			
			//restore the icode state...
		}		
	}
	icode_ptr = &g_icode_gen;
}

void HCCCodeGenerator::EmitCommaExprListComment()
{	
	EmitExprListComment();
	while(token_type==HCC_COMMA_OP)
	{
		//expr1, expr2, expr3,...,exprN
		unit() << comma << space;
		getToken(); // skip the ','		
		EmitExprListComment();
	}
}


void HCCCodeGenerator::EmitExprListComment()
{
	EmitExprComment();
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
		{
			unit() << space << symbolStrings[token_type] << space;
			getToken();
			//simple expression
			EmitExprListComment();
		}
		break;
	case HCC_TERNARY_OP:
		{
			emitting_ternary_expr = true;
			unit() << space 
				   << symbolStrings[token_type] //?
				   << space;
			getToken(); //?
			EmitExprListComment();			
			unit() << space 
				   << symbolStrings[HCC_COLON] //:
				   << space;

			getToken(); //:
			EmitExprListComment();
			emitting_ternary_expr = false;
			return;
		};
		break;
	};
}

void HCCCodeGenerator::EmitExprComment()
{
	//arithmetic operators
	//If unary op - | + , skip...	
	if(token_type==HCC_MINUS_OP || token_type==HCC_PLUS_OP){
		unit() << symbolStrings[token_type];
		getToken();
	}	
	EmitTermComment();
	for(;;)
		switch(token_type)
	{
	case HCC_PLUS_OP:				// +
	case HCC_MINUS_OP:				// -
		//B I T   S H I F T I N G   O P
	case HCC_LEFT_SHIFT_OP:			// <<
	case HCC_RIGHT_SHIFT_OP:		// >>	
		{
			unit() << space << symbolStrings[token_type] << space;
			//skip this operator
			getToken();
			EmitTermComment();
		}
		break;
	default:
		return;
		break;
	}
}

void HCCCodeGenerator::EmitTermComment()
{
	//arithmetic operators
	EmitFactorComment();
	for(;;)
		switch(token_type)
	{
	case HCC_MUL_OP:			// *
	case HCC_DIV_OP:			// /	
	case HCC_DIV:				// Integer Division	
	case HCC_MOD_OP:			// %
		{
			unit() << space << symbolStrings[token_type] << space;
			//skip this operator
			getToken();
			EmitFactorComment();
		}
		break;
	default:
		return;
		break;
	}
}

void HCCCodeGenerator::EmitFactorComment()
{
	//number
	//string
	//true/false/null

	//identifier = expr;
	//identifier[const-expr] = expr;
	//identifier(param-list);
	//identifier.identifier;

	switch(token_type)
	{
	case HCC_NUMBER:
		{
			unit() << token_ptr->String();
			getToken();
		}
		break;
	case HCC_IDENTIFIER:
		{
		unit() << token_ptr->String();
		getToken();

__ASSIGNMENT_EXPR_COMMENT:
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
				case HCC_COLON:				//labeled-statement
					{
						if(!emitting_ternary_expr)
						{
							unit() << token_ptr->String() << _T(' ');
							getToken();
							EmitCommaExprListComment();
						}
					}
					break;
				case HCC_PERIOD: //obj->member-access
					{
						unit() << token_ptr->String();
						getToken();
						EmitObjectInstanceComment();
					}
					break;
				case HCC_LBRACKET:	//id[x][y][...
					{
						while(token_type==HCC_LBRACKET)
						{
							unit() << token_ptr->String(); //[
							getToken();
							EmitCommaExprListComment();
							unit() << token_ptr->String(); //]
							getToken();							
						}
						goto __ASSIGNMENT_EXPR_COMMENT;
					}
					break;
				case HCC_LPAREN:	//function call
					{
						EmitCommaExprListComment();

						//BEGIN - ADDED Mar 01, 2009
						if(HCC_PERIOD==token_type)
						{
							unit() << token_ptr->String();
							getToken();
							EmitObjectInstanceComment();
						}
						//END - ADDED Mar 01, 2009
					}
					break;
				}
		}
	break;
	case HCC_LPAREN:	//(expr);
		{
			unit() << token_ptr->String(); //(
			getToken();
			EmitCommaExprListComment();
			unit() << token_ptr->String(); //)
			getToken();
		}
		break;
	case HCC_CONTROL_CHAR:
	case HCC_CHARACTER:
		{
			unit() << _T("\'") << token_ptr->value.Character << _T("\'");
			getToken();
		}
		break;
	case HCC_STRING_LITERAL:
	case HCC_TRUE:
	case HCC_FALSE:
	case HCC_NULL:	
		{
			unit() << token_ptr->String();			
			getToken();
		}
		break;	
	case HCC_SEMICOLON:
		{
			//unit() << token_ptr->String();			
		}
		break;
	case HCC_NOT_OP:		//!  : !1;   == 0
	case HCC_COMPL_OP:		//~  : ~0x0; == 0xFFFFFFFF;
	case HCC_INCREMENT:		//prefix ++ : ++id; == (id = id + 1)
	case HCC_DECREMENT:		//prefix -- : --id; == (id = id - 1)
		{
			unit() << token_ptr->String();
			getToken();	
			EmitFactorComment();
		}
		break;
	case HCC_NEW:
		{
			unit() << token_ptr->String();
			getToken();
			unit() << space;
			if(symbol_ptr!=NULL)
			{
				if(symbol_ptr->getTypeSpecifier().specifier()==DSPEC_ARRAY)
				{
					unit() << symbol_ptr->getTypeSpecifier().array.pItemType->getTypeName();

					getToken(); //type-class
					while(token_type==HCC_LBRACKET)
					{
						unit() << token_ptr->String(); //[
						getToken();
						EmitCommaExprListComment();
						unit() << token_ptr->String(); //]
						getToken();							
					}
				}else{
					EmitCommaExprListComment();
				}
			}
		}
		break;
	case HCC_DESTROY:
		{
			unit() << token_ptr->String();
			getToken();
			unit() << space;
			if(token_type==HCC_LBRACKET)
			{
				unit() << token_ptr->String(); //[
				getToken();
				unit() << token_ptr->String(); //]
				getToken();
			}
			EmitCommaExprListComment();
		}
		break;
	case HCC_SIZEOF:
		{
			unit() << token_ptr->String();
			getToken();
			unit() << space;
			EmitCommaExprListComment();
		}
		break;
	case HCC_DYNAMIC_CAST:
		{
			unit() << token_ptr->String();
			getToken();
			EmitCommaExprListComment();
		}
		break;
	case HCC_POINTER_ADDRESSOF:
	case HCC_POINTER_DEREFERENCE:
		{
			unit() << token_ptr->String();
			getToken();			
			EmitCommaExprListComment();
		}
		break;		
	}
}

void HCCCodeGenerator::EmitObjectInstanceComment()
{
	EmitFactorComment();
}

void HCCCodeGenerator::EmitStatementComment(Symbol *function_ptr)
{
	assert(function_ptr!=NULL);
	if(function_ptr!=NULL)
	{
		__tstring stmt_string;
		switch(token_type)
		{
		case HCC_IF:			stmt_string = (_T("if"));			goto __EXPRESSION_ANNOTATION;		break;
		case HCC_WHILE:			stmt_string = (_T("while"));		goto __EXPRESSION_ANNOTATION;		break;
		case HCC_DO:			EmitNewLineComment(_T("do\n"));			break;
		case HCC_FOR:
			{
				SaveICodePos();
				stmt_string = (_T("for("));
				getToken();	//keyword 'for'
				getToken();	//'('				
				TCHAR lpszLineNumber[10];
				_stprintf(lpszLineNumber, _T("{%ld}:"), icode_ptr->currentLineNumber());				
				EmitNewLineComment(lpszLineNumber);
				unit() << stmt_string;
				EmitCommaExprListComment(); //expr1
				getToken();					// the ';'
				unit() << _T("; ");
				EmitCommaExprListComment(); //expr2
				getToken();					// the ';'
				unit() << _T("; ");
				EmitCommaExprListComment(); //expr3
				getToken();					// the ')'
				unit() << _T(")");
				unit() << endl;
				RestoreICodePos();
				getToken();
			}	
			break;
		case HCC_SWITCH:		stmt_string = (_T("switch"));		goto __EXPRESSION_ANNOTATION;		break;		
		
			//{ statement-list; }
		case HCC_LBLOCK_KEY:	EmitComment(_T("{\n"));				break;
		case HCC_RBLOCK_KEY:	EmitComment(_T("}\n"));				break;
		
		case HCC_TRY:			EmitNewLineComment(_T("try\n"));			break;
		case HCC_WITH:
			{
				SaveICodePos();
				stmt_string = (_T("with"));
				getToken();	//keyword 'with'
				getToken();	//'('
				//the object identifier...
				Symbol* object_ptr = symbol_ptr;
				getToken(); //')'
				getToken(); //'{'
				TCHAR lpszLineNumber[10];
				_stprintf(lpszLineNumber, _T("{%ld}:"), icode_ptr->currentLineNumber());				
				EmitNewLineComment(lpszLineNumber);
				unit() << stmt_string << _T("(") << object_ptr->getName() << _T(")") << endl;
				RestoreICodePos();
				getToken();
			}
			break;		
		case HCC_CASE:			EmitNewLineComment(_T("case:\n"));			break;
		case HCC_DEFAULT:		EmitNewLineComment(_T("default:\n"));		break;
		case HCC_GOTO:			EmitNewLineComment(_T("goto\n"));			break;
		case HCC_RETURN:		stmt_string = (_T("return "));		goto __EXPRESSION_ANNOTATION;		break;
		case HCC_BREAK:			EmitNewLineComment(_T("break;\n"));			break;
		case HCC_CONTINUE:		EmitNewLineComment(_T("continue;\n"));		break;
		}
		goto __END_COMMENT_STMT;
__EXPRESSION_ANNOTATION:
				SaveICodePos();
				getToken(); //the 'keyword' must be skipped to print the expression...
				TCHAR lpszLineNumber[10];
				_stprintf(lpszLineNumber, _T("{%ld}:"), icode_ptr->currentLineNumber());				
				EmitNewLineComment(lpszLineNumber);
				unit() << stmt_string;
				EmitCommaExprListComment();
				unit() << endl;
				RestoreICodePos();
				getToken();
	}
__END_COMMENT_STMT:
	return;
}

extern bool bOptimizePrologAndEpilog;

void HCCCodeGenerator::EmitFunctionProlog(int stack_variables_size)
{
	CPUInstruction push(this, PUSH);
	CPUInstruction mov(this, MOV);
	CPUInstruction enter(this, ENTER);

	//EmitNewLineComment(_T("Prolog\n"));	
	unit() << endl;

	if(bOptimizePrologAndEpilog)
	{
		TCHAR lpszVariables[10];
		_stprintf(lpszVariables, _T("%05Xh"), stack_variables_size);

		enter(no_op_ptr(lpszVariables), 0);
	}else{
		push(EBP);
		mov(EBP, ESP);
		//
		if(stack_variables_size!=0)
		{
			CPUInstruction sub(this, SUB);
			CPUInstruction add(this, ADD);

			if(stack_variables_size > 0){
				if(stack_variables_size > sizeof(int))
					sub(ESP, stack_variables_size);	//sub esp, 20h
				else
					push(ECX);
			}
			else
				add(ESP, stack_variables_size); //add esp, -32
		}
	}
	//save general purpose registers
	push(EBX);
	push(ESI);
	push(EDI);

	return;
}

extern bool bOptimizeEpilogOnly;

void HCCCodeGenerator::EmitFunctionEpilog(int stack_params_size)
{
	CPUInstruction pop(this, POP);
	CPUInstruction mov(this, MOV);	
	CPUInstruction leave(this, LEAVE);

	unit() << endl;

	//restore general purpose registers before exitting procedure
	pop(EDI);
	pop(ESI);
	pop(EBX);

	if(bOptimizePrologAndEpilog || bOptimizeEpilogOnly)
	{
		leave();
	}else{
		mov(ESP, EBP);
		pop(EBP);
	}

	if(stack_params_size > 0)
	{
		CPUInstruction retn(this, RETN);
		retn(stack_params_size);
	}else{
		CPUInstruction ret(this, RET);
		ret();
	}
}

void HCCCodeGenerator::EmitMainEntryPoint(Symbol *entry_point_ptr)
{
	assert(entry_point_ptr!=NULL);
	if(entry_point_ptr!=NULL)
	{
		__tstring label = getSymbolLabel(entry_point_ptr);

		unit() << label << space << _T("PROC NEAR") << space;
		if(bSourceAnnotation)
			EmitFunctionComment(entry_point_ptr);

		int stack_size = entry_point_ptr->getDeclDefinition().function.locals.stack_variables.total_variables_size;
		int modulus = stack_size % sizeof(int);
		if(stack_size > 0 && modulus > 0){
			stack_size += sizeof(int) - modulus;
			//
			entry_point_ptr->getDeclDefinition().function.locals.stack_variables.total_variables_size = stack_size;
		}
		//
		EmitFunctionProlog(stack_size);
		CPUInstruction call(this, CALL);

		//add code for pattern 'CC' initialization of variables...
		if(bPatternInitStackVariables)
		{
			int stack_init_size_bytes = stack_size;
			if(stack_init_size_bytes > 0)
			{
				CPUInstruction mov(this, MOV);
				CPUInstruction lea(this, LEA);
				CPUInstruction rep_stos(this, REP_STOS);

				lea(EDI, dword_ptr(EBP, __minus, stack_init_size_bytes));
				mov(ECX, (int)(stack_init_size_bytes/4));
				mov(EAX, (int)0xCCCCCCCC);
				rep_stos(dword_ptr(EDI));				
			}
		}
		//the globals with expressions are initialized here in this call...
	//	call(no_op_ptr(USER_GLOBALS_INIT));

		//change to the function's icode object...
		icode_ptr = theICodeResponsible.getFunctionICode(entry_point_ptr, false);		
		//set the icode position to the beginning of the code (critical!!!)... 
		icode_ptr->reset();
		//get the first token in the intermediate code...
		getToken();
		//function body (statements and expressions)
		EmitFunctionBody(entry_point_ptr);
	//
		//this label is used to support the return statements
		EmitStatementLabel(entry_point_ptr->getLabelIndex());
		//call destructor for all variable object instances...
		//in reverse order...
		vector<Symbol*>::reverse_iterator it_obj = obj_instance_in_scope.rbegin();
		while(it_obj != obj_instance_in_scope.rend())
			//
			EmitDestructorCall(*it_obj++, entry_point_ptr);

		//clear the array list...
		obj_instance_in_scope.clear();
		//
		if(is_operand_in_stack)
		{
			EmitPopOperand(stack_operand_type);
			is_operand_in_stack = false;
		}

		int params_size = entry_point_ptr->getDeclDefinition().function.locals.stack_params.total_params_size;
		modulus = params_size % sizeof(int);
		if(params_size > 0 && modulus > 0)
			params_size += sizeof(int) - modulus;	
		//		
		EmitFunctionEpilog(params_size);		
		unit() << label << space << _T("ENDP") 
			   << endl 
			   << endl;
	}
}

void HCCCodeGenerator::EmitStatementLabel(int index)
{
	unit() << CreateLabel(STMT_LABEL_PREFIX, index) << _T(":") << endl;
}

void HCCCodeGenerator::EmitSEHStatementLabel(int index)
{
	unit() << CreateLabel(STRUCTURED_EXCEPTION_HANDLING_PREFIX, index) << _T(":") << endl;
}

//----------------------------------------------------------
//	EmitLoadValue		- Emit the load of a scalar value
//						  into EAX or EDX:EAX
//
//						- Cannot be used for Custom type greater than 64-bits
//----------------------------------------------------------
void HCCCodeGenerator::EmitLoadValue(Symbol *symbl_ptr)
{
	assert(symbl_ptr!=NULL);
	if(symbl_ptr==NULL)
		return;

	CPUInstruction mov(this, MOV);
	CPUInstruction xor(this, XOR);
	CPUInstruction lea(this, LEA);
	CPUInstruction movzx(this, MOVZX);
	CPUInstruction movsx(this, MOVSX);
	

	int offset = symbl_ptr->getDeclDefinition().user_data.offset;
	
	HCC_IDENTIFIER_SCOPE_TYPE scope_type = symbl_ptr->getDeclDefinition().identifier_scope();

	TypeSpecifier* ts_param_type = symbl_ptr->getTypeSpecifier().getBaseTypeSpec();	

	bool bIsUnsignedInt = HCC_UNSIGNED_TYPE==ts_param_type->getDataTypeModifier();

	switch(symbl_ptr->getDeclDefinition().identifier_type())
	{
	case DECL_PARAM_VALUE:
	case DECL_VARIABLE:	
		//params by value and variables
		{
			if(scope_type==SCOPE_LOCAL)
			{
				//L O C A L   V A R I A B L E S 
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;				
				if(ts_param_type->is_scalar())
				{
					if(ts_param_type==HccTypeChecker::ts_type_boolean_ptr || 
						ts_param_type->getDataType()==HCC_CHAR_TYPE)
					{
						if(bIsUnsignedInt)
							movzx(EAX, byte_ptr(EBP, op, abs(offset)));
						else
							movsx(EAX, byte_ptr(EBP, op, abs(offset)));
						//
					}else if(ts_param_type==HccTypeChecker::ts_type_short_ptr || 
							 ts_param_type->getDataTypeSize()==sizeof(short))
					{
						if(bIsUnsignedInt)
							movzx(EAX, word_ptr(EBP, op, abs(offset)));
						else
							movsx(EAX, word_ptr(EBP, op, abs(offset)));
						//
					}else if(ts_param_type==HccTypeChecker::ts_type_Int64_ptr)
					{
						//BEGIN - FIXED Dec 23, 2008
						//EDX:EAX
						mov(EAX, dword_ptr(EBP, op, (int)abs((int)offset)));				//low
						mov(EDX, dword_ptr(EBP, op, (int)abs((int)offset + (int)sizeof(int))));	//high
						//
						//BEGIN - FIXED Dec 23, 2008
					}else if(ts_param_type==HccTypeChecker::ts_type_double_ptr)
					{
						//EDX:EAX
						mov(EDX, dword_ptr(EBP, op, (int)abs((int)offset)));				//high
						mov(EAX, dword_ptr(EBP, op, (int)abs((int)offset + (int)sizeof(int))));	//low
						//
					}else
						mov(EAX, dword_ptr(EBP, op, abs(offset)));
					//
				}else if(ts_param_type->specifier()==DSPEC_CLASS)
					lea(ECX, dword_ptr(EBP, op, abs(offset)));
				else{ //for arrays and strings...
					if(ts_param_type->getDataType()==HCC_STRING_TYPE)
						mov(EAX, dword_ptr(EBP, op, abs(offset)));
					else
						lea(EAX, dword_ptr(EBP, op, abs(offset)));
				}				
			}else{
				//G L O B A L   V A R I A B L E S 
				__tstring label_name = getSymbolLabel(symbl_ptr);

				if(ts_param_type->is_scalar())
				{
					if(ts_param_type==HccTypeChecker::ts_type_boolean_ptr || 
						ts_param_type->getDataType()==HCC_CHAR_TYPE)
					{
						if(bIsUnsignedInt)
							movzx(EAX, byte_ptr(label_name));
						else
							movsx(EAX, byte_ptr(label_name));
						//
					}else if(ts_param_type==HccTypeChecker::ts_type_short_ptr || 
							 ts_param_type->getDataTypeSize()==sizeof(short))
					{
						if(bIsUnsignedInt)
							movzx(EAX, word_ptr(label_name));
						else
							movsx(EAX, word_ptr(label_name));
						//
					}else if(ts_param_type==HccTypeChecker::ts_type_Int64_ptr)
					{
						//BEGIN - FIXED Dec 23, 2008
						//EDX:EAX
						mov(EAX, dword_ptr(label_name));						//low
						mov(EDX, dword_ptr(label_name + QWORD_OFFSET_HIGH));	//high
						//END - FIXED Dec 23, 2008
					}else if(ts_param_type==HccTypeChecker::ts_type_double_ptr)
					{
						//EDX:EAX
						mov(EDX, dword_ptr(label_name));					//high
						mov(EAX, dword_ptr(label_name + QWORD_OFFSET_LOW));	//low
						//
					}else
						mov(EAX, no_op_ptr(label_name));
				}else if(ts_param_type->specifier()==DSPEC_CLASS)
					lea(ECX, no_op_ptr(label_name));
				else
					lea(EAX, no_op_ptr(label_name));				
			}
		}
		break;
	default:{
		assert(!"Undefined behavior for this decl definition type.");
			}
		break;
	};
}

void HCCCodeGenerator::EmitLoadNumber(const CONST_VALUE_INFO& valueInfo)
{
	//BEGIN - COMPILER OPTIMIZATION - Dec 27, 2008
	CPUInstruction mov(this, MOV);
	CPUInstruction xor(this, XOR);

	if(valueInfo.type_ptr==HccTypeChecker::ts_type_double_ptr)
	{
		if(valueInfo.edxVal==0)
		{
			if(valueInfo.eaxVal==0)
			{
				FPUInstruction fldz(this, FLDZ);
				fldz();
				is_floating_point_in_fpu = true;
			}else if(valueInfo.eaxVal==0x3ff00000) //1.0 ?
			{
				FPUInstruction fld1(this, FLD1);
				fld1();
				is_floating_point_in_fpu = true;
			}else{
				xor(EDX, EDX);
				mov(EAX, valueInfo.eaxVal);
			}
		}else{
			mov(EDX, valueInfo.edxVal);
			if(valueInfo.eaxVal==0)
				xor(EAX, EAX);
			else
				mov(EAX, valueInfo.eaxVal);
		}
	}else if(valueInfo.eaxVal==0)
	{
		if(valueInfo.edxVal==0)
		{
			if(valueInfo.type_ptr==HccTypeChecker::ts_type_Int64_ptr)
			{
				xor(EDX, EDX);
			}
			xor(EAX, EAX);
		}else{
			__asm int 3;
			/*
			//the value may be in the high part then...
			if(valueInfo.type_ptr==HccTypeChecker::ts_type_Int64_ptr)
			{
				xor(EDX, EDX);
			}
			mov(EAX, valueInfo.edxVal);
			*/
		}
	}else{
		//for large numbers, and floating points with two parts, this is the way to go
		if(valueInfo.type_ptr==HccTypeChecker::ts_type_Int64_ptr)
		{
			mov(EDX, valueInfo.edxVal);
		}
		mov(EAX, valueInfo.eaxVal);		
	}
	//END - COMPILER OPTIMIZATION - Dec 27, 2008
}

void HCCCodeGenerator::EmitConstNumberValue(Symbol *number_ptr, LPCONST_VALUE_INFO lpConstValue)
{
	assert(number_ptr!=NULL);
	if(number_ptr==NULL)
		return;

	assert(lpConstValue!=NULL);
	if(lpConstValue==NULL)
		return;

	FLOATING_POINT_CONVERSION& conv = fp_conv;

	if(number_ptr->getType()==HCC_NUMBER &&
		number_ptr->getDeclDefinition().identifier_type()==DECL_CONSTANT)
	{
		bool bNumberIsDouble = false;
		if(number_ptr->getTypeSpecifier().getBaseTypeSpec()==HccTypeChecker::ts_type_double_ptr){
			::FromDouble(number_ptr->getDeclDefinition().constant.value.Double);
			bNumberIsDouble = true;
		}
		else
			::FromInt64((__int64)number_ptr->getDeclDefinition().constant.value.Integer);

		TypeSpecifier* number_base_type_ptr = number_ptr->getTypeSpecifier().getBaseTypeSpec();

		lpConstValue->type_ptr = number_base_type_ptr;

		if(number_base_type_ptr==HccTypeChecker::ts_type_double_ptr)
		{
			lpConstValue->eaxVal = conv.dwLowPart;
			lpConstValue->edxVal = conv.dwHighPart;
			//
		}else if(conv.dwLowPart==0)
		{
			if(conv.dwHighPart!=0)
			{
				lpConstValue->eaxVal = conv.dwHighPart;
				lpConstValue->edxVal = 0;
			}else{
				lpConstValue->eaxVal = 0;
				lpConstValue->edxVal = 0;
			}
		}else{
			lpConstValue->eaxVal = conv.dwLowPart;
			lpConstValue->edxVal = conv.dwHighPart;
		}
	}
}

void HCCCodeGenerator::EmitPushNumber(Symbol *number_ptr)
{
	assert(number_ptr!=NULL);
	if(number_ptr==NULL)
		return;

	FLOATING_POINT_CONVERSION& conv = fp_conv;

	if(number_ptr->getType()==HCC_NUMBER &&
		number_ptr->getDeclDefinition().identifier_type()==DECL_CONSTANT)
	{
		CPUInstruction push(this, PUSH);
		if(number_ptr->getTypeSpecifier().getBaseTypeSpec()==HccTypeChecker::ts_type_double_ptr){
			::FromDouble(number_ptr->getDeclDefinition().constant.value.Double);
			//
			push(conv.dwLowPart);
			push(conv.dwHighPart);
		}
		else{
			::FromInt64(number_ptr->getDeclDefinition().constant.value.Integer);
			//FIXED - Dec 20, 2008
			push(conv.dwHighPart);
			push(conv.dwLowPart);
		}
	}
}

void HCCCodeGenerator::EmitFloatingPointLoad(Symbol *number_ptr, bool bDestroyTempVariable)
{
	assert(number_ptr!=NULL);
	if(number_ptr==NULL)
		return;

	FLOATING_POINT_CONVERSION& conv = fp_conv;

	if(number_ptr->getType()==HCC_NUMBER &&
		number_ptr->getDeclDefinition().identifier_type()==DECL_CONSTANT)
	{		
		if(number_ptr->getTypeSpecifier().getBaseTypeSpec()!=HccTypeChecker::ts_type_double_ptr)
		{
			assert(0);
			return;
		}

		CPUInstruction pop(this, POP);
		FPUInstruction fld(this, FLD);
		//
		EmitPushNumber(number_ptr);
		fld(qword_ptr(ESP));

		if(bDestroyTempVariable)
		{
			pop.set_new_comment(_T("popping double number from stack"));
			pop(ECX);
			pop(ECX);
		}
	}
}

void HCCCodeGenerator::EmitPushStringLiteral(Symbol *string_ptr)
{
	if(string_ptr==NULL)
		return;

	if(string_ptr->getType()==HCC_STRING_LITERAL)
	{
		CPUInstruction mov(this, MOV);
		CPUInstruction push(this, PUSH);

		__tstring string_lbl = CreateLabel(STRING_LABEL_PREFIX, string_ptr->getLabelIndex());

		mov(EAX, VAR_OFFSET, string_lbl);
		push(EAX);
	}
}

//-------------------------------------------------------------
//	 EmitPushAddressOf		-	Emit a push address_of onto the stack
//								for params and variables
//
//-------------------------------------------------------------
void HCCCodeGenerator::EmitPushAddressOf(Symbol *symbl_ptr, Symbol* function_ptr)
{
	assert(symbl_ptr!=NULL);
	if(symbl_ptr==NULL)
		return;	

	CPUInstruction mov(this, MOV);
	CPUInstruction lea(this, LEA);
	CPUInstruction push(this, PUSH);

	int offset = symbl_ptr->getDeclDefinition().user_data.offset;

	switch(symbl_ptr->getDeclDefinition().identifier_type())
	{		
	case DECL_PARAM_BYREF:			//params byref contain the address of a variable
	case DECL_PARAM_CONST_BYREF:

	case DECL_PARAM_POINTER:		//all of this are pointers; so also contain the address of a variable
	case DECL_PARAM_CONST_POINTER:			

	case DECL_PARAM_ARRAY:
	case DECL_PARAM_CONST_ARRAY:
		{
			//a param byref or pointer is passed to a function as an address in the stack;
			//therefore, this must be moved before pushing it again onto stack.
			if(symbl_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				//T H E   V A L U E 
				push(dword_ptr(EBP, __plus, offset));	//&i

			}else{
				//T H E   V A L U E 
				push(dword_ptr(getSymbolLabel(symbl_ptr)));	//&i
			}


			//in EAX, we have now the argument address, instead of the parameter address [EBP+offset]
			/*
			   some examples:

				void funct1(int^ ptr, int ref aref, int value, int [] array)
				{
					*ptr = 100;

					aref = 200;
				}

				for param ptr: 
						push dword ptr [ebp+8];
						

				for param aref:
						push dword ptr [ebp+0Ch];
						

				for param value:
						lea eax, dword ptr [ebp+10h]
						push eax;

				for param array:
						push dword ptr [ebp+14h]						
			*/
		}
		break;
	case DECL_POINTER_VARIABLE: //FIXED Mar 01, 2009
		{
			if(symbl_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				//T H E   V A L U E  I S   T H E   A D D R E S S 
				push(dword_ptr(EBP, __minus, offset));	//&i
				//
			}else{
				//T H E   V A L U E  I S   T H E   A D D R E S S 
				push(dword_ptr(getSymbolLabel(symbl_ptr)));	//&i
				//
			}
		}
		break;
	case DECL_NEW_DATA_MEMBER:
		{
			if(symbl_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				//BEGIN - CHANGED Jan 26, 2009
				/*
				if(function_ptr==DATA_MEMBERS_INIT_PROC_ADDRESS)
				{
					//the this pointer saved in the unique local variable for this init function...
					mov(ECX, dword_ptr(EBP, __minus, sizeof(int)));
				}else if(function_ptr!=NULL)
				{
					//the variable offset where to look for the this pointer...
					int this_ptr_localv_offset =
							function_ptr->getDeclDefinition().function.locals.stack_variables.total_variables_size;
					//the this pointer saved at the last local variable for this function_ptr
					mov(ECX, dword_ptr(EBP, __minus, this_ptr_localv_offset));
				}
				*/
				EmitLoadObjectInstancePointer_This(function_ptr);
				//END - CHANGED Jan 26, 2009
				//the data member...
				if(offset > 0 )					
					push(dword_ptr(ECX, __plus, offset));
				else
					push(dword_ptr(ECX));
			}else
				push(VAR_OFFSET, getSymbolLabel(symbl_ptr));
		}
		break;
	case DECL_PARAM_VALUE:
		{
			lea(EAX, dword_ptr(EBP, __plus, abs(offset)));
			push(EAX);
		}
		break;	
	case DECL_VARIABLE:		
		{
			if(symbl_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
				lea(EAX, dword_ptr(EBP, __minus, abs(offset)));
			else 
				lea(EAX, VAR_OFFSET, getSymbolLabel(symbl_ptr));
			push(EAX);
		}
		break;
	default:
		{
			push(VAR_OFFSET, getSymbolLabel(symbl_ptr));
		}
		break;
	};
}

void HCCCodeGenerator::EmitPromoteToFloatingPoint(TypeSpecifier *type_ptr, bool bSaveOntoStack)
{
	assert(type_ptr!=NULL);
	if(type_ptr==NULL)
		return;
	//promotion to: EAX, or EDX:EAX
	//the value must be already in the stack located from ESP
	if(type_ptr->getDataType()!=HCC_FLOATING_POINT)
	{
		CPUInstruction call(this, CALL);
		CPUInstruction push(this, PUSH);
		if(type_ptr==HccTypeChecker::ts_type_Int64_ptr)
			call(no_op_ptr(FLOATING_POINT_FROM_INT64));
			//now, can be located from ESP
		else
			call(no_op_ptr(FLOATING_POINT_FROM_INT));

		if(bSaveOntoStack)
		{
			push(EAX);	//low
			push(EDX);	//high
		}
	}
}

void HCCCodeGenerator::EmitProgramProlog(const __tstring& min_cpu, 
										 int stack_size, 
										 const __tstring& mem_model, 
										 const __tstring& calling_conv)
{
	unit() << _T("TITLE") << ctab << _T(" ") << hpp_source_file
		   << endl
		   << _T("SUBTITLE ") << _T("This file was automatically generated by the 32-bit H++ Compiler")
		   << endl
		   << endl
		   << endl
		   << ctab << min_cpu 
		   << endl
		   << ctab << _T(".MODEL ") << mem_model << comma << calling_conv
		   << endl
		   << ctab << _T(".STACK ") << stack_size 
		   << endl
		   << ctab << _T("INCLUDE stdhpp\\hcclib32.INC	; the H++ library.")
		   << endl
		   << endl
		   << _T(";Windows API prototypes")
		   << endl
		   << _T("GetVersion PROTO")
		   << endl
		   << endl
		   << _T(";This assembly file was generated by the 32-bit H++ Compiler")
		   << endl
		   << endl;
}

void HCCCodeGenerator::EmitCodeSegment()
{
	unit() << _T(".CODE ;code segment") 
		   << endl;
}

void HCCCodeGenerator::EmitDataSegment()
{
	unit() << _T(".DATA ;data segment") 
		   << endl;
}


void HCCCodeGenerator::EmitProgramEpilog()
{
	//D A T A   S E G M E N T 
	EmitDataSegment();	

	//V T B L   D E F I N I T I O N S
	EmitClassVtables();

	//7. Generate all Constants as Assembly Symbolic Constants (strings, numbers, chars, etc)
	EmitGlobalConstants();

	//8. Generate all Global Variables
	//9. Generate all Global Variables Expressions for assignments.
	EmitGlobalVariables();	

	if(the_entry_point_ptr!=NULL)
	{
		//to denote the program entry point
		unit() << _T("END") << space << SYSTEM_HPP_RUNTIME_INIT
			   << endl
			   << endl;
	}else 
		unit() << _T("END")
			   << endl
			   << endl;
}

extern GlobalVariables g_program_global_variables;

extern GlobalVariables g_program_global_constants;

#ifndef MAX_COMPUTERNAME_LENGTH
#define MAX_COMPUTERNAME_LENGTH 31
#endif

void HCCCodeGenerator::EmitGlobalVariables()
{	
	// 1rst: Create the Data Segment, and System Global variables

	//S Y S T E M   G L O B A L S 
	unit() << __OSVER		<< _T(" \t\tDWORD \t0") << _T(";Windows OS version")		<< endl;
	unit() << __WINMINOR	<< _T(" \t\tDWORD \t0") << _T(";Windows minor version")		<< endl;
	unit() << __WINMAJOR	<< _T(" \t\tDWORD \t0") << _T(";Windows major version")		<< endl;
	unit() << __WINVER		<< _T(" \t\tDWORD \t0") << _T(";Windows version")			<< endl;
	unit() << __COMPUTER	<< _T(" \t\tBYTE \t")	
							<< hex 
							<< MAX_COMPUTERNAME_LENGTH + 1 
							<< _T(" DUP(?)") << _T(";Computer name")	<< endl;

	unit() << ___ARGV		<< _T(" \t\tDWORD \t0") << _T(";Array of argument values")	<< endl;
	unit() << ___ARGC		<< _T(" \t\tDWORD \t0") << _T(";Argument count")			<< endl;
	

	GlobalVariables::iterator it = g_program_global_variables.begin();
	__tstring label;
	while(it != g_program_global_variables.end())
	{
		Symbol* global_ptr = *it++;
		//
		assert(global_ptr!=NULL);
		if(global_ptr==NULL)
			continue;
		//this function return the correct name based on namespaces or class boundaries
		label = getSymbolLabel(global_ptr);
		unit() << label << ctab << space;
		TypeSpecifier* type_ptr = global_ptr->getTypeSpecifier().getBaseTypeSpec();
		assert(type_ptr!=NULL);
		if(type_ptr!=NULL && 
			global_ptr->getDeclDefinition().identifier_type()!=DECL_POINTER_VARIABLE)
		{
			if(type_ptr==HccTypeChecker::ts_type_char_ptr || 
				type_ptr==HccTypeChecker::ts_type_boolean_ptr ||
				type_ptr->getDataTypeSize()==sizeof(char))
			{
				unit() << _T("BYTE\t0");
				//
			}else if(type_ptr==HccTypeChecker::ts_type_short_ptr || 
					type_ptr->getDataTypeSize()==sizeof(short))
			{
				unit() << _T("WORD\t0");
				//
			}else if(type_ptr==HccTypeChecker::ts_type_int_ptr ||
					type_ptr==HccTypeChecker::ts_type_long_ptr ||
					type_ptr->getDataTypeSize()==sizeof(int))
			{
				unit() << _T("DWORD\t0");
				//
			}else if(type_ptr==HccTypeChecker::ts_type_Int64_ptr)
			{
				unit() << _T("DQ\t0");
				//
			}else if(type_ptr==HccTypeChecker::ts_type_double_ptr ||
					 type_ptr->getDataTypeSize()==sizeof(double))
			{
				unit() << _T("REAL8\t0.0");
				//
			}else if(false==type_ptr->is_scalar())	//strings, object instances, others.
			{
				if(type_ptr->specifier()==DSPEC_ARRAY)
				{
					TypeSpecifier* scalar_type_ptr = getArrayScalarType(type_ptr);
					int array_count = getArrayCountFromType(type_ptr);

					if(scalar_type_ptr==HccTypeChecker::ts_type_short_ptr || 
							scalar_type_ptr==HccTypeChecker::ts_type_Int16_ptr)
					{
						unit() << _T("WORD ") << hex << array_count << space
							   << _T("DUP(?) ");
						//
					}else if(scalar_type_ptr==HccTypeChecker::ts_type_Int32_ptr ||
						scalar_type_ptr==HccTypeChecker::ts_type_int_ptr ||
						scalar_type_ptr==HccTypeChecker::ts_type_long_ptr)
					{
						unit() << _T("DWORD ") << hex << array_count << space
							   << _T("DUP(?) ");
						//
					}else if(scalar_type_ptr==HccTypeChecker::ts_type_Int64_ptr)
					{
						unit() << _T("DQ ") << hex << array_count << space
							   << _T("DUP(?) ");
						//
					}else if(scalar_type_ptr==HccTypeChecker::ts_type_double_ptr)
					{
						unit() << _T("REAL8 ") << hex << array_count << space
							   << _T("DUP(?) ");
						//
					}else{
						unit() << _T("BYTE ") << hex << array_count << space
							   << _T("DUP(?) ");
					}
				}else
				{
					unit() << _T("BYTE ")
						   << type_ptr->getDataTypeSize() << space
						   << _T("DUP(?) ");
				}
			}
		}else{
			unit() << _T("DWORD\t0");	//for pointers
		}

		unit() << endl;
	}	
	unit() << endl;
	g_program_global_variables.clear();
	//Next: Create the Expression Initialization blocks
}

void HCCCodeGenerator::EmitGlobalConstants()
{
	__tstring label;
	GlobalVariables::iterator it = g_program_global_constants.begin();
	while(it != g_program_global_constants.end())
	{
		Symbol* global_ptr = *it++;
		//
		assert(global_ptr!=NULL);
		if(global_ptr==NULL)
			continue;
		//this function return the correct name based on namespaces or class boundaries
		label = getSymbolLabel(global_ptr);
		unit() << label << ctab;
		TypeSpecifier* type_ptr = global_ptr->getTypeSpecifier().getBaseTypeSpec();
		assert(type_ptr!=NULL);
		if(type_ptr!=NULL && 
			global_ptr->getDeclDefinition().identifier_type()==DECL_CONSTANT)
		{
			if(type_ptr==HccTypeChecker::ts_type_char_ptr)
			{
				unit() << _T("=\t\'") << global_ptr->getDeclDefinition().constant.value.Character << _T("\'");
				//
			}else if(type_ptr==HccTypeChecker::ts_type_boolean_ptr)
			{
				unit() << _T("=\t") << (int)global_ptr->getDeclDefinition().constant.value.Boolean;
				//
			}else if(type_ptr->getDataType()==HCC_INTEGER && type_ptr->is_scalar())
			{
				if(type_ptr->getDataTypeSize()==sizeof(__int64))
					unit() << _T("QWORD\t") << global_ptr->getDeclDefinition().constant.value.Integer;
				else
				//F O R   A L L   I N T E G E R S 
					unit() << _T("=\t") << (int)global_ptr->getDeclDefinition().constant.value.Integer;
				//
			}else if(type_ptr==HccTypeChecker::ts_type_double_ptr ||
					 type_ptr==HccTypeChecker::ts_type_float_ptr)
			{
				TCHAR szDoubleValue[100];
				_stprintf(szDoubleValue, _T("%f"), global_ptr->getDeclDefinition().constant.value.Double);
				if(type_ptr==HccTypeChecker::ts_type_double_ptr)
					unit() << _T("REAL8\t");
				else
					unit() << _T("REAL4\t");
				unit() << szDoubleValue;
				//
			}else if(false==type_ptr->is_scalar())	//strings, object instances, others.
			{
				if(global_ptr->getDeclDefinition().constant.value.String!=NULL)
					unit() << _T("BYTE \"") 
						   << global_ptr->getDeclDefinition().constant.value.String
						   << _T("\",0"); //generate it as a null-terminated string...
				else 
					unit() << _T("BYTE \"(*undefined string constant*)\"");
			}
		}
		unit() << endl;
	}
	unit() << endl;

	//Emit all literal constants as global variables...	
	SYMBOL_TABLE::iterator it_symbl = symbol_table_ptr->begin();	
	while(it_symbl != symbol_table_ptr->end())
	{
		Symbol* symbol_ptr = (*it_symbl).second;		

		TCHAR* ptr_leak = NULL;
		__uint pos = 0;
	try{
		if(symbol_ptr!=NULL && symbol_ptr->getType()==HCC_STRING_LITERAL) //S T R I N G S  &  C H A R S 
		{
			//for now, we'll have null-terminated strings
			label = CreateLabel(STRING_LABEL_PREFIX, symbol_ptr->getLabelIndex());
			//
			const __tstring& text = symbol_ptr->getName();
			TCHAR* escaped_string_buffer = ptr_leak = new TCHAR[(int)(text.length()*1.75) + sizeof(TCHAR)];
			//__uint pos = 0;
			__tstring::const_iterator it_ch = text.begin();
			volatile bool _bGotChars = false;
			__uint nNormals = 0;
			volatile bool _bPrevIsEscChar = false;
			while(it_ch != text.end())
			{
				_bGotChars = (nNormals > 0);
				TCHAR chr = *it_ch++;
				if(chr==_T('\\'))
				{
					//P R O C E S S   E S C A P E   C H A R A C T E R S
					if(_bPrevIsEscChar){
						escaped_string_buffer[pos++] = _T(',');
						_bPrevIsEscChar = false; //turn off here
					}
					if(it_ch == text.end())
						break;
					chr = *it_ch++;
					nNormals = 0; //reset the counter
					if(_bGotChars){
						if(chr!=_T('\"') && chr!=_T('\'') && chr!=_T('\\'))
						{
							escaped_string_buffer[pos++] = 0x22;
							escaped_string_buffer[pos++] = _T(',');
						}
					}
					
					switch(chr)
					{
					case _T('\"'): //0x22
						{
							escaped_string_buffer[pos++] = 0x22;
						}
						break;
					case _T('\''): //0x27
						{
							escaped_string_buffer[pos++] = 0x27;
						}
						break;
					case _T('\\'): //0x5C
						escaped_string_buffer[pos] = 0x5C; // inverted
						break;
					case _T('t'): //0x09
						{
							escaped_string_buffer[pos++] = _T('0');escaped_string_buffer[pos++] = _T('9');
							escaped_string_buffer[pos++] = _T('h');
							_bPrevIsEscChar = true;
						}
						break;
					case _T('r'): //0x0D
						{
							escaped_string_buffer[pos++] = _T('0');escaped_string_buffer[pos++] = _T('D');
							escaped_string_buffer[pos++] = _T('h');
							_bPrevIsEscChar = true;
						}
						break;
					case _T('n'): //0x0A
						{
							escaped_string_buffer[pos++] = _T('0');escaped_string_buffer[pos++] = _T('A');
							escaped_string_buffer[pos++] = _T('h');
							_bPrevIsEscChar = true;
						}
						break;
					case _T('f'): //0x0C
						{
							escaped_string_buffer[pos++] = _T('0');escaped_string_buffer[pos++] = _T('C');
							escaped_string_buffer[pos++] = _T('h');
							_bPrevIsEscChar = true;
						}
						break;
					case _T('b'): //0x08
						{
							escaped_string_buffer[pos++] = _T('0');escaped_string_buffer[pos++] = _T('8');
							escaped_string_buffer[pos++] = _T('h');
							_bPrevIsEscChar = true;
						}
						break;
					case _T('0'): // '\0' --> 0x00
						{
							escaped_string_buffer[pos++] = _T('0');escaped_string_buffer[pos++] = _T('0');
							escaped_string_buffer[pos++] = _T('h');
							_bPrevIsEscChar = true;
						}
						break;
					default:
						__asm int 3; //should never get here!
						break;
					};
				}else{
					//"\nA simple string\n";	-->0Ah,"A simple string",0Ah,0
					//"\n\nA simple string\n";  -->0Ah,0Ah,"A simple string",0Ah,0
					//"A simple string\n";		-->"A simple string",0Ah,0
					//"\nAsimple\nstring\n";	-->0Ah,"A simple",0Ah,"string",0Ah,0
					
					escaped_string_buffer[pos] = chr; //normal characters
					//next char is a escape sequence?
					if(it_ch != text.end())
					{
						//next char is esc char?
						if(*it_ch==_T('\\'))
						{
							if(pos < 1)
								continue;	
						}
						
						if(_bPrevIsEscChar)
						{
							if(*it_ch==_T('\\'))
								continue;
							else
							{
								escaped_string_buffer[pos++]	= _T(',');
								escaped_string_buffer[pos++]	= 0x22;
								escaped_string_buffer[pos]		= chr; //normal characters
								_bPrevIsEscChar = false; //turn off here
							}
						}
					}else if(_bPrevIsEscChar)
							break;

					nNormals++;
					pos++;
				}
			}

			escaped_string_buffer[pos] = _T('\0'); //null-terminate the string before generating the definition
			//cout << _T("debugging label: ") << label << " -> " << escaped_string_buffer << endl;
			//is a null string?
			if(escaped_string_buffer[0]==_T('\"') && escaped_string_buffer[1]==_T('\"'))
				unit() << label << _T(" \t\tBYTE \t0,0 ;null-string") << endl;
			else //is a full string?
				unit() << label << _T(" \t\tBYTE \t") << escaped_string_buffer << _T(",0") << endl;
			//to avoid memory leaks
			delete []escaped_string_buffer;
		}
		
	}catch(...){ 
		//FIXED - Mar 07, 2009 - AV for first symbol in XP
	cout << _T("FATAL ERROR: Failed processing the following partial string:");
		if(ptr_leak!=NULL){
			ptr_leak[pos] = 0;
			cout << ptr_leak;
		}	
	}
		//next symbol..
		it_symbl++;
	}
	unit() << endl;
}

void HCCCodeGenerator::EmitFunction(Symbol *function_ptr)
{
	assert(function_ptr!=NULL);
	if(function_ptr==NULL)
		return;

	CPUInstruction mov(this, MOV);

	__tstring label = getLabelFromName(function_ptr->getCompleteName());

	unit() << label << space << _T("PROC NEAR") << space;
	//
	if(bSourceAnnotation)
		EmitFunctionComment(function_ptr);

	int stack_size = function_ptr->getDeclDefinition().function.locals.stack_variables.total_variables_size;
	/*
	int modulus = stack_size % sizeof(int);
	if(stack_size > 0 && modulus > 0)
		stack_size += sizeof(int) - modulus;
	*/
	//we leave space for the this pointer's variable holder...
	stack_size += sizeof(int);
	EmitFunctionProlog(stack_size);
	//we changed the stack size, so, we must keep this data saved...
	function_ptr->getDeclDefinition().function.locals.stack_variables.total_variables_size = stack_size;
	//change to the function's icode object...
	icode_ptr = theICodeResponsible.getFunctionICode(function_ptr, false);
	assert(icode_ptr!=NULL);
	//set the icode position to the beginning of the code (critical!!!)... 
	icode_ptr->reset();
	//get the first token in the intermediate code...
	getToken();
	//save the this pointer (at the last variable) before executing any kind of expression...
	if(function_ptr->getDeclDefinition().identifier_type()!=DECL_NEW_STATIC_CLASS_MEMBER)
	{
		if(bOptimizePrologAndEpilog || stack_size > sizeof(int))
		{
			//for object member functions only (if optimized with 'enter', or this function has stack variables)
			mov(dword_ptr(EBP, __minus, stack_size), ECX);
		}
	}
	//add code for pattern 'CC' initialization of variables...
	if(bPatternInitStackVariables)
	{
		int stack_init_size_bytes = stack_size - sizeof(int);
		if(stack_init_size_bytes > 0)
		{
			CPUInstruction lea(this, LEA);
			CPUInstruction rep_stos(this, REP_STOS);

			lea(EDI, dword_ptr(EBP, __minus, stack_init_size_bytes));
			mov(ECX, (int)(stack_init_size_bytes/4));
			mov(EAX, (int)0xCCCCCCCC);
			rep_stos(dword_ptr(EDI));			

			if(function_ptr->getDeclDefinition().identifier_type()!=DECL_NEW_STATIC_CLASS_MEMBER)
			{
				//for object member functions only
				mov(ECX, dword_ptr(EBP, __minus, stack_size));
			}
		}		
	}
	//function body (statements and expressions)
	EmitFunctionBody(function_ptr);
	//
	int params_size = function_ptr->getDeclDefinition().function.locals.stack_params.total_params_size;
	/*
	modulus = params_size % sizeof(int);
	if(params_size > 0 && modulus > 0)
		params_size += sizeof(int) - modulus;
	*/
	//this label is used to support the return statements
	EmitStatementLabel(function_ptr->getLabelIndex());
	//call destructor for all variable object instances...
	//in reverse order...
	vector<Symbol*>::reverse_iterator it_obj = obj_instance_in_scope.rbegin();
	while(it_obj != obj_instance_in_scope.rend())
	{
		if((*it_obj)->getTypeSpecifier().specifier()==DSPEC_ARRAY)
		{
			EmitLoadValue(*it_obj);
			mov(ESI, EAX);
			EmitMultipleDestructorCall(&(*it_obj)->getTypeSpecifier());
		}else
			EmitDestructorCall(*it_obj, function_ptr);

		//next object variable...
		it_obj++;
	}

	//clear the array list...
	obj_instance_in_scope.clear();
	//
	if(is_operand_in_stack)
	{
		EmitPopOperand(stack_operand_type);
		is_operand_in_stack = false;
	}
	EmitFunctionEpilog(params_size);
	unit() << label << space << _T("ENDP") 
		   << endl 
		   << endl;
}

void HCCCodeGenerator::EmitFunctionBody(Symbol *function_ptr)
{
	assert(function_ptr!=NULL);
	if(function_ptr==NULL)
		return;
	if(bSourceAnnotation && function_ptr!=NULL)
		EmitStatementComment(function_ptr);	
	emitCompoundStatement(function_ptr); // { statement; statement-list... }
}

void HCCCodeGenerator::EmitProgramProcedures()
{
	//DECL_NEW_VIRTUAL_ABSTRACT_FUNC_MEMBER,	//a virtual function in an abstract class
	SYMBOL_TABLE::iterator it_symbl = symbol_table_ptr->begin();	
	while(it_symbl != symbol_table_ptr->end())
	{
		Symbol* symbl_ptr = (*it_symbl).second;

		DECLARATION_TYPE symbl_type = symbl_ptr->getDeclDefinition().identifier_type();
		if(symbl_ptr->getTypeSpecifier().specifier()==DSPEC_CLASS)
		{
			 LPSYMBOL_TABLE symbl_tbl_ptr = symbl_ptr->getTypeSpecifier().getSymbolTable();

			 volatile bool bHasConstructor = false;

			 SYMBOL_TABLE::iterator it_member = symbl_tbl_ptr->begin();
			 while(it_member!= symbl_tbl_ptr->end())
			 {		
				Symbol* fn_symbol_ptr = (*it_member).second;
				DECLARATION_TYPE symbl_type = fn_symbol_ptr->getDeclDefinition().identifier_type();

				if(symbl_type==DECL_NEW_FUNC_MEMBER			|| 			//a function in a class
				   symbl_type==DECL_NEW_VIRTUAL_FUNC_MEMBER)			//a virtual function in a class					   
				{					
					{
						EmitFunction(fn_symbol_ptr);
					}
				}
				else if(symbl_type==DECL_NEW_CLASS_CONSTRUCTOR) 			//a constructor member
				{
					bHasConstructor = true;
					EmitClassConstructor(fn_symbol_ptr, symbl_ptr);
				}
				else if(symbl_type==DECL_UNIQUE_DESTRUCTOR)					//a object's destructor for a class
				{
					EmitFunction(fn_symbol_ptr);
				}
				else if(symbl_type==DECL_READONLY_PROPERTY)			//a read-only property
				{
					//if null, this symbol belongs to a read/write property
					Symbol* property_get_ptr = fn_symbol_ptr->getDeclDefinition().function.property[_T("get")];					

					if(property_get_ptr!=NULL)
						EmitFunction(property_get_ptr);
				}
				else if(symbl_type==DECL_WRITEONLY_PROPERTY)		//a write-only property
				{
					//if null, this symbol belongs to a read/write property
					Symbol* property_put_ptr = fn_symbol_ptr->getDeclDefinition().function.property[_T("put")];					

					if(property_put_ptr!=NULL)
						EmitFunction(property_put_ptr);
				}
				else if(symbl_type==DECL_READWRITE_PROPERTY)		//a read-write property
				{
					Symbol* property_get_ptr = fn_symbol_ptr->getDeclDefinition().function.property[_T("get")];
					assert(property_get_ptr!=NULL);

					Symbol* property_put_ptr = fn_symbol_ptr->getDeclDefinition().function.property[_T("put")];
					assert(property_put_ptr!=NULL);

					if(property_get_ptr!=NULL)
						EmitFunction(property_get_ptr);

					if(property_put_ptr!=NULL)
						EmitFunction(property_put_ptr);
				}
				it_member++;
			 }

			 //we have to create a default constructor, if this type has none;
			 //only if this type have at least data members, or virtual functions...
			 if(false==bHasConstructor)
			 {
				 if(IsTypeQualifiedForDefaultConstructor(symbl_ptr->getTypeSpecifier()))
				 {
						Symbol* ctor_ptr = CreateDefaultClassConstructor(symbl_ptr);
						//emit it now...
						EmitClassConstructor(ctor_ptr, symbl_ptr);
				 }
			 }
		}else if(symbl_type==DECL_NEW_STATIC_CLASS_MEMBER)			//a static function in a class	
		{
			if(false==symbl_ptr->getDeclDefinition().symbol_is_external() && symbl_ptr!=the_entry_point_ptr)
			{
				EmitFunction(symbl_ptr);
			}
		}
		it_symbl++;
	}
	//restore to the global icode ptr
	icode_ptr = &g_icode_gen;
}

void HCCCodeGenerator::EmitActualParameterList(Symbol *callee_fn_ptr, Symbol* caller_fn_ptr)
{
	assert(callee_fn_ptr!=NULL);
	if(callee_fn_ptr==NULL)
		return;

	vector<Symbol*> &params = callee_fn_ptr->getDeclDefinition().function.locals.stack_params.params;
	
	//process each actual parameter expression one by one, from left to right;
	//then persist each argument expression code in a stack; that way we can 
	//pass each parameter onto the stack from right to left as done in Visual C++ and other compilers.
	//
	stack<__tstring> code_params;

	CPUInstruction push(this, PUSH);
	CPUInstruction lea(this, LEA);
	CPUInstruction call(this, CALL);

	getToken(); //'('

	//save the unit file ptr
	__tostream* unit_file_ptr = unit().detach();

	vector<Symbol*>::iterator it_param = params.begin();
	while(it_param != params.end())
	{		
		Symbol* param_ptr = *it_param++;
		//
		__tostring_stream* str_stream_ptr = new __tostring_stream();
		unit().attach(str_stream_ptr);
		//impl.
		DECLARATION_TYPE param_type = param_ptr->getDeclDefinition().identifier_type();

		if(param_type==DECL_PARAM_BYREF || param_type==DECL_PARAM_CONST_BYREF)
		{
			if(HCC_POINTER_DEREFERENCE==token_type)
				getToken();
			assert(token_type==HCC_IDENTIFIER);
			//BEGIN - FIXED Jan 26, 2009
			TypeSpecifier* actual_type_ptr = symbol_ptr->getTypeSpecifier().getBaseTypeSpec();
			//Required when passing an object from an array, using a subscript expression like: array[expr] to a byref param
			if(actual_type_ptr->specifier()==DSPEC_ARRAY)
			{
				//BEGIN - FIXED Jan 27, 2009
				bRequiresAddressOf = true;
				//END - FIXED Jan 27, 2009
				actual_type_ptr = EmitFactor(caller_fn_ptr);
				if(actual_type_ptr->specifier()==DSPEC_CLASS)
					push(ECX);
				else
					push(EAX);
				bRequiresAddressOf = false; //FIXED Jan 27, 2009
			}else{
				EmitPushAddressOf(symbol_ptr, caller_fn_ptr); //FIXED - Mar 04, 2009
				getToken(); //skip id
			}
			//END - FIXED Jan 26, 2009
		}else if(param_type!=DECL_PARAM_VALUE)	//is a pointer param?
		{
			TypeSpecifier* actual_type_ptr = EmitExprList(caller_fn_ptr);
			assert(actual_type_ptr!=NULL);
			//any pointer value
			push(EAX);
			//
		}else{
			//G E N E R A T E   E X P R E S S I O N 
			TypeSpecifier* actual_type_ptr = EmitExprList(caller_fn_ptr);
			assert(actual_type_ptr!=NULL);

			TypeSpecifier* formal_type_ptr = param_ptr->getTypeSpecifier().getBaseTypeSpec();

			//F L O A T I N G   P O I N T   I M P L I C I T   C O N V E R S I O N   F R O M   I N T E G E R
			if(formal_type_ptr==HccTypeChecker::ts_type_double_ptr || 
				formal_type_ptr==HccTypeChecker::ts_type_float_ptr)
			{
				//do implicit conversion
				if(actual_type_ptr!=HccTypeChecker::ts_type_double_ptr &&
					actual_type_ptr!=HccTypeChecker::ts_type_float_ptr)
				{
					if(actual_type_ptr!=HccTypeChecker::ts_type_Int64_ptr){
						if(!is_operand_in_stack)
						{
							push(EAX);
						}
						is_operand_in_stack = false;
					}else{
						if(!is_operand_in_stack)
						{
							push(EDX);
							push(EAX); //Fixed Dec 27, 2008
						}
						is_operand_in_stack = false;
					}
					EmitPromoteToFloatingPoint(actual_type_ptr);
				}else{
					//Floating Point argument
					if(!is_operand_in_stack)
					{
						push(EAX);
						push(EDX);
					}
					is_operand_in_stack = false;
				}
			}else if(formal_type_ptr->is_scalar())
			{
				if(formal_type_ptr->getDataType()==HCC_INTEGER && 
					actual_type_ptr->getDataType()==HCC_FLOATING_POINT)
				{
					if(is_operand_in_stack)
					{
						if(formal_type_ptr->getDataTypeSize()==sizeof(__int64))
						{
							call(no_op_ptr(FLOATING_POINT_TO_INT64));
							//push(EAX);
							push(EDX);
							push(EAX); //Fixed Dec 27, 2008
						}else{
							call(no_op_ptr(FLOATING_POINT_TO_INT));
							push(EAX);
						}
					}else{
						if(formal_type_ptr->getDataTypeSize()==sizeof(__int64))
						{
							push(EAX);
							push(EDX);
							call(no_op_ptr(FLOATING_POINT_TO_INT64));
							//push(EAX);
							push(EDX);
							push(EAX); //Fixed Dec 27, 2008
						}else{
							push(EAX);
							push(EDX);
							call(no_op_ptr(FLOATING_POINT_TO_INT));
							push(EAX);
						}
					}
					is_operand_in_stack = false;
				}else{
					//BEGIN - FIXED Dec 28, 2008
					if(!is_operand_in_stack)
					{
						if(formal_type_ptr->getDataType()==HCC_INTEGER &&
							formal_type_ptr->getDataTypeSize()==sizeof(__int64))
						{
							push(EDX); //high part
							push(EAX); //low part
						}else if(formal_type_ptr->getDataType()==HCC_FLOATING_POINT &&
							formal_type_ptr->getDataTypeSize()==sizeof(double))
						{
							push(EAX); //low part
							push(EDX); //high part
						}else
							push(EAX);
					}
					//END - FIXED Dec 28, 2008
				}
			}
				//S T R I N G   A R R A Y S : S T R I N G   D A T A   T Y P E 
			else if(formal_type_ptr->getDataType()==HCC_STRING_TYPE && 
					(actual_type_ptr->getDataType()==HCC_STRING_TYPE ||
					 (
						actual_type_ptr->specifier()==DSPEC_ARRAY && 
						actual_type_ptr->array.pItemType==HccTypeChecker::ts_type_char_ptr
					 )
					)					
				 )
			{
				push(EAX);
				//A L L   O T H E R   A R R A Y S 
			}else if(formal_type_ptr->specifier()==DSPEC_ARRAY &&
					 actual_type_ptr->specifier()==DSPEC_ARRAY)
			{
				push(EAX);
			}
			else
			{
				CPUInstruction cld(this, CLD);
				CPUInstruction mov(this, MOV);
				CPUInstruction sub(this, SUB);
				CPUInstruction pop(this, POP);
				CPUInstruction rep_movsb(this, REP_MOVSB);
				
				//COPY BLOCK
				int size = actual_type_ptr->getDataTypeSize();
				//allocate an even memory block for this param...
				//EYE OPEN: may be necessary to reserve to a multiple of sizeof(int);
				int alloc_space = size & 1 ? size + 1 : size;
				//				

				cld(); //clear direction flag...				
				mov(ESI, ECX); //the this pointer is loaded in ECX

				sub(ESP, alloc_space);
				mov(EDI, ESP);
				mov(ECX, size);
				rep_movsb.set_new_comment(_T("param passed by value."));
				rep_movsb();
			}
		}
		//
		//save the param's assembly code in reverse order
		__tstring code = str_stream_ptr->str();
		code_params.push(code);

		if(it_param != params.end())
			getToken(); // ','
	}

	getToken(); //')'

	//restore the unit file ptr
	unit().attach(unit_file_ptr);
	//pass actual parameters onto stack...
	while(!code_params.empty())
	{
		__tstring paramx = code_params.top();
		unit() << paramx;		//to debug what we are passing onto stack
		//unit() << code_params.top();
		code_params.pop();
	}
}


void HCCCodeGenerator::EmitFunctionCall(Symbol *callee_fn_ptr, Symbol* caller_fn_ptr, Symbol* object_ptr, x86_REGISTER x86RegThisPointer)
{
	assert(callee_fn_ptr!=NULL);
	if(callee_fn_ptr==NULL)
		return;

	DECLARATION_TYPE callee_decl_type = 
							callee_fn_ptr->getDeclDefinition().identifier_type();

	assert((token_type==HCC_LPAREN) || 
		   (token_type!=HCC_LPAREN && 
		    callee_fn_ptr->getDeclDefinition().identifier_type()==DECL_NEW_CLASS_CONSTRUCTOR));
	//emit the argument list...
	if(token_type==HCC_LPAREN)
	{
		EmitActualParameterList(callee_fn_ptr, caller_fn_ptr);
	}
	//write the code to call this function...
	if(caller_fn_ptr!=NULL && 
		caller_fn_ptr!=DATA_MEMBERS_INIT_PROC_ADDRESS)
	{	
		DECLARATION_TYPE caller_decl_type = caller_fn_ptr->getDeclDefinition().identifier_type();
		if(
		   (
			caller_decl_type!=DECL_NEW_STATIC_CLASS_MEMBER				//a static function in a class	
			) 
			&&
		   (
			callee_decl_type!=DECL_NEW_STATIC_CLASS_MEMBER				//a static function in a class	
			)
			&&
			(
				object_ptr==NULL && 
					(
						DECL_NEW_FUNC_MEMBER==callee_decl_type			||
						DECL_NEW_VIRTUAL_FUNC_MEMBER==callee_decl_type	||
						DECL_NEW_VIRTUAL_ABSTRACT_FUNC_MEMBER==callee_decl_type
					)
			)
		   )	//a virtual function in a class	
		{
			//mov the this pointer to ECX register...
			//the variable offset where to look for the this pointer...
			int this_ptr_localv_offset =
					caller_fn_ptr->getDeclDefinition().function.locals.stack_variables.total_variables_size;

			CPUInstruction mov(this, MOV);
			//the this pointer...
			mov(ECX, dword_ptr(EBP, __minus, this_ptr_localv_offset));

			//BEGIN - ADDED Mar 04, 2009
			//for virtual members
			if(DECL_NEW_VIRTUAL_FUNC_MEMBER==callee_decl_type ||
			   DECL_NEW_VIRTUAL_ABSTRACT_FUNC_MEMBER==callee_decl_type)
			{
				Symbol* class_symbl_ptr = callee_fn_ptr->getOwner();
				TypeSpecifier* baseTypeSpec = &class_symbl_ptr->getTypeSpecifier();
				emitVirtualMemberFunctionCall(NULL, callee_fn_ptr, baseTypeSpec, caller_fn_ptr, false);
			}
			//END - ADDED Mar 04, 2009
		}
	//
	}

	if(object_ptr!=NULL)
	{
		EmitLoadValueAddressOf(object_ptr, caller_fn_ptr, true);
	}

	//BEGIN - FIXED Mar 04, 2009
	if(x86RegThisPointer!=ECX)
	{
		CPUInstruction mov(this, MOV);
		mov(ECX, x86RegThisPointer);
	}
	//END - FIXED Mar 04, 2009
	if(DECL_NEW_VIRTUAL_FUNC_MEMBER!=callee_decl_type && DECL_NEW_VIRTUAL_ABSTRACT_FUNC_MEMBER!=callee_decl_type)
	{
		CPUInstruction call(this, CALL);
		call(no_op_ptr(getLabelFromName(callee_fn_ptr->getCompleteName())));
	}
	//this time, we are only supporting stdcall calling convension,
	//that's why we are not clearing the top of the stack from params,
	//because each function takes care of it.

	//BEGIN - ADDED Mar 1, 2009
	if(token_type==HCC_PERIOD)
	{
		CPUInstruction mov(this, MOV);
		mov(ECX, EAX);
		TypeSpecifier* pItemType = callee_fn_ptr->getDeclDefinition().function.return_type;
		//the this pointer is now in ECX as expected!
		emitObjectInstanceMember(NULL, pItemType, caller_fn_ptr);
	}
	//END - ADDED Mar 1, 2009
}

//-----------------------------------------------
//	EmitExpression	- Emit a expression with
//				  binary operators : + | - | << | >>
//-----------------------------------------------
TypeSpecifier* HCCCodeGenerator::EmitExpression(Symbol *function_ptr)
{	
	HCC_TOKEN_TYPE unaryOp = HCC_PLUS_OP;

	CPUInstruction neg(this, NEG);
	CPUInstruction mov(this, MOV);
	CPUInstruction push(this, PUSH);
	CPUInstruction pop(this, POP);
	CPUInstruction add(this, ADD);
	CPUInstruction sub(this, SUB);
	CPUInstruction shl(this, SHL);
	CPUInstruction shr(this, SHR);

	FPUInstruction fld(this, FLD);
	FPUInstruction fstp(this, FSTP);
	FPUInstruction fistp(this, FISTP);
	FPUInstruction fchs(this, FCHS);
	FPUInstruction fadd(this, FADD);
	FPUInstruction fsub(this, FSUB);
	
	//If unary op - | + , skip...
	if(token_type==HCC_PLUS_OP || token_type==HCC_MINUS_OP)
	{
		unaryOp = token_type;
		getToken();
	}	

	TypeSpecifier* operand1_type = EmitTerm(function_ptr); //result in EAX | EDX:EAX
	//assert(operand1_type!=NULL);

	for(;operand1_type!=NULL;)
		switch(token_type)
	{
	case HCC_PLUS_OP:				// +
	case HCC_MINUS_OP:				// -
		{
			if(unaryOp==HCC_MINUS_OP)
			{				
				changeOperandSign(operand1_type, unaryOp);
				//to avoid changing sign again in the next sub-expr
				unaryOp = HCC_PLUS_OP;
			}
			//operand1 onto stack...			
			if(!is_floating_point_in_fpu)
			{
				if(!is_operand_in_stack)
					EmitPushOperand(operand1_type);
			}else{
				push(ECX);
				push(ECX);
				fstp(qword_ptr(ESP));
				is_floating_point_in_fpu = false;
			}

			is_operand_in_stack = false;

			HCC_TOKEN_TYPE _operator = token_type;
			//skip this operator
			getToken();
			TypeSpecifier* operand2_type = EmitTerm(function_ptr); //result in EAX | EDX:EAX
			//
			volatile bool bIsInt64Data = false;
			if(operand1_type==HccTypeChecker::ts_type_Int64_ptr || 
				operand2_type==HccTypeChecker::ts_type_Int64_ptr)
			{
				//for Int64 data, we convert them to floating point, then we extract then as Int64 at the end.
				bIsInt64Data = true;
				goto __FLOATING_POINT_OPERATION;
				//				
			}else if( (operand1_type->is_scalar() && operand2_type->is_scalar())
					  &&
					  (operand1_type->getDataType()!=HCC_FLOATING_POINT &&
					   operand2_type->getDataType()!=HCC_FLOATING_POINT)
					)
			{
				//I N T E G E R S   O N L Y 
				//execute operation between operands...
				if(_operator==HCC_PLUS_OP)
				{
					pop(EDX);
					add(EAX, EDX);
				}else{
					pop(EDX);
					sub(EDX, EAX);
					mov(EAX, EDX);
				}

				//next sub-expr
				//BEGIN - FIXED Mar 03, 2009
				if(HCC_UNSIGNED_TYPE==operand1_type->getDataTypeModifier() &&
					HCC_UNSIGNED_TYPE==operand2_type->getDataTypeModifier())
				{
					operand1_type = HccTypeChecker::ts_type_unsigned_ptr;
				}else
					operand1_type = HccTypeChecker::ts_type_Int32_ptr;
				//END - FIXED Mar 03, 2009
			}else if(operand1_type->getDataType()==HCC_FLOATING_POINT ||
					 operand2_type->getDataType()==HCC_FLOATING_POINT)
			{
__FLOATING_POINT_OPERATION:
				if(!is_floating_point_in_fpu)
				{
					if(!is_operand_in_stack)
						EmitPushOperand(operand2_type);
					
					if(operand2_type->getDataType()!=HCC_FLOATING_POINT)
						EmitPromoteToFloatingPoint(operand2_type);

					is_operand_in_stack = false;

					//both operands exist on top of the CPU stack...
					//we must move the operand on top of the stack, to the 
					fld(qword_ptr(ESP)); //operand2 sent to the FPU stack...
					pop(ECX);	//pop it off the CPU stack
					pop(ECX);					
				}
				is_floating_point_in_fpu = false;

				if(operand1_type->getDataType()!=HCC_FLOATING_POINT)
					EmitPromoteToFloatingPoint(operand1_type);

				//execute operation between operands...
				if(_operator==HCC_PLUS_OP)
				{
					fadd(qword_ptr(ESP));	// st = st + qword_ptr(ESP)
				}else{
					fld(qword_ptr(ESP)); //operand1 sent to the FPU stack...					
					//now, we have: 
					// st(0) = operand1
					// st(1) = operand2
					fsub(st, st1);
				}
				if(false==bIsInt64Data){
					fstp(qword_ptr(ESP));	//the result from st(0)...floating-point
					//next sub-expr
					operand1_type = HccTypeChecker::ts_type_double_ptr;
				}
				else{
					fistp(qword_ptr(ESP));	//the result from st(0)...int64
					//next sub-expr
					operand1_type = HccTypeChecker::ts_type_Int64_ptr;
				}
				//the result in EDX:EAX
				is_operand_in_stack = true;
				stack_operand_type = operand1_type;
				//pop(EDX);
				//pop(EAX);
			}
		}
		break;
		//B I T   S H I F T I N G   O P
	case HCC_LEFT_SHIFT_OP:			// <<
	case HCC_RIGHT_SHIFT_OP:		// >>	
		{
			HCC_TOKEN_TYPE _operator = token_type;
			//skip this operator
			getToken();

			//BEGIN - OPTIMIZED Jan 17, 2009
			if(token_type==HCC_NUMBER)
			{
				TypeSpecifier* result_type_ptr = NULL;
				assert(symbol_ptr!=NULL);
				if(symbol_ptr!=NULL){
					result_type_ptr = symbol_ptr->getTypeSpecifier().getBaseTypeSpec();
					assert(result_type_ptr!=NULL);
				}
				else
					result_type_ptr = HccTypeChecker::ts_type_int_ptr;
				
				lastConstValueInfo.type_ptr = NULL; //no type means that the assignment is not direct
				CONST_VALUE_INFO cvInfo(result_type_ptr);
				EmitConstNumberValue(symbol_ptr, &cvInfo);
				getToken();
				//

				if(is_operand_in_stack)
					EmitPopOperand(operand1_type);
				//
				if(operand1_type==HccTypeChecker::ts_type_Int64_ptr)
				{
					CPUInstruction shld(this, SHLD);
					CPUInstruction shrd(this, SHRD);

					if(_operator==HCC_LEFT_SHIFT_OP)
						shld(EAX, EDX, cvInfo.eaxVal);
					else
						shrd(EAX, EDX, cvInfo.eaxVal);
					//next sub-expr
					operand1_type = HccTypeChecker::ts_type_Int64_ptr;
				}else{
					if(_operator==HCC_LEFT_SHIFT_OP)
						shl(EAX, cvInfo.eaxVal);
					else
						shr(EAX, cvInfo.eaxVal);
					//next sub-expr

					//BEGIN - FIXED Mar 03, 2009
					if(HCC_UNSIGNED_TYPE==operand1_type->getDataTypeModifier())
						operand1_type = HccTypeChecker::ts_type_unsigned_ptr;
					else
						operand1_type = HccTypeChecker::ts_type_int_ptr;
					//END - FIXED Mar 03, 2009
				}
				//END - FIRST OPTIMIZATION
			}else{
				if(!is_operand_in_stack)
					EmitPushOperand(operand1_type);
				is_operand_in_stack = false;
				
				TypeSpecifier* operand2_type = EmitTerm(function_ptr);
				//
				// x << y == integer type
				// x >> y == integer type
				//
				//the count operand must be set in the 8-bit CL register
				mov(CL, AL);
				if(operand1_type==HccTypeChecker::ts_type_Int64_ptr)
				{
					if(_operator==HCC_LEFT_SHIFT_OP)
						shl(qword_ptr(ESP), CL);
					else
						shr(qword_ptr(ESP), CL);

					EmitPopOperand(operand1_type);
					//next sub-expr
					operand1_type = HccTypeChecker::ts_type_Int64_ptr;
				}else{
					pop(EAX);
					if(_operator==HCC_LEFT_SHIFT_OP)
						shl(EAX, CL);
					else
						shr(EAX, CL);
					//next sub-expr
					//BEGIN - FIXED Mar 03, 2009
					if(HCC_UNSIGNED_TYPE==operand1_type->getDataTypeModifier())
						operand1_type = HccTypeChecker::ts_type_unsigned_ptr;
					else
						operand1_type = HccTypeChecker::ts_type_int_ptr;
					//END - FIXED Mar 03, 2009
				}
				//END - SECOND OPTIMIZATION
			}
			//END - OPTIMIZED Jan 17, 2009
		}
		break;
	default:
		{
			if(unaryOp==HCC_MINUS_OP)
				changeOperandSign(operand1_type, unaryOp);

			return operand1_type;
		}
		break;
	};
	
	return operand1_type;
}

//-----------------------------------------------
//	EmitTerm	- Emit a expression with
//				  binary operators : * | / | div | % 
//-----------------------------------------------
TypeSpecifier* HCCCodeGenerator::EmitTerm(Symbol *function_ptr)
{
	CPUInstruction push(this, PUSH);
	CPUInstruction pop(this, POP);
	CPUInstruction imul(this, IMUL);
	CPUInstruction idiv(this, IDIV);
	CPUInstruction cdqe(this, CDQ);
	CPUInstruction xor(this, XOR);
	CPUInstruction mov(this, MOV);
	CPUInstruction or(this, OR);
	CPUInstruction shl(this, SHL);

	FPUInstruction fld(this, FLD);
	FPUInstruction fstp(this, FSTP);
	FPUInstruction fistp(this, FISTP);

	FPUInstruction fmul(this, FMUL);
	FPUInstruction fdivr(this, FDIVR);
	FPUInstruction fclex(this, FCLEX);	

	TypeSpecifier* operand1_type = EmitFactor(function_ptr);
	//assert(operand1_type!=NULL);	

	for(;operand1_type!=NULL;)
		switch(token_type)
	{
	case HCC_MUL_OP:			// *
	case HCC_DIV_OP:			// /	
		{
			if(!is_floating_point_in_fpu)
			{
				if(!is_operand_in_stack)
					EmitPushOperand(operand1_type);				
			}else{
				push(ECX);
				push(ECX);
				fstp(qword_ptr(ESP));
				is_floating_point_in_fpu = false;
			}
			is_operand_in_stack = false;

			HCC_TOKEN_TYPE _operator = token_type;
			getToken();

			TypeSpecifier* operand2_type = EmitFactor(function_ptr);

			volatile bool bIsInt64Data = false;
			if(operand1_type==HccTypeChecker::ts_type_Int64_ptr ||
				//
				operand2_type==HccTypeChecker::ts_type_Int64_ptr)
			{
				//we turn the Int64 into a floating point value, then we operate them and 
				//if multiplication is executed, we set it as Int64, else, if division, 
				//we leave it as floating point value
				bIsInt64Data = true;
				goto __FLOATING_POINT_OPERATION;
				//
			}else if( (operand1_type->getDataType()==HCC_INTEGER && operand1_type->is_scalar())
				   &&
					  (operand2_type->getDataType()==HCC_INTEGER && operand2_type->is_scalar())
					)
			{
				//I N T E G E R S   O N L Y 
				//intx * intx = intx
				//								
				if(_operator==HCC_MUL_OP)
				{
					pop(ECX);
					if(operand1_type->getDataTypeSize()==sizeof(int) &&
						operand2_type->getDataTypeSize()==sizeof(int))
					{
						cdqe();		//if the multiplication exceeds 32-bits, then we prepare for a 64-bit result...
						imul(EAX, ECX);
						//the result is a 64-bit number...
						//BEGIN - Fix for operations involving only int 32 bits - March 13, 2011
						//operand1_type=HccTypeChecker::ts_type_Int64_ptr;
						operand1_type=HccTypeChecker::ts_type_int_ptr;
						//END - Fix for operations involving only int 32 bits - March 13, 2011
						//
					}else{
						xor(EDX, EDX);
						//imul(AX, CX);
						imul(EAX, ECX);
						//shl(EDX, 0x10);	//to avoid having the result in DX:AX we convert it to EAX only...
						//or(EAX, EDX);
						//the result is a 32-bit number...						
						//BEGIN - FIXED Mar 03, 2009
						if(HCC_UNSIGNED_TYPE==operand1_type->getDataTypeModifier() &&
							HCC_UNSIGNED_TYPE==operand2_type->getDataTypeModifier())
						{
							operand1_type = HccTypeChecker::ts_type_unsigned_ptr;
						}else
							operand1_type = HccTypeChecker::ts_type_int_ptr;
						//END - FIXED Mar 03, 2009
						//
					}
				}else{				
					//intx / intx = floating-point
					//the division between integers, gives a floating point result; that's why we do this operation
					//in the floating point section...
					goto __FLOATING_POINT_OPERATION;
				}
				//
			}else if(operand1_type->getDataType()==HCC_FLOATING_POINT || 
					 operand2_type->getDataType()==HCC_FLOATING_POINT )
			{
__FLOATING_POINT_OPERATION:									
				fclex();	//clear floating point exceptions that may exists, before doing a new fp operation...

				if(!is_floating_point_in_fpu)
				{
					if(!is_operand_in_stack)
						EmitPushOperand(operand2_type);

					if(operand2_type->getDataType()!=HCC_FLOATING_POINT)
						EmitPromoteToFloatingPoint(operand2_type);

					//(ESP + 8	== operand1);
					//(ESP		== operand2);
					fld(qword_ptr(ESP));		//operand2;
					pop(ECX);
					pop(ECX);
				}
				is_floating_point_in_fpu = false;

				//these operators provoke implicit conversions between operands to floating-point type
				if(operand1_type->getDataType()!=HCC_FLOATING_POINT)
					EmitPromoteToFloatingPoint(operand1_type);			

				is_operand_in_stack = false;

				if(_operator==HCC_MUL_OP)
				{
					fmul(qword_ptr(ESP));	//st = st * operand1;

					if(bIsInt64Data && operand2_type->getDataType()!=HCC_FLOATING_POINT)
					{
						operand1_type = HccTypeChecker::ts_type_Int64_ptr;
						//get an int64 result...
						fistp(qword_ptr(ESP));
					}
					else{
						operand1_type = HccTypeChecker::ts_type_double_ptr;
						//get a double precision result...
						fstp(qword_ptr(ESP));
					}
				}else{
					//this time, we must divide correctly operand1/operand2 : st = operand1 / st
					//reverse divide...
					fdivr(qword_ptr(ESP));

					operand1_type = HccTypeChecker::ts_type_double_ptr;
					//get a double precision result...
					fstp(qword_ptr(ESP));
				}
				//the result in EDX:EAX
				is_operand_in_stack = true;
				stack_operand_type = HccTypeChecker::ts_type_double_ptr; // operand1_type; //Fixed @Dec 28, 2008
				//pop(EDX);
				//pop(EAX);
			}
		}
		break;
	case HCC_DIV:				// Integer Division	
	case HCC_MOD_OP:			// %
		{
			HCC_TOKEN_TYPE _operator = token_type;

			if(!is_operand_in_stack)
				EmitPushOperand(operand1_type);
			is_operand_in_stack = false;
			//skip this operator
			getToken();
			TypeSpecifier* operand2_type = EmitFactor(function_ptr);

			if(operand1_type==HccTypeChecker::ts_type_Int64_ptr || 
				//
				operand2_type==HccTypeChecker::ts_type_Int64_ptr)
			{				
				//intx		div intx64

				//intx64	div intx64	--> EDX:EAX  div qword_ptr [ESP]
				//intx64	div int
				//
				if(operand2_type==HccTypeChecker::ts_type_Int64_ptr)
					mov(ECX, EDX);	//high
				mov(EBX, EAX);		//low

				if(operand1_type==HccTypeChecker::ts_type_Int64_ptr)
					pop(EDX);
				else
					xor(EDX, EDX);
				pop(EAX);

				//
				if(operand2_type==HccTypeChecker::ts_type_Int64_ptr)
				{
					push(EBX);	//low
					push(ECX);	//high

					//TODO: correct the result to EAX == Quotient ; EDX == Modulus ;
					//
					idiv(qword_ptr(ESP));  //EDX:EAX div (64-bit integer) -->RAX == Quotient ; RDX == Modulus ;
					//the result is a 64-bit number...
					operand1_type=HccTypeChecker::ts_type_Int64_ptr;
				}else{
					idiv(EBX);	//EAX | EDX:EAX div (32-bit integer) -->EAX == Quotient ; EDX == Modulus ;

					//the result is a 32-bit number...
					operand1_type=HccTypeChecker::ts_type_Int32_ptr;
				}
				//
			}else{ //we assume integers <= 32-bits here
				//intx div intx
				
				mov(ECX, EAX);	//(EAX == operand2);
				pop(EAX);		//(EAX == operand1);	
				xor(EDX, EDX);		
				idiv(ECX);			//	EAX == Quotient ; EDX == Modulus
				if(_operator==HCC_MOD_OP)
					mov(EAX, EDX);

				//the result is a 32-bit number...
				//BEGIN - FIXED Mar 03, 2009
				if(HCC_UNSIGNED_TYPE==operand1_type->getDataTypeModifier() &&
					HCC_UNSIGNED_TYPE==operand2_type->getDataTypeModifier())
				{
					operand1_type = HccTypeChecker::ts_type_unsigned_ptr;
				}else
					operand1_type = HccTypeChecker::ts_type_int_ptr;
				//END - FIXED Mar 03, 2009
			}
		}
		break;
	default:
		return operand1_type;
		break;
	};
	return operand1_type;
}

//-----------------------------------------------
//	EmitAssignmentExpr	- Emit the assignment-expressions
//-----------------------------------------------
TypeSpecifier* HCCCodeGenerator::EmitAssignmentExpr(Symbol *target_ptr, 
													HCC_TOKEN_TYPE _operator, 
													Symbol* function_ptr,
													bool bIsAddressOnStack,
													bool bIsPropertyPut,
													Symbol* object_ptr,
													TypeSpecifier* array_item_type)
{
	CPUInstruction push(this, PUSH);
	CPUInstruction pop(this, POP);
	CPUInstruction mov(this, MOV);
	CPUInstruction sub(this, SUB);	
	CPUInstruction call(this, CALL);
	CPUInstruction lea(this, LEA);

	CPUInstruction not(this, NOT);
	CPUInstruction xor(this, XOR);
	CPUInstruction cdq(this, CDQ);

	FPUInstruction fild(this, FILD);
	FPUInstruction fld(this, FLD);
	FPUInstruction fstp(this, FSTP);

	getToken();	//assign op

	if(token_type==HCC_NEW)
		return EmitNewDynamicAllocation(target_ptr, function_ptr, bIsAddressOnStack, bIsPropertyPut);
	else if(token_type==HCC_DYNAMIC_CAST)
		return emitDynamicCastOperator(target_ptr, function_ptr, bIsAddressOnStack, bIsPropertyPut, object_ptr);

	volatile bool bAssign_Null = token_type==HCC_NULL;
	//result in EAX or EDX:EAX
	TypeSpecifier* result_type_ptr = EmitExprList(function_ptr);
	//based on the target_ptr data type, we proceed to generate code
	//and make data type conversions when necessary.
	TypeSpecifier* target_type = array_item_type,
					*source_type = result_type_ptr;
	
	if(target_type==NULL)
		target_type = target_ptr->getTypeSpecifier().getBaseTypeSpec();				   

	assert(source_type!=NULL);

	//BEGIN - FIXED Dec 25, 2008
	if(target_type->getDataType()==HCC_INTEGER && source_type->getDataType()==HCC_FLOATING_POINT)
	{
		//emitImplicitTypeConversion(target_type, source_type);		

		if(!is_operand_in_stack)
		{
			//EDX:EAX
			EmitPushOperand(source_type);
		}
		//fld(qword_ptr(ESP));
		is_operand_in_stack = true;		
		if(target_type==HccTypeChecker::ts_type_Int64_ptr)
			call(no_op_ptr(FLOATING_POINT_TO_INT64)); //EDX:EAX
		else
			call(no_op_ptr(FLOATING_POINT_TO_INT)); //EAX
		is_operand_in_stack = false;
		
		//we must change the resultant type to target_type to avoid another implicit conversions between assigments
		result_type_ptr = target_type;
	}
	//END - FIXED Dec 25, 2008

	switch(_operator)
	{
		//F L O A T I N G - P O I N T   A N D   I N T E G E R   T Y P E S 
		case HCC_INC_ASSIGN:		// +=
			return EmitPlusAssignExpr(target_ptr, source_type, _operator, function_ptr, bIsAddressOnStack, array_item_type);
			break;
		case HCC_DEC_ASSIGN:		// -=
			return EmitMinusAssignExpr(target_ptr, source_type, _operator, function_ptr, bIsAddressOnStack, array_item_type);
			break;
		case HCC_MUL_ASSIGN:		// *=
			return EmitMultAssignExpr(target_ptr, source_type, _operator, function_ptr, bIsAddressOnStack, array_item_type);
			break;
		case HCC_DIV_ASSIGN:		// /=
			return EmitDivAssignExpr(target_ptr, source_type, _operator, function_ptr, bIsAddressOnStack, array_item_type);
			break;

			//I N T E G E R   T Y P E S  O N L Y
		case HCC_MOD_ASSIGN:		// %=
			return EmitModAssignExpr(target_ptr, source_type, _operator, function_ptr, bIsAddressOnStack, array_item_type);
			break;
	//BEGIN - CODE REUSE/REDUCE REDUNDANCY/OPTIMIZE COMPILER - Dec 20, 2008
		case HCC_XOR_ASSIGN:		// ^=
		case HCC_BIT_OR_ASSIGN:		// |=
		case HCC_BIT_AND_ASSIGN:	// &=
			return EmitLogicalOpAssignExpr(target_ptr, source_type, _operator, function_ptr, bIsAddressOnStack, array_item_type);
			break;
	//END - CODE REUSE/REDUCE REDUNDANCY/OPTIMIZE COMPILER - Dec 20, 2008
	};

	//if a previous expression was generated, there may be the 'LAST-X' in the stack
	if(is_operand_in_stack && 
		target_type->getDataType()!=HCC_FLOATING_POINT && 
		!bIsPropertyPut)
	{
		EmitPopOperand(stack_operand_type);
		is_operand_in_stack = false;
	}

	//BEGIN - REFACTORED - Jan 27, 2009
	if(target_ptr->getDeclDefinition().identifier_type()==DECL_POINTER_VARIABLE ||
		target_ptr->getDeclDefinition().user_data.bDataMemberIsPointer)
	{
		//BEGIN - FIXED - Jan 10, 2009
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			EmitAddressFromAddressOf(target_ptr, 
									 function_ptr, 
									 true, 
									 target_ptr->getDeclDefinition().user_data.bDataMemberIsPointer ? HccTypeChecker::ts_type_int_ptr : target_type); //result in EBX when manually assigning
			//
		}
		//END - FIXED - Jan 10, 2009
		else if(bIsAddressOnStack)
		{
			pop(EBX);			
			//EBX is a pointer address
			//BEGIN - FIXED Jan 27, 2009
			if(target_type->getDataType()==HCC_FLOATING_POINT)
			{
				mov(dword_ptr(EBX), EDX);
				mov(dword_ptr(EBX, sizeof(int)), EAX);

			}else if(target_type->getDataType()==HCC_INTEGER)
			{
				if(target_type==HccTypeChecker::ts_type_Int64_ptr)
				{
					mov(dword_ptr(EBX), EAX);
					mov(dword_ptr(EBX, sizeof(int)), EDX);
					//
				}else if(target_type->getDataTypeSize()==sizeof(short))
					mov(word_ptr(EBX), AX);
				else if(target_type->getDataTypeSize()==sizeof(char))
					mov(byte_ptr(EBX), AL);
				else 
					mov(dword_ptr(EBX), EAX);
			}else
				mov(dword_ptr(EBX), EAX);
			//END - FIXED Jan 27, 2009
			//
		}else{
			//								
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;
				mov(dword_ptr(EBP, op, abs(offset)), EAX);
			}else
				mov(no_op_ptr(getSymbolLabel(target_ptr)), EAX);
		}
	}
	//END - REFACTORED - Jan 27, 2009
	//F L O A T I N G   P O I N T   A S S I G N M E N T 
	else if(target_type==HccTypeChecker::ts_type_double_ptr || 
		target_type==HccTypeChecker::ts_type_float_ptr)
	{
		//for integer types, do conversion to floating point type...
		if(source_type==HccTypeChecker::ts_type_Int64_ptr)
		{
			//BEGIN - FIXED Dec 22, 2008
			emitPushInteger(source_type);
			//END - FIXED Dec 22, 2008
		}else if(source_type!=HccTypeChecker::ts_type_double_ptr &&
				 source_type!=HccTypeChecker::ts_type_float_ptr)
		{
			emitPushInteger(source_type);
		}else{
			
			if(!is_floating_point_in_fpu)
			{
				FPUInstruction ffree(this, FFREE);
				if(!is_operand_in_stack)
				{
					//BEGIN - FIXED Jan 26, 2009
					EmitPushOperand(source_type);
					//END - FIXED Jan 26, 2009
				}
				ffree(st); //FIXED - Mar 06, 2009
				fld(qword_ptr(ESP));
				is_operand_in_stack = true;
			}			
		}
		
		//now for operator HCC_ASSIGN_OP
		//EDX:EAX
		if(bIsPropertyPut)
		{
			EmitPropertyPutCall(target_type, object_ptr, function_ptr, bIsAddressOnStack); //assign to an object property...
			//
		}else if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			//BEGIN - FIXED Dec 23, 2008
			EmitAddressFromAddressOf(id_ptr, function_ptr, true, target_type); //result in EBX
			//END - FIXED Dec 23, 2008
			//
		}else if(bIsAddressOnStack)
		{
			if(is_operand_in_stack)
			{
				EmitPopOperand(source_type);
				is_operand_in_stack = false;
			}
			pop(EBX);
			fstp(qword_ptr(EBX));

		}else{								
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;				
				fstp(qword_ptr(EBP, op, abs(offset)));
			}else{
				fstp(qword_ptr(getSymbolLabel(target_ptr)));
			}
		}
		is_floating_point_in_fpu = false;
		//
	}else if(target_type==HccTypeChecker::ts_type_Int64_ptr)
	{
		if(source_type->getDataTypeSize() <=HccTypeChecker::ts_type_Int32_ptr->getDataTypeSize()) //FIXED - Dec 18, 2008
		{
			//promote to Int64...
			cdq();
		}

		if(bIsPropertyPut)
		{
			EmitPropertyPutCall(target_type, object_ptr, function_ptr, bIsAddressOnStack); //assign to an object property...
			//
		}else if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX			
			/*
			mov(dword_ptr(EBX), EAX);				//low
			mov(dword_ptr(EBX, sizeof(int)), EDX);	//high
			*/
			emitAssignInt64ToAddress();
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);
			/*
			mov(dword_ptr(EBX), EAX);				//low
			mov(dword_ptr(EBX, sizeof(int)), EDX);	//high
			*/
			emitAssignInt64ToAddress();
		}else{								
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;				
				/*
				mov(dword_ptr(EBP, op, abs(offset)), EAX);				//low
				mov(dword_ptr(EBP, op, abs(offset + sizeof(int))), EDX);	//high
				*/
				emitAssignInt64ToAddress(true, op, offset);
			}else{
				mov(no_op_ptr(getSymbolLabel(target_ptr)), EAX); //low
				__tstring HIGH_PART = __tstring(_T("["))		+ 
									getSymbolLabel(target_ptr)	+ 
									__tstring(QWORD_OFFSET_HIGH) + 
									__tstring(_T("]"));
				mov(no_op_ptr(HIGH_PART), EDX); //high
			}
		}
	}else if(target_type==HccTypeChecker::ts_type_char_ptr ||
			target_type==HccTypeChecker::ts_type_boolean_ptr)
	{
		//EAX
		if(bIsPropertyPut)
		{
			EmitPropertyPutCall(target_type, object_ptr, function_ptr, bIsAddressOnStack); //assign to an object property...
			//
		}else if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			//BEGIN - FIXED Dec 23, 2008
			/*
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			mov(byte_ptr(EBX), AL);
			*/
			EmitAddressFromAddressOf(id_ptr, function_ptr, true, target_type, MOV, AL); //result in EBX
			//END - FIXED Dec 23, 2008
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);
			mov(byte_ptr(EBX), AL);				
		}else{								
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;
				mov(byte_ptr(EBP, op, abs(offset)), AL);
			}else
				mov(no_op_ptr(getSymbolLabel(target_ptr)), AL);
		}
	}else if(target_type==HccTypeChecker::ts_type_Int16_ptr ||
		target_type==HccTypeChecker::ts_type_short_ptr)
	{
		//AX
		if(bIsPropertyPut)
		{
			EmitPropertyPutCall(target_type, object_ptr, function_ptr, bIsAddressOnStack); //assign to an object property...
			//
		}else if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			//BEGIN - FIXED Dec 23, 2008
			/*
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			mov(word_ptr(EBX), AX);
			*/
			EmitAddressFromAddressOf(id_ptr, function_ptr, true, target_type, MOV, AX); //result in EBX
			//END - FIXED Dec 23, 2008
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);
			mov(word_ptr(EBX), AX);				
		}else{								
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;
				mov(word_ptr(EBP, op, abs(offset)), AX);				
			}else
				mov(no_op_ptr(getSymbolLabel(target_ptr)), AX); //high
		}
	}else if(target_type->is_scalar() || target_type->getDataType()==HCC_STRING_TYPE)
	{
		//F O R   A L L   O T H E R   I N T E G E R S 
		//EAX
		if(bIsPropertyPut)
		{
			EmitPropertyPutCall(target_type, object_ptr, function_ptr, bIsAddressOnStack); //assign to an object property...
			//
		}else if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			//BEGIN - FIXED Dec 23, 2008
			EmitAddressFromAddressOf(id_ptr, function_ptr, true, target_type, MOV, EAX); //result in EBX
			//END - FIXED Dec 23, 2008
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);
			mov(dword_ptr(EBX), EAX);
		}else{								
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;				
				mov(dword_ptr(EBP, op, abs(offset)), EAX);
			}else
				mov(no_op_ptr(getSymbolLabel(target_ptr)), EAX);
		}
	}else if(bAssign_Null)
	{
		if(bIsPropertyPut)
		{
			EmitPropertyPutCall(target_type, object_ptr, function_ptr, bIsAddressOnStack); //assign to an object property...
			//
		}else if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			EmitAddressFromAddressOf(id_ptr, function_ptr, true, target_type, MOV, EAX); //result in EBX
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);			
			//EBX is a pointer address
			mov(dword_ptr(EBX), 0);
			//
		}else{
			//								
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;
				mov(dword_ptr(EBP, op, abs(offset)), 0);
			}else
				mov(no_op_ptr(getSymbolLabel(target_ptr)), 0);
		}
	}else if(bIsPropertyPut)
	{
		EmitPropertyPutCall(target_type, object_ptr, function_ptr, bIsAddressOnStack); //assign to an object property...
			//
	}else{
		CPUInstruction cld(this, CLD);							
		CPUInstruction rep_movsb(this, REP_MOVSB);

		if(target_type->specifier()==DSPEC_CLASS)
		{
			//ECX is the this pointer for the source object instance...
			mov(ESI, ECX);
		}else{
			//EAX is a pointer address
			mov(ESI, EAX);
		}

		int size = target_type->getDataTypeSize();

		if(bIsPropertyPut)
		{
			//reserve the space from stack...
			sub(ESP, size);
			mov(EDI, ESP);
			//
		}else if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			//EBX is a pointer address
			mov(EDI, EBX);
			//
		}else if(bIsAddressOnStack)
		{
			pop(EDI);
			//
		}else{
			//								
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;
				lea(EDI, dword_ptr(EBP, op, abs(offset)));
			}else
				lea(EDI, no_op_ptr(getSymbolLabel(target_ptr)));
		}
		mov(ECX, size);
		cld();
		rep_movsb();							

		if(bIsPropertyPut)
		{
			Symbol* prop_put_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			__tstring prop_label = getLabelFromName(prop_put_ptr->getCompleteName());			

			if(object_ptr!=NULL)
			{
				EmitLoadValueAddressOf(object_ptr, function_ptr);
			}else if(bIsAddressOnStack)
			{
				pop(ECX);
			}
			//
			call(VAR_OFFSET, prop_label);
		}
	}
	return result_type_ptr;
}


//-----------------------------------------------
//	EmitFactor	- Emit sub-expressions and
//					   assignment-expressions
//-----------------------------------------------
TypeSpecifier* HCCCodeGenerator::EmitFactor(Symbol *function_ptr)
{
	CPUInstruction push(this, PUSH);
	CPUInstruction pop(this, POP);
	CPUInstruction mov(this, MOV);
	CPUInstruction sub(this, SUB);	
	CPUInstruction call(this, CALL);
	CPUInstruction lea(this, LEA);
	CPUInstruction inc(this, INC);
	CPUInstruction decc(this, DEC);
	CPUInstruction not(this, NOT);
	CPUInstruction xor(this, XOR);
	CPUInstruction cdq(this, CDQ);
	CPUInstruction int_(this, INT_);
	CPUInstruction neg(this, NEG);
	CPUInstruction test(this, TEST);
	CPUInstruction jz(this, JZ);

	volatile bool bArraySubscript	= false;
	volatile bool bIsAddressOnStack = false;
	volatile bool bIsPropertyPut	= false;
	volatile bool bConstructorCalled = false;


	Symbol* target_ptr = NULL;
	TypeSpecifier* baseTypeSpec_ptr = NULL;
	TypeSpecifier* result_type_ptr = NULL;
	HCC_TOKEN_TYPE prev_token = token_type;
	switch(token_type)
	{
	case HCC_NUMBER:
		{
			assert(symbol_ptr!=NULL);
			if(symbol_ptr!=NULL){
				result_type_ptr = symbol_ptr->getTypeSpecifier().getBaseTypeSpec();
				assert(result_type_ptr!=NULL);
			}
			else
				result_type_ptr = HccTypeChecker::ts_type_int_ptr;
			/*
			EmitLoadNumber(symbol_ptr);			
			getToken();
			*/
			lastConstValueInfo.type_ptr = NULL; //no type means that the assignment is not direct
			CONST_VALUE_INFO cvInfo(result_type_ptr);
			EmitConstNumberValue(symbol_ptr, &cvInfo); //EmitLoadNumber(symbol_ptr);			
			getToken();
			EmitLoadNumber(cvInfo); 
		}
		break;
	case HCC_IDENTIFIER:
		{
			target_ptr = symbol_ptr;
			assert(target_ptr!=NULL);			
			getToken();			
			if(target_ptr!=NULL)
			{
				//call the object constructor, if it's the first time instantiation...
				baseTypeSpec_ptr = target_ptr->getTypeSpecifier().getBaseTypeSpec();
				//BEGIN - OPTIMIZED - Jan 10, 2009
				//this impl. is only intended for local variables (object variables, and array of objects variables)
				if(target_ptr->getDeclDefinition().identifier_type()==DECL_VARIABLE)
				{
					if(!!baseTypeSpec_ptr && baseTypeSpec_ptr->specifier()==DSPEC_CLASS)
					{
						if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
						{
							//S C O P E _ L O C A L 							
							vector<Symbol*>::iterator it_var = find(obj_instance_in_scope.begin(), 
																	obj_instance_in_scope.end(), 
																	target_ptr);
							//if this variable was not initialized, then call its constructor...
							if(it_var==obj_instance_in_scope.end())
							{
								obj_instance_in_scope.push_back(target_ptr);
																
								//O B J E C T   C O N S T R U C T O R								
								EmitObjectConstructorCall(baseTypeSpec_ptr, function_ptr, target_ptr);
								bConstructorCalled = true;
							}
						}else{
							//S C O P E _ G L O B A L
							vector<Symbol*>::iterator it_var = find(obj_instance_in_globals.begin(), 
																	obj_instance_in_globals.end(), 
																	target_ptr);
							if(it_var==obj_instance_in_globals.end())
							{								
								obj_instance_in_globals.push_back(target_ptr);
																
								//O B J E C T   C O N S T R U C T O R								
								EmitObjectConstructorCall(baseTypeSpec_ptr, function_ptr, target_ptr);
								bConstructorCalled = true;
							}
						}
						//BEGIN - ADDED - Jan 10, 2009
					}else if(target_ptr->getTypeSpecifier().specifier()==DSPEC_ARRAY &&
						target_ptr->getTypeSpecifier().array.pItemType->specifier()==DSPEC_CLASS)
					{
						baseTypeSpec_ptr = &target_ptr->getTypeSpecifier();
						//call multiple constructors		
						if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
						{
							//S C O P E _ L O C A L 							
							vector<Symbol*>::iterator it_var = find(obj_instance_in_scope.begin(), 
																	obj_instance_in_scope.end(), 
																	target_ptr);
							//if this variable was not initialized, then call its constructor...
							if(it_var==obj_instance_in_scope.end())
							{
								obj_instance_in_scope.push_back(target_ptr);
																
								//M U L T I P L E   O B J E C T   C O N S T R U C T O R	  C A L L
								EmitLoadValue(target_ptr);
								mov(ESI, EAX);
								EmitMultipleConstructorCall(baseTypeSpec_ptr);
								bConstructorCalled = true;
							}
							//
						}else{
							//S C O P E _ G L O B A L
							vector<Symbol*>::iterator it_var = find(obj_instance_in_globals.begin(), 
																	obj_instance_in_globals.end(), 
																	target_ptr);
							if(it_var==obj_instance_in_globals.end())
							{								
								obj_instance_in_globals.push_back(target_ptr);
																
								//M U L T I P L E   O B J E C T   C O N S T R U C T O R	  C A L L
								EmitLoadValue(target_ptr);
								mov(ESI, EAX);

								EmitMultipleConstructorCall(baseTypeSpec_ptr);
								bConstructorCalled = true;
							}
							//
						}
						//END - ADDED - Jan 10, 2009
					}
				}
				//END - OPTIMIZED - Jan 10, 2009
__ASSIGNMENT_EXPRESSION:
				//A S S I G N M E N T - E X P R E S S I O N S 
				switch(token_type)
				{
				case HCC_INCREMENT:		//postfix ++
				case HCC_DECREMENT:		//postfix --
					{
						result_type_ptr = EmitPostFixAssignmentExpr(target_ptr, function_ptr, bArraySubscript);
					}
					break;
				case HCC_ASSIGN_OP:			// =
				case HCC_INC_ASSIGN:		// +=
				case HCC_DEC_ASSIGN:		// -=
				case HCC_MUL_ASSIGN:		// *=
				case HCC_DIV_ASSIGN:		// /=
					//for integer types only
				case HCC_MOD_ASSIGN:		// %=
				case HCC_XOR_ASSIGN:		// ^=
				case HCC_BIT_OR_ASSIGN:		// |=
				case HCC_BIT_AND_ASSIGN:	// &=
					{
						if(!bArraySubscript)
						{
							result_type_ptr = NULL;
							switch(target_ptr->getDeclDefinition().identifier_type())
							{	
								//
							case DECL_VARIABLE:
							case DECL_PARAM_VALUE:
								//
							case DECL_PARAM_BYREF:
							case DECL_PARAM_CONST_BYREF:

							case DECL_PARAM_POINTER:
							case DECL_PARAM_ARRAY:
							case DECL_PARAM_CONST_ARRAY:
							case DECL_PARAM_CONST_POINTER:												
							case DECL_POINTER_VARIABLE:  //FIXED - Jan 10, 2009 - p = &i; p += 1;  (&i + 1)
							case DECL_NEW_DATA_MEMBER:
								{
									/* I found a better way to do assignments: 
										instead of pushing the variable's address into the stack,
										we save it to the compiler stack, and then pop it 
										when ready to assign the expression's result.
									*/
									stack_of_ids.push(target_ptr);
								}
								break;
							case DECL_WRITEONLY_PROPERTY:		//a write-only property
							case DECL_READWRITE_PROPERTY:		//a read-write property
								{
									//for writable properties (P U T), we will push the function address onto stack...
									Symbol* prop_put_ptr = target_ptr->getDeclDefinition().function.property[_T("put")];
									assert(prop_put_ptr!=NULL);
									//
									stack_of_ids.push(prop_put_ptr);
									bIsPropertyPut = true;									
								}
								break;
							default:
								break;
							};							
						}else{
							//the address from emitArraySubscript...
							push(EAX);
							bIsAddressOnStack = true;
							//
							bIsPropertyPut	  = false;
						}
						//result in EAX or EDX:EAX
						result_type_ptr = EmitAssignmentExpr(target_ptr, 
															 token_type, 
															 function_ptr, 
															 bIsAddressOnStack, 
															 bIsPropertyPut,
															 NULL,
															 result_type_ptr);
					}
					break;
				case HCC_COLON:				//labeled-statement
					{	
						if(!emitting_ternary_expr)
						{
							getToken();
							//emit this labeled-statement...
							unit() << target_ptr->getName() <<  _T(":") << endl;
						}else							
							goto EMIT_LOAD_FACTOR;
					}
					break;
				case HCC_PERIOD: //obj->member-access
					{						
						TypeSpecifier* baseTypeSpec = &target_ptr->getTypeSpecifier(); //.getBaseTypeSpec();						
						//ECX == EBX == array address...
						if(!bArraySubscript)
							result_type_ptr = emitObjectInstanceMember(target_ptr, NULL, function_ptr);
						else{
							mov(ECX, ESI);
							result_type_ptr = emitObjectInstanceMember(NULL, result_type_ptr, function_ptr);
						}
					}
					break;
				case HCC_LBRACKET:	//id[x][y][...
					{					
						//accessing an array element...
						//BEGIN - REFACTORED TO FUNCTION - OPTIMIZED - Jan 23, 2009
						result_type_ptr = emitArraySubscript(target_ptr, NULL, NULL, function_ptr);
						//the address of the (array+offset*sizeof(type))...												
						bArraySubscript = true;						
						//if an assignment...
						goto __ASSIGNMENT_EXPRESSION;

						//END - REFACTORED TO FUNCTION - OPTIMIZED - Jan 23, 2009
					}
					break;
				case HCC_LPAREN:	//function call
					{						
						if(target_ptr->getDeclDefinition().identifier_type()==DECL_BUILTIN_CONSOLE_WRITE || 
							target_ptr->getDeclDefinition().identifier_type()==DECL_BUILTIN_CONSOLE_WRITELN)
						{
							result_type_ptr = emitCallWriteWriteLnBuiltInFunction(target_ptr, function_ptr);
						}else{
							if(target_ptr->getName()==SYSTEM_BREAKPOINT)
							{
								getToken(); //(
								getToken(); //)
								//emits an int 3 system breakpoint...
								int_.set_new_comment(_T("User breakpoint"));
								int_(3);
							}else{
								DECLARATION_TYPE callee_decl_type = 
													target_ptr->getDeclDefinition().identifier_type();

								
								if(callee_decl_type==DECL_OPERATOR_MIN)
								{
									//the inline-min between two integer types
									emit_InlineMin(function_ptr);

								}else if(callee_decl_type==DECL_OPERATOR_MAX)
								{
									//the inline-max between two integer types
									emit_InlineMax(function_ptr);

								}else
								{
									EmitFunctionCall(target_ptr, function_ptr, NULL);
								}
							}
						}
						//the return type from the function spec...
						result_type_ptr = target_ptr->getDeclDefinition().function.return_type;
					}
					break;
				default:
					{
EMIT_LOAD_FACTOR:
						if(!bArraySubscript)
						{
							//L O A D   T H E   I D E N T I F I E R   V A L U E							(EAX | EDX:EAX)
							//L E A V E   T H E   I D E N T I F I E R   A D D R E S S   V I S I B L E	(EBX)
							
							result_type_ptr = target_ptr->getTypeSpecifier().getBaseTypeSpec();
							if(!bConstructorCalled)
							{
								EmitLoadValueAddressOf(target_ptr, function_ptr);
							}
						}else{		
							//should never get here; array expressions are processed in the emitArraySubscript member
							//do nothing!
						}
					}
					break;
				};
				goto EMIT_FACTOR_EXIT;				
			}else 
				assert(!"Should never happen having an identifier null!"); //if(target_ptr!=NULL)
		}
		break;
		case HCC_LPAREN:	//(expr);
		{
			//skip this '('
			getToken();
			//the node in the syntax tree...
			result_type_ptr = emitCommaExprList(function_ptr);
			//skip this ')'
			getToken();
		}
		break;
		case HCC_CONTROL_CHAR:
		case HCC_CHARACTER:
			{
				//type char...
				result_type_ptr = symbol_ptr->getTypeSpecifier().getBaseTypeSpec();
				//
				//AX high = 0000
				//AX low  = AH, AL
				//AH	  = 00
				//AL	  = the character.
				if(symbol_ptr->getDeclDefinition().constant.value.Character==0)
					xor(EAX, EAX);
				else
					mov(EAX, (int)symbol_ptr->getDeclDefinition().constant.value.Character);
				getToken();
			}
			break;		
		case HCC_STRING_LITERAL:
			{			
				result_type_ptr = symbol_ptr->getTypeSpecifier().getBaseTypeSpec();
				//BEGIN - hlm March 13, 2011 - fix for strings with just one char
				if(result_type_ptr != HccTypeChecker::ts_type_char_ptr)
				{
					//it's a string. Ok!
					__tstring string_address = CreateLabel(STRING_LABEL_PREFIX, symbol_ptr->getLabelIndex());				
					lea(EAX, VAR_OFFSET, string_address);
				}else{
					//it's a char. Ok!
					//AX high = 0000
					//AX low  = AH, AL
					//AH	  = 00
					//AL	  = the character.
					if(symbol_ptr->getDeclDefinition().constant.value.Character==0)
						xor(EAX, EAX);
					else
						mov(EAX, (int)symbol_ptr->getDeclDefinition().constant.value.Character);
				}
				//END - hlm March 13, 2011 - fix for strings with just one char
				getToken();
			}
			break;
		case HCC_TRUE:
			{
				mov(EAX, 1);
				getToken();
				result_type_ptr = HccTypeChecker::ts_type_boolean_ptr;
			}
			break;
		case HCC_FALSE:
		case HCC_NULL:
			{
				if(token_type==HCC_FALSE)
					result_type_ptr = HccTypeChecker::ts_type_boolean_ptr;
				else
					result_type_ptr = HccTypeChecker::ts_type_int_ptr;

				xor(EAX, EAX);
				getToken();
			}
			break;
		//P R E F I X   O P E R A T O R S  (T H E   H I G H E S T   P R E C E D E N C E )
		case HCC_NOT_OP:		//!  : !1;   == 0
		case HCC_COMPL_OP:		//~  : ~0x0; == 0xFFFFFFFF;
			{
				HCC_TOKEN_TYPE prefix_op = token_type;
				getToken();
				result_type_ptr = EmitFactor(function_ptr);

				//BEGIN - FIXED Dec 25, 2008
				if(HCC_NOT_OP==prefix_op && 
					result_type_ptr == HccTypeChecker::ts_type_boolean_ptr)
				{
					neg(EAX);
					not(EAX);
				}
				//END - FIXED Dec 25, 2008
				else if(result_type_ptr==HccTypeChecker::ts_type_Int64_ptr)
				{
					EmitPushOperand(result_type_ptr);
					not(qword_ptr(ESP));
					//BEGIN - FIXED Dec 25, 2008
					/*
					pop(EDX);
					pop(EAX);
					*/
					EmitPopOperand(result_type_ptr);
					//END - FIXED Dec 25, 2008
				}else 
					not(EAX);
			}
			break;		
		case HCC_INCREMENT:		//prefix ++ : ++id; == (id = id + 1)
		case HCC_DECREMENT:		//prefix -- : --id; == (id = id - 1)
			{
				result_type_ptr = EmitPrefixAssignmentExpr(function_ptr);
			}
			break;
			//BEGIN - ADDED - Jan 1, 2009
		case HCC_NEW:
			{
				EmitNewDynamicAllocation(NULL, function_ptr, bIsAddressOnStack, bIsPropertyPut);
				result_type_ptr = HccTypeChecker::ts_type_int_ptr;
			}
			break;
			//END - ADDED - Jan 1, 2009
		case HCC_DESTROY:
			{
				//destroy a dynamically allocated object
				getToken();
				//destroy object1;
				//destroy array1;
				//destroy dynamic_cast(obj_ptr);
				//destroy dynamic_cast(type-spec, obj_ptr);
				//assert(token_type==HCC_IDENTIFIER);
				bool bHasDestructor = false;
				bool bMultipleDtorCall = false;
				if(token_type==HCC_LBRACKET)
				{
					getToken(); //[
					getToken(); //]

					bMultipleDtorCall = true;
				}
				unsigned int __nodestroy_null_index = SYMBOL_TABLE::__asm_label_index++;
				__tstring __no_destroy_null = CreateLabel(STMT_LABEL_PREFIX, __nodestroy_null_index);

				if(token_type==HCC_IDENTIFIER)
				{
					assert(symbol_ptr!=NULL);
					//we only destroy DECL_POINTER_VARIABLE;
					TypeSpecifier* ptr_type_ptr = symbol_ptr->getTypeSpecifier().getBaseTypeSpec();
					if(ptr_type_ptr!=NULL && 
						(ptr_type_ptr->is_scalar() || ptr_type_ptr->specifier()==DSPEC_ARRAY))
					{
						EmitLoadValueAddressOf(symbol_ptr, function_ptr);
						test(EAX, EAX);
						jz(VAR_OFFSET, __no_destroy_null);
						//if the item type is class, and multiple destructor call...
						TypeSpecifier* pItemType = getArrayScalarType(ptr_type_ptr);
						if(pItemType!=NULL && pItemType->specifier()==DSPEC_CLASS && bMultipleDtorCall)
						{
							mov(ESI, EAX);
							EmitMultipleDestructorCall(ptr_type_ptr);
							CPUInstruction add(this, ADD);
							add(ESI, -(int)sizeof(int)); //destroy this memory from the first block!
							push(ESI);
						}else
							push(EAX);
						call(no_op_ptr(SYSTEM_MEMORY_OPER_DESTROY));
						result_type_ptr = ptr_type_ptr;
						//
						EmitStatementLabel(__nodestroy_null_index);
					}else
					{
						//for class types...
						result_type_ptr = EmitDestructorCall(symbol_ptr, function_ptr, NULL, &bHasDestructor); //in ECX the variable address
						//assert(symbol_ptr->getDeclDefinition().identifier_type()==DECL_POINTER_VARIABLE);
						//the this pointer...
						if(false==bHasDestructor)
						{
							EmitLoadValueAddressOf(symbol_ptr, function_ptr);
							push(EAX);
						}else
							push(ESI);
						call(no_op_ptr(SYSTEM_MEMORY_OPER_DESTROY));
					}
					getToken(); //<id>
				}else if(token_type==HCC_DYNAMIC_CAST)
				{
					TypeSpecifier* type_ptr = NULL;
					Symbol *theSymbol = NULL, 
						   *theDynamicClass = NULL;
					result_type_ptr = emitDynamicCastOperator(NULL, 
															  function_ptr, 
															  false, 
															  false, 
															  NULL, 
															  &theDynamicClass,
															  &theSymbol);

					if(theDynamicClass!=NULL)
						type_ptr = &theDynamicClass->getTypeSpecifier();
					symbol_ptr = theSymbol;
					result_type_ptr = EmitDestructorCall(symbol_ptr, function_ptr, type_ptr, &bHasDestructor); //in ECX the variable address
					//assert(symbol_ptr->getDeclDefinition().identifier_type()==DECL_POINTER_VARIABLE);
					//the this pointer...
					if(false==bHasDestructor)
					{
						EmitLoadValueAddressOf(symbol_ptr, function_ptr);
						push(EAX);
					}else
						push(ESI);
					call(no_op_ptr(SYSTEM_MEMORY_OPER_DESTROY));
				}
			}
			break;
		case HCC_DYNAMIC_CAST:
			{
				result_type_ptr = emitDynamicCastOperator(NULL, function_ptr);
			}
			break;
		case HCC_POINTER_ADDRESSOF:
			{
				//type-spec ^ pointer = &variable;
				//HccErrorManager::Error(HccErrorManager::errInvalidPointerExpression);
				HCC_TOKEN_TYPE prefix_op = token_type;				

				//BEGIN - FIXED Feb 22, 2009
				result_type_ptr = emitAddressOf(function_ptr);
				//END - FIXED Feb 22, 2009
			}
			break;
		case HCC_POINTER_DEREFERENCE:
			{
				// *ptr;
				//HccErrorManager::Error(HccErrorManager::errInvalidPointerExpression);				
				HCC_TOKEN_TYPE prefix_op = token_type;				
				getToken();	//*
				//
				result_type_ptr = NULL;
				if(symbol_ptr!=NULL)
				{
					result_type_ptr = symbol_ptr->getTypeSpecifier().getBaseTypeSpec();				
					target_ptr = symbol_ptr;				
					EmitLoadValueAddressOf(symbol_ptr, function_ptr);
					getToken(); //<identifier>						
				}
				//
				// *p == obj == dword ptr [EAX]; EAX == &obj ; EBX == &p;								
				switch(token_type)
				{
				case HCC_INCREMENT:		//postfix ++
				case HCC_DECREMENT:		//postfix --

				case HCC_ASSIGN_OP:			// =
				case HCC_INC_ASSIGN:		// +=
				case HCC_DEC_ASSIGN:		// -=
				case HCC_MUL_ASSIGN:		// *=
				case HCC_DIV_ASSIGN:		// /=
					//for integer types only
				case HCC_MOD_ASSIGN:		// %=
				case HCC_XOR_ASSIGN:		// ^=
				case HCC_BIT_OR_ASSIGN:		// |=
				case HCC_BIT_AND_ASSIGN:	// &=
					{
						//when found an assignment operator, try this pointer as an array subscript...
						bArraySubscript = true;
						//if an assignment expression, confirm it (rely on target_ptr)...
						goto __ASSIGNMENT_EXPRESSION;
					}
					break;
				default:
					{	
						//L O A D   A D D R E S S   V A L U E   I N   P O I N T E R
						if(result_type_ptr !=NULL && result_type_ptr->is_scalar())
						{
							if(result_type_ptr->getDataType()==HCC_FLOATING_POINT)
							{								
								mov(EDX, dword_ptr(EAX));				//high value in scalar
								mov(EAX, dword_ptr(EAX, sizeof(int)));	//low value in scalar
							}else if(result_type_ptr->getDataType()==HCC_INTEGER &&
								result_type_ptr->getDataTypeSize()==sizeof(__int64))
							{
								mov(EDX, dword_ptr(EAX, sizeof(int)));	//high value in scalar
								mov(EAX, dword_ptr(EAX));				//low value in scalar
							}else
								mov(EAX, dword_ptr(EAX));	//value in scalar
						}
					}
				}
			}
			break;
		case HCC_SIZEOF:
			{
				getToken(); //skip operator
				emit_SizeOf(function_ptr);
				result_type_ptr = HccTypeChecker::ts_type_int_ptr;
			}
			break;
		case HCC_SEMICOLON:
			goto EMIT_FACTOR_EXIT; //for empty-expression-statement
			break;
		default:
		{
			//not implemented yet!
			assert(!"not implemented yet!");
		}
		break;
	}
EMIT_FACTOR_EXIT:

	return result_type_ptr;
}


HCC_TOKEN_TYPE HCCCodeGenerator::EmitStatementList(HCC_TOKEN_TYPE terminator, Symbol *function_ptr)
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
		stmt_type = EmitStatement(function_ptr);
		//eliminate all occurrencies of semicolons before parsing the next statement...
		//this is called: eliminating empty statements!
		if(token_type==HCC_SEMICOLON)
			getToken();
	}while(token_type!=terminator && token_type!=HCC_EOF);
	//if Eof token...
	if(token_type==HCC_EOF)
	{
		delete token_ptr;
		token_ptr = NULL;	
	}
	return stmt_type;
}

HCC_TOKEN_TYPE HCCCodeGenerator::emitCompoundStatement(Symbol *function_ptr)
{	
	/*
		//Statement of this form:

		'{' statement-seq_opt '}'

		//where statement-seq_opt is:

		statement-seq:

			statement
			statement-seq  statement
	*/
	getToken(); // {
	HCC_TOKEN_TYPE stmt = EmitStatementList(HCC_RBLOCK_KEY, function_ptr); //{ statement-list }
	if(bSourceAnnotation && function_ptr!=NULL)
		EmitStatementComment(function_ptr);
	getToken(); // }
	return stmt;
}


HCC_TOKEN_TYPE HCCCodeGenerator::EmitStatement(Symbol *function_ptr)
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

	if(bSourceAnnotation && function_ptr!=NULL)
		EmitStatementComment(function_ptr);
	HCC_TOKEN_TYPE stmt_type = token_type;
	switch(stmt_type)
	{
		case HCC_IDENTIFIER:
		case HCC_NUMBER:
		case HCC_STRING_LITERAL:
		case HCC_NULL:
		case HCC_TRUE:
		case HCC_FALSE:
		case HCC_INCREMENT:
		case HCC_DECREMENT:
		case HCC_DESTROY:
		case HCC_POINTER_ADDRESSOF:
		case HCC_POINTER_DEREFERENCE:
		case HCC_LPAREN:	//for (expr) without assignments
		case HCC_MINUS_OP:		//for -factor op ...affects the whole expression if using operators * , / , %
		case HCC_PLUS_OP:		//for +factor op ...
		case HCC_DYNAMIC_CAST:	//dynamic_cast([type-spec,]object-variable)
		{			
			if(bSourceAnnotation)
			{				
				SaveICodePos();				
				TCHAR lpszLineNumber[10];
				_stprintf(lpszLineNumber, _T("{%ld}:"), icode_ptr->currentLineNumber());
				EmitNewLineComment(lpszLineNumber);
				EmitCommaExprListComment();
				unit() << _T(";") << endl;
				RestoreICodePos();
				getToken();
			}

			//if a previous expression was generated, there may be the 'LAST-X' in the stack
			if(is_operand_in_stack)
			{
				EmitPopOperand(stack_operand_type);
				is_operand_in_stack = false;
			}
			//all type of expression-statement
			//++x; --y; x += 5; 5 + 4;
			TypeSpecifier* expr_type_ptr = emitCommaExprList(function_ptr);
		}
		break;		
			//S T A T E M E N T S 
		case HCC_IF:			emitIfStatement(function_ptr);			break;
		case HCC_WHILE:			emitWhile(function_ptr);				break;
		case HCC_DO:			emitDoWhile(function_ptr);				break;
		case HCC_FOR:			emitFor(function_ptr);					break;
		case HCC_SWITCH:		emitSwitch(function_ptr);				break;		

		case HCC_CASE:		
		case HCC_DEFAULT:
				//do nothing in this cases because when emitting the switch/case statement
				//we call this function directly expecting to stop when these tokens
				//were found in the way.
			break;
		
			//{ statement-list; }
		case HCC_LBLOCK_KEY:	emitCompoundStatement(function_ptr);	break;			
		
		case HCC_TRY:			emitTryBlock(function_ptr);				break;
		case HCC_WITH:			emitWithStatement(function_ptr);		break;		
		
		case HCC_SEMICOLON:
			break;

		case HCC_IMPORT:	//we don't care about these tokens in the code generator
		case HCC_NAMESPACE:
		case HCC_USING:
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
		
		case HCC_PUBLIC:
		case HCC_PROTECTED:
		case HCC_PRIVATE:
			
		case HCC_RBLOCK_KEY: // '}' this one is caused when added the last semicolon to the icode
			{
				getToken(); //skip this
			}
			break;
			//J U M P   S T A T E M E N T S 
		case HCC_BREAK:
		case HCC_CONTINUE:
		case HCC_GOTO:
		case HCC_RETURN:
			{
				//jump-statement				
				emitJumpStatement(function_ptr);
			}
			break;
		case HCC_DEBUGGER:
			{
				emitDebuggerCall();				
			}
			break;
		default:
			{
				if(token_type >HCC_LINE_MARKER && token_type <= HCC_WITH){
					if(DATA_MEMBERS_INIT_PROC_ADDRESS!=function_ptr)
					{
						//to silent errors of some unknown bad sequence, I comment the next line sometimes
						HccErrorManager::Error(HccErrorManager::errUnexpectedToken, __tstring(_T(": \'")) + symbolStrings[token_type] + __tstring("\'."));
					}
					getToken();
				}else{
					//we should never get here...
					token_type = HCC_EOF;
				}
			}
		break;
	};
	return stmt_type;
}

TypeSpecifier* HCCCodeGenerator::emitCommaExprList(Symbol *function_ptr)
{
	TypeSpecifier* type_ptr = NULL;
	// ','	::* expr1, expr2, expr3,..., expr_n <-- this one is the only left in the stack 
	type_ptr = EmitExprList(function_ptr);
	while(token_type==HCC_COMMA_OP)
	{
		getToken();

		if(is_operand_in_stack)
		{
			EmitPopOperand(stack_operand_type);
			is_operand_in_stack = false;
		}
		type_ptr = EmitExprList(function_ptr);
	}
	//*
	return type_ptr;
}

//-----------------------------------------------
// EmitRelationalExprList	- Emit an expression (also with: binary relational 
//  							operators | binary bitwise operators, between expressions )
//-----------------------------------------------
TypeSpecifier* HCCCodeGenerator::EmitRelationalExprList(Symbol *function_ptr)
{
	TypeSpecifier* result_type_ptr = NULL;
	//simple expression
	TypeSpecifier* operand1_type = EmitExpression(function_ptr);

	result_type_ptr = operand1_type;	

	CPUInstruction push(this, PUSH);
	CPUInstruction pop(this, POP);
	CPUInstruction mov(this, MOV);
	CPUInstruction cmp(this, CMP);
	CPUInstruction call(this, CALL);
	CPUInstruction test(this, TEST);
	CPUInstruction xor(this, XOR);
	CPUInstruction neg(this, NEG);

	FPUInstruction fstp(this, FSTP);

	//save for later processing
	HCC_TOKEN_TYPE _operator = token_type;
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
			//operand1 onto stack...			
			if(!is_floating_point_in_fpu)
			{
				if(!is_operand_in_stack)
					EmitPushOperand(operand1_type);
			}else{
				push(ECX);
				push(ECX);
				fstp(qword_ptr(ESP));
				is_floating_point_in_fpu = false;
			}
			is_operand_in_stack = false; //@Dec 28, 2008

			getToken();	//skip operator
			//simple expression...
			TypeSpecifier* operand2_type = EmitExpression(function_ptr);

			//B I T W I S E   O P E R A T O R S   I N T E G E R S   O N L Y
			if(_operator==HCC_BIT_AND_OP)			// &
			{
				CPUInstruction and(this, AND);

				pop(EDX);
				and(EAX, EDX);
				goto __EXIT_BITWISE;
			}else if(_operator==HCC_XOR_OP)			// ^
			{				
				pop(EDX);
				xor(EAX, EDX);
				goto __EXIT_BITWISE;
			}else if(_operator==HCC_BIT_OR_OP)		// |
			{
				CPUInstruction or(this, OR);

				pop(EDX);
				or(EAX, EDX);
				goto __EXIT_BITWISE;
			}
			else if(operand1_type==HccTypeChecker::ts_type_Int64_ptr ||
				operand2_type==HccTypeChecker::ts_type_Int64_ptr)
			{
				//convert Int64 to leverage the FPU comparing features for number of >=64bits
				goto __FLOATING_POINT_CONVERSION;

			}else if(
					  (operand1_type==HccTypeChecker::ts_type_char_ptr 
					  &&
					   operand2_type==HccTypeChecker::ts_type_char_ptr)							||

					  (operand1_type==HccTypeChecker::ts_type_boolean_ptr 
					  &&
					   operand2_type==HccTypeChecker::ts_type_boolean_ptr)
					   )
			{
				/*
					char <op> char
					boolean <op> boolean
				*/
				pop(EDX);
				cmp(DL, AL); //set EFLAGS
			}else if( (operand1_type->is_scalar() && operand1_type->getDataType()==HCC_INTEGER
					  &&
					  operand2_type->is_scalar() && operand2_type->getDataType()==HCC_INTEGER)	|| 

					  (operand1_type->specifier()==DSPEC_ENUM 
					  &&
					   operand2_type->specifier()==DSPEC_ENUM)									||

					 (operand1_type->specifier()==DSPEC_ENUM 
					  &&
					  operand2_type->getDataType()==HCC_INTEGER && operand2_type->is_scalar())	||

					 (operand1_type->getDataType()==HCC_INTEGER && operand1_type->is_scalar()
					  &&
					   operand2_type->specifier()==DSPEC_ENUM)									||

					 (operand1_type->specifier()==DSPEC_ARRAY && operand1_type->array.bIsDynamicArray
					  && 
					  operand2_type->getDataType()==HCC_INTEGER)								||
					  
					  (operand2_type->specifier()==DSPEC_ARRAY && operand2_type->array.bIsDynamicArray
					  && 
					   operand1_type->getDataType()==HCC_INTEGER)
				    )
			{
				//I N T E G E R   T Y P E S  <= 32bits
				/*
					intx <op> intx
					enum <op> enum
					enum <op> intx
					intx <op> enum
				*/
				pop(EDX);
				cmp(EDX, EAX); //set EFLAGS 
			}else if(operand1_type->getDataType()==HCC_FLOATING_POINT ||
					 operand2_type->getDataType()==HCC_FLOATING_POINT)
			{
__FLOATING_POINT_CONVERSION:
				//we must convert the integer operand to a floating point value,
				//so, we can use the H++ FloatingPoint_Compare and FloatingPoint_FromInt
				//apis;
				/*
				 double op int
				 double op double
				 int	op double
				 int64	op int64
				 int64	op double
				 double op int64
				 int64	op xtype
				 xtype	op int64	
				*/

				//BEGIN - FIXED Dec 28, 2008 - OPTIMIZED Dec 30, 2008

				// BEGIN - @@---When first operand's type is integer, and second is activated, we must save 
				//second's before attempting to promote the first's...
				//saving the second operand's data
				if(operand2_type->getDataType()==HCC_FLOATING_POINT)
				{
					if(is_floating_point_in_fpu)
					{
						push(ECX);
						push(ECX);
						fstp(qword_ptr(ESP));
						is_floating_point_in_fpu = false;
						is_operand_in_stack = true;
					}else if(!is_operand_in_stack)
					{
						EmitPushOperand(operand2_type);
						is_operand_in_stack = true;
					}

					if(operand1_type->getDataType()==HCC_INTEGER)
					{
						if(operand1_type==HccTypeChecker::ts_type_Int64_ptr)
						{
							if(is_operand_in_stack)
							{
								pop(EDI);
								pop(ESI);
								is_operand_in_stack = false;
							}else{
								mov(ESI, EAX);
								mov(EDI, EDX);
							}
							//
							EmitPromoteToFloatingPoint(operand1_type); //first operand
							//
							push(ESI);
							push(EDI);	//second operand
							//
						}else{
							//if not onto stack, push second operand...
							if(!is_operand_in_stack)
								EmitPushOperand(operand2_type);
						}
					}
					//else, operand 1 is already a floating point value...
					//do nothing!
				}else{

					if(!is_operand_in_stack){
						EmitPushOperand(operand2_type);
						is_operand_in_stack = true;
					}

					if(operand1_type->getDataType()==HCC_FLOATING_POINT)
					{
						EmitPromoteToFloatingPoint(operand2_type); //second operand
						is_operand_in_stack = true;
					}else{
						//neither operands are floating point values...
						if(is_operand_in_stack)
						{
							EmitPromoteToFloatingPoint(operand2_type, false);
							is_operand_in_stack = false;
						}
						mov(ESI, EAX);
						mov(EDI, EDX);
						EmitPromoteToFloatingPoint(operand1_type); //first operand inverse
						push(ESI);
						push(EDI);								   //second operand inverse
					}
				}
				//END - @@---
				//whatever data type, we must push it onto the stack...
				//
				//make the floating point comparison...
				if(operand1_type->getDataType()==HCC_INTEGER && 
					operand1_type!=HccTypeChecker::ts_type_Int64_ptr)
					call(no_op_ptr(FLOATING_POINT_COMPARE_INT));
				else
					call(no_op_ptr(FLOATING_POINT_COMPARE));

				cmp(EAX, 0); //set EFLAGS - FIXED Dec 28, 2008
				/*
					EAX==1  ? op1  > op2
					EAX==0  ? op1 == op2
					EAX==-1 ? op1 <  op2
				*/
				is_operand_in_stack = false; //@Dec 28, 2008
				/*
					EAX==1  ? neg --> -1
					EAX==0  ? neg --> 0
					EAX==-1 ? neg --> 1
				*/
				neg(EAX);
				//END - FIXED Dec 28, 2008
			}else if(operand1_type->getDataType()==HCC_STRING_TYPE && 
					 operand2_type->getDataType()==HCC_STRING_TYPE)
			{
				//BEGIN - FIXED Jan 1, 2008
				pop(EDI);
				mov(ESI, EAX);
				push(EAX);
				call(no_op_ptr(STRING_HANDLING_LENGTH));
				push(EAX);
				push(ESI);
				push(EDI);
				call(no_op_ptr(STRING_HANDLING_NCOMPARE));
				cmp(EAX, 0);
				//END - FIXED Jan 1, 2008
				//
			}else if(operand1_type->specifier()==DSPEC_CLASS && operand2_type->getDataType()==HCC_INTEGER)
			{
				pop(EBX);
				cmp(EBX, EAX);
			}else if(operand1_type->getDataType()==HCC_INTEGER && operand2_type->specifier()==DSPEC_CLASS)
			{
				pop(ECX);
				cmp(ECX, EAX);
			}
			else{ //F O R   A L L   O T H E R   O B J E C T S 
				CPUInstruction cld(this, CLD);
				CPUInstruction repe_cmpsb(this, REPE_CMPSB);
				//other types : strings, arrays, objects...
				//compare the memory bytes pointed to by EDI (operand1)
				//to the memory bytes pointed to by ESI (operand2)
				pop(EDI);
				mov(ESI, ECX);
				mov(ECX, operand1_type->getDataTypeSize());
				cld();
				repe_cmpsb(); //set EFLAGS with the comparison result
			}
			/*
			x86_INSTRUCTION x86jmp = JMP;

			switch(_operator)
			{
				//R E L A T I O N A L  O P
				//G R E A T E R   P R E C E D E N C E 
			case HCC_LESS_OP:		x86jmp = JL;	break;			//<
			case HCC_LESS_EQ_OP:	x86jmp = JLE;	break;			//<=
			case HCC_GREATER_OP:	x86jmp = JG;	break;			//>
			case HCC_GTER_EQ_OP:	x86jmp = JGE;	break;			//>=
				//L E S S   P R E C E D E N C E 
			case HCC_EQUAL_OP:		x86jmp = JZ;	break;			//==
			case HCC_NOT_EQ_OP:		x86jmp = JNZ;	break;			//!=
			};			
			
			assert(x86jmp!=JMP);
			if(x86jmp!=JMP)
			{
				mov(EAX, 1);		//denotes true (00000001)

				int jmpLabelIndex = SYMBOL_TABLE::__asm_label_index++;
				CPUInstruction jx(this, x86jmp);
				jx(no_op_ptr(CreateLabel(STMT_LABEL_PREFIX, jmpLabelIndex)));

				xor(EAX, EAX);		//denotes false (00000000)

				EmitStatementLabel(jmpLabelIndex);
				
				result_type_ptr = HccTypeChecker::ts_type_boolean_ptr;
			}
			*/
			x86_INSTRUCTION x86set_x = NOP;
			switch(_operator)
			{
				//R E L A T I O N A L  O P
				//G R E A T E R   P R E C E D E N C E 
			case HCC_LESS_OP:		x86set_x = SETL;	break;			//<
			case HCC_LESS_EQ_OP:	x86set_x = SETLE;	break;			//<=
			case HCC_GREATER_OP:	x86set_x = SETG;	break;			//>
			case HCC_GTER_EQ_OP:	x86set_x = SETGE;	break;			//>=
				//L E S S   P R E C E D E N C E 
			case HCC_EQUAL_OP:		x86set_x = SETZ;	break;			//==
			case HCC_NOT_EQ_OP:		x86set_x = SETNZ;	break;			//!=
			};	
			
			CPUInstruction setx(this, x86set_x);

			setx(AL);
			result_type_ptr = HccTypeChecker::ts_type_boolean_ptr;
		}
		break;
	};

__EXIT_BITWISE:

	return result_type_ptr;
}

//-----------------------------------------------
// EmitExprList	- Emit an expression (for &&, || and ternary '?')
//-----------------------------------------------
TypeSpecifier* HCCCodeGenerator::EmitExprList(Symbol *function_ptr)
{
	TypeSpecifier* result_type_ptr = NULL;

	CPUInstruction push(this, PUSH);
	CPUInstruction pop(this, POP);	
	CPUInstruction test(this, TEST);
	CPUInstruction jz(this, JZ);
	CPUInstruction jnz(this, JNZ);
	CPUInstruction jmp(this, JMP);
	CPUInstruction and(this, AND);
	CPUInstruction or(this, OR);
	CPUInstruction setnz(this, SETNZ);
	
	//simple expression
	TypeSpecifier* operand1_type = EmitRelationalExprList(function_ptr);	

	result_type_ptr = operand1_type;

	switch(token_type)
	{
	case HCC_AND_OP:			// &&
		{
			__uint __nAndFalseIndex = SYMBOL_TABLE::__asm_label_index++;
			__tstring __AndFalseLabel = CreateLabel(STMT_LABEL_PREFIX, __nAndFalseIndex);
			test(AL, AL);
			jz(VAR_OFFSET, __AndFalseLabel);
			getToken(); //&&
			//simple expression
			TypeSpecifier* operand2_type = EmitExprList(function_ptr);
			//
			EmitStatementLabel(__nAndFalseIndex);

			result_type_ptr = HccTypeChecker::ts_type_boolean_ptr;
		}
		break;
	case HCC_OR_OP:				// || 											
		{
			__uint __nPartialEvaluationTrueIndex = SYMBOL_TABLE::__asm_label_index++;
			__tstring __PartialEvaluationTrueLabel = CreateLabel(STMT_LABEL_PREFIX, __nPartialEvaluationTrueIndex);
			test(AL, AL);
			jnz(VAR_OFFSET, __PartialEvaluationTrueLabel);

			getToken();
			//simple expression
			TypeSpecifier* operand2_type = EmitExprList(function_ptr);

			EmitStatementLabel(__nPartialEvaluationTrueIndex);
				
			result_type_ptr = HccTypeChecker::ts_type_boolean_ptr;
		}
		break;
	case HCC_TERNARY_OP:
		{
			emitting_ternary_expr = true; getToken(); //?
			HccTypeChecker::CheckBoolean(operand1_type);

			int jumpExpr3_LabelIndex	 = SYMBOL_TABLE::__asm_label_index++;
			int jumpOutTernaryLabelIndex = SYMBOL_TABLE::__asm_label_index++;
			
			// expr1 ? expr2 : expr3
			test(AL,AL);	//set EFLAGS			
			jz(no_op_ptr(CreateLabel(STMT_LABEL_PREFIX, jumpExpr3_LabelIndex)));
			//			
			TypeSpecifier* expr2_type = EmitExprList(function_ptr); 
			getToken(); // ':' colon
			//
			jmp(no_op_ptr(CreateLabel(STMT_LABEL_PREFIX, jumpOutTernaryLabelIndex)));			
			
			EmitStatementLabel(jumpExpr3_LabelIndex);
			TypeSpecifier* expr3_type = EmitExprList(function_ptr);

			EmitStatementLabel(jumpOutTernaryLabelIndex);
			
			//we take as the result value any of the expression types in the ternary operator.
			result_type_ptr = expr3_type;

			emitting_ternary_expr = false;
		};
		break;
	};
	return result_type_ptr;
}


//--------------------------------------------------------------
//  EmitPushOperand		- Emits a push of an operand's value
//						  in an expression.
//
//--------------------------------------------------------------
void HCCCodeGenerator::EmitPushOperand(TypeSpecifier *operand_type_ptr)
{
	assert(operand_type_ptr!=NULL);
	if(operand_type_ptr==NULL)
		return;

	CPUInstruction push(this, PUSH);
	CPUInstruction mov(this, MOV);

	stack_operand_type = operand_type_ptr; //ADDED/FIXED Jan 26, 2009

	if(operand_type_ptr==HccTypeChecker::ts_type_Int64_ptr) //FIXED - Dec 18, 2008
		push(EDX);	//high part in a qword
	
	if(operand_type_ptr->specifier()==DSPEC_CLASS) //for comparing objects like in : (obj1==obj2)
		push(ECX);
	else
		push(EAX);
	if(
		operand_type_ptr==HccTypeChecker::ts_type_double_ptr	||
		operand_type_ptr==HccTypeChecker::ts_type_float_ptr
		)
		push(EDX);	//high part in a qword
}

void HCCCodeGenerator::EmitLoadValueAddressOf(Symbol *target_ptr, Symbol* function_ptr, bool bSetThisPointer)
{
	CPUInstruction mov(this, MOV);
	CPUInstruction lea(this, LEA);
	CPUInstruction pop(this, POP);
	CPUInstruction call(this, CALL);
	CPUInstruction xor(this, XOR);
	CPUInstruction movzx(this, MOVZX);
	CPUInstruction movsx(this, MOVSX);

	TypeSpecifier* result_type_ptr = target_ptr->getTypeSpecifier().getBaseTypeSpec();

	is_operand_in_stack = false; //@Jan 3, 2009

	volatile bool bIsLoadedInRegister = false;
	switch(target_ptr->getDeclDefinition().identifier_type())
	{	
	case DECL_PARAM_BYREF:			// in EAX == i ; in EBX == &i;	
	case DECL_PARAM_CONST_BYREF:

	case DECL_PARAM_POINTER:		// void func(int^ p1 = &i); // in EAX == &i ; in EBX == p1;
	case DECL_PARAM_ARRAY:
	case DECL_PARAM_CONST_ARRAY:
	case DECL_PARAM_CONST_POINTER:
		{
			EmitAddressFromAddressOf(target_ptr, function_ptr, false, NULL, MOV, EAX, EDX, bSetThisPointer);
			bIsLoadedInRegister = true;
		}
		break;
	case DECL_NEW_DATA_MEMBER: //now, we are generating two instead of three instructions for this load
		{
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				EmitLoadObjectInstancePointer_This(function_ptr);
				//the data member offset from the this pointer...
				int offset = target_ptr->getDeclDefinition().user_data.offset;

				x86_INSTRUCTION x86mov = MOVSX;
				if(HCC_UNSIGNED_TYPE==result_type_ptr->getDataTypeModifier())
					x86mov = MOVZX;
				CPUInstruction _movxx(this, x86mov);

				//the data member value...
				if(result_type_ptr->getDataType()==HCC_STRING_TYPE)
				{
					if(offset > 0)
						mov(EAX, dword_ptr(ECX, __plus, offset));
					else
						mov(EAX, dword_ptr(ECX));
				}else if(result_type_ptr->specifier()!=DSPEC_ARRAY)
				{
					//BEGIN - FIXED Dec 23, 2008
					if(result_type_ptr==HccTypeChecker::ts_type_Int64_ptr)
					{
						mov(EDX, dword_ptr(ECX, __plus, offset + sizeof(int)));	//high
						if(offset > 0)
							mov(EAX, dword_ptr(ECX, __plus, offset));	//low
						else
							mov(EAX, dword_ptr(ECX));					//low

					}
					//END - FIXED Dec 23, 2008
					else if(result_type_ptr==HccTypeChecker::ts_type_double_ptr ||
							 result_type_ptr==HccTypeChecker::ts_type_float_ptr)
					{
						mov(EAX, dword_ptr(ECX, __plus, offset + sizeof(int)));	//low
						if(offset > 0)
							mov(EDX, dword_ptr(ECX, __plus, offset));	//high
						else
							mov(EDX, dword_ptr(ECX));					//high
					}else if(result_type_ptr==HccTypeChecker::ts_type_short_ptr || 
							result_type_ptr->getDataTypeSize()==sizeof(short))
					{
						if(offset > 0)
							_movxx(EAX, word_ptr(ECX, __plus, offset));
						else
							_movxx(EAX, word_ptr(ECX));
					}else if(result_type_ptr==HccTypeChecker::ts_type_char_ptr || 
							result_type_ptr->getDataTypeSize()==sizeof(char))
					{
						if(offset > 0)
							_movxx(EAX, byte_ptr(ECX, __plus, offset));
						else
							_movxx(EAX, byte_ptr(ECX));
					}else
					{
						if(offset > 0)
							mov(EAX, dword_ptr(ECX, __plus, offset));
						else
							mov(EAX, dword_ptr(ECX));
					}
				}else{
					//BEGIN - FIXED Jan 10, 2009
					if(false==result_type_ptr->array.bIsDynamicArray)
					{
						//in case of arrays of chars, we must use the address as its value...
						if(offset > 0)
							lea(EAX, dword_ptr(ECX, __plus, offset));
						else
							lea(EAX, dword_ptr(ECX));
					}else{
						//in case of strings, we must use the address as its value...
						if(offset > 0)
							mov(EAX, dword_ptr(ECX, __plus, offset));
						else
							mov(EAX, dword_ptr(ECX));
					}
					//END - FIXED Jan 10, 2009
				}

				//is this data member a pointer variable?
				if(target_ptr->getDeclDefinition().user_data.bDataMemberIsPointer)
				{
					mov(ECX, EAX);
				}
				return;

			}else{
				//is this data member a pointer variable?
				if(target_ptr->getDeclDefinition().user_data.bDataMemberIsPointer)
				{
					mov(EAX, VAR_OFFSET, getSymbolLabel(target_ptr));
					mov(ECX, EAX);
					return;
				}else{
					lea(EBX, VAR_OFFSET, getSymbolLabel(target_ptr));
					bIsLoadedInRegister = true;
				}
			}
		}
		break;
	case DECL_POINTER_VARIABLE:		// p = &i; in EAX == &i ; in EBX == &p;
		{
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;

				if(false==bSetThisPointer){
					mov(EAX, dword_ptr(EBP, offset));	//&i
					//BEGIN - ADHOC Jan 18, 2009
					if(
					   HCC_EQUAL_OP==token_type || 		//==
					   HCC_NOT_EQ_OP==token_type		//!=
					   )
					{
						if(target_ptr->getTypeSpecifier().getBaseTypeSpec()->specifier()==DSPEC_CLASS)
							mov(ECX, EAX);
					}
					//END - ADHOC Jan 18, 2009
				}else
					mov(ECX, dword_ptr(EBP, offset));	//&i
				//lea(EBX, dword_ptr(EBP, offset));	//&p
			}else{
				//mov(EBX, VAR_OFFSET, getSymbolLabel(target_ptr));	//&p

				if(false==bSetThisPointer)
					mov(EAX, no_op_ptr(getSymbolLabel(target_ptr)));							//&i
				else
					mov(ECX, no_op_ptr(getSymbolLabel(target_ptr)));							//&i
			}

			return;
		}
		break;							
	case DECL_READONLY_PROPERTY:			//a read-only property
	case DECL_READWRITE_PROPERTY:			//a read-write property
		{
			//obj.prop1;
			/*
				call prop1;
				//EAX | EDX:EAX
			*/
			Symbol* prop_get_ptr = target_ptr->getDeclDefinition().function.property[_T("get")];
			assert(prop_get_ptr!=NULL);
			//
			if(prop_get_ptr!=NULL)
			{
				EmitLoadObjectInstancePointer_This(function_ptr);
				//emit a call to member...
				__tstring prop_label = getLabelFromName(prop_get_ptr->getCompleteName());				
				call(no_op_ptr(prop_label));
				//EAX | EDX:EAX

				Symbol* prop_put_ptr = target_ptr->getDeclDefinition().function.property[_T("put")];
				if(prop_put_ptr!=NULL)
				{
					/*
					//set in EBX the prop put function address...
					prop_label = getLabelFromName(prop_put_ptr->getCompleteName());
					mov(EBX, VAR_OFFSET, prop_label);
					*/
				}
			}
			return;
		}
		break;
	case DECL_WRITEONLY_PROPERTY:
		{
			xor(EAX,EAX);
			xor(EDX,EDX);

			EmitLoadObjectInstancePointer_This(function_ptr);

			Symbol* prop_put_ptr = target_ptr->getDeclDefinition().function.property[_T("put")];
			assert(prop_put_ptr!=NULL);

			if(prop_put_ptr!=NULL)
			{
				//set in EBX the prop put function address...
				__tstring prop_label = getSymbolLabel(prop_put_ptr);
				mov(EBX, VAR_OFFSET, prop_label);
			}

			return;
		}
		break;
	case DECL_CONSTANT:
		{
			__tstring const_label = getSymbolLabel(target_ptr);				

			if(result_type_ptr->specifier()!=DSPEC_ARRAY)
			{
				FLOATING_POINT_CONVERSION i64val = ::FromInt64(target_ptr->getDeclDefinition().constant.value.Integer);

				if(result_type_ptr==HccTypeChecker::ts_type_Int64_ptr)
				{
					if(i64val.dwLowPart==0)
					{						
						mov(EAX, i64val.dwHighPart);				//low
						xor(EDX, EDX);								//high
					}else{
						mov(EAX, i64val.dwLowPart);					//low
						mov(EDX, i64val.dwHighPart);				//high
					}
					//
				}else if(result_type_ptr==HccTypeChecker::ts_type_double_ptr ||
						 result_type_ptr==HccTypeChecker::ts_type_float_ptr)
				{
					FLOATING_POINT_CONVERSION fpVal = ::FromDouble(target_ptr->getDeclDefinition().constant.value.Double);

					mov(EAX, fpVal.dwLowPart);			//low
					mov(EDX, fpVal.dwHighPart);			//high
					//
				}else if(result_type_ptr==HccTypeChecker::ts_type_char_ptr)
					mov(EAX, (int)target_ptr->getDeclDefinition().constant.value.Character);
				//for all integer values including the boolean type...
				else if(result_type_ptr==HccTypeChecker::ts_type_boolean_ptr)
					mov(EAX, (int)target_ptr->getDeclDefinition().constant.value.Boolean);
				else{
					if(i64val.dwLowPart==0)
					{						
						mov(EAX, i64val.dwHighPart);				//low
						//xor(EDX, EDX);								//high	REMOVED - Jan 17, 2009
					}else{
						mov(EAX, i64val.dwLowPart);					//low
						//mov(EDX, i64val.dwHighPart);				//high		REMOVED - Jan 17, 2009
					}
				}
			}else
				lea(EAX, dword_ptr(EBX)); //EYE OPEN: i don't know yet if this is correct, because is an array type

			return;
		}
		break;
	default:
		break;
	};

	//F O R   A L L   T Y P E S   O F   V A R I A B L E S 		
	if(bIsLoadedInRegister)
	{
		if(result_type_ptr->specifier()!=DSPEC_ARRAY)
		{
			if(result_type_ptr->specifier()!=DSPEC_CLASS)
			{
				if(result_type_ptr==HccTypeChecker::ts_type_Int64_ptr)
				{
					mov(EDX, dword_ptr(EBX, sizeof(int)));	//high
					mov(EAX, dword_ptr(EBX));				//low
				}else if(result_type_ptr==HccTypeChecker::ts_type_double_ptr ||
						 result_type_ptr==HccTypeChecker::ts_type_float_ptr)
				{
					mov(EAX, dword_ptr(EBX, sizeof(int)));	//low
					mov(EDX, dword_ptr(EBX));				//high
				}else if(result_type_ptr->getDataTypeSize()==sizeof(int))
					mov(EAX, dword_ptr(EBX));
				else if(result_type_ptr->getDataTypeSize()==sizeof(short))
				{
					if(HCC_UNSIGNED_TYPE==result_type_ptr->getDataTypeModifier())
						movzx(EAX, word_ptr(EBX));
					else
						movsx(EAX, word_ptr(EBX));
				}
				else if(result_type_ptr->getDataTypeSize()==sizeof(char))
				{
					if(HCC_UNSIGNED_TYPE==result_type_ptr->getDataTypeModifier())
						movzx(EAX, byte_ptr(EBX));
					else
						movsx(EAX, byte_ptr(EBX));
				}
			}
		}
	}else
		EmitLoadValue(target_ptr);
}

long HCCCodeGenerator::getArrayCountFromType(TypeSpecifier *array_ptr)
{
	if(array_ptr!=NULL)
	{
		//double array[10][10][10]; 
		/*
				size==10*10*10*sizeof(double);	== 8000
				dim	== 1+1+1;					== 3
				item_count == 10*10*10;			== 1000
			*/
		if(array_ptr->specifier()==DSPEC_ARRAY)
			return array_ptr->array.item_count * getArrayCountFromType(array_ptr->array.pItemType);
		else
			return 1;
	}
	return 0;
}

long HCCCodeGenerator::getArrayDimensionsFromType(TypeSpecifier *array_ptr)
{
	if(array_ptr!=NULL)
	{
		//double array[10][10][10]; 
		/*
				size==10*10*10*sizeof(double);	== 8000
				dim	== 1+1+1;					== 3
				item_count == 10*10*10;			== 1000
			*/
		if(array_ptr->specifier()==DSPEC_ARRAY)
			return 1 + getArrayDimensionsFromType(array_ptr->array.pItemType);
		else
			return 0;
	}
	return 0;
}


TypeSpecifier* HCCCodeGenerator::emitArraySubscript(Symbol* array_var_syptr, TypeSpecifier *type_ptr, TypeSpecifier* pItemType, Symbol* function_ptr)
{
	//BEGIN - NEW IMPL - FIXED - Jan 23, 2009
	if(array_var_syptr==NULL && type_ptr==NULL)
		return NULL;

	if(type_ptr==NULL)
		type_ptr = array_var_syptr->getTypeSpecifier().getBaseTypeSpec();
	//END - NEW IMPL - FIXED - Jan 23, 2009

	CPUInstruction lea(this, LEA);
	CPUInstruction mov(this, MOV);
	CPUInstruction imul(this, IMUL);
	CPUInstruction and(this, AND);
	CPUInstruction add(this, ADD);
	CPUInstruction push(this, PUSH);
	CPUInstruction pop(this, POP);

	int final_offset = 0;

	TypeSpecifier* arrayTypeSpec = type_ptr;

	//BEGIN - NEW IMPL - FIXED - Jan 23, 2009
	//this time, we have the type and length at arrayTypeSpec

	//BEGIN - FIXED Feb 22, 2009
	//TypeSpecifier* pItemType = getArrayScalarType(arrayTypeSpec);
	//if the pItemType was not passed, we get it from the array type-spec;
	//when pItemType is passed, it containts the current type and size for the subscript
	if(NULL==pItemType)
		pItemType = getArrayScalarType(arrayTypeSpec);
	//END - FIXED Feb 22, 2009
	int type_size = pItemType->getDataTypeSize();

	long nDimensions = getArrayDimensionsFromType(arrayTypeSpec);	
	if(arrayTypeSpec->array.bIsDynamicArray)
	{
		// D Y N A M I C A L L Y   A L L O C A T E D   A R R A Y S 
		if(array_var_syptr!=NULL)
		{
			EmitLoadValueAddressOf(array_var_syptr, function_ptr);
			push(EAX);
		}else
			push(ESI);
		//
		if(nDimensions != 1)
		{
			//that could be a broken run in the parser, but it never happens!
			__asm int 3; should never get here, because dynamic arrays can have 1 dimension only in H++
				
		}

		if(token_type==HCC_LBRACKET)
		{
			getToken(); //[
			TypeSpecifier* last_expr_type_ptr = emitCommaExprList(function_ptr);
			getToken(); //]
			
			//__asm lea EAX, dword ptr [ESI+EAX*pItemType->getDataTypeSize()];
		}

		pop(ESI);
		pItemType = emitArraySubscriptReadorWrite(type_size, pItemType, function_ptr);
		//
	}else{
		//irow * (ncols * sizeof(type-spec)) + (jcol * sizeof(type-spec))
		//(irow * ncols + jcol) * sizeof(type-spec)
		//up to 2 dimensions
		if(token_type==HCC_LBRACKET)
		{
			if(array_var_syptr!=NULL)
			{
				//a different implementation
				if(false==(array_var_syptr->getDeclDefinition().identifier_type()==DECL_VARIABLE))
				{
					EmitLoadValueAddressOf(array_var_syptr, function_ptr);
					push(EAX);
				}
			}else
				push(ESI);

			getToken(); //[
			TypeSpecifier* last_expr_type_ptr = emitCommaExprList(function_ptr);
			getToken(); //]

			//T W O   D I M E N S I O N S   A R R A Y
			if(token_type==HCC_LBRACKET) //because this may be a two-dimensions array
			{
				if(arrayTypeSpec->array.pItemType==NULL || 
					arrayTypeSpec->array.pItemType->specifier()!=DSPEC_ARRAY)
				{
					__asm int 3; should never get here!
				}

				int sz = arrayTypeSpec->array.pItemType->array.item_count;

				if(array_var_syptr!=NULL && 
					array_var_syptr->getDeclDefinition().identifier_type()==DECL_VARIABLE)
				{
					// EAX = i*n*type_size
					imul(EAX, EAX, sz * type_size);
					//
					if(array_var_syptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
					{
						//__asm lea eax, [ebp+edx-offset];

						int offset = array_var_syptr->getDeclDefinition().user_data.offset;

						lea(EDX, dword_ptr(EBP, __plus, EAX, __minus, abs(offset)));
					}else{
						//global otherwise...
						lea(EDX, dword_ptr(EAX, __plus, getSymbolLabel(array_var_syptr)));
					}

					//we save it onto stack before attempting to process the next subscript expression
					push(EDX);

				}else{
					//because it's a fixed-size array parameter, we must save the value:
					// ESI = i*n*type_size					
					imul(EAX, EAX, sz);
					mov(ESI, EAX);
				}
			}else{
				//O N E   D I M E N S I O N   A R R A Y
				//just one dimension...
				if(array_var_syptr!=NULL && 
					array_var_syptr->getDeclDefinition().identifier_type()==DECL_VARIABLE)
				{
					if(array_var_syptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
					{
						//L O C A L
						imul(EAX, EAX, type_size);
						//
						int offset = array_var_syptr->getDeclDefinition().user_data.offset;
						//
						if(token_type==HCC_PERIOD)
						{
							//lea(ECX, dword_ptr(ESI, __plus, ECX, __mult, type_size));
							lea(ECX, dword_ptr(EBP, __plus, EAX, __minus, abs(offset)));
							//the this pointer is now in ECX as expected!
							return emitObjectInstanceMember(NULL, pItemType, function_ptr);
						}else{
							switch(token_type)
							{
								case HCC_INCREMENT:		//postfix ++
								case HCC_DECREMENT:		//postfix --

								case HCC_ASSIGN_OP:			// =
								case HCC_INC_ASSIGN:		// +=
								case HCC_DEC_ASSIGN:		// -=
								case HCC_MUL_ASSIGN:		// *=
								case HCC_DIV_ASSIGN:		// /=
									//for integer types only
								case HCC_MOD_ASSIGN:		// %=
								case HCC_XOR_ASSIGN:		// ^=
								case HCC_BIT_OR_ASSIGN:		// |=
								case HCC_BIT_AND_ASSIGN:	// &=
								{
									//the final address	: lea EAX, dword ptr [ESI+ECX*type_size]
									lea(EAX, dword_ptr(EBP, __plus, EAX, __minus, abs(offset)));
								}
								break;
								default: //the value is loaded instead
								{
									//BEGIN - CHANGED Jan 27, 2009
									if(bRequiresAddressOf)
									{
										//BEGIN - FIXED Feb 22, 2009
										if(pItemType->specifier()==DSPEC_CLASS)
											lea(ECX, dword_ptr(EBP, __plus, EAX, __minus, abs(offset)));
										else 
											lea(EAX, dword_ptr(EBP, __plus, EAX, __minus, abs(offset)));
										//END - FIXED Feb 22, 2009
									}else{
										if(pItemType->getDataType()==HCC_FLOATING_POINT)
										{
											mov(EDX, dword_ptr(EBP, __plus, EAX, __minus, abs(offset)));
											add.set_new_comment(_T("low part"));
											add(EAX, (int)sizeof(int));
											mov(EAX, dword_ptr(EBP, __plus, EAX, __minus, abs(offset)));
										}else if(pItemType->getDataType()==HCC_INTEGER)
										{
											mov(EAX, dword_ptr(EBP, __plus, EAX, __minus, abs(offset)));
											if(pItemType==HccTypeChecker::ts_type_Int64_ptr)
											{
												add.set_new_comment(_T("high part"));
												add(EAX, (int)sizeof(int));
												mov(EDX, dword_ptr(EBP, __plus, EAX, __minus, abs(offset)));
											}
										}else{
											//BEGIN - FIXED Jan 24, 2009
											if(pItemType->specifier()==DSPEC_CLASS)
												lea(ECX, dword_ptr(EBP, __plus, EAX, __minus, abs(offset)));
											//END - FIXED Jan 24, 2009
											//the final value	: mov EAX, dword ptr [ESI+ECX*type_size]
											mov(EAX, dword_ptr(EBP, __plus, EAX, __minus, abs(offset)));
										}
									}
									//END - CHANGED Jan 27, 2009
								}
								break;
							}
							return pItemType; //we MUST return from here!
						}
					}else{
						//G L O B A L , otherwise...
						if(token_type==HCC_PERIOD)
						{
							//lea(ECX, dword_ptr(ESI, __plus, ECX, __mult, type_size));
							emitArrayAddressLoadGlobalFromSubscript(array_var_syptr, type_size, LEA, ECX);
							//the this pointer is now in ECX as expected!
							return emitObjectInstanceMember(NULL, pItemType, function_ptr);
						}else{
							switch(token_type)
							{
								case HCC_INCREMENT:		//postfix ++
								case HCC_DECREMENT:		//postfix --

								case HCC_ASSIGN_OP:			// =
								case HCC_INC_ASSIGN:		// +=
								case HCC_DEC_ASSIGN:		// -=
								case HCC_MUL_ASSIGN:		// *=
								case HCC_DIV_ASSIGN:		// /=
									//for integer types only
								case HCC_MOD_ASSIGN:		// %=
								case HCC_XOR_ASSIGN:		// ^=
								case HCC_BIT_OR_ASSIGN:		// |=
								case HCC_BIT_AND_ASSIGN:	// &=
								{
									//the final address	: lea EAX, dword ptr [ESI+ECX*type_size]
									emitArrayAddressLoadGlobalFromSubscript(array_var_syptr, type_size, LEA, EAX);
								}
								break;
								default: //the value is loaded instead
								{			
									//the final value	: mov EAX, dword ptr [ESI+ECX*type_size]
									emitArrayAddressLoadGlobalFromSubscript(array_var_syptr, 
																			type_size, 
																			bRequiresAddressOf ? LEA : MOV, 
																			bRequiresAddressOf && arrayTypeSpec->specifier()==DSPEC_CLASS ? ECX : EAX,
																			EDX);
								}
								break;
							}
							return pItemType; //we MUST return from here!
						}						
					}
						
				}else{
					//DO NOTHING
				}
			}
		}
		if(token_type==HCC_LBRACKET)
		{
			//for two dimensions only
			getToken(); //[
			TypeSpecifier* last_expr_type_ptr = emitCommaExprList(function_ptr);
			getToken(); //]
			
			//BEGIN - hlm Nov 11,2011
			if(array_var_syptr!=NULL &&
				array_var_syptr->getDeclDefinition().identifier_type()!=DECL_VARIABLE)
			{
				add(EAX, ESI);
			}
			//END - hlm Nov 11,2011
			//__asm lea eax, [esi+eax*type_size];
		}
		pop(ESI);
		return emitArraySubscriptReadorWrite(type_size, pItemType, function_ptr);
	}
	//END - NEW IMPL - FIXED - Jan 23, 2009

	return pItemType;
}

TypeSpecifier* HCCCodeGenerator::EmitPostFixAssignmentExpr(Symbol *target_ptr, 
														   Symbol *function_ptr, 
														   bool bArraySubscript)
{

	CPUInstruction push(this, PUSH);
	CPUInstruction pop(this, POP);
	CPUInstruction mov(this, MOV);
	CPUInstruction lea(this, LEA);
	CPUInstruction inc(this, INC);
	CPUInstruction decc(this, DEC);

	HCC_TOKEN_TYPE  postfix_op = token_type;
	getToken();
	TypeSpecifier* result_type_ptr = target_ptr->getTypeSpecifier().getBaseTypeSpec();

	volatile bool bIsAddressOnStack = false;
	if(!bArraySubscript)
	{
		switch(target_ptr->getDeclDefinition().identifier_type())
		{	
		case DECL_PARAM_BYREF:
		case DECL_PARAM_CONST_BYREF:

		case DECL_PARAM_POINTER:
		case DECL_PARAM_ARRAY:
		case DECL_PARAM_CONST_ARRAY:
		case DECL_PARAM_CONST_POINTER:	
		case DECL_NEW_DATA_MEMBER:

			{
				/* I found a better way to do assignments: 
					instead of pushing the variable's address into the stack,
					we save it to the compiler stack, and then pop it 
					when ready to assign the expression's result.
				*/
				stack_of_ids.push(target_ptr);
			}
			break;
		case DECL_POINTER_VARIABLE:	//p = &i; p++ = (&i + 1)						
			break;
		default:
			break;
		};
	}
	else{
		//the address from emitArraySubscript...
		push(EAX);
		bIsAddressOnStack = true;
	}
	//
	if(postfix_op==HCC_INCREMENT)
	{
		//i++;
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			goto __POSTFIX_INC_TYPE;
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);

__POSTFIX_INC_TYPE:
			if(result_type_ptr==HccTypeChecker::ts_type_Int64_ptr)
			{
				//BEGIN - FIXED /REFACTORED CODE Dec 23, 2008
				/*
				mov(EDX, dword_ptr(EBX, sizeof(int)));	//low
				mov(EAX, dword_ptr(EBX));				//high
				*/
				emitLoadInt64ToRegisters();
				//
				//END - FIXED /REFACTORED CODE Dec 23, 2008
				inc(qword_ptr(EBX));
			}else{
				mov(EAX, dword_ptr(EBX));
				inc(dword_ptr(EBX));
			}
		}else{
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;

				if(result_type_ptr==HccTypeChecker::ts_type_Int64_ptr)
				{
					//BEGIN - FIXED /REFACTORED CODE Dec 23, 2008
					/*
					mov(EDX, dword_ptr(EBP, offset + sizeof(int)));		//high
					mov(EAX, dword_ptr(EBP, offset));					//low
					*/
					emitLoadInt64ToRegisters(true, (offset < 0 ? __minus : __plus), abs(offset) );
					//
					//END - FIXED /REFACTORED CODE Dec 23, 2008

					inc(qword_ptr(EBP, offset));
				}else{
					mov(EAX, dword_ptr(EBP, offset));
					inc(dword_ptr(EBP, offset));
				}
			}else{
				lea(EBX, no_op_ptr(getSymbolLabel(target_ptr)));

				if(result_type_ptr==HccTypeChecker::ts_type_Int64_ptr)
				{
					//END - FIXED /REFACTORED CODE Dec 23, 2008
					/*
					mov(EDX, dword_ptr(EBX, sizeof(int)));		//high
					mov(EAX, dword_ptr(EBX));					//low
					*/
					emitLoadInt64ToRegisters();
					//
					//END - FIXED /REFACTORED CODE Dec 23, 2008

					inc(qword_ptr(EBX));
				}else{										
					mov(EAX, dword_ptr(EDX));
					inc(dword_ptr(EDX));
				}
			}
		}
		
	}else{
		//i--;
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			goto __POSTFIX_DEC_TYPE;
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);

__POSTFIX_DEC_TYPE:
			if(result_type_ptr==HccTypeChecker::ts_type_Int64_ptr)
			{
				//BEGIN - FIXED /REFACTORED CODE Dec 23, 2008
				/*
				mov(EDX, dword_ptr(EBX, sizeof(int)));	//low
				mov(EAX, dword_ptr(EBX));				//high
				*/
				emitLoadInt64ToRegisters();
				//
				//END - FIXED /REFACTORED CODE Dec 23, 2008

				decc(qword_ptr(EBX));
			}else{
				mov(EAX, dword_ptr(EBX));
				decc(dword_ptr(EBX));
			}
		}else{
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;

				if(result_type_ptr==HccTypeChecker::ts_type_Int64_ptr)
				{
					//BEGIN - FIXED /REFACTORED CODE Dec 23, 2008
					/*
					mov(EDX, dword_ptr(EBP, offset + sizeof(int)));		//high
					mov(EAX, dword_ptr(EBP, offset));					//low
					*/
					emitLoadInt64ToRegisters(true, (offset < 0 ? __minus : __plus), abs(offset) );
					//
					//END - FIXED /REFACTORED CODE Dec 23, 2008

					decc(qword_ptr(EBP, offset));
				}else{
					mov(EAX, dword_ptr(EBP, offset));
					decc(dword_ptr(EBP, offset));
				}
			}else{
				lea(EBX, no_op_ptr(getSymbolLabel(target_ptr)));

				if(result_type_ptr==HccTypeChecker::ts_type_Int64_ptr)
				{
					//BEGIN - FIXED /REFACTORED CODE Dec 23, 2008
					/*
					mov(EDX, dword_ptr(EBX, sizeof(int)));	//low
					mov(EAX, dword_ptr(EBX));				//high
					*/
					emitLoadInt64ToRegisters();
					//
					//END - FIXED /REFACTORED CODE Dec 23, 2008

					decc(qword_ptr(EBX));
				}else{										
					mov(EAX, dword_ptr(EDX));
					decc(dword_ptr(EDX));
				}
			}
		}
	}
	return result_type_ptr;
}

TypeSpecifier* HCCCodeGenerator::EmitPrefixAssignmentExpr(Symbol *function_ptr)
{
	CPUInstruction push(this, PUSH);
	CPUInstruction pop(this, POP);
	CPUInstruction mov(this, MOV);
	CPUInstruction lea(this, LEA);
	CPUInstruction inc(this, INC);
	CPUInstruction decc(this, DEC);

	HCC_TOKEN_TYPE  prefix_op = token_type;	
	getToken();
	//Aug 14, 2009 : to avoid the yet unknown-reason crash...
	if(NULL==symbol_ptr)
		return HccTypeChecker::ts_type_int_ptr;
	//
	TypeSpecifier* result_type_ptr = symbol_ptr->getTypeSpecifier().getBaseTypeSpec();								
	Symbol* target_ptr = symbol_ptr;
	volatile bool bIsAddressOnStack = false;
	
	if(result_type_ptr->specifier()==DSPEC_ARRAY)
	{
		//++array[1];
		//--array[1];
		result_type_ptr = emitArraySubscript(NULL, result_type_ptr, NULL, function_ptr);
		push(EAX);
		bIsAddressOnStack = true;
	}
	else switch(symbol_ptr->getDeclDefinition().identifier_type())
	{	
	case DECL_PARAM_BYREF:
	case DECL_PARAM_CONST_BYREF:

	case DECL_PARAM_POINTER:
	case DECL_PARAM_ARRAY:
	case DECL_PARAM_CONST_ARRAY:
	case DECL_PARAM_CONST_POINTER:	
	case DECL_NEW_DATA_MEMBER:
		{
				/* I found a better way to do assignments: 
					instead of pushing the variable's address into the stack,
					we save it to the compiler stack, and then pop it 
					when ready to assign the expression's result.
				*/
				stack_of_ids.push(symbol_ptr);
		}
		break;					
	case DECL_POINTER_VARIABLE:	// p = &i ; ++p == &i + 1
			break;
	default:
		break;
	};
	//
	if(prefix_op==HCC_INCREMENT)
	{
		//++i;
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==symbol_ptr);
			//
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			goto __PREFIX_INC_TYPE;
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);

__PREFIX_INC_TYPE:
			if(result_type_ptr==HccTypeChecker::ts_type_Int64_ptr)
			{
				inc(qword_ptr(EBX));

				//BEGIN - FIXED /REFACTORED CODE Dec 23, 2008
				/*
				mov(EDX, dword_ptr(EBX, sizeof(int)));	//low
				mov(EAX, dword_ptr(EBX));				//high
				*/
				emitLoadInt64ToRegisters();
				//
				//END - FIXED /REFACTORED CODE Dec 23, 2008
			}else{							
				inc(dword_ptr(EBX));
				mov(EAX, dword_ptr(EBX));
			}
		}else{
			if(symbol_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = symbol_ptr->getDeclDefinition().user_data.offset;

				if(result_type_ptr==HccTypeChecker::ts_type_Int64_ptr)
				{
					inc(qword_ptr(EBP, offset));

					//BEGIN - FIXED /REFACTORED CODE Dec 23, 2008
					/*
					mov(EDX, dword_ptr(EBP, offset + sizeof(int)));		//high
					mov(EAX, dword_ptr(EBP, offset));					//low
					*/
					emitLoadInt64ToRegisters(true, (offset < 0 ? __minus : __plus), abs(offset) );
					//
					//END - FIXED /REFACTORED CODE Dec 23, 2008
				}else{								
					inc(dword_ptr(EBP, offset));
					mov(EAX, dword_ptr(EBP, offset));
				}
			}else{
				lea(EBX, no_op_ptr(getSymbolLabel(symbol_ptr)));

				if(result_type_ptr==HccTypeChecker::ts_type_Int64_ptr)
				{
					inc(qword_ptr(EBX));

					//BEGIN - FIXED /REFACTORED CODE Dec 23, 2008
					/*
					mov(EDX, dword_ptr(EBX, sizeof(int)));	//low
					mov(EAX, dword_ptr(EBX));				//high
					*/
					emitLoadInt64ToRegisters();
					//
					//END - FIXED /REFACTORED CODE Dec 23, 2008
				}else{																		
					inc(dword_ptr(EDX));
					mov(EAX, dword_ptr(EDX));
				}
			}
		}
		
	}else{
		//--i;
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			goto __PREFIX_DEC_TYPE;
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);

__PREFIX_DEC_TYPE:
			if(result_type_ptr==HccTypeChecker::ts_type_Int64_ptr)
			{
				decc(qword_ptr(EBX));

				//BEGIN - FIXED /REFACTORED CODE Dec 23, 2008
				/*
				mov(EDX, dword_ptr(EBX, sizeof(int)));	//low
				mov(EAX, dword_ptr(EBX));				//high
				*/
				emitLoadInt64ToRegisters();
				//
				//END - FIXED /REFACTORED CODE Dec 23, 2008
			}else{							
				decc(dword_ptr(EBX));
				mov(EAX, dword_ptr(EBX));
			}
		}else{
			if(symbol_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = symbol_ptr->getDeclDefinition().user_data.offset;

				if(result_type_ptr==HccTypeChecker::ts_type_Int64_ptr)
				{
					decc(qword_ptr(EBP, offset));

					//BEGIN - FIXED /REFACTORED CODE Dec 23, 2008
					/*
					mov(EDX, dword_ptr(EBP, offset + sizeof(int)));		//high
					mov(EAX, dword_ptr(EBP, offset));					//low
					*/
					emitLoadInt64ToRegisters(true, (offset < 0 ? __minus : __plus), abs(offset) );
					//
					//END - FIXED /REFACTORED CODE Dec 23, 2008
				}else{								
					decc(dword_ptr(EBP, offset));
					mov(EAX, dword_ptr(EBP, offset));
				}
			}else{
				lea(EBX, no_op_ptr(getSymbolLabel(symbol_ptr)));

				if(result_type_ptr==HccTypeChecker::ts_type_Int64_ptr)
				{
					decc(qword_ptr(EBX));

					//BEGIN - FIXED /REFACTORED CODE Dec 23, 2008
					/*
					mov(EDX, dword_ptr(EBX, sizeof(int)));	//low
					mov(EAX, dword_ptr(EBX));				//high
					*/
					emitLoadInt64ToRegisters();
					//
					//END - FIXED /REFACTORED CODE Dec 23, 2008
				}else{																		
					decc(dword_ptr(EDX));
					mov(EAX, dword_ptr(EDX));
				}
			}
		}
	}
	getToken(); //id
	return result_type_ptr; //EmitFactor(function_ptr);
}

TypeSpecifier* HCCCodeGenerator::emitCallWriteWriteLnBuiltInFunction(Symbol *write_fn_ptr, Symbol* function_ptr)
{
	CPUInstruction push(this, PUSH);
	CPUInstruction call(this, CALL);

	TypeSpecifier* result_type_ptr = NULL;
	do{
		getToken();	//	'(' | ','
		result_type_ptr = EmitExprList(function_ptr);
		
		if(result_type_ptr==HccTypeChecker::ts_type_string_ptr || 
			(result_type_ptr->specifier()==DSPEC_ARRAY && 
			 result_type_ptr->array.pItemType==HccTypeChecker::ts_type_char_ptr)
			)
		{
			push(result_type_ptr->array.item_count);	//length
			push(EAX);									//address
			call(no_op_ptr(CONSOLE_WRITE_STRING));
		}else if(result_type_ptr==HccTypeChecker::ts_type_Int64_ptr)
		{
			if(!is_operand_in_stack)
			{
				push(EDX);
				push(EAX); //Fixed Dec 28, 2008
			}
			is_operand_in_stack = false;
			call(no_op_ptr(CONSOLE_WRITE_INT64));
		}else if(result_type_ptr==HccTypeChecker::ts_type_double_ptr || 
				 result_type_ptr==HccTypeChecker::ts_type_float_ptr)
		{
			if(!is_operand_in_stack)
			{
				push(EAX);
				push(EDX);
			}
			is_operand_in_stack = false;
			call(no_op_ptr(CONSOLE_WRITE_FLOATING_POINT));
		}else if(result_type_ptr->getDataType()==HCC_INTEGER)
		{
			push(EAX);
			call(no_op_ptr(CONSOLE_WRITE_INTEGER));
		}else if(result_type_ptr==HccTypeChecker::ts_type_char_ptr)
		{
			push(EAX);
			call(no_op_ptr(CONSOLE_WRITE_CHARACTER));
		}else if(result_type_ptr==HccTypeChecker::ts_type_boolean_ptr)
		{
			push(EAX);
			call(no_op_ptr(CONSOLE_WRITE_BOOLEAN));
		}
		else{
			HccErrorManager::Error(HccErrorManager::errWrongTypeForThisOperator, _T("; expected types: boolean, integers, floating points, or strings."));
		}
		//
	}while(token_type==HCC_COMMA_OP);

	getToken(); // )
	
	if(write_fn_ptr->getDeclDefinition().identifier_type()==DECL_BUILTIN_CONSOLE_WRITELN)
	{
		//prints a new line...
		call(no_op_ptr(CONSOLE_WRITE_CRLF));
	}	
	return HccTypeChecker::ts_type_void_ptr;
}



TypeSpecifier* HCCCodeGenerator::emitObjectInstanceMember(Symbol* object_ptr, 
														  TypeSpecifier *arrayTypeSpec, 
														  Symbol* function_ptr)
{
	CPUInstruction pop(this, POP);
	CPUInstruction push(this, PUSH);
	CPUInstruction add(this, ADD);
	CPUInstruction mov(this, MOV);
	CPUInstruction call(this, CALL);
	CPUInstruction lea(this, LEA);

	TypeSpecifier *baseTypeSpec = (object_ptr!=NULL) ? object_ptr->getTypeSpecifier().getBaseTypeSpec() : 
													   arrayTypeSpec;

	TypeSpecifier* result_type_ptr = NULL;
	getToken();	// skip the '.'
	
	//the 'this' pointer is already in ECX...
	//the called/accessed member...
	Symbol* member_ptr = symbol_ptr;

	getToken(); //skip identifier

	//get the type spec...
	result_type_ptr = member_ptr->getTypeSpecifier().getBaseTypeSpec();

	volatile DECLARATION_TYPE type = member_ptr->getDeclDefinition().identifier_type();

	volatile bool bIsPropertyPut	= false;
	volatile bool bIsAddressOnStack = false;
	volatile bool bArraySubscript	= false;

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
			if(type==DECL_WRITEONLY_PROPERTY	||		//a write-only property
			   type==DECL_READWRITE_PROPERTY)		//a read-write property						
			{
				//for writable properties (P U T), we will push the function address onto stack...
				Symbol* prop_put_ptr = member_ptr->getDeclDefinition().function.property[_T("put")];
				assert(prop_put_ptr!=NULL);

				stack_of_ids.push(prop_put_ptr);
				bIsPropertyPut = true;
				//the new result type...
				if(object_ptr==NULL)
				{
					push(ECX);
					bIsAddressOnStack = true;
					Symbol* param_ptr = prop_put_ptr->getDeclDefinition().function.locals.stack_params.params[0];
					result_type_ptr = arrayTypeSpec = param_ptr->getTypeSpecifier().getBaseTypeSpec();
				}
			}else{
				//is a data member access...
				int offset = member_ptr->getDeclDefinition().user_data.offset;
			
				if(!bArraySubscript)
				{				//add(EAX, offset);
					//BEGIN - FIXED Jan 17, 2009
					if(object_ptr!=NULL)
						EmitLoadValueAddressOf(object_ptr, function_ptr, true);
					if(offset > 0)
						lea(EAX, dword_ptr(ECX, __plus, offset));
					else
						lea(EAX, dword_ptr(ECX));
					//END - FIXED Jan 17, 2009
					push(EAX);

					if(arrayTypeSpec!=NULL)
					{
						arrayTypeSpec = member_ptr->getTypeSpecifier().getBaseTypeSpec();
					}
				}else{
					push(EAX);
					bIsPropertyPut	  = false;					
				}
				bIsAddressOnStack = true;				
			}

			result_type_ptr = EmitAssignmentExpr(member_ptr, 
												token_type, 
												function_ptr,
												bIsAddressOnStack,
												bIsPropertyPut,
												object_ptr,
												arrayTypeSpec);
		}
		break;
	case HCC_PERIOD:		// .
		{
			if(arrayTypeSpec==NULL)
				result_type_ptr = emitObjectInstanceMember(member_ptr, result_type_ptr, function_ptr);
			else
				result_type_ptr = emitObjectInstanceMember(NULL, arrayTypeSpec, function_ptr);
		}
		break;
	case HCC_LBRACKET:	//id[x][y][...
		{
			//is an array like: array1[x].array2[y] = expr			
			lea.set_new_comment(_T("array data member as element on an object"));
			//is a data member access...
			int offset = member_ptr->getDeclDefinition().user_data.offset;
			//BEGIN - FIXED Jan 17, 2009
			if(offset > 0)
				lea(EAX, dword_ptr(ECX, __plus, offset));
			else
				mov(EAX, ECX);
			//END - FIXED Jan 17, 2009

			//EAX == ESI == array address...
			mov(ESI, EAX);
			//accessing an array element...
			baseTypeSpec = member_ptr->getTypeSpecifier().getBaseTypeSpec();
			result_type_ptr = arrayTypeSpec = emitArraySubscript(NULL, baseTypeSpec, NULL, function_ptr);
			//the address of the (array+offset*sizeof(type))...
			bArraySubscript = true;
			//if an assignment...
			goto __ASSIGNMENT_EXPRESSION;						
		}
		break;
	case HCC_LPAREN:
		{
			if(type==DECL_NEW_VIRTUAL_FUNC_MEMBER || 
				type==DECL_NEW_VIRTUAL_ABSTRACT_FUNC_MEMBER)
				emitVirtualMemberFunctionCall(object_ptr, member_ptr, baseTypeSpec, function_ptr);
			   /*
			{
				__tstring vtbl_ptr_symbol_name = CLASS_VPTR_VTBL_NAME;
				Symbol* vtbl_symbol_ptr = baseTypeSpec->getSymbolTable()->find(vtbl_ptr_symbol_name);
				//the vptr...
				assert(vtbl_symbol_ptr!=NULL);
				if(object_ptr!=NULL)
				{
					//emit the argument list...
					EmitActualParameterList(member_ptr, function_ptr);
					//the this pointer must be loaded in ECX...
					EmitLoadValueAddressOf(object_ptr, function_ptr, true); //Jan 3, 2009
				}else{					
					int this_ptr_offset = 
							member_ptr->getDeclDefinition().function.locals.stack_params.total_params_size;					

					if(this_ptr_offset > 0)
					{
						push(ECX);
					}
					//emit the argument list...
					EmitActualParameterList(member_ptr, function_ptr);
					if(this_ptr_offset > 0)
					{
						mov(ECX, dword_ptr(ESP, __plus, this_ptr_offset));
					}
				}

				//the vtbl offset...
				int vptr_offset = vtbl_symbol_ptr->getDeclDefinition().user_data.offset;
				if(vptr_offset > 0)
					//get the vtbl
					mov(EDX, dword_ptr(ECX, __plus, vptr_offset));
				else
					//get the vtbl
					mov(EDX, dword_ptr(ECX));

				//the virtual function offset...
				int offset = member_ptr->getDeclDefinition().user_data.offset;
				if(offset > 0)
					//call the virtual function...
					call(dword_ptr(EDX, __plus, offset));
				else
					//call the virtual function...
					call(dword_ptr(EDX));
				//
				if(object_ptr==NULL)
				{
					int this_ptr_offset = 
							member_ptr->getDeclDefinition().function.locals.stack_params.total_params_size;					
					if(this_ptr_offset > 0)
					{
						pop(ECX);
					}
				}
			}
			*/else{
				EmitFunctionCall(member_ptr, function_ptr, object_ptr); //(type==DECL_NEW_FUNC_MEMBER)
			}

			//the function return in EAX | EDX:EAX
			result_type_ptr = member_ptr->getDeclDefinition().function.return_type;

			//BEGIN - ADDED Mar 1, 2009
			if(token_type==HCC_PERIOD)
			{
				mov(ECX, EAX);
				TypeSpecifier* pItemType = member_ptr->getDeclDefinition().function.return_type;
				//the this pointer is now in ECX as expected!
				result_type_ptr = emitObjectInstanceMember(NULL, pItemType, function_ptr);
			}
			//END - ADDED Mar 1, 2009
		}
		break;
	default:
		//this was a missed implementation: load the value from this property or attribute...
		if(false==bArraySubscript) //<-- ADDED - Jan 23, 2009
		{
			//the this pointer must be loaded in ECX...
			if(object_ptr!=NULL)
			{
				//BEGIN - FIXED Jan 26, 2009
				//EmitLoadValue(object_ptr);
				EmitLoadValueAddressOf(object_ptr, function_ptr, true);
				//END - FIXED Jan 26, 2009
			}
			//load the member value...
			if(type==DECL_READONLY_PROPERTY	||		//a read-only property
			   type==DECL_READWRITE_PROPERTY)		//a read-write property						
			{
				//for readable properties, call the member...
				Symbol* prop_get_ptr = member_ptr->getDeclDefinition().function.property[_T("get")];
				assert(prop_get_ptr!=NULL);				
				
				if(prop_get_ptr!=NULL)
				{
					//for objects that belongs to arrays...
					if(object_ptr==NULL || arrayTypeSpec!=NULL)
					{
						result_type_ptr = prop_get_ptr->getDeclDefinition().function.return_type;
					}
					__tstring prop_label = getLabelFromName(prop_get_ptr->getCompleteName());
					call(no_op_ptr(prop_label));
				}				
			}else{
				//BEGIN - FIXED Jan 26, 2009
				/*
				//is a data member access...
				int offset = member_ptr->getDeclDefinition().user_data.offset;
				//add(EAX, offset);
				if(offset > 0)
					mov(EAX, dword_ptr(ECX, __plus, offset));
				else
					mov(EAX, dword_ptr(ECX));
				*/
				EmitLoadValueAddressOf(member_ptr, NULL);
				//END - FIXED Jan 26, 2009
			}
		}
		break;
	}

	return result_type_ptr;
}

void HCCCodeGenerator::SaveICodePos()
{
	m_nICodePos = icode_ptr->get_pos();

	if(token_type==HCC_IDENTIFIER	|| 
	   token_type==HCC_NUMBER		||
	   token_type==HCC_CONTROL_CHAR ||
	   token_type==HCC_CHARACTER	||
	   token_type==HCC_STRING_LITERAL)
	{
		m_nICodePos -= sizeof(int);		//symbol address
		m_nICodePos -= sizeof(wchar_t); //token_type
	}else
		m_nICodePos -= sizeof(wchar_t); //token_type
}

void HCCCodeGenerator::RestoreICodePos()
{
	icode_ptr->set_pos(m_nICodePos);
}

//---------------------------------------------------------------
//	getArraySizeFromType() - get the user defined array size from an array type specifier
//
//		double my_array[10]; ---> getArraySizeFromType(&my_array->getTypeSpecifier())	== 80;
//		double my_array[10][10];  item_count * item_count * 8							== 800;
//		double my_array[10][10][10];  item_count * item_count * item_count * 8			== 8000;
//---------------------------------------------------------------
long HCCCodeGenerator::getArraySizeFromType(TypeSpecifier *array_ptr)
{
	if(array_ptr!=NULL)
	{
		if(array_ptr->specifier()==DSPEC_ARRAY){
			//this is a special case: for string types in arrays, we just specify the size of an address
			if(array_ptr->getDataType()==HCC_STRING_TYPE)
				return sizeof(int);

			return array_ptr->array.item_count * getArraySizeFromType(array_ptr->array.pItemType);
		}
		else
			return array_ptr->getDataTypeSize();
	}
	return 0;
}

TypeSpecifier* HCCCodeGenerator::getArrayScalarType(TypeSpecifier *array_ptr)
{
	if(array_ptr!=NULL)
	{
		if(array_ptr->specifier()==DSPEC_ARRAY){
			//this is a special case: for string types in arrays, we just specify the size of an address
			if(array_ptr->getDataType()==HCC_STRING_TYPE)
				return HccTypeChecker::ts_type_string_ptr;

			return getArrayScalarType(array_ptr->array.pItemType);
		}
		else
			return array_ptr;
	}
	return 0;
}

extern bool bPatternInitHeapVariables;

//-------------------------------------------------------------------
//	type-spec ^ p = new type-spec;				--> all scalar types | custom-types
//	type-spec ^ p = new type-spec(param-list);	--> all custom types for now only.
//	type-spec ^ p = new type-spec[integer-expression];
//	type-spec[] p = new type-spec[integer-expression];
//
//-------------------------------------------------------------------
TypeSpecifier* HCCCodeGenerator::EmitNewDynamicAllocation(Symbol *target_ptr, 
														  Symbol *function_ptr, 
														  bool bIsAddressOnStack, 
														  bool bIsPropertyPut)
{
	CPUInstruction push(this, PUSH);
	CPUInstruction pop(this, POP);
	CPUInstruction call(this, CALL);
	CPUInstruction mov(this, MOV);
	CPUInstruction imul(this, IMUL);
	CPUInstruction sub(this, SUB);

	CPUInstruction xor(this, XOR);
	CPUInstruction test(this, TEST);
	CPUInstruction jz(this, JZ);
	CPUInstruction jmp(this, JMP);

	CPUInstruction lea(this, LEA);
	CPUInstruction cmp(this, CMP);
	CPUInstruction add(this, ADD);
	CPUInstruction jnz(this, JNZ);

	int idx_dynamic_failed		= SYMBOL_TABLE::__asm_label_index++;
	int idx_continue_success	= SYMBOL_TABLE::__asm_label_index++;

	__tstring label_memory_failed = CreateLabel(STMT_LABEL_PREFIX, idx_dynamic_failed);
	__tstring label_memory_success = CreateLabel(STMT_LABEL_PREFIX, idx_continue_success);

	//just in case that this is a data member that is also a pointer, and not a simple pointer variable 
	//then we must take it way before doing the job...
	if(!stack_of_ids.empty() && target_ptr==stack_of_ids.top())
		stack_of_ids.pop();

	int size = 0;

	getToken(); //new 	
	//
	//TypeSpecifier* result_type_ptr = target_ptr->getTypeSpecifier().getBaseTypeSpec();
	//BEGIN - FIXED - Jan 2, 2008
	TypeSpecifier* result_type_ptr = NULL;
	if(symbol_ptr!=NULL){
		if(symbol_ptr->getDeclDefinition().identifier_type()==DECL_NEW_CLASS_CONSTRUCTOR)
		{
			Symbol* class_ptr = symbol_ptr->getOwner();
			if(class_ptr!=NULL)
				result_type_ptr = &class_ptr->getTypeSpecifier();
			else
				result_type_ptr = target_ptr->getTypeSpecifier().getBaseTypeSpec(); //for scalar types it's required
		}else if(symbol_ptr->getTypeSpecifier().specifier()==DSPEC_ARRAY)
		{
			//now, we have an array to construct dynamically
			result_type_ptr = &symbol_ptr->getTypeSpecifier();
		}else
			result_type_ptr = target_ptr->getTypeSpecifier().getBaseTypeSpec(); //for scalar types it's required
	}else
		result_type_ptr = target_ptr->getTypeSpecifier().getBaseTypeSpec();
	//END - FIXED - Jan 2, 2008

	getToken(); //type-spec

	if(result_type_ptr->specifier()==DSPEC_ARRAY)
	{
		//BEGIN - FIXED - Jan 21, 2009
		//because we don't know yet the array size, we must calculate it based on the expressions and
		//the size of the itemType...
		TypeSpecifier* pItemType = getArrayScalarType(result_type_ptr);
		//the size in bytes of the objects to be held in the array
		size = pItemType->getDataTypeSize();
		//
		if(token_type==HCC_LBRACKET)
		{
			getToken(); //[
			//type-spec [] array = new type-spec[expr1];
			//size = expr1 * scalar_type_size
			EmitExprList(function_ptr);
			//
			imul(EAX, size);
			getToken(); //]
		}
		//this is done only to preserve the actual size in bytes of this object array;
		//this data is used later when calling the constructors for each object,
		//or when calling the destructors;
		if(pItemType->specifier()==DSPEC_CLASS)
		{
			lea(EBX, dword_ptr(EAX, sizeof(int))); //EBX = (EAX + 4 bytes) extra space for prefixing array length
			push(EBX);
		}else
			push(EAX);
		//
		mov(EBX, EAX);
		size = sizeof(int);
		//END - FIXED - Jan 21, 2009
	}else{
		//use the class type specifier to get the correct object layout...
		size = result_type_ptr->getDataTypeSize();
		if(size < 1) 
			size = 1;
		push(size);
	}	
	//size is currently on the stack (ESP)
	//allocate the space in the heap...	
	call(no_op_ptr(SYSTEM_MEMORY_OPER_NEW));	
	test(EAX, EAX);
	jz(no_op_ptr(label_memory_failed));

	mov(ESI, EAX);	//the 'this' pointer
	
	//BEGIN - OPTIMIZED - Jan 23, 2009
	if(bPatternInitHeapVariables && size >= sizeof(int))
	{
		CPUInstruction lea(this, LEA);
		CPUInstruction rep_stos(this, REP_STOS);

		int heap_size = 0;
		lea(EDI, dword_ptr(ESI));
		if(result_type_ptr->specifier()==DSPEC_ARRAY)
		{
			mov(ECX, EBX);
		}
		else
		{
			//for other types
			//M E M O R Y   A L I G N M E N T 
			int heap_size = (int)(size/sizeof(int));
			int modulus	= size%sizeof(int);
			if(modulus > 0)
				heap_size -= modulus;

			mov(ECX, (int)(heap_size));
		}
		mov(EAX, (int)0xCDCDCDCD);

		if(result_type_ptr->specifier()==DSPEC_ARRAY)
			rep_stos(byte_ptr(EDI));
		else
			rep_stos(dword_ptr(EDI));
	}

	if(result_type_ptr->specifier()==DSPEC_ARRAY)
	{
		if(getArrayScalarType(result_type_ptr)->specifier()==DSPEC_CLASS)
		{
			mov(dword_ptr(ESI), EBX); //prefix the length of the allocated memory
			add(ESI, (int)sizeof(int));	  //to point to the beginning of this array
		}
	}
	//END - OPTIMIZED - Jan 23, 2009

	//BEGIN - REFACTORED Jan 18, 2009
	jmp(no_op_ptr(label_memory_success));	

	EmitStatementLabel(idx_dynamic_failed);		//M E M O R Y   F A I L E D
	xor(ESI, ESI); //denotes a memory error (AV)

	EmitStatementLabel(idx_continue_success);	//S U C C E S S  B L O C K

	if(bIsAddressOnStack)
	{
		pop(EBX);	//the pointer variable address...
		mov(dword_ptr(EBX), ESI);
	}else if(bIsPropertyPut)
	{
		pop(EBX);	//the pointer variable address...

		push(ESI);		
		call(dword_ptr(EBX));
		//
	}else if(target_ptr!=NULL)
	{
		x86_REGISTER ctx_reg = EBP;
		if(target_ptr->getDeclDefinition().identifier_type()==DECL_NEW_DATA_MEMBER)
		{
			ctx_reg = ECX;
			//mov the this pointer to ECX register...
			//the variable offset where to look for the this pointer...
			int this_ptr_localv_offset = sizeof(int);

			if(DATA_MEMBERS_INIT_PROC_ADDRESS!=function_ptr)
				this_ptr_localv_offset = function_ptr->getDeclDefinition().function.locals.stack_variables.total_variables_size;

			CPUInstruction mov(this, MOV);
			//the this pointer...
			mov(ECX, dword_ptr(EBP, __minus, this_ptr_localv_offset));
		}

		if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
		{
			int offset = target_ptr->getDeclDefinition().user_data.offset;
			if(offset < 0)
				mov(dword_ptr(ctx_reg, __minus, abs(offset)), ESI);
			else{
				if(!offset)
					mov(dword_ptr(ctx_reg), ESI);
				else
					mov(dword_ptr(ctx_reg, __plus, abs(offset)), ESI);
			}
		}else
			mov(no_op_ptr(getSymbolLabel(target_ptr)), ESI);
	}	

	if(result_type_ptr->specifier()==DSPEC_CLASS)
	{
		//the this pointer...
		//mov(ECX, ESI); //<-- REMOVED - Mar 04, 2009
		//this constructor is never called if there was a memory failure...
		//O B J E C T   C O N S T R U C T O R
		result_type_ptr = EmitObjectConstructorCall(result_type_ptr, function_ptr, NULL, ESI);
		mov(EAX, ESI); //REFACTORED - Jan 18, 2009
		//
	}else if(result_type_ptr->specifier()==DSPEC_ARRAY)
	{
		TypeSpecifier* pItemType = getArrayScalarType(result_type_ptr);
		//call mutiple ctors for array of objects...
		if(pItemType->specifier()==DSPEC_CLASS)
		{
			//emit multiple constructor call...
			unsigned int __multiple_ctor_call_loop_index = SYMBOL_TABLE::__asm_label_index++;
			__tstring __mult_ctor_call_loop_label = CreateLabel(STMT_LABEL_PREFIX, __multiple_ctor_call_loop_index);

			mov(ECX, dword_ptr(ESI, __minus, sizeof(int)));
			lea(EDI, dword_ptr(ESI, __plus, ECX));
			mov(EBX, ESI);
			EmitStatementLabel(__multiple_ctor_call_loop_index);
			//
			mov(ECX, EBX);
			EmitObjectConstructorCall(pItemType, function_ptr, NULL);
			//
			add(EBX, pItemType->getDataTypeSize());
			cmp(EBX, EDI);
			jnz(VAR_OFFSET, __mult_ctor_call_loop_label);
			//
			mov(EAX, ESI);
		}
	}

	return result_type_ptr;
}

void HCCCodeGenerator::EmitClassVtables()
{
	set<__tstring> generated_vtbls;
	//Emit all literal constants as global variables...	
	SYMBOL_TABLE::iterator it_symbl = symbol_table_ptr->begin();	
	while(it_symbl != symbol_table_ptr->end())
	{
		Symbol* symbol_ptr = (*it_symbl).second;		

		TypeSpecifier* baseTypeSpec = &symbol_ptr->getTypeSpecifier();

		while(baseTypeSpec!=NULL)
		{
			if(baseTypeSpec->specifier()==DSPEC_CLASS)
			{
				volatile bool bNotGenerated = 
								(generated_vtbls.find(baseTypeSpec->getTypeName())==generated_vtbls.end());

				if(bNotGenerated)
				{
					__tstring vtbl_ptr_symbol_name = CLASS_VPTR_VTBL_NAME;
					
					//if a vtable ptr is found, generate the vtable for this class symbol...
					Symbol* vtbl_symbol_ptr = baseTypeSpec->getSymbolTable()->find(vtbl_ptr_symbol_name);			
					if(vtbl_symbol_ptr!=NULL)
					{
						assert(vtbl_symbol_ptr->getDeclDefinition().identifier_type()==DECL_VTBL_PTR);
						//				
						__tstring class_vptr_name = CLASS_VTBL_VPTR_PREFIX;				
						
						unit() << class_vptr_name << getLabelFromName(baseTypeSpec->getTypeName())
							   << ctab
							   << _T("DWORD")
							   << ctab;

						vector<Symbol*>& virtuals = vtbl_symbol_ptr->getTypeSpecifier().user_type_members.function_members;

						vector<Symbol*>::iterator it_virtual = virtuals.begin();
						while(it_virtual!= virtuals.end())
						{
							Symbol* virtual_fn_ptr = *it_virtual++;
							//
							DECLARATION_TYPE virtual_decl_type =
												virtual_fn_ptr->getDeclDefinition().identifier_type();

							if(virtual_decl_type!=DECL_NEW_VIRTUAL_ABSTRACT_FUNC_MEMBER)							
								unit() << _T("OFFSET ")
									   << getLabelFromName(virtual_fn_ptr->getCompleteName());
							else
								unit() << _T("OFFSET ")
									   << __PURE_VIRTUAL_CALL;

							if(it_virtual!= virtuals.end())
								unit() << comma 
									   << endl 
									   << ctab << ctab << ctab
									   << ctab << ctab << ctab;
							else
								unit() << endl;
						}
						//
						unit() << endl;
					}
					//flag as generated...
					generated_vtbls.insert(baseTypeSpec->getTypeName());
				}
				if(baseTypeSpec==baseTypeSpec->getBaseTypeSpec())
					break;
				else{
					baseTypeSpec = baseTypeSpec->getBaseTypeSpec();
					continue;
				}
			}
			break;			
		}

		//next symbol...
		it_symbl++;
	}
}

TypeSpecifier* HCCCodeGenerator::EmitObjectConstructorCall(TypeSpecifier *class_type_ptr, 
														   Symbol* function_ptr, 
														   Symbol* object_ptr,
														   x86_REGISTER x86RegThisPointer)
{
	CPUInstruction mov(this, MOV);
	CPUInstruction call(this, CALL);
	//in ECX must be the this pointer...
	if(class_type_ptr==NULL)
		return class_type_ptr;

	//The only declaration for object instantiation actually, is a call to a constructor...		
	const __tstring& lpszType = class_type_ptr->getTypeName();
	assert(lpszType.length() > 0);

	//O B J E C T   C O N S T R U C T O R
	if(lpszType.length() > 0)
	{
		//a call to a constructor for an object declaration...
		Symbol* constructor_ptr = class_type_ptr->getSymbolTable()->find(lpszType);
		if(constructor_ptr!=NULL)
		{
			assert(constructor_ptr->getDeclDefinition().identifier_type()==DECL_NEW_CLASS_CONSTRUCTOR);
			
			EmitFunctionCall(constructor_ptr, function_ptr, object_ptr, x86RegThisPointer);
		}		
	}
	return class_type_ptr;
}

TypeSpecifier* HCCCodeGenerator::EmitDestructorCall(Symbol* symbol_ptr, 
													Symbol* function_ptr, 
													TypeSpecifier* dyn_type_ptr, 
													bool* pbHasDestructor)
{
	CPUInstruction push(this, PUSH);
	CPUInstruction pop(this, POP);
	CPUInstruction mov(this, MOV);
	CPUInstruction call(this, CALL);	
	CPUInstruction test(this, TEST);
	CPUInstruction jz(this, JZ);

	TypeSpecifier *class_type_ptr = NULL;
	
	if(symbol_ptr!=NULL)
		class_type_ptr = symbol_ptr->getTypeSpecifier().getBaseTypeSpec();

	//the known dynamic type...
	if(dyn_type_ptr!=NULL)
		class_type_ptr = dyn_type_ptr;

	if(pbHasDestructor!=NULL)
		*pbHasDestructor = false;
	//do nothing for scalar types...
	if(class_type_ptr==NULL || class_type_ptr->is_scalar())
		return HccTypeChecker::ts_type_int_ptr;

	unsigned int __nodestroy_null_index = SYMBOL_TABLE::__asm_label_index++;
	__tstring __no_destroy_null = CreateLabel(STMT_LABEL_PREFIX, __nodestroy_null_index);
	
	LPSYMBOL_TABLE symbl_tbl = class_type_ptr->getSymbolTable();
	if(symbl_tbl!=NULL)
	{
		//BEGIN - D E T E R M I N E   A   V I R T U A L   D E S T R U C T O R - Jan 5, 2009
		__tstring vtbl_ptr_symbol_name = CLASS_VPTR_VTBL_NAME;
		Symbol* vtbl_symbol_ptr = symbl_tbl->find(vtbl_ptr_symbol_name);
		if(vtbl_symbol_ptr!=NULL)
		{
			TypeDataMembers& vtbl_members = vtbl_symbol_ptr->getTypeSpecifier().user_type_members.function_members;
			//to save the virtual function's offset
			int vfn_offset = 0;

			TypeDataMembers::iterator it_virtual_fn = vtbl_members.begin();
			while(it_virtual_fn != vtbl_members.end())
			{
				Symbol* func_ptr = *it_virtual_fn;

				if(func_ptr->getName()==HPP_CLASS_DESTRUCTOR)
				{
					if(pbHasDestructor!=NULL)
						*pbHasDestructor = true;
					//the this pointer must be loaded in ECX...
					if(symbol_ptr!=NULL)
					{
						EmitLoadValueAddressOf(symbol_ptr, function_ptr, true);
						if(pbHasDestructor!=NULL)
						{
							//*pbHasDestructor = true;
							if(symbol_ptr->getDeclDefinition().identifier_type()!=DECL_VARIABLE)
								mov(ESI, ECX);
						}
					}
					//BEGIN - ADDED Jan 18, 2009
					if(symbol_ptr==NULL || 
						symbol_ptr->getDeclDefinition().identifier_type()!=DECL_VARIABLE)
					{
						test(ECX, ECX);
						jz(VAR_OFFSET, __no_destroy_null);
					}
					//END - ADDED Jan 18, 2009
					//the vtbl offset...
					int vptr_offset = vtbl_symbol_ptr->getDeclDefinition().user_data.offset;
					if(vptr_offset > 0)
						//get the vtbl
						mov(EAX, dword_ptr(ECX, __plus, vptr_offset));
					else
						//get the vtbl
						mov(EAX, dword_ptr(ECX));

					//the virtual function offset...
					int offset = func_ptr->getDeclDefinition().user_data.offset;
					if(offset > 0)
						//call the virtual function...
						call(dword_ptr(EAX, __plus, offset));
					else
						//call the virtual function...
						call(dword_ptr(EAX));
			
					//
					EmitStatementLabel(__nodestroy_null_index);
					//
					return HccTypeChecker::ts_type_int_ptr;
				}
				
				it_virtual_fn++;
			}
		}
		//END - D E T E R M I N E   A   V I R T U A L   D E S T R U C T O R - Jan 5, 2009

		Symbol* destructor = symbl_tbl->find(HPP_CLASS_DESTRUCTOR);	
		//a destructor call...
		if(destructor!=NULL)
		{
			if(pbHasDestructor!=NULL)
				*pbHasDestructor = true;

			if(symbol_ptr!=NULL)
			{
				EmitLoadValueAddressOf(symbol_ptr, function_ptr, true);
				assert(destructor->getDeclDefinition().identifier_type()==DECL_UNIQUE_DESTRUCTOR);		

				if(symbol_ptr->getDeclDefinition().identifier_type()!=DECL_VARIABLE)
					mov(ESI, ECX);
			}
			//BEGIN - ADDED Jan 18, 2009
			if(symbol_ptr->getDeclDefinition().identifier_type()!=DECL_VARIABLE)
			{
				test(ECX, ECX);
				jz(VAR_OFFSET, __no_destroy_null);
			}
			//END - ADDED Jan 18, 2009
			call(no_op_ptr(getLabelFromName(destructor->getCompleteName())));
			//
			EmitStatementLabel(__nodestroy_null_index);
			//
		}
	}

	return HccTypeChecker::ts_type_int_ptr;
}

void HCCCodeGenerator::emitMainHPPRT_Startup(Symbol *entry_point_ptr)
{
	assert(entry_point_ptr!=NULL);

	if(entry_point_ptr==NULL)
		return;

	__tstring main_entry = getSymbolLabel(entry_point_ptr);

	CPUInstruction mov(this, MOV);	
	CPUInstruction and(this, AND);
	CPUInstruction shr(this, SHR);
	CPUInstruction shl(this, SHL);
	CPUInstruction call(this, CALL);
	CPUInstruction push(this, PUSH);
	CPUInstruction add(this, ADD);
	CPUInstruction lea(this, LEA);
//1-------------------------------------------------------------------------------
/*		
0040F7B6   call        dword ptr [__imp__GetVersion@0 (004871b0)]
0040F7BC   mov         [__osver (00484004)],eax		;a global variable has the OS version

		;the minor version
0040F7C1   mov         eax,[__osver (00484004)]
0040F7C6   shr         eax,8
0040F7C9   and         eax,0FFh
0040F7CE   mov         [__winminor (00484010)],eax	;a global variable

		;the major version
0040F7D3   mov         ecx,dword ptr [__osver (00484004)]
0040F7D9   and         ecx,0FFh
0040F7DF   mov         dword ptr [__winmajor (0048400c)],ecx

		;the Windows version		
0040F7E5   mov         edx,dword ptr [__winmajor (0048400c)]
0040F7EB   shl         edx,8
0040F7EE   add         edx,dword ptr [__winminor (00484010)]
0040F7F4   mov         dword ptr [__winver (00484008)],edx

0040F7FA   mov         eax,[__osver (00484004)]
0040F7FF   shr         eax,10h
0040F802   and         eax,0FFFFh
0040F807   mov         [__osver (00484004)],eax
*/
//1-------------------------------------------------------------------------------
	
	unit() << SYSTEM_HPP_RUNTIME_INIT << space << _T("PROC") << endl;
	EmitFunctionProlog(4);
	//
	call(no_op_ptr(_T("GetVersion")));
	mov.set_new_comment(_T("os version"));
	mov(no_op_ptr(__OSVER), EAX);

	mov(EAX, no_op_ptr(__OSVER));
	shr(EAX, 8);
	and(EAX, 0x0FF);
	mov.set_new_comment(_T("windows minor"));
	mov(no_op_ptr(__WINMINOR), EAX);

	mov(ECX, no_op_ptr(__OSVER));
	and(ECX, 0x0FF);
	mov.set_new_comment(_T("windows major"));
	mov(no_op_ptr(__WINMAJOR), ECX);

	mov(EDX, dword_ptr(__WINMAJOR));
	shl(EDX, 8);
	add(EDX, dword_ptr(__WINMINOR));
	mov.set_new_comment(_T("windows version"));
	mov(dword_ptr(__WINVER), EDX);

	mov(EAX, no_op_ptr(__OSVER));
	shr(EAX, 0x10);
	and(EAX, 0xFFFF);
	mov(no_op_ptr(__OSVER), EAX);

//2-------------------------------------------------------------------------------
/*
		;arg values : param 2
0040F867   mov         eax,[___argv (00484018)]
0040F86C   push        eax

		;arg count : param 1
0040F86D   mov         ecx,dword ptr [___argc (00484014)]
0040F873   push        ecx

		;a call to the user main function (cdecl calling convention)
0040F874   call        @ILT+880(_main) (00401375)
0040F879   add         esp,0Ch

		;the result value is put in a local variable		
0040F87C   mov         dword ptr [mainret],eax
0040F87F   mov         edx,dword ptr [mainret]
		;the exit result
0040F882   push        edx
0040F883   call        exit (0040f600)		;terminate the process
*/
//2-------------------------------------------------------------------------------
	DeclDefinition* decl_ptr = &entry_point_ptr->getDeclDefinition();
	
	if(decl_ptr->function.locals.stack_params.params.size()==2)
	{
		//lea(EBX, dword_ptr(EBP, __minus, sizeof(int)));
		lea(EBX, dword_ptr(___ARGC));
		push(EBX);
		call(no_op_ptr(SYSTEM_COMMAND_LINE_ARGV));
		mov(dword_ptr(___ARGV), EAX);
		//pass typical params via the stack...
		//mov(EAX, VAR_OFFSET, ___ARGV);
		push(EAX);
		mov(ECX, dword_ptr(___ARGC));
		push(ECX);
	}	
	call.set_new_comment(_T("user entry point (stdcall calling convention)"));
	call(no_op_ptr(main_entry));
	push(EAX);

	if(decl_ptr->function.locals.stack_params.params.size()==2)
	{
		mov(EAX, dword_ptr(___ARGV));
		push(EAX);
		call(no_op_ptr(SYSTEM_MEMORY_PROCESSHEAP));
		push(EAX);
		call(no_op_ptr(SYSTEM_MEMORY_FREE));
	}
	//use the first param in stack when calling exit...
	call(no_op_ptr(SYSTEM_EXIT));
	//
	EmitFunctionEpilog();
	unit() << SYSTEM_HPP_RUNTIME_INIT << space << _T("ENDP") 
		   << endl
		   << endl;
}

void HCCCodeGenerator::emitProgramGlobalsInit()
{	
	//Ok! I must admit that I was tempted to add global variable init like in C or Pascal, but H++ is an OOP language
	//so I rejected the idea completely.
	/*
	//this icode is the one from where to make the expression calls...
	icode_ptr = &g_icode_gen;
	//reset the cursor pointer from the icode object...
	icode_ptr->reset();
	//ask for the first token
	getToken();
	*/
	//
	unit() << USER_GLOBALS_INIT << space << _T("PROC") << endl;
	EmitFunctionProlog();
	//
	//HCC_TOKEN_TYPE stmt = EmitStatementList(HCC_EOF, NULL);
	//
	EmitFunctionEpilog();
	unit() << USER_GLOBALS_INIT << space << _T("ENDP") 
		   << endl
		   << endl;
}

void HCCCodeGenerator::EmitInitObjectDataMembersFunction(TypeSpecifier *class_type_ptr)
{
	assert(class_type_ptr!=NULL);
	if(class_type_ptr==NULL)
		return;

	CPUInstruction mov(this, MOV);
	__tstring member_init_func = __MEMBER_INIT;
	member_init_func += getLabelFromName(class_type_ptr->getTypeName());

	//get the icode for every class type...
	icode_ptr = theICodeResponsible.getClassICode(class_type_ptr, false);
	//if this class has no member initialization, icode_ptr==NULL
	if(icode_ptr!=NULL)
	{
		unit() << member_init_func << space << _T("PROC NEAR") << endl;
		//we must save the this pointer in a local variable, before executing an expression
		EmitFunctionProlog(sizeof(int));
		//
		assert(icode_ptr!=NULL);
		//reset the cursor pointer from the icode object...
		icode_ptr->reset();
		//ask for the first token
		getToken();
		//we save the this pointer before executing any kind of expressions...
		mov(dword_ptr(EBP, __minus, sizeof(int)), ECX);
		HCC_TOKEN_TYPE stmt = EmitStatementList(HCC_EOF, DATA_MEMBERS_INIT_PROC_ADDRESS);
		//
		//restore to the global icode object...	
		icode_ptr = &g_icode_gen;
		assert(icode_ptr!=NULL);
		//
		EmitFunctionEpilog();
		unit() << member_init_func << space << _T("ENDP") << endl;
	}
}

void HCCCodeGenerator::EmitObjectDataMembersInitForClasses()
{

	//Emit all literal constants as global variables...	
	SYMBOL_TABLE::iterator it_symbl = symbol_table_ptr->begin();	
	while(it_symbl != symbol_table_ptr->end())
	{
		Symbol* symbol_ptr = (*it_symbl).second;

		if(symbol_ptr->getTypeSpecifier().specifier()==DSPEC_CLASS)
		{
			if(symbol_ptr->getTypeSpecifier().user_type_members.data_members.size() > 0){
				EmitInitObjectDataMembersFunction(&symbol_ptr->getTypeSpecifier());
			}
		}

		it_symbl++;
	}
}

void HCCCodeGenerator::setHppSourceFile(const char* hpp_file_string)
{
#ifdef _UNICODE 
	TCHAR lpszHppFile[MAX_PATH * 4];
	mbstowcs(lpszHppFile, hpp_file_string, MAX_PATH);
#else
	hpp_source_file = hpp_file_string;
#endif
	//hpp_source_file = (const TCHAR*)_bstr_t(hpp_file_string);
}

void HCCCodeGenerator::emitJumpStatement(Symbol *function_ptr)
{
	CPUInstruction jmp(this, JMP);
	CPUInstruction leave(this, LEAVE);
	CPUInstruction ret(this, RET);
	CPUInstruction pop(this, POP);

	__tstring __label_in_stmt;
	switch(token_type)
	{
	case HCC_BREAK:
		{
			if(!stack_of_stmts.empty())
			{
				const HPP_STATEMENT_INFO& __stmt = stack_of_stmts.top();				

				if(__stmt.type==HCC_WHILE	|| 
					__stmt.type==HCC_FOR	|| 
					__stmt.type==HCC_DO)
				{
					__label_in_stmt = CreateLabel(STMT_LABEL_PREFIX, __stmt.stmt_end_block);

					jmp(VAR_OFFSET, __label_in_stmt);
					getToken(); //skip this token
				}
			}else{
				getToken();
			}
		}
		break;
	case HCC_CONTINUE:
		{
			if(!stack_of_stmts.empty())
			{
				const HPP_STATEMENT_INFO& __stmt = stack_of_stmts.top();				

				if(__stmt.type==HCC_WHILE	|| 
					__stmt.type==HCC_FOR	||
					__stmt.type==HCC_DO)
				{
					__label_in_stmt = CreateLabel(STMT_LABEL_PREFIX, __stmt.stmt_start_block);

					jmp(VAR_OFFSET, __label_in_stmt);
					getToken(); //skip this token
				}
			}else{
				getToken();
			}
		}
		break;
	case HCC_GOTO:
		{
			getToken();
			if(symbol_ptr!=NULL)
			{
				jmp(VAR_OFFSET, symbol_ptr->getName());
			}
			getToken(); //skip this identifier
		}
		break;
	case HCC_RETURN:
		{		
			getToken(); //skip the keyword 'return' ; next, process the expression...
			if(is_operand_in_stack)
			{
				EmitPopOperand(stack_operand_type);
				is_operand_in_stack = false;
			}
			//BEGIN - FIXED Dec 28, 2008
			TypeSpecifier* expr_type_ptr = EmitExprList(function_ptr);
			TypeSpecifier* func_type_ptr = function_ptr->getDeclDefinition().function.return_type;
			
			if(func_type_ptr!=HccTypeChecker::ts_type_void_ptr && 
			   expr_type_ptr->getDataType()!=func_type_ptr->getDataType())
			{
				if(func_type_ptr->getDataType()==HCC_FLOATING_POINT && 
					expr_type_ptr->getDataType()==HCC_INTEGER)
				{
					if(!is_operand_in_stack)
						EmitPushOperand(expr_type_ptr);
					//
					EmitPromoteToFloatingPoint(expr_type_ptr, false);
				}else if(function_ptr->getDeclDefinition().function.bReturnTypeIsPointer &&
						 expr_type_ptr->getDataType()==HCC_INTEGER)
				{
					//allow this return for pointer types
				}else{
					//TODO:
					__asm int 3;	
				}
				is_operand_in_stack = false;
			}

			if(is_operand_in_stack)
			{
				EmitPopOperand(stack_operand_type);
				is_operand_in_stack = false;
			}
			//END - FIXED Dec 28, 2008
			jmp(VAR_OFFSET, CreateLabel(STMT_LABEL_PREFIX, function_ptr->getLabelIndex()));
			/*
			int params_size = function_ptr->getDeclDefinition().function.locals.stack_params.total_params_size;
			
			pop(EDI);
			pop(ESI);
			leave();
			if(params_size>0)
				ret(params_size);
			else
				ret();
			*/
		}
		break;
	default:
		__asm int 3;
		break;
	}
}

void HCCCodeGenerator::EmitPopOperand(TypeSpecifier *_operand_type)
{
	CPUInstruction pop(this, POP);

	if(_operand_type==HccTypeChecker::ts_type_Int64_ptr){
		pop(EAX);
		pop(EDX);
	}else if(_operand_type==HccTypeChecker::ts_type_double_ptr || 
			_operand_type==HccTypeChecker::ts_type_float_ptr)
	{
		//result : EDX:EAX
		pop(EDX);
		pop(EAX);
	}else if(_operand_type->getDataTypeSize() > 0)
		//result : EAX
		pop(EAX);
	//
	is_operand_in_stack = false;
}

void HCCCodeGenerator::EmitClassConstructor(Symbol *constructor_ptr, Symbol* onwer_class_ptr)
{
	assert(constructor_ptr!=NULL);
	if(constructor_ptr==NULL)
		return;

	CPUInstruction mov(this, MOV);
	CPUInstruction call(this, CALL);

	__tstring label = getLabelFromName(constructor_ptr->getCompleteName());

	unit() << label << space << _T("PROC NEAR") << space;
	//
	if(bSourceAnnotation)
		EmitFunctionComment(constructor_ptr);

	int stack_size = constructor_ptr->getDeclDefinition().function.locals.stack_variables.total_variables_size;
	/*
	int modulus = stack_size % sizeof(int);
	if(stack_size > 0 && modulus > 0)
		stack_size += sizeof(int) - modulus;
	*/
	//we leave space for the this pointer's variable holder...
	stack_size += sizeof(int);
	EmitFunctionProlog(stack_size);
	//we changed the stack size, so, we must keep this data saved...
	constructor_ptr->getDeclDefinition().function.locals.stack_variables.total_variables_size = stack_size;
	//change to the function's icode object...
	icode_ptr = theICodeResponsible.getFunctionICode(constructor_ptr, false);
	if(icode_ptr!=NULL)
	{
		//set the icode position to the beginning of the code (critical!!!)... 
		icode_ptr->reset();
		//get the first token in the intermediate code...
		getToken();
	}
	//save the this pointer (at the last variable) before executing any kind of expression...
	if(bOptimizePrologAndEpilog || stack_size > sizeof(int))
	{
		//for object member functions only (if optimized with 'enter', or this function has stack variables)
		mov(dword_ptr(EBP, __minus, stack_size), ECX);
	}

	//add code for pattern 'CC' initialization of variables...
	if(bPatternInitStackVariables)
	{
		int stack_init_size_bytes = stack_size - sizeof(int);
		if(stack_init_size_bytes > 0)
		{
			CPUInstruction lea(this, LEA);
			CPUInstruction rep_stos(this, REP_STOS);

			lea(EDI, dword_ptr(EBP, __minus, stack_init_size_bytes));
			mov(ECX, (int)(stack_init_size_bytes/4));
			mov(EAX, (int)0xCCCCCCCC);
			rep_stos(dword_ptr(EDI));			

		}		
	}

	//call the base constructor, if this is a derived class...
	assert(onwer_class_ptr!=NULL);
	if(onwer_class_ptr!=NULL)
	{
		//BEGIN - V I R T U A L   T A B L E   S E T U P 
		//B A S E   C O N S T R U C T O R   C A L L 
		TypeSpecifier* baseTypeSpec = onwer_class_ptr->getTypeSpecifier().getBaseTypeSpec();
		TypeSpecifier* thisTypeSpec = &onwer_class_ptr->getTypeSpecifier();
		if(thisTypeSpec!=baseTypeSpec)
		{
			//base type's symbol table...
			LPSYMBOL_TABLE symbl_tbl_ptr = baseTypeSpec->getSymbolTable();
			assert(symbl_tbl_ptr!=NULL);
			if(symbl_tbl_ptr!=NULL)
			{
				Symbol* base_ctor_ptr = symbl_tbl_ptr->find(baseTypeSpec->getTypeName());
				//emit the call, if and only if, this is a constructor with no arguments;
				//else the user must do the call...
				if(base_ctor_ptr!=NULL)
				{
					if(base_ctor_ptr->getDeclDefinition().function.locals.stack_params.total_params_size==0)
					{
						//for object member functions only
						mov(ECX, dword_ptr(EBP, __minus, stack_size));
						call.set_new_comment(_T("base constructor call"));
						call(no_op_ptr(getLabelFromName(base_ctor_ptr->getCompleteName())));
					}
				}else if(baseTypeSpec!=NULL && IsTypeQualifiedForDefaultConstructor(*baseTypeSpec))
				{
						//for object member functions only
						mov(ECX, dword_ptr(EBP, __minus, stack_size));
						call.set_new_comment(_T("base constructor call"));
						call(no_op_ptr(getLabelFromName(baseTypeSpec->getTypeName())));
				}

			}
		}
		//The only declaration for object instantiation actually, is a call to a constructor...		
		const __tstring& lpszType = thisTypeSpec->getTypeName();
		assert(lpszType.length() > 0);
		//set vtbl ptr		
		//V I R T U A L   T A B L E   C O N S T R U C T I O N
		if(lpszType.length() > 0)
		{			
			__tstring vtbl_ptr_symbol_name = CLASS_VPTR_VTBL_NAME;
			Symbol* vtbl_symbol_ptr = thisTypeSpec->getSymbolTable()->find(vtbl_ptr_symbol_name);
			//if this object instance is from a virtual class, then assign the class_vtable_ptr...
			if(vtbl_symbol_ptr!=NULL)
			{
				//the vptr...
				int vptr_offset = vtbl_symbol_ptr->getDeclDefinition().user_data.offset;
				assert(vtbl_symbol_ptr->getDeclDefinition().identifier_type()==DECL_VTBL_PTR);

				__tstring class_vptr_name = CLASS_VTBL_VPTR_PREFIX;
				class_vptr_name += getLabelFromName(lpszType);
				//set the vptr...				
				//for object member functions only
				mov(ECX, dword_ptr(EBP, __minus, stack_size));
				if(vptr_offset > 0)
					mov(dword_ptr(ECX, __plus, vptr_offset), VAR_OFFSET, class_vptr_name);
				else
					mov(dword_ptr(ECX), VAR_OFFSET, class_vptr_name);
			}
		}
		//END - V I R T U A L   T A B L E   S E T U P 
		//the 'this' pointer must be in register ECX...
		//initialize all data members before calling the constructor if any...
		if(thisTypeSpec->user_type_members.data_members.size() > 0)
		{
			//get the icode for every class type...
			icode_generator* class_icode_ptr = theICodeResponsible.getClassICode(thisTypeSpec, false);

			if(class_icode_ptr!=NULL)
			{
				__tstring member_init_func = __MEMBER_INIT;
				member_init_func += getLabelFromName(thisTypeSpec->getTypeName());		
				//for object member functions only
				mov(ECX, dword_ptr(EBP, __minus, stack_size));
				call(no_op_ptr(member_init_func));		
			}
		}
	}

	if(icode_ptr!=NULL)
	{
		//function body (statements and expressions)
		EmitFunctionBody(constructor_ptr);
	}
	//
	int params_size = constructor_ptr->getDeclDefinition().function.locals.stack_params.total_params_size;
	/*
	modulus = params_size % sizeof(int);
	if(params_size > 0 && modulus > 0)
		params_size += sizeof(int) - modulus;
	*/
	//call destructor for all variable object instances...
	//in reverse order...
	vector<Symbol*>::reverse_iterator it_obj = obj_instance_in_scope.rbegin();
	while(it_obj != obj_instance_in_scope.rend())
		//
		EmitDestructorCall(*it_obj++, constructor_ptr);

	//clear the array list...
	obj_instance_in_scope.clear();
	//
	if(is_operand_in_stack)
	{
		EmitPopOperand(stack_operand_type);
		is_operand_in_stack = false;
	}
	EmitFunctionEpilog(params_size);
	unit() << label << space << _T("ENDP") 
		   << endl 
		   << endl;

}

Symbol* HCCCodeGenerator::CreateDefaultClassConstructor(Symbol *class_ptr)
{
	assert(class_ptr!=NULL);
	if(class_ptr==NULL)
		return NULL;

	//parse the constructor...
	Symbol* ctor_ptr = class_ptr->getTypeSpecifier().getSymbolTable()->insert(class_ptr->getName());
	assert(ctor_ptr!=NULL);
	if(ctor_ptr!=NULL)
	{
		ctor_ptr->getDeclDefinition().set_identifier_type(DECL_NEW_CLASS_CONSTRUCTOR);

		DeclDefinition* ctor_decl_ptr = &ctor_ptr->getDeclDefinition();
		//the return type...
		ctor_decl_ptr->function.return_type = HccTypeChecker::ts_type_void_ptr;

		assert(ctor_decl_ptr->function.symbol_table_ptr==0);
		//the symbol table is not necessary this time, because, there are not statements
		//in an empty default constructor...
		//ctor_decl_ptr->function.symbol_table_ptr = new SymbolTable<__tstring, Symbol>(_T("__constructor__"));
	}
	return ctor_ptr;
}

/*
	id += expr;

	  |  _____
	-----
	  |	 -----
*/
TypeSpecifier* HCCCodeGenerator::EmitPlusAssignExpr(Symbol *target_ptr, 
												   TypeSpecifier* source_type, 
												   HCC_TOKEN_TYPE _operator, 
												   Symbol *function_ptr, 
												   bool bIsAddressOnStack,
												   TypeSpecifier* array_item_type)
{
	assert(_operator==HCC_INC_ASSIGN);

	CPUInstruction push(this, PUSH);
	CPUInstruction pop(this, POP);

	CPUInstruction add(this, ADD);	
	CPUInstruction adc(this, ADC);	

	CPUInstruction clc(this, CLC);
	
	CPUInstruction cdq(this, CDQ);

	FPUInstruction fld(this, FLD);
	FPUInstruction fild(this, FILD);	
	FPUInstruction fadd(this, FADD);
	FPUInstruction fstp(this, FSTP);
	

	clc(); //clear the carry flag


	TypeSpecifier* target_type = array_item_type,
				*result_type_ptr = target_type;
	
	if(target_type==NULL){
		result_type_ptr = target_type = target_ptr->getTypeSpecifier().getBaseTypeSpec();
	}
	//if a previous expression was generated, there may be the 'LAST-X' in the stack
	//for all types different to double/float...
	if(is_operand_in_stack && target_type->getDataType()!=HCC_FLOATING_POINT)
	{
		EmitPopOperand(stack_operand_type);
		is_operand_in_stack = false;
	}
	
	//F L O A T I N G   P O I N T   A S S I G N M E N T 
	if(target_type==HccTypeChecker::ts_type_double_ptr || 
		target_type==HccTypeChecker::ts_type_float_ptr)
	{
		//for integer types, do conversion to floating point type...
		if(source_type==HccTypeChecker::ts_type_Int64_ptr)
		{
			if(!is_operand_in_stack)
			{
				//EDX:EAX
				push(EAX);
				push(EDX);
			}
			//integer value loaded in FPU...
			fild(qword_ptr(ESP));
			is_operand_in_stack = true;
		}else if(source_type!=HccTypeChecker::ts_type_double_ptr &&
				 source_type!=HccTypeChecker::ts_type_float_ptr)
		{
			emitPushInteger(source_type);
			/*
			if(!is_operand_in_stack)
			{
				push(EAX);
			}
			//integer value loaded in FPU...
			fild(dword_ptr(ESP));
			is_operand_in_stack = true;
			*/
		}else{
			if(!is_floating_point_in_fpu)
			{
				if(!is_operand_in_stack)
				{
					//EDX:EAX
					push(EAX);
					push(EDX);
				}
				fld(qword_ptr(ESP));
				is_operand_in_stack = true;
			}			
			is_floating_point_in_fpu = false;
		}
		//now for operator HCC_INC_ASSIGN ---> '+='

		//if a previous expression was generated, there may be the 'LAST-X' in the stack
		if(is_operand_in_stack)
		{
			EmitPopOperand(source_type);
			is_operand_in_stack = false;
		}
		
		//EDX:EAX
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			fadd(qword_ptr(EBX)); //st = st + m64
			fstp(qword_ptr(EBX)); //m64 = st
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);
			fadd(qword_ptr(EBX)); //st = st + m64
			fstp(qword_ptr(EBX)); //m64 = st
		}else
		{								
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;				

				fadd(qword_ptr(EBP, op, abs(offset))); //st = st + m64
				fstp(qword_ptr(EBP, op, abs(offset))); //m64 = st
			}else{
				fadd(qword_ptr(getSymbolLabel(target_ptr))); //st = st + m64
				fstp(qword_ptr(getSymbolLabel(target_ptr))); //m64 = st
			}
		}
	}else if(target_type==HccTypeChecker::ts_type_Int64_ptr)
	{
		if(source_type->getDataTypeSize() <= HccTypeChecker::ts_type_Int32_ptr->getDataTypeSize())
		{
			//promote to Int64...
			cdq();
		}
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			add(dword_ptr(EBX), EAX);	//low
			adc(dword_ptr(EBX, sizeof(int)), EDX);	//high
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);			
			add(dword_ptr(EBX), EAX);	//low
			adc(dword_ptr(EBX, sizeof(int)), EDX);	//high
		}else{
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;								
				add(dword_ptr(EBP, op, (int)abs((int)offset)), EAX);	//low
				adc(dword_ptr(EBP, op, (int)abs((int)offset + (int)sizeof(int))), EDX);	//high
			}else{
				add(no_op_ptr(getSymbolLabel(target_ptr)), EAX); //low
				adc(no_op_ptr(getSymbolLabel(target_ptr) + QWORD_OFFSET_HIGH), EDX); //high
			}
		}
	}else if(target_type==HccTypeChecker::ts_type_char_ptr ||
			target_type==HccTypeChecker::ts_type_boolean_ptr)
	{
		//EAX
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			/*
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			add(byte_ptr(EBX), AL);
			*/
			EmitAddressFromAddressOf(id_ptr, function_ptr, true, target_type, ADD, AL); //result in EBX
			//END - FIXED Dec 23, 2008
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);
			add(byte_ptr(EBX), AL);
		}else{			
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;

				add(byte_ptr(EBP, op, abs(offset)), AL);
			}else
				add(byte_ptr(getSymbolLabel(target_ptr)), AL);
		}
	}else if(target_type==HccTypeChecker::ts_type_Int16_ptr ||
		target_type==HccTypeChecker::ts_type_short_ptr)
	{
		//AX
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			/*
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			add(word_ptr(EBX), AX);
			*/
			EmitAddressFromAddressOf(id_ptr, function_ptr, true, target_type, ADD, AX); //result in EBX
			//END - FIXED Dec 23, 2008
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);
			add(word_ptr(EBX), AX);			
		}else{								
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;

				add(word_ptr(EBP, op, abs(offset)), AX);
			}else
				add(word_ptr(getSymbolLabel(target_ptr)), AX);
		}
	}else if(target_type->is_scalar())
	{		
		//F O R   A L L   O T H E R   I N T E G E R S 
		//EAX
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			/*
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			add(dword_ptr(EBX), EAX);
			*/
			EmitAddressFromAddressOf(id_ptr, function_ptr, true, target_type, ADD, EAX); //result in EBX
			//END - FIXED Dec 23, 2008
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);
			add(dword_ptr(EBX), EAX);
		}else{								
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;				
				add(dword_ptr(EBP, op, abs(offset)), EAX);
			}else
				add(no_op_ptr(getSymbolLabel(target_ptr)), EAX);
		}
	}else{
		//TODO : we shouldn't allow this type of operator for custom types until implementation of overloaded operators
		assert(0);
		if(bIsAddressOnStack)
		{
			pop(EDI);
			//
		}

		/*
		CPUInstruction cld(this, CLD);							
		CPUInstruction rep_movsb(this, REP_MOVSB);


		int size = target_type->getDataTypeSize();
		//EAX is a pointer address
		mov(ESI, EAX);
		if(bIsAddressOnStack)
		{
			pop(EDI);
			//
		}else{
			//								
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;
				lea(EDI, dword_ptr(EBP, op, abs(offset)));
			}else
				lea(EDI, no_op_ptr(getSymbolLabel(target_ptr)));
		}
		mov(ECX, size);
		cld();
		rep_movsb();
		*/
	}
	return result_type_ptr;
}


/*
	id -= expr;

	     _____
	-----
		 -----
*/
TypeSpecifier* HCCCodeGenerator::EmitMinusAssignExpr(Symbol *target_ptr, TypeSpecifier* source_type, HCC_TOKEN_TYPE _operator, Symbol* function_ptr, bool bIsAddressOnStack, TypeSpecifier* array_item_type)
{
	assert(_operator==HCC_DEC_ASSIGN);

	CPUInstruction push(this, PUSH);
	CPUInstruction pop(this, POP);

	CPUInstruction sub(this, SUB);
	CPUInstruction sbb(this, SBB);

	CPUInstruction clc(this, CLC);
	
	CPUInstruction cdq(this, CDQ);

	FPUInstruction fld(this, FLD);
	FPUInstruction fild(this, FILD);	
	FPUInstruction fsubr(this, FSUBR);
	FPUInstruction fstp(this, FSTP);	

	clc(); //clear the carry flag...


	TypeSpecifier* target_type = array_item_type,
				*result_type_ptr = target_type;
	
	if(target_type==NULL){
		result_type_ptr = target_type = target_ptr->getTypeSpecifier().getBaseTypeSpec();
	}
	//if a previous expression was generated, there may be the 'LAST-X' in the stack
	//for all types different to double/float...
	if(is_operand_in_stack && target_type->getDataType()!=HCC_FLOATING_POINT)
	{
		EmitPopOperand(stack_operand_type);
		is_operand_in_stack = false;
	}
	
	//F L O A T I N G   P O I N T   A S S I G N M E N T 
	if(target_type==HccTypeChecker::ts_type_double_ptr || 
		target_type==HccTypeChecker::ts_type_float_ptr)
	{
		//for integer types, do conversion to floating point type...
		if(source_type==HccTypeChecker::ts_type_Int64_ptr)
		{
			if(!is_operand_in_stack)
			{
				//EDX:EAX
				push(EAX);
				push(EDX);
			}
			//integer value loaded in FPU...
			fild(qword_ptr(ESP));
			is_operand_in_stack = true;
		}else if(source_type!=HccTypeChecker::ts_type_double_ptr &&
				 source_type!=HccTypeChecker::ts_type_float_ptr)
		{
			emitPushInteger(source_type);
			/*
			if(!is_operand_in_stack)
			{
				push(EAX);
			}
			//integer value loaded in FPU...
			fild(dword_ptr(ESP));
			is_operand_in_stack = true;
			*/
		}else{
			if(!is_floating_point_in_fpu)
			{
				if(!is_operand_in_stack)
				{
					//EDX:EAX
					push(EAX);
					push(EDX);
				}
				fld(qword_ptr(ESP));
				is_operand_in_stack = true;
			}
			is_floating_point_in_fpu = false;
		}
		//now for operator HCC_DEC_ASSIGN ---> '-='

		//if a previous expression was generated, there may be the 'LAST-X' in the stack
		if(is_operand_in_stack)
		{
			EmitPopOperand(source_type);
			is_operand_in_stack = false;
		}
		
		//EDX:EAX
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			fsubr(qword_ptr(EBX)); //st = m64 - st
			fstp(qword_ptr(EBX)); //m64 = st
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);
			fsubr(qword_ptr(EBX)); //st = m64 - st
			fstp(qword_ptr(EBX)); //m64 = st
		}else
		{								
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;				

				fsubr(qword_ptr(EBP, op, abs(offset))); //st = st + m64
				fstp(qword_ptr(EBP, op, abs(offset))); //m64 = st
			}else{
				fsubr(qword_ptr(getSymbolLabel(target_ptr))); //st = st + m64
				fstp(qword_ptr(getSymbolLabel(target_ptr))); //m64 = st
			}
		}
	}else if(target_type==HccTypeChecker::ts_type_Int64_ptr)
	{
		if(source_type->getDataTypeSize() <= HccTypeChecker::ts_type_Int32_ptr->getDataTypeSize())
		{
			//promote to Int64...
			cdq();
		}

		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			sub(dword_ptr(EBX), EAX);	//low
			sbb(dword_ptr(EBX, sizeof(int)), EDX);	//high
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);			
			sub(dword_ptr(EBX), EAX);	//low
			sbb(dword_ptr(EBX, sizeof(int)), EDX);	//high
		}else{
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;								
				sub(dword_ptr(EBP, op, (int)abs((int)offset)), EAX);	//low
				sbb(dword_ptr(EBP, op, (int)abs((int)offset + (int)sizeof(int))), EDX);	//high
			}else{
				sub(no_op_ptr(getSymbolLabel(target_ptr)), EAX); //low
				sbb(no_op_ptr(getSymbolLabel(target_ptr) + QWORD_OFFSET_HIGH), EDX); //high
			}
		}
	}else if(target_type==HccTypeChecker::ts_type_char_ptr ||
			target_type==HccTypeChecker::ts_type_boolean_ptr)
	{
		//EAX
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			/*
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			sub(byte_ptr(EBX), AL);
			*/
			EmitAddressFromAddressOf(id_ptr, function_ptr, true, target_type, SUB, AL); //result in EBX
			//END - FIXED Dec 23, 2008
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);
			sub(byte_ptr(EBX), AL);
		}else{			
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;

				sub(byte_ptr(EBP, op, abs(offset)), AL);
			}else
				sub(byte_ptr(getSymbolLabel(target_ptr)), AL);
		}
	}else if(target_type==HccTypeChecker::ts_type_Int16_ptr ||
		target_type==HccTypeChecker::ts_type_short_ptr)
	{
		//AX
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			/*
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			sub(word_ptr(EBX), AX);
			*/
			EmitAddressFromAddressOf(id_ptr, function_ptr, true, target_type, SUB, AX); //result in EBX
			//END - FIXED Dec 23, 2008
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);
			sub(word_ptr(EBX), AX);
		}else{								
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;

				sub(word_ptr(EBP, op, abs(offset)), AX);
			}else
				sub(word_ptr(getSymbolLabel(target_ptr)), AX);
		}
	}else if(target_type->is_scalar())
	{		
		//F O R   A L L   O T H E R   I N T E G E R S 
		//EAX
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			//BEGIN - FIXED Dec 23, 2008
			/*
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			sub(dword_ptr(EBX), EAX);
			*/
			EmitAddressFromAddressOf(id_ptr, function_ptr, true, target_type, SUB, EAX); //result in EBX
			//END - FIXED Dec 23, 2008
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);
			sub(dword_ptr(EBX), EAX);
		}else{								
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;				
				sub(dword_ptr(EBP, op, abs(offset)), EAX);
			}else
				sub(no_op_ptr(getSymbolLabel(target_ptr)), EAX);
		}
	}else{
		//TODO : we shouldn't allow this type of operator for custom types until implementation of overloaded operators
		assert(0);
		if(bIsAddressOnStack)
		{
			pop(EDI);
			//
		}

		/*
		CPUInstruction cld(this, CLD);							
		CPUInstruction rep_movsb(this, REP_MOVSB);


		int size = target_type->getDataTypeSize();
		//EAX is a pointer address
		mov(ESI, EAX);
		if(bIsAddressOnStack)
		{
			pop(EDI);
			//
		}else{
			//								
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;
				lea(EDI, dword_ptr(EBP, op, abs(offset)));
			}else
				lea(EDI, no_op_ptr(getSymbolLabel(target_ptr)));
		}
		mov(ECX, size);
		cld();
		rep_movsb();
		*/
	}
	return result_type_ptr;
}




TypeSpecifier* HCCCodeGenerator::EmitMultAssignExpr(Symbol *target_ptr, TypeSpecifier *source_type, HCC_TOKEN_TYPE _operator, Symbol *function_ptr, bool bIsAddressOnStack, TypeSpecifier* array_item_type)
{
	assert(_operator==HCC_MUL_ASSIGN);

	CPUInstruction push(this, PUSH);
	CPUInstruction pop(this, POP);
	CPUInstruction mov(this, MOV);

	CPUInstruction imul(this, IMUL);	//for now, let's use the signed version of multiplication to preserve the sign
	CPUInstruction shl(this, SHL);
	CPUInstruction or(this, OR);
	
	CPUInstruction cdq(this, CDQ);

	CPUInstruction xchg(this, XCHG);

	FPUInstruction fld(this, FLD);
	FPUInstruction fild(this, FILD);	
	FPUInstruction fmul(this, FMUL);
	FPUInstruction fstp(this, FSTP);	
	FPUInstruction fistp(this, FISTP);	


	TypeSpecifier* target_type = array_item_type,
				*result_type_ptr = target_type;
	
	if(target_type==NULL){
		result_type_ptr = target_type = target_ptr->getTypeSpecifier().getBaseTypeSpec();
	}
	//if a previous expression was generated, there may be the 'LAST-X' in the stack
	//for all types different to double/float...
	if(is_operand_in_stack && target_type->getDataType()!=HCC_FLOATING_POINT)
	{
		EmitPopOperand(stack_operand_type);
		is_operand_in_stack = false;
	}
	
	//F L O A T I N G   P O I N T   A S S I G N M E N T 
	if(target_type==HccTypeChecker::ts_type_double_ptr || 
		target_type==HccTypeChecker::ts_type_float_ptr)
	{
		//for integer types, do conversion to floating point type...
		if(source_type==HccTypeChecker::ts_type_Int64_ptr)
		{
			if(!is_operand_in_stack)
			{
				//EDX:EAX
				push(EAX);
				push(EDX);
			}
			//integer value loaded in FPU...
			fild(qword_ptr(ESP));
			is_operand_in_stack = true;
		}else if(source_type!=HccTypeChecker::ts_type_double_ptr &&
				 source_type!=HccTypeChecker::ts_type_float_ptr)
		{
			emitPushInteger(source_type);
			/*
			if(!is_operand_in_stack)
			{
				push(EAX);
			}
			//integer value loaded in FPU...
			fild(dword_ptr(ESP));
			is_operand_in_stack = true;
			*/
		}else{
			if(!is_floating_point_in_fpu)
			{
				if(!is_operand_in_stack)
				{
					//EDX:EAX
					push(EAX);
					push(EDX);
				}
				fld(qword_ptr(ESP));
				is_operand_in_stack = true;
			}
			is_floating_point_in_fpu = false;
		}
		//now for operator HCC_MUL_ASSIGN ---> '+='

		//if a previous expression was generated, there may be the 'LAST-X' in the stack
		if(is_operand_in_stack)
		{
			EmitPopOperand(source_type);
			is_operand_in_stack = false;
		}
		
		//EDX:EAX
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			fmul(qword_ptr(EBX)); //st = st * m64
			fstp(qword_ptr(EBX)); //m64 = st
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);
			fmul(qword_ptr(EBX)); //st = st * m64
			fstp(qword_ptr(EBX)); //m64 = st
		}else
		{								
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;				

				fmul(qword_ptr(EBP, op, abs(offset))); //st = st * m64
				fstp(qword_ptr(EBP, op, abs(offset))); //m64 = st
			}else{
				fmul(qword_ptr(getSymbolLabel(target_ptr))); //st = st * m64
				fstp(qword_ptr(getSymbolLabel(target_ptr))); //m64 = st
			}
		}
	}else if(target_type==HccTypeChecker::ts_type_Int64_ptr)
	{
		if(source_type->getDataTypeSize() <= HccTypeChecker::ts_type_Int32_ptr->getDataTypeSize())
		{
			//promote to Int64...
			cdq();
		}
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			goto __EMIT_MULT_TYPE_INT64;
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);
			
__EMIT_MULT_TYPE_INT64:
			//the qword pointer is not supported over direct memory access; so I decided
			//to load it using the FPU...
			EmitPushOperand(target_type);
			fild(qword_ptr(ESP));
			fild(qword_ptr(EBX));
			fmul(st, st1);
			fistp(qword_ptr(ESP));
			EmitPopOperand(target_type);
			//imul(qword_ptr(EBX)); // EDX:EAX = m64 * EDX:EAX			
			mov(dword_ptr(EBX, sizeof(int)), EDX);	//high
			mov(dword_ptr(EBX), EAX);				//low
		}else{
			//TODO:
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;								
				imul(qword_ptr(EBP, op, abs(offset)));	// EDX:EAX = m64 * EDX:EAX				
				mov(dword_ptr(EBP, op, (int)abs((int)offset + (int)sizeof(int))), EDX);	//high
				mov(dword_ptr(EBP, op, (int)abs((int)offset)), EAX);					//low
			}else{
				imul(no_op_ptr(getSymbolLabel(target_ptr))); // EDX:EAX = m64 * EDX:EAX							
				__tstring HIGH_PART = __tstring(_T("[")) + 
									getSymbolLabel(target_ptr) + 
									__tstring(QWORD_OFFSET_LOW) + 
									__tstring(_T("]"));

				mov(no_op_ptr(HIGH_PART), EDX); //high
				mov(no_op_ptr(getSymbolLabel(target_ptr)), EAX); //low
			}
		}
	}else if(target_type==HccTypeChecker::ts_type_char_ptr ||
			target_type==HccTypeChecker::ts_type_boolean_ptr)
	{
		//EAX
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			goto __EMIT_MULT_TYPE_INT8;
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);

__EMIT_MULT_TYPE_INT8:
			imul(byte_ptr(EBX));	// AX = m8 * AL
			mov(word_ptr(EBX), AX);
		}else{			
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;

				imul(byte_ptr(EBP, op, abs(offset))); // AX = m8 * AL --> TODO: promote target type
				mov(word_ptr(EBP, op, abs(offset)), AX);
			}else{
				imul(byte_ptr(getSymbolLabel(target_ptr)));
				mov(word_ptr(getSymbolLabel(target_ptr)), AX);
			}
		}
	}else if(target_type==HccTypeChecker::ts_type_Int16_ptr ||
		target_type==HccTypeChecker::ts_type_short_ptr)
	{
		//AX
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			goto __EMIT_MULT_TYPE_INT16;
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);

__EMIT_MULT_TYPE_INT16:
			imul(word_ptr(EBX));	//DX:AX = m16 * AX --> TODO: promote target type
			shl(EDX, 0x10);
			or(EAX, EDX);
			mov(dword_ptr(EBX), EAX);
		}else{								
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;

				imul(word_ptr(EBP, op, abs(offset)));
				shl(EDX, 0x10);
				or(EAX, EDX);
				mov(dword_ptr(EBP, op, abs(offset)), EAX);
			}else{
				imul(word_ptr(getSymbolLabel(target_ptr)));
				shl(EDX, 0x10);
				or(EAX, EDX);
				mov(dword_ptr(getSymbolLabel(target_ptr)), EAX);
			}
		}
	}else if(target_type->is_scalar())
	{		
		//F O R   A L L   O T H E R   I N T E G E R S 
		//EAX
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			goto __EMIT_MULT_TYPE_INT32;
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);

__EMIT_MULT_TYPE_INT32:			
			/*
			imul(dword_ptr(EBX)); //EDX:EAX = m32 * EAX --> TODO: promote target type				
			cout << _T("Source = ") << source_type->getTypeName()
				 << _T(", Target = ") << target_ptr->getTypeSpecifier().getBaseTypeSpec()->getTypeName() << endl;
			*/
			//BEGIN - fix for mixing of Int64 and ints - March 13, 2011
			if(
				(source_type->getDataTypeSize() == 8 && source_type->getDataType()==HCC_INTEGER) &&
				(target_ptr->getTypeSpecifier().getBaseTypeSpec()!=NULL && 
					target_ptr->getTypeSpecifier().getBaseTypeSpec()->getDataTypeSize()==4)
					)
			{
					xchg(EAX, EDX);
					imul(dword_ptr(EBX)); //EDX:EAX = m32 * EAX --> TODO: promote target type
					mov.set_new_comment(_T("INT64 -> INT Assign!"));
					mov(dword_ptr(EBX), EAX);
			}else{
				imul(dword_ptr(EBX)); //EDX:EAX = m32 * EAX --> TODO: promote target type				
				mov(dword_ptr(EBX), EAX);
			}
			//END - fix for mixing of Int64 and ints - March 13, 2011
		}else{								
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;				
				imul(dword_ptr(EBP, op, abs(offset)));
				mov(dword_ptr(EBP, op, abs(offset)), EAX);
			}else{
				imul(no_op_ptr(getSymbolLabel(target_ptr)));
				mov(no_op_ptr(getSymbolLabel(target_ptr)), EAX);
			}
		}
	}else{
		//TODO : we shouldn't allow this type of operator for custom types until implementation of overloaded operators
		assert(0);
		if(bIsAddressOnStack)
		{
			pop(EDI);
			//
		}

		/*
		CPUInstruction cld(this, CLD);							
		CPUInstruction rep_movsb(this, REP_MOVSB);


		int size = target_type->getDataTypeSize();
		//EAX is a pointer address
		mov(ESI, EAX);
		if(bIsAddressOnStack)
		{
			pop(EDI);
			//
		}else{
			//								
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;
				lea(EDI, dword_ptr(EBP, op, abs(offset)));
			}else
				lea(EDI, no_op_ptr(getSymbolLabel(target_ptr)));
		}
		mov(ECX, size);
		cld();
		rep_movsb();
		*/
	}
	return result_type_ptr;
}

TypeSpecifier* HCCCodeGenerator::EmitDivAssignExpr(Symbol *target_ptr, TypeSpecifier *source_type, HCC_TOKEN_TYPE _operator, Symbol *function_ptr, bool bIsAddressOnStack, TypeSpecifier* array_item_type)
{
	assert(_operator==HCC_DIV_ASSIGN);

	CPUInstruction push(this, PUSH);
	CPUInstruction pop(this, POP);
	CPUInstruction mov(this, MOV);

	CPUInstruction idiv(this, IDIV);	//for now, let's use the signed version of division to preserve the sign
	
	CPUInstruction cdq(this, CDQ);	//DWORD-to-QWORD
	CPUInstruction cbw(this, CBW);	//BYTE-to-WORD
	CPUInstruction cwd(this, CWD);	//WORD-to-DWORD

	FPUInstruction fld(this, FLD);
	FPUInstruction fild(this, FILD);	
	FPUInstruction fdivr(this, FDIVR);
	FPUInstruction fstp(this, FSTP);	
	FPUInstruction fdiv(this, FDIV);
	FPUInstruction fistp(this, FISTP);
	FPUInstruction frndint(this, FRNDINT);

	TypeSpecifier* target_type = array_item_type,
				*result_type_ptr = target_type;
	
	if(target_type==NULL){
		result_type_ptr = target_type = target_ptr->getTypeSpecifier().getBaseTypeSpec();
	}
	//if a previous expression was generated, there may be the 'LAST-X' in the stack
	//for all types different to double/float...
	if(is_operand_in_stack && target_type->getDataType()!=HCC_FLOATING_POINT)
	{
		EmitPopOperand(stack_operand_type);
		is_operand_in_stack = false;
	}
	
	//F L O A T I N G   P O I N T   A S S I G N M E N T 
	if(target_type==HccTypeChecker::ts_type_double_ptr || 
		target_type==HccTypeChecker::ts_type_float_ptr)
	{
		//for integer types, do conversion to floating point type...
		if(source_type==HccTypeChecker::ts_type_Int64_ptr)
		{
			if(!is_operand_in_stack)
			{
				//EDX:EAX
				push(EAX);
				push(EDX);
			}
			//integer value loaded in FPU...
			fild(qword_ptr(ESP));
			is_operand_in_stack = true;
		}else if(source_type!=HccTypeChecker::ts_type_double_ptr &&
				 source_type!=HccTypeChecker::ts_type_float_ptr)
		{
			emitPushInteger(source_type);
			/*
			if(!is_operand_in_stack)
			{
				push(EAX);
			}
			//integer value loaded in FPU...
			fild(dword_ptr(ESP));
			is_operand_in_stack = true;
			*/
		}else{
			if(!is_floating_point_in_fpu)
			{
				if(!is_operand_in_stack)
				{
					//EDX:EAX
					push(EAX);
					push(EDX);
				}
				fld(qword_ptr(ESP));
				is_operand_in_stack = true;
			}			
			is_floating_point_in_fpu = false;
		}
		//now for operator HCC_MUL_ASSIGN ---> '+='

		//if a previous expression was generated, there may be the 'LAST-X' in the stack
		if(is_operand_in_stack)
		{
			EmitPopOperand(source_type);
			is_operand_in_stack = false;
		}
		
		//EDX:EAX
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			fdivr(qword_ptr(EBX)); //st = m64 / st
			fstp(qword_ptr(EBX)); //m64 = st
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);
			fdivr(qword_ptr(EBX)); //st = m64 / st
			fstp(qword_ptr(EBX)); //m64 = st
		}else
		{								
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;				

				fdivr(qword_ptr(EBP, op, abs(offset))); //st = m64 / st
				fstp(qword_ptr(EBP, op, abs(offset))); //m64 = st
			}else{
				fdivr(qword_ptr(getSymbolLabel(target_ptr))); //st = m64 / st
				fstp(qword_ptr(getSymbolLabel(target_ptr))); //m64 = st
			}
		}
	}else if(target_type==HccTypeChecker::ts_type_Int64_ptr)
	{
		if(source_type->getDataTypeSize() <= HccTypeChecker::ts_type_Int32_ptr->getDataTypeSize())
		{
			//promotes divisor to 64-bits...
			cdq();
		}
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			goto __EMIT_DIV_TYPE_INT64;
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);

__EMIT_DIV_TYPE_INT64:
			EmitPushOperand(target_type);
			fild(qword_ptr(ESP)); // EDX:EAX = QW[ESP]
			fild(qword_ptr(EBX));				//st(0) = QW[EBX]
			fdiv(st, st1);
			frndint();
			fistp(qword_ptr(ESP));  // EDX:EAX = QW[ESP]
			EmitPopOperand(target_type);
			//the result...			
			mov(dword_ptr(EBX, sizeof(int)), EDX);	//EAX = quotient ; EDX = remainder
			mov(dword_ptr(EBX), EAX);			
		}else{
			//TODO:
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;								

				/*
				push(EAX);
				push(EDX);
				*/
				EmitPushOperand(target_type);
				mov(EDX, dword_ptr(EBP, op, (int)abs((int)offset + (int)sizeof(int))));	//high
				mov(EAX, qword_ptr(EBP, op, (int)abs((int)offset)));					//low
				idiv(qword_ptr(ESP)); // EDX:EAX = EDX:EAX / [ESP]
				pop(ECX);
				pop(ECX);
				//the result...
				mov(dword_ptr(EBP, op, (int)abs((int)offset + (int)sizeof(int))), 0);	//EAX = quotient ; EDX = remainder
				mov(dword_ptr(EBP, op, (int)abs((int)offset)), EAX);
			}else{				
				__tstring HIGH_PART = __tstring(_T("[")) + 
									getSymbolLabel(target_ptr) + 
									__tstring(QWORD_OFFSET_LOW) + 
									__tstring(_T("]"));

				/*
				push(EAX);
				push(EDX);
				*/
				EmitPushOperand(target_type);
				mov(EDX, no_op_ptr(HIGH_PART));								//high
				mov(EAX, no_op_ptr(getSymbolLabel(target_ptr)));			//low
				idiv(qword_ptr(ESP)); // EDX:EAX = EDX:EAX / [ESP]
				pop(ECX);
				pop(ECX);
				//the result...
				mov(no_op_ptr(HIGH_PART), 0);								//EAX = quotient ; EDX = remainder
				mov(no_op_ptr(getSymbolLabel(target_ptr)), EAX);
			}
		}
	}else if(target_type==HccTypeChecker::ts_type_char_ptr ||
			target_type==HccTypeChecker::ts_type_boolean_ptr)
	{
		//AX / AL  :  AL = Quotient ; AH = Remainder
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			goto __EMIT_DIV_TYPE_INT8;
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);

__EMIT_DIV_TYPE_INT8:
			push(EAX);
			mov(AL, byte_ptr(EBX));	
			cbw(); //promotes the divident to 16 bits : AX = AH | AL
			idiv(byte_ptr(ESP));	// AL = AX / m8	; AH = remainder
			mov(byte_ptr(EBX), AL);				
			pop(ECX);
		}else{
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;

				push(EAX);
				mov(AL, byte_ptr(EBP, op, abs(offset)));	
				cbw(); //promotes the divident to 16 bits : AX = AH | AL
				idiv(byte_ptr(ESP));	// AL = AX / m8	; AH = remainder
				mov(byte_ptr(EBP, op, abs(offset)), AL);
				pop(ECX);
			}else{
				push(EAX);
				mov(AL, byte_ptr(getSymbolLabel(target_ptr)));	
				cbw(); //promotes the divident to 16 bits : AX = AH | AL
				idiv(byte_ptr(ESP));	// AL = AX / m8	; AH = remainder
				mov(byte_ptr(getSymbolLabel(target_ptr)), AL);
				pop(ECX);
			}
		}
	}else if(target_type==HccTypeChecker::ts_type_Int16_ptr ||
		target_type==HccTypeChecker::ts_type_short_ptr)
	{
		//DX:AX / AX  : AX = Quotient ; DX = Remainder
		//result = AX
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			goto __EMIT_DIV_TYPE_INT16;
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);

__EMIT_DIV_TYPE_INT16:
			push(EAX);
			mov(AX, word_ptr(EBX));	//
			cwd(); //promotes the dividend to 32 bits (DX:AX)
			idiv(word_ptr(ESP));		//AX = DX:AX / m16 ; DX = remainder
			mov(word_ptr(EBX), AX);
			pop(ECX);
		}else{								
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;

				push(EAX);
				mov(AX, word_ptr(EBP, op, abs(offset)));	//
				cwd(); //promotes the dividend to 32 bits (DX:AX)
				idiv(word_ptr(ESP));	//AX = DX:AX / m16 ; DX = remainder
				mov(word_ptr(EBP, op, abs(offset)), AX);
				pop(ECX);

			}else{
				push(EAX);
				mov(AX, word_ptr(getSymbolLabel(target_ptr)));	//
				cwd(); //promotes the dividend to 32 bits (DX:AX)
				idiv(word_ptr(ESP));	//AX = DX:AX / m16 ; DX = remainder
				mov(word_ptr(getSymbolLabel(target_ptr)), AX);
				pop(ECX);
			}
		}
	}else if(target_type->is_scalar())
	{		
		//F O R   A L L   O T H E R   I N T E G E R S 
		//result = EAX
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			goto __EMIT_DIV_TYPE_INT32;
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);

__EMIT_DIV_TYPE_INT32:
			push(EAX);
			mov(EAX, dword_ptr(EBX)); 
			cdq(); //promotes this 32-bit integer to 64-bits before division is made (EDX:EAX)
			idiv(dword_ptr(ESP));			//EAX = Quotient ; EDX = Remainder
			mov(dword_ptr(EBX), EAX);
			pop(ECX);
		}else{								
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;				

				push(EAX);
				mov(EAX, dword_ptr(EBP, op, abs(offset))); 
				cdq(); //promotes this 32-bit integer to 64-bits before division is made (EDX:EAX)
				idiv(dword_ptr(ESP));			//EAX = Quotient ; EDX = Remainder
				mov(dword_ptr(EBP, op, abs(offset)), EAX);
				pop(ECX);
			}else{
				push(EAX);
				mov(EAX, no_op_ptr(getSymbolLabel(target_ptr))); 
				cdq(); //promotes this 32-bit integer to 64-bits before division is made (EDX:EAX)
				idiv(dword_ptr(ESP));			//EAX = Quotient ; EDX = Remainder
				mov(no_op_ptr(getSymbolLabel(target_ptr)), EAX);
				pop(ECX);
			}
		}
	}else{
		//TODO : we shouldn't allow this type of operator for custom types until implementation of overloaded operators
		assert(0);
		if(bIsAddressOnStack)
		{
			pop(EDI);
			//
		}

		/*
		CPUInstruction cld(this, CLD);							
		CPUInstruction rep_movsb(this, REP_MOVSB);


		int size = target_type->getDataTypeSize();
		//EAX is a pointer address
		mov(ESI, EAX);
		if(bIsAddressOnStack)
		{
			pop(EDI);
			//
		}else{
			//								
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;
				lea(EDI, dword_ptr(EBP, op, abs(offset)));
			}else
				lea(EDI, no_op_ptr(getSymbolLabel(target_ptr)));
		}
		mov(ECX, size);
		cld();
		rep_movsb();
		*/
	}
	return result_type_ptr;
}

void HCCCodeGenerator::changeOperandSign(TypeSpecifier* operand1_type, HCC_TOKEN_TYPE unaryOp)
{
	if(unaryOp==HCC_MINUS_OP)
	{
		CPUInstruction neg(this, NEG);
		CPUInstruction push(this, PUSH);
		CPUInstruction pop(this, POP);

		FPUInstruction fld(this, FLD);
		FPUInstruction fstp(this, FSTP);
		FPUInstruction fchs(this, FCHS);
		
		if(operand1_type==HccTypeChecker::ts_type_Int64_ptr)
		{
			//BEGIN - FIXED, Dec 20, 2008
			/*
			if(!is_operand_in_stack)
			{
				push(EAX);
				push(EDX);
				is_operand_in_stack = false;
			}
			//change sign
			neg(qword_ptr(ESP));
			pop(EDX);
			pop(EAX);
			is_operand_in_stack = false; //was not in the initial version of this function
			*/
			if(!is_operand_in_stack)
				EmitPushOperand(operand1_type);
			//change sign
			neg(qword_ptr(ESP));
			EmitPopOperand(operand1_type);
			//END - FIXED, Dec 20, 2008
			//
		}else if(operand1_type->getDataType()==HCC_INTEGER && operand1_type->is_scalar())
		{
			if(is_operand_in_stack)
				EmitPopOperand(operand1_type);
			//change sign
			neg(EAX);
		}else if(operand1_type->getDataType()==HCC_FLOATING_POINT)
		{
			if(!is_floating_point_in_fpu)
			{
				if(!is_operand_in_stack)
				{
					push(EAX);
					push(EDX);					
				}
				//change sign
				fld(qword_ptr(ESP));
				fchs();
				fstp(qword_ptr(ESP));
				//important: to avoid double popping from the stack at the end of the function body
				is_operand_in_stack = true;
			}else
				fchs();
		}

		//to avoid changing sign again in the next sub-expr
		unaryOp = HCC_PLUS_OP;
	}
}

//----------------------------------------------------------------
// MODULUS	- F O R   I N T E G E R   O P E R A N D S   O N L Y
//
//----------------------------------------------------------------
TypeSpecifier* HCCCodeGenerator::EmitModAssignExpr(Symbol *target_ptr, TypeSpecifier *source_type, HCC_TOKEN_TYPE _operator, Symbol *function_ptr, bool bIsAddressOnStack, TypeSpecifier* array_item_type)
{
	assert(_operator==HCC_MOD_ASSIGN);

	CPUInstruction push(this, PUSH);
	CPUInstruction pop(this, POP);
	CPUInstruction mov(this, MOV);
	CPUInstruction xor(this, XOR);

	CPUInstruction idiv(this, IDIV);	//for now, let's use the signed version of division to preserve the sign

	CPUInstruction cdq(this, CDQ);	//DWORD-to-QWORD
	CPUInstruction cbw(this, CBW);	//BYTE-to-WORD
	CPUInstruction cwd(this, CWD);	//WORD-to-DWORD

	TypeSpecifier* target_type = array_item_type,
				*result_type_ptr = target_type;
	
	if(target_type==NULL){
		result_type_ptr = target_type = target_ptr->getTypeSpecifier().getBaseTypeSpec();
	}

	//if a previous expression was generated, there may be the 'LAST-X' in the stack
	if(is_operand_in_stack)
	{
		EmitPopOperand(stack_operand_type);
		is_operand_in_stack = false;
	}

	if(target_type==HccTypeChecker::ts_type_Int64_ptr)
	{
		if(source_type->getDataTypeSize() <= HccTypeChecker::ts_type_Int32_ptr->getDataTypeSize())
		{
			//promotes divisor to 64-bits...
			cdq();
		}
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			goto _EMIT_MOD_TYPE_INT64;
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);

_EMIT_MOD_TYPE_INT64:
			//BEGIN - FIXED - Dec 18, 2008
			/*
			push(EDX); 
			push(EAX);
			*/
			EmitPushOperand(target_type);
			mov(EDX, dword_ptr(EBX, sizeof(int)));	//low
			mov(EAX, dword_ptr(EBX));				//high
			
			idiv(qword_ptr(ESP)); // EDX:EAX = EDX:EAX / [ESP]
			pop(ECX);
			pop(ECX);
			//the result...			
			mov(dword_ptr(EBX, sizeof(int)), 0);	//EAX = quotient ; EDX = remainder
			mov(dword_ptr(EBX), EDX);
			//END - FIXED - Dec 18, 2008
		}else{
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;								

				//BEGIN - FIXED - Dec 18, 2008
				/*
				push(EDX);
				push(EAX);
				*/
				EmitPushOperand(target_type);
				mov(EDX, dword_ptr(EBP, op, (int)abs((int)offset + (int)sizeof(int))));	//high
				mov(EAX, qword_ptr(EBP, op, (int)abs((int)offset)));					//low
				idiv(qword_ptr(ESP)); // EDX:EAX = EDX:EAX / [ESP]
				pop(ECX);
				pop(ECX);
				//the result...
				mov(dword_ptr(EBP, op, (int)abs((int)offset + (int)sizeof(int))), 0);	//EAX = quotient ; EDX = remainder
				mov(dword_ptr(EBP, op, (int)abs((int)offset)), EDX);
				//END - FIXED - Dec 18, 2008

			}else{				
				__tstring HIGH_PART = __tstring(_T("[")) + 
									getSymbolLabel(target_ptr) + 
									__tstring(QWORD_OFFSET_LOW) + 
									__tstring(_T("]"));

				//END - FIXED - Dec 18, 2008
				/*
				push(EDX);
				push(EAX);
				*/
				EmitPushOperand(target_type);
				mov(EDX, no_op_ptr(HIGH_PART));								//high
				mov(EAX, no_op_ptr(getSymbolLabel(target_ptr)));			//low
				idiv(qword_ptr(ESP)); // EDX:EAX = EDX:EAX / [ESP]
				pop(ECX);
				pop(ECX);
				//the result...
				mov(no_op_ptr(HIGH_PART), 0);								//EAX = quotient ; EDX = remainder
				mov(no_op_ptr(getSymbolLabel(target_ptr)), EDX);
			}
		}
	}else if(target_type==HccTypeChecker::ts_type_char_ptr ||
			target_type==HccTypeChecker::ts_type_boolean_ptr)
	{
		//AX / AL  :  AL = Quotient ; AH = Remainder
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			goto _EMIT_MOD_TYPE_INT8;
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);

_EMIT_MOD_TYPE_INT8:
			push(EAX);
			mov(AL, byte_ptr(EBX));	
			cbw(); //promotes the divident to 16 bits : AX = AH | AL
			idiv(byte_ptr(ESP));	// AL = AX / m8	; AH = remainder
			mov(byte_ptr(EBX), AH);				
			pop(ECX);
		}else{
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;

				push(EAX);
				mov(AL, byte_ptr(EBP, op, abs(offset)));	
				cbw(); //promotes the divident to 16 bits : AX = AH | AL
				idiv(byte_ptr(ESP));	// AL = AX / m8	; AH = remainder
				mov(byte_ptr(EBP, op, abs(offset)), AH);
				pop(ECX);
			}else{
				push(EAX);
				mov(AL, byte_ptr(getSymbolLabel(target_ptr)));	
				cbw(); //promotes the divident to 16 bits : AX = AH | AL
				idiv(byte_ptr(ESP));	// AL = AX / m8	; AH = remainder
				mov(byte_ptr(getSymbolLabel(target_ptr)), AH);
				pop(ECX);
			}
		}
	}else if(target_type==HccTypeChecker::ts_type_Int16_ptr ||
		target_type==HccTypeChecker::ts_type_short_ptr)
	{
		//DX:AX / AX  : AX = Quotient ; DX = Remainder
		//result = AX
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			goto _EMIT_MOD_TYPE_INT16;
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);

_EMIT_MOD_TYPE_INT16:
			push(EAX);
			mov(AX, word_ptr(EBX));	//
			cwd(); //promotes the dividend to 32 bits (DX:AX)
			idiv(word_ptr(ESP));		//AX = DX:AX / m16 ; DX = remainder
			mov(word_ptr(EBX), DX);
			pop(ECX);
		}else{								
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;

				push(EAX);
				mov(AX, word_ptr(EBP, op, abs(offset)));	//
				cwd(); //promotes the dividend to 32 bits (DX:AX)
				idiv(word_ptr(ESP));	//AX = DX:AX / m16 ; DX = remainder
				mov(word_ptr(EBP, op, abs(offset)), DX);
				pop(ECX);

			}else{
				push(EAX);
				mov(AX, word_ptr(getSymbolLabel(target_ptr)));	//
				cwd(); //promotes the dividend to 32 bits (DX:AX)
				idiv(word_ptr(ESP));	//AX = DX:AX / m16 ; DX = remainder
				mov(word_ptr(getSymbolLabel(target_ptr)), DX);
				pop(ECX);
			}
		}
	}else if(target_type->is_scalar())
	{		
		//F O R   A L L   O T H E R   I N T E G E R S 
		//result = EAX
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			goto _EMIT_MOD_TYPE_INT32;
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);

_EMIT_MOD_TYPE_INT32:
			push(EAX);
			mov(EAX, dword_ptr(EBX)); 
			cdq(); //promotes this 32-bit integer to 64-bits before division is made (EDX:EAX)
			idiv(dword_ptr(ESP));			//EAX = Quotient ; EDX = Remainder
			mov(dword_ptr(EBX), EDX);
			pop(ECX);
		}else{								
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;				

				push(EAX);
				mov(EAX, dword_ptr(EBP, op, abs(offset))); 
				cdq(); //promotes this 32-bit integer to 64-bits before division is made (EDX:EAX)
				idiv(dword_ptr(ESP));			//EAX = Quotient ; EDX = Remainder
				mov(dword_ptr(EBP, op, abs(offset)), EDX);
				pop(ECX);
			}else{
				push(EAX);
				mov(EAX, no_op_ptr(getSymbolLabel(target_ptr))); 
				cdq(); //promotes this 32-bit integer to 64-bits before division is made (EDX:EAX)
				idiv(dword_ptr(ESP));			//EAX = Quotient ; EDX = Remainder
				mov(no_op_ptr(getSymbolLabel(target_ptr)), EDX);
				pop(ECX);
			}
		}
	}else{
		//TODO : we shouldn't allow this type of operator for custom types until implementation of overloaded operators
		assert(0);
		if(bIsAddressOnStack)
		{
			pop(EDI);
			//
		}

		/*
		CPUInstruction cld(this, CLD);							
		CPUInstruction rep_movsb(this, REP_MOVSB);


		int size = target_type->getDataTypeSize();
		//EAX is a pointer address
		mov(ESI, EAX);
		if(bIsAddressOnStack)
		{
			pop(EDI);
			//
		}else{
			//								
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;
				lea(EDI, dword_ptr(EBP, op, abs(offset)));
			}else
				lea(EDI, no_op_ptr(getSymbolLabel(target_ptr)));
		}
		mov(ECX, size);
		cld();
		rep_movsb();
		*/
	}

	return result_type_ptr;
}

//--------------------------------------------------------------------
// BITWISE AND, OR, XOR	- F O R   I N T E G E R   O P E R A N D S   O N L Y
//
//--------------------------------------------------------------------

TypeSpecifier* HCCCodeGenerator::EmitLogicalOpAssignExpr(Symbol *target_ptr, TypeSpecifier *source_type, HCC_TOKEN_TYPE _operator, Symbol *function_ptr, bool bIsAddressOnStack, TypeSpecifier* array_item_type)
{
	x86_INSTRUCTION logical_inst = XOR;
	//
	if(_operator==HCC_BIT_AND_ASSIGN)
		logical_inst = AND;
	else if(_operator==HCC_BIT_OR_ASSIGN)
		logical_inst = OR;

	CPUInstruction logical_op(this, logical_inst);
	CPUInstruction cdq(this, CDQ);
	CPUInstruction pop(this, POP);

	TypeSpecifier* target_type = array_item_type,
				*result_type_ptr = target_type;
	
	if(target_type==NULL){
		result_type_ptr = target_type = target_ptr->getTypeSpecifier().getBaseTypeSpec();
	}

	//if a previous expression was generated, there may be the 'LAST-X' in the stack
	if(is_operand_in_stack)
	{
		EmitPopOperand(stack_operand_type);
		is_operand_in_stack = false;
	}

	if(target_type==HccTypeChecker::ts_type_Int64_ptr)
	{
		if(source_type!=HccTypeChecker::ts_type_Int64_ptr)
		{
			//promote to Int64...
			cdq();
		}
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			//BEGIN - FIXED Dec 23, 2008
			/*
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			logical_op(dword_ptr(EBX, sizeof(int)), EDX);	//high
			logical_op(dword_ptr(EBX), EAX);				//low
			*/
			EmitAddressFromAddressOf(id_ptr, function_ptr, true, target_type, logical_inst); //result in EBX
			//END - FIXED Dec 23, 2008
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);			
			logical_op(dword_ptr(EBX, sizeof(int)), EDX);	//high
			logical_op(dword_ptr(EBX), EAX);				//low
		}else{
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;								
				logical_op(dword_ptr(EBP, op, (int)abs((int)offset + (int)sizeof(int))), EDX);	//high
				logical_op(dword_ptr(EBP, op, (int)abs((int)offset)), EAX);	//low
			}else{
				logical_op(no_op_ptr(getSymbolLabel(target_ptr)), EAX); //low
				__tstring HIGH_PART = __tstring(_T("[")) + 
									getSymbolLabel(target_ptr) + 
									__tstring(QWORD_OFFSET_LOW) + 
									__tstring(_T("]"));

				logical_op(no_op_ptr(HIGH_PART), EDX); //high
			}
		}
	}else if(target_type==HccTypeChecker::ts_type_char_ptr ||
			target_type==HccTypeChecker::ts_type_boolean_ptr)
	{
		//EAX
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			//BEGIN - FIXED Dec 23, 2008
			/*
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			logical_op(byte_ptr(EBX), AL);
			*/
			EmitAddressFromAddressOf(id_ptr, function_ptr, true, target_type, logical_inst, AL); //result in EBX
			//END - FIXED Dec 23, 2008
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);
			logical_op(byte_ptr(EBX), AL);
		}else{			
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;

				logical_op(byte_ptr(EBP, op, abs(offset)), AL);
			}else
				logical_op(byte_ptr(getSymbolLabel(target_ptr)), AL);
		}
	}else if(target_type==HccTypeChecker::ts_type_Int16_ptr ||
		target_type==HccTypeChecker::ts_type_short_ptr)
	{
		//AX
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			//BEGIN - FIXED Dec 23, 2008
			/*
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			logical_op(word_ptr(EBX), AX);
			*/
			EmitAddressFromAddressOf(id_ptr, function_ptr, true, target_type, logical_inst, AX); //result in EBX
			//END - FIXED Dec 23, 2008
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);
			logical_op(word_ptr(EBX), AX);
		}else{								
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;

				logical_op(word_ptr(EBP, op, abs(offset)), AX);
			}else
				logical_op(word_ptr(getSymbolLabel(target_ptr)), AX);
		}
	}else if(target_type->is_scalar())
	{		
		//F O R   A L L   O T H E R   I N T E G E R S 
		//EAX
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			//BEGIN - FIXED Dec 23, 2008
			/*
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			logical_op(dword_ptr(EBX), EAX);
			*/
			EmitAddressFromAddressOf(id_ptr, function_ptr, true, target_type, logical_inst, EAX); //result in EBX
			//END - FIXED Dec 23, 2008
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);
			logical_op(dword_ptr(EBX), EAX);
		}else{								
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;				
				logical_op(dword_ptr(EBP, op, abs(offset)), EAX);
			}else
				logical_op(no_op_ptr(getSymbolLabel(target_ptr)), EAX);
		}
	}else{
		//TODO : we shouldn't allow this type of operator for custom types until implementation of overloaded operators
		assert(0);
		if(bIsAddressOnStack)
		{
			pop(EDI);
			//
		}

		/*
		CPUInstruction cld(this, CLD);							
		CPUInstruction rep_movsb(this, REP_MOVSB);


		int size = target_type->getDataTypeSize();
		//EAX is a pointer address
		mov(ESI, EAX);
		if(bIsAddressOnStack)
		{
			pop(EDI);
			//
		}else{
			//								
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;
				lea(EDI, dword_ptr(EBP, op, abs(offset)));
			}else
				lea(EDI, no_op_ptr(getSymbolLabel(target_ptr)));
		}
		mov(ECX, size);
		cld();
		rep_movsb();
		*/
	}
	return result_type_ptr;
}

/*
//------------------------------------------------------------------
// BITWISE XOR	- F O R   I N T E G E R   O P E R A N D S   O N L Y
//
//------------------------------------------------------------------
TypeSpecifier* HCCCodeGenerator::EmitXorAssignExpr(Symbol *target_ptr, TypeSpecifier *source_type, HCC_TOKEN_TYPE _operator, Symbol *function_ptr, bool bIsAddressOnStack, TypeSpecifier* array_item_type)
{
	CPUInstruction xor(this, XOR);
	CPUInstruction cdq(this, CDQ);
	CPUInstruction pop(this, POP);

	TypeSpecifier* target_type = array_item_type,
				*result_type_ptr = target_type;
	
	if(target_type==NULL){
		result_type_ptr = target_type = target_ptr->getTypeSpecifier().getBaseTypeSpec();
	}

	//if a previous expression was generated, there may be the 'LAST-X' in the stack
	if(is_operand_in_stack)
	{
		EmitPopOperand(stack_operand_type);
		is_operand_in_stack = false;
	}

	if(target_type==HccTypeChecker::ts_type_Int64_ptr)
	{
		if(source_type!=HccTypeChecker::ts_type_Int64_ptr)
		{
			//promote to Int64...
			cdq();
		}
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			xor(dword_ptr(EBX, sizeof(int)), EDX);	//high
			xor(dword_ptr(EBX), EAX);				//low
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);			
			xor(dword_ptr(EBX, sizeof(int)), EDX);	//high
			xor(dword_ptr(EBX), EAX);				//low
		}else{
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;								
				xor(dword_ptr(EBP, op, abs(offset + sizeof(int))), EDX);	//high
				xor(dword_ptr(EBP, op, abs(offset)), EAX);	//low
			}else{
				xor(no_op_ptr(getSymbolLabel(target_ptr)), EAX); //low
				__tstring HIGH_PART = __tstring(_T("[")) + 
									getSymbolLabel(target_ptr) + 
									__tstring(QWORD_OFFSET_LOW) + 
									__tstring(_T("]"));

				xor(no_op_ptr(HIGH_PART), EDX); //high
			}
		}
	}else if(target_type==HccTypeChecker::ts_type_char_ptr ||
			target_type==HccTypeChecker::ts_type_boolean_ptr)
	{
		//EAX
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			xor(byte_ptr(EBX), AL);
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);
			xor(byte_ptr(EBX), AL);
		}else{			
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;

				xor(byte_ptr(EBP, op, abs(offset)), AL);
			}else
				xor(byte_ptr(getSymbolLabel(target_ptr)), AL);
		}
	}else if(target_type==HccTypeChecker::ts_type_Int16_ptr ||
		target_type==HccTypeChecker::ts_type_short_ptr)
	{
		//AX
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			xor(word_ptr(EBX), AX);
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);
			xor(word_ptr(EBX), AX);
		}else{								
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;

				xor(word_ptr(EBP, op, abs(offset)), AX);
			}else
				xor(word_ptr(getSymbolLabel(target_ptr)), AX);
		}
	}else if(target_type->is_scalar())
	{		
		//F O R   A L L   O T H E R   I N T E G E R S 
		//EAX
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			xor(dword_ptr(EBX), EAX);
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);
			xor(dword_ptr(EBX), EAX);
		}else{								
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;				
				xor(dword_ptr(EBP, op, abs(offset)), EAX);
			}else
				xor(no_op_ptr(getSymbolLabel(target_ptr)), EAX);
		}
	}else{
		//TODO : we shouldn't allow this type of operator for custom types until implementation of overloaded operators
		assert(0);
		if(bIsAddressOnStack)
		{
			pop(EDI);
			//
		}

	}
	return result_type_ptr;
}
*/
/*
//--------------------------------------------------------------------
// BITWISE OR	- F O R   I N T E G E R   O P E R A N D S   O N L Y
//
//--------------------------------------------------------------------
TypeSpecifier* HCCCodeGenerator::EmitOrAssignExpr(Symbol *target_ptr, TypeSpecifier *source_type, HCC_TOKEN_TYPE _operator, Symbol *function_ptr, bool bIsAddressOnStack, TypeSpecifier* array_item_type)
{
	CPUInstruction or(this, OR);
	CPUInstruction cdq(this, CDQ);
	CPUInstruction pop(this, POP);

	TypeSpecifier* target_type = array_item_type,
				*result_type_ptr = target_type;
	
	if(target_type==NULL){
		result_type_ptr = target_type = target_ptr->getTypeSpecifier().getBaseTypeSpec();
	}

	//if a previous expression was generated, there may be the 'LAST-X' in the stack
	if(is_operand_in_stack)
	{
		EmitPopOperand(stack_operand_type);
		is_operand_in_stack = false;
	}


	if(target_type==HccTypeChecker::ts_type_Int64_ptr)
	{
		if(source_type!=HccTypeChecker::ts_type_Int64_ptr)
		{
			//promote to Int64...
			cdq();
		}
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			or(dword_ptr(EBX, sizeof(int)), EDX);	//high
			or(dword_ptr(EBX), EAX);	//low
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);			
			or(dword_ptr(EBX, sizeof(int)), EDX);	//high
			or(dword_ptr(EBX), EAX);	//low
		}else{
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;								
				or(dword_ptr(EBP, op, abs(offset + sizeof(int))), EDX);	//high
				or(dword_ptr(EBP, op, abs(offset)), EAX);	//low
			}else{
				or(no_op_ptr(getSymbolLabel(target_ptr)), EAX); //low
				__tstring HIGH_PART = __tstring(_T("[")) + 
									getSymbolLabel(target_ptr) + 
									__tstring(QWORD_OFFSET_LOW) + 
									__tstring(_T("]"));

				or(no_op_ptr(HIGH_PART), EAX); //high
			}
		}
	}else if(target_type==HccTypeChecker::ts_type_char_ptr ||
			target_type==HccTypeChecker::ts_type_boolean_ptr)
	{
		//EAX
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			or(byte_ptr(EBX), AL);
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);
			or(byte_ptr(EBX), AL);
		}else{			
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;

				or(byte_ptr(EBP, op, abs(offset)), AL);
			}else
				or(byte_ptr(getSymbolLabel(target_ptr)), AL);
		}
	}else if(target_type==HccTypeChecker::ts_type_Int16_ptr ||
		target_type==HccTypeChecker::ts_type_short_ptr)
	{
		//AX
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			or(word_ptr(EBX), AX);
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);
			or(word_ptr(EBX), AX);
		}else{								
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;

				or(word_ptr(EBP, op, abs(offset)), AX);
			}else
				or(word_ptr(getSymbolLabel(target_ptr)), AX);
		}
	}else if(target_type->is_scalar())
	{		
		//F O R   A L L   O T H E R   I N T E G E R S 
		//EAX
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			or(dword_ptr(EBX), EAX);
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);
			or(dword_ptr(EBX), EAX);
		}else{								
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;				
				or(dword_ptr(EBP, op, abs(offset)), EAX);
			}else
				or(no_op_ptr(getSymbolLabel(target_ptr)), EAX);
		}
	}else{
		//TODO : we shouldn't allow this type of operator for custom types until implementation of overloaded operators
		assert(0);
		if(bIsAddressOnStack)
		{
			pop(EDI);
			//
		}

	}
	return result_type_ptr;
}
*/
/*
//--------------------------------------------------------------------
// BITWISE AND	- F O R   I N T E G E R   O P E R A N D S   O N L Y
//
//--------------------------------------------------------------------
TypeSpecifier* HCCCodeGenerator::EmitAndAssignExpr(Symbol *target_ptr, TypeSpecifier *source_type, HCC_TOKEN_TYPE _operator, Symbol *function_ptr, bool bIsAddressOnStack, TypeSpecifier* array_item_type)
{
	CPUInstruction and(this, AND);
	CPUInstruction cdq(this, CDQ);
	CPUInstruction pop(this, POP);

	TypeSpecifier* target_type = array_item_type,
				*result_type_ptr = target_type;
	
	if(target_type==NULL){
		result_type_ptr = target_type = target_ptr->getTypeSpecifier().getBaseTypeSpec();
	}

	//if a previous expression was generated, there may be the 'LAST-X' in the stack
	if(is_operand_in_stack)
	{
		EmitPopOperand(stack_operand_type);
		is_operand_in_stack = false;
	}

	if(target_type==HccTypeChecker::ts_type_Int64_ptr)
	{
		if(source_type!=HccTypeChecker::ts_type_Int64_ptr)
		{
			//promote to Int64...
			cdq();
		}
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			and(dword_ptr(EBX, sizeof(int)), EDX);	//high
			and(dword_ptr(EBX), EAX);	//low
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);			
			and(dword_ptr(EBX, sizeof(int)), EDX);	//high
			and(dword_ptr(EBX), EAX);	//low
		}else{
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;								
				and(dword_ptr(EBP, op, abs(offset + sizeof(int))), EDX);	//high
				and(dword_ptr(EBP, op, abs(offset)), EAX);	//low
			}else{
				and(no_op_ptr(getSymbolLabel(target_ptr)), EAX); //low
				__tstring HIGH_PART = __tstring(_T("[")) + 
									getSymbolLabel(target_ptr) + 
									__tstring(QWORD_OFFSET_LOW) + 
									__tstring(_T("]"));

				and(no_op_ptr(HIGH_PART), EDX); //high
			}
		}
	}else if(target_type==HccTypeChecker::ts_type_char_ptr ||
			target_type==HccTypeChecker::ts_type_boolean_ptr)
	{
		//EAX
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			and(byte_ptr(EBX), AL);
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);
			and(byte_ptr(EBX), AL);
		}else{			
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;

				and(byte_ptr(EBP, op, abs(offset)), AL);
			}else
				and(byte_ptr(getSymbolLabel(target_ptr)), AL);
		}
	}else if(target_type==HccTypeChecker::ts_type_Int16_ptr ||
		target_type==HccTypeChecker::ts_type_short_ptr)
	{
		//AX
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			and(word_ptr(EBX), AX);
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);
			and(word_ptr(EBX), AX);
		}else{								
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;

				and(word_ptr(EBP, op, abs(offset)), AX);
			}else
				and(word_ptr(getSymbolLabel(target_ptr)), AX);
		}
	}else if(target_type->is_scalar())
	{		
		//F O R   A L L   O T H E R   I N T E G E R S 
		//EAX
		if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			EmitAddressFromAddressOf(id_ptr, function_ptr); //result in EBX
			and(dword_ptr(EBX), EAX);
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);
			and(dword_ptr(EBX), EAX);
		}else{								
			if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				int offset = target_ptr->getDeclDefinition().user_data.offset;
				TCHAR op = __minus;
				if(offset > 0 )
					op = __plus;				
				and(dword_ptr(EBP, op, abs(offset)), EAX);
			}else
				and(no_op_ptr(getSymbolLabel(target_ptr)), EAX);
		}
	}else{
		//TODO : we shouldn't allow this type of operator for custom types until implementation of overloaded operators
		assert(0);
		if(bIsAddressOnStack)
		{
			pop(EDI);
			//
		}

	}
	return result_type_ptr;
}
*/
bool HCCCodeGenerator::IsTypeQualifiedForDefaultConstructor(TypeSpecifier &typeSpec)
{
	//if type has data members, or is an abstract class...
	return (typeSpec.user_type_members.data_members.size() > 0 ||
			typeSpec.getSymbolTable()->find(CLASS_VPTR_VTBL_NAME)!=0);
}


//-----------------------------------------------------------------------
// emitIfStatement	- emits the if statements
//
//	if(<expr>)
//		<stmt1>;
//-----------------------------------------------------------------------
//
//	if(<expr>)
//		<stmt1>;
//	else
//		<stmt2>;
//
//-----------------------------------------------------------------------
void HCCCodeGenerator::emitIfStatement(Symbol *function_ptr)
{
	assert(function_ptr!=NULL);

	CPUInstruction test(this, TEST);
	CPUInstruction jz(this, JZ);
	CPUInstruction jmp(this, JMP);

	unsigned int label_else_stmt_index		= SYMBOL_TABLE::__asm_label_index++;
	unsigned int label_end_if_stmt_index	= SYMBOL_TABLE::__asm_label_index++;

	__tstring label_else_stmt	= CreateLabel(STMT_LABEL_PREFIX, label_else_stmt_index);
	__tstring label_end_if_stmt = CreateLabel(STMT_LABEL_PREFIX, label_end_if_stmt_index);
	//
	getToken(); //skip the 'if' keyword
	//
	//BEGIN - FIXED - hlm Mar 1, 2009
	if(is_operand_in_stack)
	{
		EmitPopOperand(stack_operand_type); //this fixes a stack overflow that appears between expr1 and if(expr2)
	}
	//END - FIXED - hlm Mar 1, 2009
	//<expr>
	EmitExprList(function_ptr);
	//
	test(AL,AL);
	jz(VAR_OFFSET, label_else_stmt);
	HCC_TOKEN_TYPE last_stmt = EmitStatement(function_ptr);
	if(token_type==HCC_SEMICOLON)
		getToken(); //skip the ';' token
	//
	if(token_type==HCC_ELSE)
	{
		//if following the 'if(expr) <stmt-list>' is the 'else <stmt-list>', we make a jump to the end.
		if(last_stmt!=HCC_RETURN)
		{
			jmp(VAR_OFFSET, label_end_if_stmt);
		}

		EmitNewLineComment(_T(" else {\n"));
		getToken(); //skip the 'else' keyword

		bool bStackIsDirty = is_operand_in_stack; //hlm - fixed Nov 12, 2011
		is_operand_in_stack = false;			  //hlm - fixed Nov 12, 2011	
		//else block
		EmitStatementLabel(label_else_stmt_index);
		last_stmt = EmitStatement(function_ptr);
		if(token_type==HCC_SEMICOLON)
			getToken(); //skip the ';' token
		//		
		is_operand_in_stack = bStackIsDirty; //hlm - fixed Nov 12, 2011
		EmitStatementLabel(label_end_if_stmt_index);
		EmitNewLineComment(_T(" }\n"));
	}else{
		//else block
		EmitStatementLabel(label_else_stmt_index);
	}		
}


//-----------------------------------------------------------------------
// emitWhile	- emits the while statements.
//
//	while(true) <stmt>; //infinite loops!
//
//	while(<expr>)
//		<stmt>;
//-----------------------------------------------------------------------
//	
//	while(<expr>)
//	{
//		<stmt-list>
//	}
//-----------------------------------------------------------------------
void HCCCodeGenerator::emitWhile(Symbol *function_ptr)
{
	assert(function_ptr!=NULL);
	//
	CPUInstruction test(this, TEST);
	CPUInstruction jz(this, JZ);
	CPUInstruction jmp(this, JMP);

	getToken(); //skip the 'while' keyword

	unsigned int label_while_expr_index		= SYMBOL_TABLE::__asm_label_index++;
	unsigned int label_end_while_stmt_index = SYMBOL_TABLE::__asm_label_index++;

	HPP_STATEMENT_INFO stmt_info;

	stmt_info.type				= HCC_WHILE;
	stmt_info.stmt_start_block	= label_while_expr_index;
	stmt_info.stmt_end_block	= label_end_while_stmt_index;

	//the current loop statement...
	stack_of_stmts.push(stmt_info);

	__tstring label_while_expr		= CreateLabel(STMT_LABEL_PREFIX, label_while_expr_index);

	__tstring label_end_while_stmt	= CreateLabel(STMT_LABEL_PREFIX, label_end_while_stmt_index);

	//:$Lxxx: <expr>
	EmitStatementLabel(label_while_expr_index);
	//
	EmitExprList(function_ptr);
	//
	test(AL, AL); //Fixed Feb 27, 2009
	jz(VAR_OFFSET, label_end_while_stmt);
	//:<stmt>
	EmitStatement(function_ptr);
	//go back to evaluate the expr...
	jmp(VAR_OFFSET, label_while_expr);
	//
	EmitStatementLabel(label_end_while_stmt_index);
	//
	//take this statement off the stack...
	stack_of_stmts.pop();
}


//-----------------------------------------------------------------------
//	emitDoWhile	- emits the do...while statements.
//
//  do
//		<stmt>;
//	while(<expr>);
//
//-----------------------------------------------------------------------
//	do {
//
//		<stmt-list>
//
//	} while(<expr>);
//
//-----------------------------------------------------------------------
void HCCCodeGenerator::emitDoWhile(Symbol *function_ptr)
{
	assert(function_ptr!=NULL);
	//
	CPUInstruction test(this, TEST);
	CPUInstruction jne(this, JNE);
	//

	getToken();	//skip the 'do' keyword
	unsigned int label_do_stmt_index		= SYMBOL_TABLE::__asm_label_index++;
	unsigned int label_while_stmt_index		= SYMBOL_TABLE::__asm_label_index++;
	unsigned int label_end_do_while_index	= SYMBOL_TABLE::__asm_label_index++;

	__tstring label_do_stmt = CreateLabel(STMT_LABEL_PREFIX, label_do_stmt_index);	

	HPP_STATEMENT_INFO stmt_info;

	stmt_info.type				= HCC_DO;
	stmt_info.stmt_start_block	= label_while_stmt_index;
	stmt_info.stmt_end_block	= label_end_do_while_index;

	//the current loop statement...
	stack_of_stmts.push(stmt_info);

	EmitStatementLabel(label_do_stmt_index);
	//
	EmitStatementList(HCC_WHILE, function_ptr);
	//

	//the boolean expr label must be here...
	EmitStatementLabel(label_while_stmt_index);

	EmitStatementComment(function_ptr);
	getToken(); //skip the 'while' keyword
	EmitExprList(function_ptr);
	//
	test(AL,AL); //Fixed Feb 27, 2009
	jne(VAR_OFFSET, label_do_stmt);
	//the end of statement for breaks
	EmitStatementLabel(label_end_do_while_index);

	//take this statement off the stack...
	stack_of_stmts.pop();
}


//-----------------------------------------------------------------------
//	emitFor	- emits the for statements.
//
//	for(;;) <stmt>;	//infinite loops!
//
//	for(expr1; expr2; expr3)
//		<stmt>;
//
//	for(expr1; expr2; expr3)
//	{
//		<stmt-list>
//	}
//
//-----------------------------------------------------------------------
void HCCCodeGenerator::emitFor(Symbol *function_ptr)
{
	assert(function_ptr!=NULL);

	CPUInstruction test(this, TEST);
	CPUInstruction jz(this, JZ);
	CPUInstruction jmp(this, JMP);
	CPUInstruction mov(this, MOV);

	unsigned int label_boolean_expr_index	= SYMBOL_TABLE::__asm_label_index++;
	unsigned int label_inc_dec_expr_index	= SYMBOL_TABLE::__asm_label_index++;
	unsigned int label_end_for_expr_index	= SYMBOL_TABLE::__asm_label_index++;	

	__tstring label_boolean_expr = CreateLabel(STMT_LABEL_PREFIX, label_boolean_expr_index);
	__tstring label_end_for_expr = CreateLabel(STMT_LABEL_PREFIX, label_end_for_expr_index);
	__tstring label_inc_dec_expr = CreateLabel(STMT_LABEL_PREFIX, label_inc_dec_expr_index);	

	HPP_STATEMENT_INFO stmt_info;

	stmt_info.type				= HCC_FOR;
	stmt_info.stmt_start_block	= label_inc_dec_expr_index;
	stmt_info.stmt_end_block	= label_end_for_expr_index;

	//the current loop statement...
	stack_of_stmts.push(stmt_info);

	//
	getToken(); //skip the 'for' keyword
	getToken(); //skip the '(' parentesis

	//BEGIN - FIXED Dec 28, 2008
	if(is_operand_in_stack)
	{
		EmitPopOperand(stack_operand_type);
		is_operand_in_stack = false;
	}
	//END - FIXED Dec 28, 2008
	//<expr1>
	EmitExprList(function_ptr);
	
	getToken(); //skip the ';'
	//
	jmp(VAR_OFFSET, label_boolean_expr);

	// B E G I N ------------E X P R ( 2 )   I N   T E M P   S T R E A M --------------
	bool bInfinite_Loops = false;
	//save the unit file ptr
	__tostream* unit_file_ptr = unit().detach();

	__tostring_stream* str_stream_ptr = new __tostring_stream();
	unit().attach(str_stream_ptr);

	//$Lxxx: <expr2>
	EmitStatementLabel(label_boolean_expr_index);

	if(token_type!=HCC_SEMICOLON)
		EmitExprList(function_ptr);
	else
		bInfinite_Loops = true;
	//
	if(token_type==HCC_SEMICOLON)
	{
		getToken(); //skip the ';'
	}

	//Note: because of the type of intermediate code we are using here, it's very difficult to
	//optimize all of these branches we have, that makes the processor assumes the overhead
	//of not predicting branches. It will be changed when the ICode were based on
	//Statement Trees or a better Data Structure, instead of the current Serial one.
	
	//save the expr2's assembly code to create a better brach block...
	__tstring expr2_code = str_stream_ptr->str();

	//restore the unit file ptr
	unit().attach(unit_file_ptr);

	// E N D ------------E X P R ( 2 )   I N   T E M P   S T R E A M --------------

	//E X P R ( 3 )
	//$Lxxx: <expr3>
	EmitStatementLabel(label_inc_dec_expr_index);
	//
	if(token_type!=HCC_RPAREN)
		EmitExprList(function_ptr);		
	
	if(token_type==HCC_RPAREN)
	{
		getToken(); //skip the ')'
	}

	//E X P R ( 2 )	
	//add the expr2's code after the expr3;
	unit() << expr2_code;
	if(!bInfinite_Loops)
	{
		test(AL, AL); //Fixed Dec 28, 2008
		jz(VAR_OFFSET, label_end_for_expr);		
	}
	//
	//<stmt-list>	
	EmitStatement(function_ptr);
	jmp(VAR_OFFSET, label_inc_dec_expr);
	//
	//<end-for>
	EmitStatementLabel(label_end_for_expr_index);
	//
	stack_of_stmts.pop();
}

void HCCCodeGenerator::emitSwitch(Symbol *function_ptr)
{
	CPUInstruction mov(this, MOV);
	CPUInstruction jz(this, JZ);
	CPUInstruction jmp(this, JMP);

	HPP_STATEMENT_INFO stmt_info;

	stmt_info.type				= HCC_SWITCH;
	stmt_info.stmt_start_block	= -1;
	stmt_info.stmt_end_block	= -1;

	//the current statement...
	stack_of_stmts.push(stmt_info);


	getToken(); //skip the 'switch' keyword
	EmitExprList(function_ptr);

	mov(ESI, EAX);

	getToken(); //skip the '{' token

	//we must build a Branch Table to joint all the case branches and the default one in a sole group;
	//also we have to join all the Branch Statements in one place
	/*
		All the statements will look like this:

		$LXX1:
			<stmt-list-1>
			jmp OFFSET <label-end-switch> when 'break' is specified

		$LXX2:
			<stmt-list-2>
			jmp OFFSET <label-end-switch> when 'break' is specified

		$LXXN:
			<stmt-list-n>
			jmp OFFSET <label-end-switch> when 'break' is specified
	*/

	queue<__tstring> branch_table;
	queue<__tstring> block_stmt;

	int __end_switch_stmt_block_index = SYMBOL_TABLE::__asm_label_index++;
	__tstring __end_switch_stmt_block = CreateLabel(STMT_LABEL_PREFIX, __end_switch_stmt_block_index);

	volatile bool bHasDefaultBlock = false;
	int ndef_block_index = 0;

	while((token_type==HCC_CASE || token_type==HCC_DEFAULT) && token_type!=HCC_EOF)
	{
		//this will give us the chance to go to the default block in case none of the case branches
		//get executed
		if(token_type==HCC_CASE)
		{
			emitCaseBranch(branch_table, 
						   block_stmt,  
						   function_ptr, 
						   __end_switch_stmt_block);

		}else if(token_type==HCC_DEFAULT)
		{
			bHasDefaultBlock = true;
			emitCaseBranch(branch_table, 
						   block_stmt,  
						   function_ptr, 
						   __end_switch_stmt_block,
						   &ndef_block_index);
		}
	}
	//B R A N C H   T A B L E 
	while(!branch_table.empty())
	{
		unit() << branch_table.front();
		branch_table.pop();
	}

	if(bHasDefaultBlock)
	{
		__tstring __default_block_stmt = CreateLabel(STMT_LABEL_PREFIX, ndef_block_index);
		//emits a jmp stmt to the default block
		jmp(VAR_OFFSET, __default_block_stmt);
	}
	//S T A T E M E N T - B L O C K S 
	while(!block_stmt.empty())
	{
		unit() << block_stmt.front();
		block_stmt.pop();
	}
	EmitStatementLabel(__end_switch_stmt_block_index);

	getToken(); //skip the '}' token

	//take off this statement...
	stack_of_stmts.pop();
}

//------------------------------------------------------------------
//
//	case x:
//	case y:
//		{
//			<stmt-list>;
//		}
//		break;
//
//	case a:
//		{
//		stmt-list;
//		}
//	break;
//	case b:
//		stmt2;
//	break;
//	case z:
//		stmt3;
//	break;
//	default:
//		def-stmt;
//	break;
//
//------------------------------------------------------------------
void HCCCodeGenerator::emitCaseBranch(	queue<__tstring>& branch_table,
										queue<__tstring>& block_stmt,
										Symbol *function_ptr,
										const __tstring& __end_switch_stmt_block,
										int* def_switch_block_ptr)
{
	CPUInstruction cmp(this, CMP);
	CPUInstruction jmp(this, JMP);
	CPUInstruction jz(this, JZ);	

	//save the unit file ptr
	__tostream* unit_file_ptr = unit().detach();

	if(token_type==HCC_CASE)
	{

		__tostring_stream* str_branches_ptr = new __tostring_stream();
		unit().attach(str_branches_ptr);

		while(token_type==HCC_CASE)
		{
			//BEGIN - B R A N C H   T A B L E  C O N S T R U C T I O N
			getToken();		//case
			//
			if(token_type==HCC_IDENTIFIER)
			{
				EmitLoadValueAddressOf(symbol_ptr, function_ptr);
				//
			}else if(token_type==HCC_NUMBER)
			{
				//EmitLoadNumber(symbol_ptr);
				CONST_VALUE_INFO valueInfo;
				EmitConstNumberValue(symbol_ptr, &valueInfo);
				EmitLoadNumber(valueInfo);
			}
			getToken(); //id | number
			getToken(); //:

			int __stmt_block_index = SYMBOL_TABLE::__asm_label_index++;

			__tstring __stmt_block_x = CreateLabel(STMT_LABEL_PREFIX, __stmt_block_index);
			
			//
			cmp(EAX, ESI);
			jz(VAR_OFFSET, __stmt_block_x);

			//save the branch's assembly code in the queue...
			__tstring code = str_branches_ptr->str();
			branch_table.push(code);			

			//if there is a series of case's without statements, then they belongs to 
			//the same branch block; so, we'll leaverage of the same object to avoid
			//destroying and constructing new ones...
			str_branches_ptr->seekp(0, ios::beg);
			str_branches_ptr->str(_T(""));
			//END - B R A N C H   T A B L E  C O N S T R U C T I O N

			//BEGIN - B L O C K   S T A T E M E M T   C O N S T R U C T I O N 			
			EmitStatementLabel(__stmt_block_index);	//the block-statements		
			while(token_type!=HCC_CASE && 
					token_type!=HCC_BREAK && 
					token_type!=HCC_DEFAULT)
			{
				EmitStatement(function_ptr);
				//after each single-statement, must follow a ';' token
				//
				if(token_type==HCC_SEMICOLON)
					getToken();					  //skip the ';' token
			}
			if(token_type==HCC_BREAK)
			{
				//emits a jmp stmt
				jmp(VAR_OFFSET, __end_switch_stmt_block);
			}

			//save the stmt-block's assembly code in the queue...
			__tstring stmt = str_branches_ptr->str();
			block_stmt.push(stmt);
			str_branches_ptr->seekp(0, ios::beg);
			str_branches_ptr->str(_T(""));
			//END - B L O C K   S T A T E M E M T   C O N S T R U C T I O N 			
		}
		if(token_type==HCC_BREAK)
		{
			getToken(); //break
			getToken(); // then a ';' token
		}
	}else
	{
		__tostring_stream* str_default_ptr = new __tostring_stream();
		unit().attach(str_default_ptr);

		getToken(); //default
		getToken(); //:

		int __default_block_stmt_index = SYMBOL_TABLE::__asm_label_index++;
		EmitStatementLabel(__default_block_stmt_index);
		//
		while(token_type!=HCC_BREAK			&&
			  token_type!=HCC_RBLOCK_KEY	&&
			  token_type!=HCC_EOF)
		{
			EmitStatement(function_ptr);
			//after each single-statement, must follow a ';' token
			//
			if(token_type==HCC_SEMICOLON)
				getToken();					  //skip the ';' token
		}
		//
		if(token_type==HCC_BREAK)
		{
			getToken(); //break
			getToken(); // then a ';' token
		}

		//save the stmt-block's assembly code in the queue...
		__tstring stmt = str_default_ptr->str();
		block_stmt.push(stmt);

		assert(def_switch_block_ptr!=NULL);		
		//to be used in default statement...
		*def_switch_block_ptr = __default_block_stmt_index;
	}
	
	//restore the unit file ptr
	unit().attach(unit_file_ptr);
}

void HCCCodeGenerator::emitTryBlock(Symbol *function_ptr)
{
	//TODO:future impl. will use SEH (Structured Exception Handling), 
	//to have a similar impl of the C++ exception handling mechanism...
	CPUInstruction jmp(this, JMP);
	CPUInstruction jnz(this, JNZ);

	CPUInstruction push(this, PUSH);
	CPUInstruction pop(this, POP);
	CPUInstruction mov(this, MOV);
	CPUInstruction cmp(this, CMP);


	unsigned int label_end_try_stmt_index		= SYMBOL_TABLE::__asm_label_index++;

	unsigned int __except_handler_index			= SYMBOL_TABLE::__asm_label_index++;

	unsigned int __catch_handler_index			= SYMBOL_TABLE::__asm_label_index++;

	unsigned int label_end_catch_stmt_index		= SYMBOL_TABLE::__asm_label_index++;

	__tstring label__except_handlerXXX		= CreateLabel(STRUCTURED_EXCEPTION_HANDLING_PREFIX, __except_handler_index);

	__tstring label_end_try_stmt			= CreateLabel(STMT_LABEL_PREFIX, label_end_try_stmt_index);
	__tstring label_catch_handler			= CreateLabel(STMT_LABEL_PREFIX, __catch_handler_index);
	__tstring label_end_catch_stmt			= CreateLabel(STMT_LABEL_PREFIX, label_end_catch_stmt_index);
	
	//try {
	getToken(); //skip the 'try' keyword

	push.set_new_comment(_T("pPrevESP"));
	push(ESP);
	push.set_new_comment(_T("pPrevEBP"));
	push(EBP);
	push(VAR_OFFSET, label__except_handlerXXX);
	ASSUME assume(this, FS, ASSUME_NOTHING);
	push(dword_ptr(FS, 0));
	mov(dword_ptr(FS, 0), ESP);
	//
	//EmitStatementList(HCC_RBLOCK_KEY, function_ptr); //this statement expects something else
	emitCompoundStatement(function_ptr);
	//
	jmp(VAR_OFFSET, label_end_try_stmt);

	EmitSEHStatementLabel(__except_handler_index); //the catch block
	//
	/*
__except_handlerXXX:

EXCEPTION_DISPOSITION __cdecl _except_handler (
    __in struct _EXCEPTION_RECORD *_ExceptionRecord,
    __in void * _EstablisherFrame,
   __inout struct _CONTEXT *_ContextRecord,
    __inout void * _DispatcherContext
    );
	*/
	mov.set_new_comment("_ExceptionRecord");
	mov(EBX, dword_ptr(ESP, __plus, sizeof(int)));
	mov.set_new_comment("_ExceptionRecord->ExceptionCode // The reason the exception occurred.");
	mov(ECX, dword_ptr(EBX));
	//
	mov(EBX, dword_ptr(FS, 0));
	mov(ESP, dword_ptr(EBX));
	pop(dword_ptr(FS, 0));
	mov.set_new_comment(_T("the prev EBP"));
	mov(EBP, dword_ptr(ESP, __plus, sizeof(int)));
	mov.set_new_comment(_T("the prev ESP"));
	mov(ESP, dword_ptr(ESP, __plus, sizeof(int) * 2));
	//
	do{
		getToken(); //skip the 'catch' token	

		volatile unsigned int label_seh_next_handler_index	= SYMBOL_TABLE::__asm_label_index++;

		__tstring label_seh_next_handler_stmt	= CreateLabel(STMT_LABEL_PREFIX, label_seh_next_handler_index);

		//EmitExprList(function_ptr); //the expr in catch(<expr>)
		EmitStatement(function_ptr); //the idea is to generate annotation for exception expressions

		if(is_operand_in_stack)
		{
			EmitPopOperand(stack_operand_type);
			is_operand_in_stack = false;
		}

		cmp(EAX, ECX);
		jnz(VAR_OFFSET, label_seh_next_handler_stmt);

		//EmitStatementList(HCC_RBLOCK_KEY, function_ptr); //this statement expects something else
		emitCompoundStatement(function_ptr);	//catch(<expr>){}

		jmp(VAR_OFFSET, label_end_catch_stmt);

		EmitStatementLabel(label_seh_next_handler_index);

		if(HCC_CATCH!=token_type)
			jmp(VAR_OFFSET, label_end_catch_stmt);
		//
	}while(HCC_CATCH==token_type);

	EmitStatementLabel(label_end_try_stmt_index);

	if(is_operand_in_stack)
	{
		EmitPopOperand(stack_operand_type);
		is_operand_in_stack = false;
	}

	mov(ESP, dword_ptr(FS, 0));
	pop(dword_ptr(FS, 0));
	mov.set_new_comment(_T("the prev EBP"));
	mov(EBP, dword_ptr(ESP, __plus, sizeof(int)));
	mov.set_new_comment(_T("the prev ESP"));
	mov(ESP, dword_ptr(ESP, __plus, sizeof(int) * 2));

	EmitStatementLabel(label_end_catch_stmt_index);
}

void HCCCodeGenerator::emitWithStatement(Symbol *function_ptr)
{
	//a great feature as a language statement; is very object oriented!
	getToken();	//keyword 'with'
	getToken();	//'('
	//the object identifier...
	Symbol* object_ptr = symbol_ptr;
	getToken(); //')'
	getToken(); //'{'
	if(bSourceAnnotation)
	{
		EmitComment(_T("{\n"));
	}
	while(token_type==HCC_PERIOD)
	{
		//
		TypeSpecifier* result_type_ptr = 
							emitObjectInstanceMember(object_ptr, NULL, function_ptr);
		//
		if(token_type==HCC_SEMICOLON)
			getToken();		//skip the ';' token
	}
	getToken(); //'}'

	if(bSourceAnnotation)
	{
		EmitComment(_T("}\n"));
	}
}

void HCCCodeGenerator::EmitAddressFromAddressOf(Symbol *symbl_ptr, 
												Symbol *function_ptr, 
												bool bEmitAssignment, 
												TypeSpecifier* target_type, 
												x86_INSTRUCTION __xInst,
												x86_REGISTER __AReg,
												x86_REGISTER __ARegHigh,
												bool bSetThisPointer)
{
	assert(symbl_ptr!=NULL);
	if(symbl_ptr==NULL)
		return;	

	CPUInstruction mov(this, MOV);
	CPUInstruction lea(this, LEA);
	FPUInstruction fstp(this, FSTP);
	CPUInstruction xInst(this, __xInst);

	int offset = symbl_ptr->getDeclDefinition().user_data.offset;

	switch(symbl_ptr->getDeclDefinition().identifier_type())
	{
	case DECL_PARAM_BYREF:			//params byref contain the address of a variable
	case DECL_PARAM_CONST_BYREF:

	case DECL_PARAM_POINTER:		//all of this are pointers; so also contain the address of a variable
	case DECL_PARAM_CONST_POINTER:			

	case DECL_PARAM_ARRAY:
	case DECL_PARAM_CONST_ARRAY:	
		{
			//a param byref or pointer is passed to a function as an address in the stack;
			//therefore, this must be moved before pushing it again onto stack.

			//func1(&i); type func(type* p) { *p = expr;}
			TypeSpecifier* type_ptr = symbl_ptr->getTypeSpecifier().getBaseTypeSpec();
			x86_REGISTER ctx_reg = EBX;
			//
			if(bSetThisPointer && type_ptr!=NULL)
			{
				if(type_ptr->specifier()==DSPEC_CLASS)
					ctx_reg = ECX;
			}

			if(symbl_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				TCHAR __local_op = offset > 0 ? __plus : __minus;
				//T H E   V A L U E   I S   A N   A D D R E S S 
				mov(ctx_reg, dword_ptr(EBP, __local_op, offset));	
			}else{
				//T H E   V A L U E   I S   A N   A D D R E S S 
				mov(ctx_reg, VAR_OFFSET, getSymbolLabel(symbl_ptr));
			}

			if(!bEmitAssignment && ctx_reg == EBX)
				mov(EAX, EBX);

			//in EBX, we have now the argument address, instead of the parameter address [EBP+offset]
			/*
			   some examples:

				void funct1(int^ ptr, int ref aref, int value, int [] array)
				{
					*ptr = 100;

					aref = 200;
				}

				for param ptr: 
						mov EBX, dword ptr [ebp+8];
						

				for param aref:
						mov EBX, dword ptr [ebp+0Ch];
						

				for param value:
						lea EBX, dword ptr [ebp+10h]
						

				for param array:
						mov EBX, dword ptr [ebp+14h]
						
			*/

			if(bEmitAssignment)
			{
				if(target_type->getDataType()==HCC_FLOATING_POINT)
				{
					fstp(qword_ptr(ctx_reg));					
				}
				else if(target_type==HccTypeChecker::ts_type_Int64_ptr)
				{
					xInst(dword_ptr(ctx_reg), __AReg);						
					xInst(dword_ptr(ctx_reg, sizeof(int)), __ARegHigh);						
					//
				}else if(target_type==HccTypeChecker::ts_type_char_ptr ||
						target_type==HccTypeChecker::ts_type_boolean_ptr ||
						target_type->getDataTypeSize()==sizeof(char))
				{
					xInst(byte_ptr(ctx_reg), getReg8From(__AReg));
				}else if(target_type==HccTypeChecker::ts_type_short_ptr || 
						 target_type->getDataTypeSize()==sizeof(short))
				{
					xInst(word_ptr(ctx_reg), getReg16From(__AReg));
				}else //for all user data types, scalars and strings if(target_type->is_scalar() || target_type->getDataType()==HCC_STRING_TYPE)
				{
					xInst(dword_ptr(ctx_reg), __AReg);
				}
			}
		}
		break;
		//BEGIN - FIXED Jan 10, 2009
	case DECL_POINTER_VARIABLE:
		{
			//a pointer variable, is a local variable that contains the address of another variable

			TypeSpecifier* type_ptr = symbl_ptr->getTypeSpecifier().getBaseTypeSpec();
			x86_REGISTER ctx_reg = EBX;
			//
			if(bSetThisPointer && type_ptr!=NULL)
			{
				if(type_ptr->specifier()==DSPEC_CLASS)
					ctx_reg = ECX;
			}

			if(symbl_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				TCHAR __local_op = offset > 0 ? __plus : __minus;
				//T H E   V A L U E   I S   A N   A D D R E S S 
				if(false==bEmitAssignment)
					mov(ctx_reg, dword_ptr(EBP, __local_op, abs(offset)));
				else
					lea(ctx_reg, dword_ptr(EBP, __local_op, abs(offset)));	
			}else{
				//T H E   V A L U E   I S   A N   A D D R E S S 
				if(false==bEmitAssignment)
					mov(ctx_reg, VAR_OFFSET, getSymbolLabel(symbl_ptr));
				else
					lea(ctx_reg, VAR_OFFSET, getSymbolLabel(symbl_ptr));
			}
			
			if(!bEmitAssignment && ctx_reg == EBX)
				mov(EAX, EBX);			

			if(bEmitAssignment)
			{
				if(target_type->getDataType()==HCC_FLOATING_POINT)
				{
					fstp(qword_ptr(ctx_reg));					
				}
				else if(target_type==HccTypeChecker::ts_type_Int64_ptr)
				{
					xInst(dword_ptr(ctx_reg), __AReg);						
					xInst(dword_ptr(ctx_reg, sizeof(int)), __ARegHigh);						
					//
				}else if(target_type==HccTypeChecker::ts_type_char_ptr ||
						target_type==HccTypeChecker::ts_type_boolean_ptr ||
						target_type->getDataTypeSize()==sizeof(char))
				{
					xInst(byte_ptr(ctx_reg), getReg8From(__AReg));							
				}else if(target_type==HccTypeChecker::ts_type_short_ptr ||
						target_type->getDataTypeSize()==sizeof(short))
				{
					xInst(word_ptr(ctx_reg), getReg16From(__AReg));
				}else //for all user data types, scalars and strings if(target_type->is_scalar() || target_type->getDataType()==HCC_STRING_TYPE)
				{
					xInst(dword_ptr(ctx_reg), __AReg);
				}
			}
		}
		//END - FIXED Jan 10, 2009
		break;
	case DECL_NEW_DATA_MEMBER:
		{
			if(symbl_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
			{
				EmitLoadObjectInstancePointer_This(function_ptr);
				//the data member...
				//BEGIN - OPTIMIZATION , Dec 22 2008
				if(false==bEmitAssignment)
				{
					if(offset > 0 )					
						lea(EBX, dword_ptr(ECX, __plus, offset));
					else
						lea(EBX, dword_ptr(ECX));
				}else{
					if(target_type->getDataType()==HCC_FLOATING_POINT)
					{
						if(offset > 0 )
							fstp(qword_ptr(ECX, __plus, offset));
						else
							fstp(qword_ptr(ECX));
					}else{
						if(target_type==HccTypeChecker::ts_type_Int64_ptr)
						{
							xInst(dword_ptr(ECX, __plus, (int)abs((int)offset)), __AReg);						//low
							xInst(dword_ptr(ECX, __plus, (int)abs((int)offset + (int)sizeof(int))), __ARegHigh);	//high
							
						}else if(target_type==HccTypeChecker::ts_type_char_ptr ||
								target_type==HccTypeChecker::ts_type_boolean_ptr ||
								target_type->getDataTypeSize()==sizeof(char))
						{
							if(offset > 0 )					
								xInst(byte_ptr(ECX, __plus, offset), getReg8From(__AReg));
							else
								xInst(byte_ptr(ECX), getReg8From(__AReg));
								
						}else if(target_type==HccTypeChecker::ts_type_short_ptr ||
								target_type->getDataTypeSize()==sizeof(short))
						{
							if(offset > 0 )					
								xInst(word_ptr(ECX, __plus, offset), getReg16From(__AReg));
							else
								xInst(word_ptr(ECX), getReg16From(__AReg));
						
						}else //for all user data types, scalars and strings if(target_type->is_scalar() || target_type->getDataType()==HCC_STRING_TYPE)
						{
							if(offset > 0 )					
								xInst(dword_ptr(ECX, __plus, offset), __AReg);
							else
								xInst(dword_ptr(ECX), __AReg);						
						}
					}
				}
				//END - OPTIMIZATION , Dec 22 2008
			}else{
				//BEGIN - FIXED Dec 28, 2008
				if(false==bEmitAssignment)
					lea(EBX, VAR_OFFSET, getSymbolLabel(symbl_ptr));
				else
				{
					if(target_type->getDataType()==HCC_FLOATING_POINT)
					{
						if(is_floating_point_in_fpu)
							fstp(qword_ptr(getSymbolLabel(symbl_ptr)));
						else{
							if(NULL==lastConstValueInfo.type_ptr)
							{
								//xInst(no_op_ptr(getSymbolLabel(symbl_ptr)), __ARegHigh); //high
								xInst(dword_ptr(getSymbolLabel(symbl_ptr)), __ARegHigh); //high: FIXED - hlm Nov 26, 2012
								__tstring LOW_PART = __tstring(_T("["))		+ 
													getSymbolLabel(symbl_ptr)	+ 
													__tstring(QWORD_OFFSET_LOW) + 
													__tstring(_T("]"));
								//xInst(no_op_ptr(LOW_PART), __AReg); //low
								xInst(dword_ptr(LOW_PART), __AReg); //low: FIXED - hlm Nov 26, 2012
								
							}else{
								xInst(no_op_ptr(getSymbolLabel(symbl_ptr)), lastConstValueInfo.edxVal); //high
								__tstring LOW_PART = __tstring(_T("["))		+ 
													getSymbolLabel(symbl_ptr)	+ 
													__tstring(QWORD_OFFSET_LOW) + 
													__tstring(_T("]"));
								xInst(no_op_ptr(LOW_PART), lastConstValueInfo.eaxVal); //low
							}
						}
					}else{
						if(NULL==lastConstValueInfo.type_ptr)
						{
							xInst(no_op_ptr(getSymbolLabel(symbl_ptr)), __AReg); //low
							if(target_type==HccTypeChecker::ts_type_Int64_ptr)
							{
								__tstring HIGH_PART = __tstring(_T("["))		+ 
													getSymbolLabel(symbl_ptr)	+ 
													__tstring(QWORD_OFFSET_HIGH) + 
													__tstring(_T("]"));
								xInst(no_op_ptr(HIGH_PART), __ARegHigh); //high
							}
						}else{
							xInst(no_op_ptr(getSymbolLabel(symbl_ptr)), lastConstValueInfo.eaxVal); //low
							if(target_type==HccTypeChecker::ts_type_Int64_ptr)
							{
								__tstring HIGH_PART = __tstring(_T("["))		+ 
													getSymbolLabel(symbl_ptr)	+ 
													__tstring(QWORD_OFFSET_HIGH) + 
													__tstring(_T("]"));
								xInst(no_op_ptr(HIGH_PART), lastConstValueInfo.edxVal); //high
							}
						}
					}
				}
				//END - FIXED Dec 28, 2008
			}
		}
		break;
	case DECL_PARAM_VALUE: //FIXED - Jan 3, 2009
	case DECL_VARIABLE:		
		{
			TCHAR __local_op = symbl_ptr->getDeclDefinition().identifier_type()==DECL_VARIABLE ? __minus : __plus;
			//BEGIN - OPTIMIZATION , Dec 22 2008
			if(false==bEmitAssignment)
			{
				if(symbl_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
					lea(EBX, dword_ptr(EBP, __local_op, abs(offset)));
				else 
					lea(EBX, VAR_OFFSET, getSymbolLabel(symbl_ptr));
			}else{
				if(symbl_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
				{
					if(target_type->getDataType()==HCC_FLOATING_POINT)
					{
						fstp(qword_ptr(EBP, __local_op, abs(offset)));

					}else if(target_type==HccTypeChecker::ts_type_Int64_ptr)
					{
						emitAssignInt64ToAddress(true, __local_op, offset, __xInst, __ARegHigh, __AReg);
					}else if(target_type==HccTypeChecker::ts_type_char_ptr ||
							target_type==HccTypeChecker::ts_type_boolean_ptr ||
							target_type->getDataTypeSize()==sizeof(char))
					{	
						xInst(byte_ptr(EBP, __local_op, abs(offset)), getReg8From(__AReg));
					}else if(target_type==HccTypeChecker::ts_type_short_ptr ||
							target_type->getDataTypeSize()==sizeof(short))
					{
						xInst(word_ptr(EBP, __local_op, abs(offset)), getReg16From(__AReg));
					}else  //for all user data types, scalars and strings if(target_type->is_scalar() || target_type->getDataType()==HCC_STRING_TYPE)
					{
						xInst(dword_ptr(EBP, __local_op, abs(offset)), __AReg);
					}
				}else {
					//G L O B A L   V A R I A B L E S
					if(target_type->getDataType()==HCC_FLOATING_POINT)
					{
						if(is_floating_point_in_fpu)
							fstp(qword_ptr(getSymbolLabel(symbl_ptr)));
						else{
							if(NULL==lastConstValueInfo.type_ptr)
							{
								//xInst(no_op_ptr(getSymbolLabel(symbl_ptr)), __ARegHigh); //high
								xInst(dword_ptr(getSymbolLabel(symbl_ptr)), __ARegHigh); //high: FIXED - hlm Nov 26, 2012

								__tstring LOW_PART = __tstring(_T("["))		+ 
													getSymbolLabel(symbl_ptr)	+ 
													__tstring(QWORD_OFFSET_LOW) + 
													__tstring(_T("]"));
								//xInst(no_op_ptr(LOW_PART), __AReg); //low
								xInst(dword_ptr(LOW_PART), __AReg); //low: FIXED - hlm Nov 26, 2012
								
							}else{
								xInst(no_op_ptr(getSymbolLabel(symbl_ptr)), lastConstValueInfo.edxVal); //high
								__tstring LOW_PART = __tstring(_T("["))		+ 
													getSymbolLabel(symbl_ptr)	+ 
													__tstring(QWORD_OFFSET_LOW) + 
													__tstring(_T("]"));
								xInst(no_op_ptr(LOW_PART), lastConstValueInfo.eaxVal); //low
							}
						}
					}else{
						if(NULL==lastConstValueInfo.type_ptr)
						{
							xInst(no_op_ptr(getSymbolLabel(symbl_ptr)), __AReg); //low
							if(target_type==HccTypeChecker::ts_type_Int64_ptr)
							{
								__tstring HIGH_PART = __tstring(_T("["))		+ 
													getSymbolLabel(symbl_ptr)	+ 
													__tstring(QWORD_OFFSET_HIGH) + 
													__tstring(_T("]"));
								xInst(no_op_ptr(HIGH_PART), __ARegHigh); //high
							}
						}else{
							xInst(no_op_ptr(getSymbolLabel(symbl_ptr)), lastConstValueInfo.eaxVal); //low
							if(target_type==HccTypeChecker::ts_type_Int64_ptr)
							{
								__tstring HIGH_PART = __tstring(_T("["))		+ 
													getSymbolLabel(symbl_ptr)	+ 
													__tstring(QWORD_OFFSET_HIGH) + 
													__tstring(_T("]"));
								xInst(no_op_ptr(HIGH_PART), lastConstValueInfo.edxVal); //high
							}
						}
					}
				}
			}
			//END - OPTIMIZATION , Dec 22 2008
		}
		break;
	default:
		{
			lea(EBX, VAR_OFFSET, getSymbolLabel(symbl_ptr));
		}
		break;
	};
}

void HCCCodeGenerator::EmitLoadObjectInstancePointer_This(Symbol *function_ptr)
{
	CPUInstruction mov(this, MOV);
	assert(function_ptr!=NULL);
	if(function_ptr==DATA_MEMBERS_INIT_PROC_ADDRESS)
	{
		//the this pointer saved in the unique local variable for this init function...
		mov(ECX, dword_ptr(EBP, __minus, sizeof(int)));
	}else if(function_ptr!=NULL)
	{
		//the variable offset where to look for the this pointer...
		volatile int this_ptr_localv_offset =
				function_ptr->getDeclDefinition().function.locals.stack_variables.total_variables_size;
		//the this pointer saved at the last local variable for this function_ptr
		mov(ECX, dword_ptr(EBP, __minus, this_ptr_localv_offset));
	}
}

void HCCCodeGenerator::emitDebuggerCall()
{
	if(token_type==HCC_DEBUGGER)
		getToken();

	CPUInstruction _int(this, INT_);

	_int.set_new_comment(_T("User breakpoint"));
	_int(0x3);
}

void HCCCodeGenerator::EmitPropertyPutCall(TypeSpecifier *target_type, 
										   Symbol* object_ptr, 
										   Symbol* function_ptr, 
										   bool bIsAddressOnStack)
{
	CPUInstruction push(this, PUSH);
	CPUInstruction pop(this, POP);
	CPUInstruction call(this, CALL);

	FPUInstruction fstp(this, FSTP);

	if(!is_operand_in_stack)
	{
		if(bIsAddressOnStack)
		{
			pop(ECX);
		}
		//push param onto stack...
		if(target_type==HccTypeChecker::ts_type_double_ptr || 
			target_type==HccTypeChecker::ts_type_float_ptr)
		{
			//EDX:EAX
			push(EAX);
			push(EDX);

			goto __LAST_X_FLOATING_POINT;

		}else if(target_type==HccTypeChecker::ts_type_Int64_ptr)
		{
			//FIXED - Dec 18, 2008
			//EAX:EDX
			push(EDX);
			push(EAX);
		}else
			push(EAX);
	}else{
		if(target_type==HccTypeChecker::ts_type_double_ptr || 
			target_type==HccTypeChecker::ts_type_float_ptr)
		{
__LAST_X_FLOATING_POINT:
			if(is_floating_point_in_fpu)
			{
				fstp(qword_ptr(ESP));
				is_floating_point_in_fpu = false;
			}

			if(bIsAddressOnStack)
			{
				EmitPopOperand(target_type);
				pop(ECX);
				EmitPushOperand(target_type);
			}
		}
	}
	is_operand_in_stack = false;

	//load the address
	Symbol* prop_put_ptr = stack_of_ids.top();
	stack_of_ids.pop();
	__tstring prop_label = getLabelFromName(prop_put_ptr->getCompleteName());			

	if(object_ptr!=NULL)
		EmitLoadValueAddressOf(object_ptr, function_ptr, true);
	else if(bIsAddressOnStack==false)
	{
		EmitLoadObjectInstancePointer_This(function_ptr);
	}

	//call this member function...
	call(no_op_ptr(prop_label));
}

void HCCCodeGenerator::emit_InlineMin(Symbol *caller_function_ptr)
{	
	CPUInstruction push(this, PUSH);
	CPUInstruction pop(this, POP);
	CPUInstruction sub(this, SUB);
	CPUInstruction sbb(this, SBB);
	CPUInstruction and(this, AND);
	CPUInstruction add(this, ADD);

	getToken(); // the '(' token

	EmitExprList(caller_function_ptr);

	if(!is_operand_in_stack)
	{
		push(EAX);
	}
	is_operand_in_stack = false;

	getToken();	// the ',' token

	EmitExprList(caller_function_ptr);

	getToken(); // the ')' token

	if(is_operand_in_stack)
	{
		pop(EAX);
		is_operand_in_stack = false;
	}
	pop(EBX);

	sub(EBX, EAX); // EBX = EBX - EAX
	sbb(ECX, ECX); // if(EBX < EAX) ECX = -1 else ECX = 0;
	and(ECX, EBX); // if(EBX < EAX) ECX = EBX else ECX = 0;
	add(EAX, ECX); // if(EBX < EAX) ((EAX = EAX + (EBX-EAX))==EBX)
}

void HCCCodeGenerator::emit_InlineMax(Symbol *caller_function_ptr)
{
	CPUInstruction setle(this, SETLE);
	CPUInstruction push(this, PUSH);
	CPUInstruction pop(this, POP);
	CPUInstruction sub(this, SUB);
	CPUInstruction cmp(this, CMP);
	CPUInstruction and(this, AND);
	CPUInstruction add(this, ADD);
	CPUInstruction xor(this, XOR);

	getToken(); // the '(' token

	EmitExprList(caller_function_ptr);

	if(!is_operand_in_stack)
	{
		push(EAX);
	}
	is_operand_in_stack = false;

	getToken();	// the ',' token

	EmitExprList(caller_function_ptr);

	getToken(); // the ')' token

	if(is_operand_in_stack)
	{
		pop(EAX);
		is_operand_in_stack = false;
	}
	pop(EBX);
	
	xor(ECX, ECX);	
	cmp(EBX, EAX);	// EBX - EAX < 0 ?
	setle(CL);		// if(EBX <= EAX) CL = 1 else CL = 0
	sub(ECX, 1);	// if(EBX <= EAX) ECX = 0 else ECX = 0xFFFFFFFFh
	sub(EBX, EAX);	// EBX = EBX - EAX;
	and(ECX, EBX);	// if(EBX <= EAX) ECX = 0 else ECX = EBX
	add(EAX, ECX);	// EAX = EAX + ECX
}

void HCCCodeGenerator::emit_SizeOf(Symbol *caller_function_ptr)
{
	CPUInstruction mov(this, MOV);
	CPUInstruction and(this, AND);
	if(token_type==HCC_LPAREN)		// (
		getToken();
	/*the next token, must be a HCC_IDENTIFIER token, and one of the following known types:
		symbol-decl: 
			DECL_VARIABLE : 
					for types others than arrays : base_type_spec_ptr->getDataTypeSize();
					for arrays		: getArraySizeFromType(type-spec);
			DECL_CONSTANT			: base_type_spec_ptr->getDataTypeSize()

			DECL_POINTER_VARIABLE	: sizeof(int)
			DECL_NEW_TYPE			: getTypeSpecifier().getDataTypeSize()
			DECL_BUILDIN_TYPE		: getTypeSpecifier().getDataTypeSize()
			
	*/
	TypeSpecifier* base_type_spec_ptr = symbol_ptr->getTypeSpecifier().getBaseTypeSpec();

	switch(symbol_ptr->getDeclDefinition().identifier_type())
	{
	case DECL_VOID:
		{
			and(EAX, 0);
		}
		break;
	case DECL_VARIABLE:
	case DECL_CONSTANT:
	case DECL_PARAM_VALUE:		//param by value
	case DECL_PARAM_BYREF:		//param by reference
	case DECL_PARAM_CONST_BYREF://const param by reference


	case DECL_PARAM_ARRAY:			//one-dimension param array
	case DECL_PARAM_CONST_ARRAY:	//one-dimension const param array

		{
			if(symbol_ptr->getTypeSpecifier().specifier()!=DSPEC_ARRAY)
			{				
				if(base_type_spec_ptr->specifier()==DSPEC_ENUM)
					mov(EAX, (int)sizeof(int));
				else
					mov(EAX, base_type_spec_ptr->getDataTypeSize());
			}else
			{
				//A R R A Y   V A R I A B L E S 
				if(symbol_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
					mov(EAX, getArraySizeFromType(base_type_spec_ptr));
				else
					mov(EAX, VAR_SIZEOF, getSymbolLabel(symbol_ptr));
			}
		}
		break;
	case DECL_POINTER_VARIABLE:
	case DECL_PARAM_POINTER:			//param pointer
	case DECL_PARAM_CONST_POINTER:		//const param pointer
		{
			mov(EAX, (int)sizeof(int)); //32-bits
		}
		break;
	case DECL_BUILDIN_TYPE:				//for all the compiler build-in types
	case DECL_NEW_TYPE:					//for class and struct keywords (will be the same behavior for now)
	case DECL_NEW_ABSTRACT_TYPE:		//for abstract class and struct keywords
		{
			mov(EAX, symbol_ptr->getTypeSpecifier().getDataTypeSize());
		}
		break;
	};

	getToken(); //skip the identifier

	if(token_type==HCC_RPAREN)		// )
		getToken();
}

void HCCCodeGenerator::emitPushInteger(TypeSpecifier* source_type, bool bLoadInFPU)
{
	//BEGIN - FIXED - Dec 18, 2008
	CPUInstruction push(this, PUSH);
	FPUInstruction fild(this, FILD);
	if(!is_operand_in_stack)
	{
		//Floating Point uses EDX:EAX in the stack, while Int64 uses EAX:EDX in the stack
		stack_operand_type = HccTypeChecker::ts_type_Int32_ptr;
		if(source_type==HccTypeChecker::ts_type_Int64_ptr){
			push(EDX);
			stack_operand_type = HccTypeChecker::ts_type_Int64_ptr;
		}
		push(EAX);
	}
	if(bLoadInFPU)
	{
		if(source_type==HccTypeChecker::ts_type_Int64_ptr)
			fild(qword_ptr(ESP));
		else
			fild(dword_ptr(ESP));
	}
	is_operand_in_stack = true;	
	//END - FIXED - Dec 18, 2008
}

void HCCCodeGenerator::emitAssignInt64ToAddress(bool bIsLocal, TCHAR op, int offset, x86_INSTRUCTION __xInst, x86_REGISTER __regHigh, x86_REGISTER __regLow)
{
	CPUInstruction xInst(this, __xInst);

	if(false==bIsLocal)
	{
		xInst(dword_ptr(EBX), __regLow);				//low
		xInst(dword_ptr(EBX, sizeof(int)), __regHigh);	//high
	}else{
		if(NULL==lastConstValueInfo.type_ptr)
		{
			xInst(dword_ptr(EBP, op, (int)abs((int)offset)), __regLow);					//low
			xInst(dword_ptr(EBP, op, (int)abs((int)offset + (int)sizeof(int))), __regHigh);	//high
		}else{
			xInst(dword_ptr(EBP, op, (int)abs((int)offset)), lastConstValueInfo.eaxVal);					//low
			xInst(dword_ptr(EBP, op, (int)abs((int)offset + (int)sizeof(int))), lastConstValueInfo.edxVal);	//high
		}
	}
}

void HCCCodeGenerator::emitLoadInt64ToRegisters(bool bIsLocal, TCHAR op, int offset, x86_REGISTER __regHigh, x86_REGISTER __regLow)
{
	CPUInstruction mov(this, MOV);

	if(false==bIsLocal)
	{
		mov(__regHigh, dword_ptr(EBX, sizeof(int)));	//high
		mov(__regLow, dword_ptr(EBX));					//low
	}else{
		mov(__regLow, dword_ptr(EBP, op, (int)abs((int)offset)));					//low
		mov(__regHigh, dword_ptr(EBP, op, (int)abs((int)offset + (int)sizeof(int))));	//high
	}
}
TypeSpecifier* HCCCodeGenerator::emitDynamicCastOperator(Symbol* target_ptr, 
														 Symbol* function_ptr, 
														 bool bIsAddressOnStack,
														 bool bIsPropertyPut,
														 Symbol* object_ptr,
														 Symbol** pDynamicClassSymbol,
														  Symbol** pObjectVarSymbol)
{
	assert(token_type==HCC_DYNAMIC_CAST);

	CPUInstruction pop(this, POP);
	CPUInstruction mov(this, MOV);
	getToken(); //dynamic_cast

	getToken(); //(
	//BEGIN - FIXED - Feb 22, 2009
	TypeSpecifier* pItemType = NULL;
	
	if(target_ptr!=NULL)
		pItemType = target_ptr->getTypeSpecifier().getBaseTypeSpec();

	Symbol* pSymbol = NULL;
	bool bAddressLoaded = false;
	if(token_type==HCC_POINTER_ADDRESSOF)
	{
		if(NULL==pObjectVarSymbol)
		{
			emitAddressOf(function_ptr, pItemType, &pSymbol);
			bAddressLoaded = true;
		}else{
			getToken(); //&
			pSymbol = symbol_ptr;
		}
	}else
		pSymbol = symbol_ptr; //could be a: type-spec | pointer

	if(false==bAddressLoaded)
	{
		//in this case, we must take out this symbol, before continue...
		getToken(); //type-spec | var
	}

	if(pDynamicClassSymbol!=NULL)
		*pDynamicClassSymbol = pSymbol;

	if(pObjectVarSymbol!=NULL)
		*pObjectVarSymbol = pSymbol;
	else if(false==bAddressLoaded)
	{
		DECLARATION_TYPE declType = pSymbol->getDeclDefinition().identifier_type();

		if(declType==DECL_NEW_TYPE || declType==DECL_NEW_ABSTRACT_TYPE)
		{
			//the user-specified type to convert to...
			pItemType = &pSymbol->getTypeSpecifier();
			//
		}else
			EmitLoadValueAddressOf(pSymbol, function_ptr);
	}
	//END - FIXED - Feb 22, 2009

	if(token_type==HCC_COMMA_OP)
	{
		getToken(); //,
		//BEGIN - FIXED - Feb 22, 2009
		bAddressLoaded = false;
		if(token_type==HCC_POINTER_ADDRESSOF)
		{
			if(NULL==pObjectVarSymbol)
			{
				emitAddressOf(function_ptr, pItemType, &pSymbol);
				bAddressLoaded = true;
			}else{
				getToken(); //&
				pSymbol = symbol_ptr;
				//getToken(); //var
			}
		}else{
			pSymbol = symbol_ptr;
			//getToken(); //var
		}

		if(false==bAddressLoaded)
		{
			getToken(); //var
		}

		if(pObjectVarSymbol!=NULL)
			*pObjectVarSymbol = pSymbol;
		else if(false==bAddressLoaded)
		{
			EmitLoadValueAddressOf(pSymbol, function_ptr);
			//in this case, we must take out this symbol, before continue...
		}
		//END - FIXED - Feb 22, 2009
	}
	getToken(); //)

	if(target_ptr!=NULL)
	{
		if(bIsPropertyPut)
		{
			EmitPropertyPutCall(HccTypeChecker::ts_type_int_ptr, 
								object_ptr, 
								function_ptr, 
								bIsAddressOnStack); //assign to an object property...
			//
		}else if(bIsAddressOnStack)
		{
			pop(EBX);
			mov(dword_ptr(EBX), EAX);
			bIsAddressOnStack = false;
		}else if(!stack_of_ids.empty())
		{
			//if on the compiler stack, load address before assignment...
			Symbol* id_ptr = stack_of_ids.top();
			stack_of_ids.pop();
			assert(id_ptr==target_ptr);
			//
			//BEGIN - FIXED Dec 23, 2008
			EmitAddressFromAddressOf(id_ptr, function_ptr, true, HccTypeChecker::ts_type_int_ptr, MOV, EAX); //result in EBX
			//END - FIXED Dec 23, 2008
			//
		}else if(target_ptr->getDeclDefinition().identifier_scope()==SCOPE_LOCAL)
		{			
			int offset = target_ptr->getDeclDefinition().user_data.offset;
			TCHAR op = __minus;
			if(offset > 0 )
				op = __plus;				
			//do assign...
			mov(dword_ptr(EBP, op, abs(offset)), EAX);
		}else				
			//do assign...
			mov(no_op_ptr(getSymbolLabel(target_ptr)), EAX);
	}

	return pItemType;
}

void HCCCodeGenerator::EmitMultipleDestructorCall(TypeSpecifier* array_type_ptr)
{
	if(array_type_ptr==NULL)
		return;

	CPUInstruction mov(this, MOV);
	CPUInstruction lea(this, LEA);
	CPUInstruction cmp(this, CMP);
	CPUInstruction jnz(this, JNZ);
	CPUInstruction add(this, ADD);

	bool bHasDestructor = false;
	//save the unit file ptr
	__tostream* unit_file_ptr = unit().detach();

	__tostring_stream* str_multiple_dtor_stmt = new __tostring_stream();
	unit().attach(str_multiple_dtor_stmt);
	//
	unsigned int __destructor_call_loop_index = SYMBOL_TABLE::__asm_label_index++;

	__tstring __dtor_call_loop_label = CreateLabel(STMT_LABEL_PREFIX, __destructor_call_loop_index);
	//
	TypeSpecifier* pItemType = getArrayScalarType(array_type_ptr);

	if(array_type_ptr->array.bIsDynamicArray)
	{
		mov(ECX, dword_ptr(ESI, __minus, sizeof(int)));
		lea(EDI, dword_ptr(ESI, __plus, ECX));
		mov(EBX, ESI);
		EmitStatementLabel(__destructor_call_loop_index);
		//the actual dtor call...
		mov(ECX, EBX);
		EmitDestructorCall(NULL, NULL, pItemType, &bHasDestructor);
		//the next object in the array...
		add(EBX, pItemType->getDataTypeSize());
		cmp(EBX, EDI);
		jnz(VAR_OFFSET, __dtor_call_loop_label);
	}else{
		//more than 4 element objects in an object array, must build a destruction loop...
		int array_count = getArrayCountFromType(array_type_ptr);
		if(array_count > 4)
		{
			unsigned int __destructor_call_loop_index = SYMBOL_TABLE::__asm_label_index++;

			__tstring __dtor_call_loop_label = CreateLabel(STMT_LABEL_PREFIX, __destructor_call_loop_index);
			//
			lea(EDI, dword_ptr(ESI, getArraySizeFromType(array_type_ptr)));	
			mov(EBX, ESI);
			EmitStatementLabel(__destructor_call_loop_index);
			//the actual dtor call...
			mov(ECX, EBX);
			EmitDestructorCall(NULL, NULL, pItemType, &bHasDestructor);
			//the next object in the array...
			add(EBX, pItemType->getDataTypeSize());
			cmp(EBX, EDI);
			jnz(VAR_OFFSET, __dtor_call_loop_label);
		}else{
			//L O O P   U N R O L L I N G 
			mov(EBX, ESI);
			for(int index = 0; index < array_count; index++)
			{
				mov(ECX, EBX);
				EmitDestructorCall(NULL, NULL, pItemType, &bHasDestructor);
				if(false==bHasDestructor)
					break;
				//the next object in the array...
				if(index + 1 < array_count)
					add(EBX, pItemType->getDataTypeSize());
			}
		}
	}

	__tstring dtor_calls = str_multiple_dtor_stmt->str();

	//restore the unit file ptr
	unit().attach(unit_file_ptr);

	//if we found a destructor in the item type-spec-class, we generate this code:
	if(bHasDestructor)
	{
		unit() << dtor_calls << endl;
	}
}

void HCCCodeGenerator::EmitMultipleConstructorCall(TypeSpecifier* array_type_ptr)
{
	if(array_type_ptr==NULL)
		return;

	if(array_type_ptr->array.bIsDynamicArray)
	{
		__asm int 3; //should never get here!
	}

	CPUInstruction mov(this, MOV);
	CPUInstruction lea(this, LEA);
	CPUInstruction cmp(this, CMP);
	CPUInstruction jnz(this, JNZ);
	CPUInstruction add(this, ADD);

	TypeSpecifier* pItemType = getArrayScalarType(array_type_ptr);
	assert(pItemType!=NULL);

	const __tstring& lpszType = pItemType->getTypeName();
	assert(lpszType.length() > 0);

	//O B J E C T   C O N S T R U C T O R
	//does this class has a constructor that we can call multiple times for each object instance?
	Symbol* ctor_ptr = pItemType->getSymbolTable()->find(lpszType);
	if(ctor_ptr!=NULL)
	{
		int array_count = getArrayCountFromType(array_type_ptr);
		//more than 4 element objects in an object array, must build a construction loop...
		if(array_count > 4)
		{
			unsigned int __constructor_call_loop_index = SYMBOL_TABLE::__asm_label_index++;

			__tstring __ctor_call_loop_label = CreateLabel(STMT_LABEL_PREFIX, __constructor_call_loop_index);
			//
			lea(EDI, dword_ptr(ESI, getArraySizeFromType(array_type_ptr)));	
			mov(EBX, ESI);
			EmitStatementLabel(__constructor_call_loop_index);
			//the actual dtor call...
			mov(ECX, EBX);
			EmitObjectConstructorCall(pItemType, NULL, NULL);
			//the next object in the array...
			add(EBX, pItemType->getDataTypeSize());
			cmp(EBX, EDI);
			jnz(VAR_OFFSET, __ctor_call_loop_label);
		}else{
			//L O O P   U N R O L L I N G 
			mov(EBX, ESI);
			for(int index = 0; index < array_count; index++)
			{
				mov(ECX, EBX);
				EmitObjectConstructorCall(pItemType, NULL, NULL);
				//the next object in the array...
				if(index + 1 < array_count)
					add(EBX, pItemType->getDataTypeSize());
			}
		}
	}
}

void HCCCodeGenerator::emitArrayAddressLoadFromSubscript(TypeSpecifier* pItemType, 
														 x86_INSTRUCTION x86Inst, 
														 x86_REGISTER regLow, 
														 x86_REGISTER regHigh)
{
	CPUInstruction imul(this, IMUL);
	CPUInstruction inst(this, x86Inst);
	CPUInstruction add(this, ADD);
	//BEGIN - OPTIMIZED
	int type_size = pItemType->getDataTypeSize();
	//the final address	: lea EAX, dword ptr [ESI+ECX*type_size]
	x86_REGISTER x86Dest = regLow;
	if(LEA==x86Inst)
	{
		if(type_size==sizeof(char) || 
			type_size==sizeof(short)||
			type_size==sizeof(int) || 
			type_size==sizeof(double))
		{
			inst(x86Dest, dword_ptr(ESI, __plus, EAX, __mult, type_size));
		}else{
			imul(EAX, type_size);
			inst(x86Dest, dword_ptr(ESI, __plus, EAX));
		}
	}else if(MOV==x86Inst)
	{
		//D O U B L E   P R E S C I S I O N
		if(pItemType->getDataType()==HCC_FLOATING_POINT)
		{
			inst(regHigh, dword_ptr(ESI, __plus, EAX, __mult, type_size));
			CPUInstruction lea(this, LEA);
			lea.set_new_comment(_T("low part"));
			lea(EAX, dword_ptr(EAX, __mult, type_size, __plus, sizeof(int)));
			inst(regLow, dword_ptr(ESI, __plus, EAX));
			//
		}else if(pItemType==HccTypeChecker::ts_type_Int64_ptr)
		{
			imul(EAX, type_size);
			inst(ECX, EAX);
			inst(regLow, dword_ptr(ESI, __plus, ECX));
			add.set_new_comment(_T("low part"));
			add(ECX, (int)sizeof(int));
			inst(regLow, dword_ptr(ESI, __plus, ECX));
		}else{
			CPUInstruction lea(this, LEA);
			//S I N G L E   P R E S C I S I O N   O R   C U S T O M   T Y P E S 
			//the final value	: mov EAX, dword ptr [ESI+ECX*type_size]
			if(type_size==sizeof(char)		|| 
				type_size==sizeof(short)	||
				type_size==sizeof(int))
			{
				//BEGIN - FIXED Jan 24, 2009
				if(pItemType->specifier()==DSPEC_CLASS)
					lea(ECX, dword_ptr(ESI, __plus, EAX, __mult, type_size));
				//END - FIXED Jan 24, 2009
				inst(x86Dest, dword_ptr(ESI, __plus, EAX, __mult, type_size));
			}else{
				imul(EAX, type_size);
				//BEGIN - FIXED Jan 24, 2009
				if(pItemType->specifier()==DSPEC_CLASS)
					lea(ECX, dword_ptr(ESI, __plus, EAX));
				//END - FIXED Jan 24, 2009
				inst(x86Dest, dword_ptr(ESI, __plus, EAX));
			}
		}
	}else
	{
		__asm int 3; should never get here!
	}
	//END - OPTIMIZED
	return;
}

TypeSpecifier* HCCCodeGenerator::emitArraySubscriptReadorWrite(int type_size, TypeSpecifier* arrayItemTypeSpec, Symbol* function_ptr)
{
	TypeSpecifier* result_type_ptr = arrayItemTypeSpec;
	if(token_type==HCC_PERIOD)
	{
		//lea(ECX, dword_ptr(ESI, __plus, ECX, __mult, type_size));
		emitArrayAddressLoadFromSubscript(arrayItemTypeSpec, LEA, ECX);
		//the this pointer is now in ECX as expected!
		return emitObjectInstanceMember(NULL, arrayItemTypeSpec, function_ptr);
	}else
	{
		switch(token_type)
		{
			case HCC_INCREMENT:		//postfix ++
			case HCC_DECREMENT:		//postfix --

			case HCC_ASSIGN_OP:			// =
			case HCC_INC_ASSIGN:		// +=
			case HCC_DEC_ASSIGN:		// -=
			case HCC_MUL_ASSIGN:		// *=
			case HCC_DIV_ASSIGN:		// /=
				//for integer types only
			case HCC_MOD_ASSIGN:		// %=
			case HCC_XOR_ASSIGN:		// ^=
			case HCC_BIT_OR_ASSIGN:		// |=
			case HCC_BIT_AND_ASSIGN:	// &=
			{
				//the final address	: lea EAX, dword ptr [ESI+ECX*type_size]
				//lea(EAX, dword_ptr(ESI, __plus, EAX, __mult, type_size));
				emitArrayAddressLoadFromSubscript(arrayItemTypeSpec);
			}
			break;
			default: //the value is loaded instead
			{				
				//the final value	: mov EAX, dword ptr [ESI+ECX*type_size]
				//mov(EAX, dword_ptr(ESI, __plus, ECX, __mult, type_size));		
				emitArrayAddressLoadFromSubscript(arrayItemTypeSpec, 
												  bRequiresAddressOf ? LEA : MOV,
												  bRequiresAddressOf && arrayItemTypeSpec->specifier()==DSPEC_CLASS ? ECX : EAX,
												  EDX);
			}
			break;
		}	
	}

	return result_type_ptr;
}

void HCCCodeGenerator::emitArrayAddressLoadGlobalFromSubscript(Symbol* array_var_syptr, 
															   int type_size, 
															   x86_INSTRUCTION x86Inst, 
															   x86_REGISTER regLow, x86_REGISTER regHigh)
{
	CPUInstruction imul(this, IMUL);
	CPUInstruction inst(this, x86Inst);
	CPUInstruction add(this, ADD);
	//BEGIN - OPTIMIZED
	x86_REGISTER x86Dest = regLow;
	if(LEA==x86Inst)
	{
		//the final address	: lea EAX, dword ptr [ESI+ECX*type_size]
		if(type_size==sizeof(char) || 
			type_size==sizeof(short)||
			type_size==sizeof(int) || 
			type_size==sizeof(double))
		{
			inst(x86Dest, dword_ptr(EAX, __mult, type_size, __plus, getSymbolLabel(array_var_syptr)));
		}else{
			imul(EAX, type_size);
			inst(x86Dest, dword_ptr(EAX, __plus, getSymbolLabel(array_var_syptr)));
		}
	}else if(MOV==x86Inst)
	{
		TypeSpecifier* pItemType = getArrayScalarType(array_var_syptr->getTypeSpecifier().getBaseTypeSpec());
		//D O U B L E   P R E S C I S I O N
		if(pItemType->getDataType()==HCC_FLOATING_POINT)
		{
			inst(regHigh, dword_ptr(EAX, __mult, type_size, __plus, getSymbolLabel(array_var_syptr)));
			CPUInstruction lea(this, LEA);
			lea.set_new_comment(_T("low part"));
			lea(EAX, dword_ptr(EAX, __mult, type_size, __plus, sizeof(int)));
			inst(regLow, dword_ptr(EAX, __plus, getSymbolLabel(array_var_syptr)));
			//
		}else if(pItemType==HccTypeChecker::ts_type_Int64_ptr)
		{
			imul(EAX, type_size);
			inst(ECX, EAX);
			inst(regLow, dword_ptr(ECX, __plus, getSymbolLabel(array_var_syptr)));
			add.set_new_comment(_T("low part"));
			add(ECX, (int)sizeof(int));
			inst(regHigh, dword_ptr(ECX, __plus, getSymbolLabel(array_var_syptr)));
		}else{
			CPUInstruction lea(this, LEA);
			//S I N G L E   P R E S C I S I O N   A N D   P O I N T E R S
			//the final value	: mov EAX, dword ptr [ESI+ECX*type_size]
			if(type_size==sizeof(char) || 
				type_size==sizeof(short)||
				type_size==sizeof(int))
			{
				//BEGIN - FIXED Jan 24, 2009
				if(pItemType->specifier()==DSPEC_CLASS)
					lea(ECX, dword_ptr(EAX, __mult, type_size, __plus, getSymbolLabel(array_var_syptr)));
				//END - FIXED Jan 24, 2009					
				inst(x86Dest, dword_ptr(EAX, __mult, type_size, __plus, getSymbolLabel(array_var_syptr)));
			}else{
				imul(EAX, type_size);
				//BEGIN - FIXED Jan 24, 2009
				if(pItemType->specifier()==DSPEC_CLASS)
					lea(ECX, dword_ptr(EAX, __plus, getSymbolLabel(array_var_syptr)));
				//END - FIXED Jan 24, 2009
				inst(x86Dest, dword_ptr(EAX, __plus, getSymbolLabel(array_var_syptr)));
			}
		}
	}else
	{
		__asm int 3; should never get here!
	}
	//END - OPTIMIZED
	return;
}

//--------------------------------------------------------------------------
//
//  address-of :: &identifier |
//                &array[subscript-expression]
//
//--------------------------------------------------------------------------
TypeSpecifier* HCCCodeGenerator::emitAddressOf(Symbol* function_ptr, TypeSpecifier* pItemType, Symbol** ppResultantSymbol)
{
	TypeSpecifier* result_type_ptr = HccTypeChecker::ts_type_unsigned_ptr;

	if(token_type==HCC_POINTER_ADDRESSOF)
		getToken(); //&
	else{
		__asm int 3; //should never get here!
		return NULL; //
	}

	//& <identifier>
	Symbol* symbl_ptr = symbol_ptr;
	//
	assert(symbl_ptr!=NULL);
	if(symbl_ptr==NULL)
	{
		__asm int 3; //should never get here!
		return NULL;
	}

	if(ppResultantSymbol!=NULL)
	{
		*ppResultantSymbol = symbl_ptr;
	}

	getToken(); //skip this symbol

	CPUInstruction lea(this, LEA);
	CPUInstruction mov(this, MOV);

	int offset = symbl_ptr->getDeclDefinition().user_data.offset;

	DECLARATION_TYPE declType = symbl_ptr->getDeclDefinition().identifier_type();

	switch(declType)
	{
	case DECL_PARAM_VALUE:
		{
			lea(EAX, dword_ptr(EBP, __plus, offset));
		}
		break;
	case DECL_PARAM_BYREF:			//params byref contain the address of a variable
	case DECL_PARAM_CONST_BYREF:
		{
			mov(EAX, dword_ptr(EBP, __plus, offset));
		}
		break;
	case DECL_PARAM_POINTER:		//all of this are pointers; so also contain the address of a variable
	case DECL_PARAM_CONST_POINTER:			
		{
			lea(EAX, dword_ptr(EBP, __plus, offset));
		}
		break;
	case DECL_VARIABLE:
	case DECL_POINTER_VARIABLE:

	case DECL_PARAM_ARRAY:
	case DECL_PARAM_CONST_ARRAY:
		{
			//first, we have to determine the type (simple, array, object)
			switch(symbl_ptr->getTypeSpecifier().getBaseTypeSpec()->specifier())
			{
			case DSPEC_SIMPLE:
			case DSPEC_CLASS:
				{
					assert(DECL_VARIABLE==declType);
					if(SCOPE_LOCAL==symbl_ptr->getDeclDefinition().identifier_scope())
						lea(EAX, dword_ptr(EBP, __minus, abs(offset)));
					else
						lea(EAX, VAR_OFFSET, getSymbolLabel(symbl_ptr));
				}
				break;
			case DSPEC_ARRAY:
				bRequiresAddressOf = true;
				pItemType = emitArraySubscript(symbl_ptr, NULL, pItemType, function_ptr);
				bRequiresAddressOf = false;
				if(pItemType->specifier()==DSPEC_CLASS)
				{
					mov(EAX, ECX);
				}
				break;
			};
		}
		break;
	case DECL_NEW_DATA_MEMBER:
		{
			switch(symbl_ptr->getTypeSpecifier().getBaseTypeSpec()->specifier())
			{
			case DSPEC_SIMPLE:
			case DSPEC_CLASS:
				{
					if(SCOPE_LOCAL==symbl_ptr->getDeclDefinition().identifier_scope())
					{
						EmitLoadObjectInstancePointer_This(function_ptr);
						
						if(offset > 0)
							lea(EAX, dword_ptr(ECX, __plus, offset));
						else
							lea(EAX, dword_ptr(ECX));
					}else
						lea(EAX, VAR_OFFSET, getSymbolLabel(symbl_ptr));
				}
				break;
			case DSPEC_ARRAY:
				{
					bRequiresAddressOf = true;
					pItemType = emitArraySubscript(symbl_ptr, NULL, pItemType, function_ptr);
					bRequiresAddressOf = false;
					if(pItemType->specifier()==DSPEC_CLASS)
					{
						mov(EAX, ECX);
					}
				}
				break;
			};
		}
		break;
	default:
		{
			__asm int 3; //should never get here!
		}
		break;
	};

	//
	return result_type_ptr;
}

x86_REGISTER HCCCodeGenerator::getReg16From(x86_REGISTER reg)
{
	x86_REGISTER reg16 = AX; //default register
	switch(reg)
	{
	case EAX:
		reg16 = AX;
		break;
	case EBX:
		reg16 = BX;
		break;
	case ECX:
		reg16 = CX;
		break;
	case EDX:
		reg16 = DX;
		break;
	case EDI:
		reg16 = DI;
		break;
	case ESI:
		reg16 = SI;
		break;
	};
	return reg16;
}

x86_REGISTER HCCCodeGenerator::getReg8From(x86_REGISTER reg)
{
	x86_REGISTER reg8 = AL; //default register
	switch(reg)
	{
	case EAX:
		reg8 = AL;
		break;
	case EBX:
		reg8 = BL;
		break;
	case ECX:
		reg8 = CL;
		break;
	case EDX:
		reg8 = DL;
		break;
	};

	return reg8;
}

bool HCCCodeGenerator::emitVirtualMemberFunctionCall(Symbol* object_ptr, 
													 Symbol* member_ptr, 
													 TypeSpecifier* baseTypeSpec, 
													 Symbol* function_ptr,
													 bool bPushParamsOntoStack)
{
	bool bResult = false;
	if(NULL==member_ptr)
		return false;

	CPUInstruction push(this, PUSH);
	CPUInstruction pop(this, POP);
	CPUInstruction mov(this, MOV);
	CPUInstruction call(this, CALL);

	DECLARATION_TYPE type = member_ptr->getDeclDefinition().identifier_type();
	//
	if(DECL_NEW_VIRTUAL_FUNC_MEMBER==type || DECL_NEW_VIRTUAL_ABSTRACT_FUNC_MEMBER==type)
	{
		__tstring vtbl_ptr_symbol_name = CLASS_VPTR_VTBL_NAME;
		Symbol* vtbl_symbol_ptr = baseTypeSpec->getSymbolTable()->find(vtbl_ptr_symbol_name);
		//the vptr...
		assert(vtbl_symbol_ptr!=NULL);
		if(object_ptr!=NULL)
		{
			//emit the argument list...
			EmitActualParameterList(member_ptr, function_ptr);
			//the this pointer must be loaded in ECX...
			EmitLoadValueAddressOf(object_ptr, function_ptr, true); //Jan 3, 2009
		}else{					
			int this_ptr_offset = 
					member_ptr->getDeclDefinition().function.locals.stack_params.total_params_size;					

			if(bPushParamsOntoStack)
			{
				if(this_ptr_offset > 0)
				{
					push(ECX);
				}			
				//emit the argument list...
				EmitActualParameterList(member_ptr, function_ptr);
				
				if(this_ptr_offset > 0)
				{
					mov(ECX, dword_ptr(ESP, __plus, this_ptr_offset));
				}
			}
		}

		//the vtbl offset...
		int vptr_offset = vtbl_symbol_ptr->getDeclDefinition().user_data.offset;
		if(vptr_offset > 0)
			//get the vtbl
			mov(EDX, dword_ptr(ECX, __plus, vptr_offset));
		else
			//get the vtbl
			mov(EDX, dword_ptr(ECX));

		//the virtual function offset...
		int offset = member_ptr->getDeclDefinition().user_data.offset;
		if(offset > 0)
			//call the virtual function...
			call(dword_ptr(EDX, __plus, offset));
		else
			//call the virtual function...
			call(dword_ptr(EDX));
		//
		if(object_ptr==NULL && bPushParamsOntoStack)
		{
			int this_ptr_offset = 
					member_ptr->getDeclDefinition().function.locals.stack_params.total_params_size;					
			if(this_ptr_offset > 0)
			{
				pop(ECX);
			}
		}

		bResult = true;
	}
	return bResult;
}
