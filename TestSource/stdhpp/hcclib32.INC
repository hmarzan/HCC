.NOLIST
;	*****************************************************************************
;					H++ Include Library.
;
;	This include file contains the externs for all the existent H++ library 
;	API set (Draft version).
;	
;	Harold L. Marzan
;	July 14, 2006.
;
;	*****************************************************************************
;
INCLUDE SmallWin.inc

; when the "C" calling convension is used, then we must use :PROC;
; when the stdcall calling convension is used, we must use :PROTO;

EXTERN __purecall: PROTO;

; H++  C O N S O L E   A P I s 

EXTERN Console_ClearScreen:PROTO;
EXTERN Console_ReadString:PROTO;
EXTERN Console_ReadChar:PROTO;
EXTERN Console_WriteString:PROTO;
EXTERN Console_WriteCrlf:PROTO;
EXTERN Console_WriteChar:PROTO;
EXTERN Console_SetCursorPos:PROTO;
EXTERN Console_SetTextColor:PROTO;
EXTERN Console_SetConsoleSize:PROTO;
EXTERN Console_SetTitle:PROTO;
EXTERN Console_GetTitle:PROTO;
EXTERN Console_MoveWindow:PROTO;

EXTERN Console_ReadDouble:PROTO;
EXTERN Console_ReadInteger:PROTO;
EXTERN Console_WriteDouble:PROTO;
EXTERN Console_WriteInteger:PROTO;
EXTERN Console_WriteInteger64:PROTO;
EXTERN Console_WriteBoolean:PROTO;

; H++  F I L E   A P I s 

EXTERN File_Open:PROTO;
EXTERN File_Close:PROTO;
EXTERN File_Read:PROTO;
EXTERN File_Write:PROTO;
EXTERN File_Seek:PROTO;
EXTERN File_FileSize:PROTO;

; H++  S Y S T E M   A P I s 

EXTERN System_Exit:PROTO;
EXTERN System_LastError:PROTO;
EXTERN System_CommandLineArgs:PROTO;
EXTERN System_Execute:PROTO;
EXTERN System_GetTickCount:PROTO;
EXTERN System_Sleep:PROTO;
EXTERN System_Debug_OutputString:PROTO;
EXTERN System_BreakPoint:PROTO;
EXTERN System_IsDebuggerPresent:PROTO;

; H++  M A T H   A P I s

EXTERN Math_Init:PROTO;
EXTERN Math_Abs:PROTO;
EXTERN Math_ChangeSign:PROTO;
EXTERN Math_Sqrt:PROTO;
EXTERN Math_Sqr:PROTO;

EXTERN Math_Sin:PROTO;
EXTERN Math_Cos:PROTO;
EXTERN Math_Tan:PROTO;
EXTERN Math_ArcTan:PROTO;
EXTERN Math_Round:PROTO;

EXTERN Math_Modulus:PROTO;
EXTERN Math_IEEEModulus:PROTO;
EXTERN Math_ModulusTruncateDivisor:PROTO;

EXTERN Math_Ln:PROTO;
EXTERN Math_log2:PROTO;
EXTERN Math_log10:PROTO;
EXTERN Math_Exp:PROTO;

EXTERN Math_MantissaOf:PROTO;
EXTERN Math_GetNumberFrom:PROTO;
EXTERN Math_LogN_base:PROTO;

EXTERN Math_Max:PROTO;
EXTERN Math_Min:PROTO;
EXTERN Math_Sign:PROTO;
EXTERN Math_Pow2:PROTO;
EXTERN Math_Pow:PROTO;
EXTERN Math_Gcd:PROTO;
EXTERN Math_Odd:PROTO;
EXTERN Math_Even:PROTO;
EXTERN Math_Trunc:PROTO;

; N O N - I N T R I N S I C   M A T H   F U N C T I O N S 

EXTERN Math_Cotan:PROTO;
EXTERN Math_Sec:PROTO;
EXTERN Math_Cosec:PROTO;
EXTERN Math_Inverse_Arcsin:PROTO;
EXTERN Math_Inverse_Arccos:PROTO;
EXTERN Math_Inverse_Arcsec:PROTO;
EXTERN Math_Inverse_Arccosec:PROTO;
EXTERN Math_Inverse_Arccotan:PROTO;

; M A T H   C O N S T A N T S  
EXTERN Math_pi:PROTO;
EXTERN Math_log_10_base2:PROTO;
EXTERN Math_log_e_base2:PROTO;
EXTERN Math_log_2_base10:PROTO;
EXTERN Math_log_2_base_e:PROTO;


; I N T E G E R   S P E C I F I C S   ( U T I L I T I E S )
EXTERN Integer_toStringEx:PROTO;
EXTERN Integer_toString:PROTO;
EXTERN Integer_fromString:PROTO;


; F L O A T I N G   P O I N T   S P E C I F I C  (U T I L I T I E S )

EXTERN FloatingPoint_Compare:PROTO;
EXTERN FloatingPoint_CompareToInteger:PROTO;
EXTERN FloatingPoint_ToInt:PROTO;
EXTERN FloatingPoint_ToInt64:PROTO;
EXTERN FloatingPoint_FromInt:PROTO;
EXTERN FloatingPoint_FromInt64:PROTO;

EXTERN FloatingPoint_toString:PROTO;
EXTERN FloatingPoint_toStringEx:PROTO;
EXTERN FloatingPoint_fromString:PROTO;

; H++  S T R I N G  H A N D L I N G   A P I s 

EXTERN StringHandling_StringLength2:PROTO;
EXTERN StringHandling_StringLength:PROTO;
EXTERN StringHandling_StringCopy:PROTO;
EXTERN StringHandling_StringCompare:PROTO;
EXTERN StringHandling_StringUCase:PROTO;
EXTERN StringHandling_StringLCase:PROTO;
EXTERN StringHandling_StringSet:PROTO;

; H++  M E M O R Y   H A N D L I N G   A P I s

EXTERN System_Memory_Memset:PROTO;
EXTERN System_Memory_GetProcessHeap:PROTO
EXTERN System_Memory_Alloc:PROTO
EXTERN System_Memory_Free:PROTO
EXTERN System_Memory_Size:PROTO
EXTERN System_Memory_New:PROTO
EXTERN System_Memory_Destroy:PROTO
EXTERN System_Memory_SizeFor:PROTO


EXTERN System_GetCommandLineArgv:PROTO;

EXTERN System_ShowMessage:PROTO;

EXTERN ConsoleHandle:DWORD;
;
.LIST