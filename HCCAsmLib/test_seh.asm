TITLE SEH testing

.386P
.MODEL flat, stdcall
.STACK 1024 * 1024

INCLUDE hcclib32.INC	; the H++ library.


.DATA

STATUS_ACCESS_VIOLATION  			EQU <0C0000005h>
STATUS_INTEGER_DIVIDE_BY_ZERO    	EQU <0C0000094h>    

lpszOopsAV BYTE "An access violation exception has occurred!",0
lpszOopsIDZ BYTE "An integer division by zero exception has occurred!",0

.CODE 

main PROC NEAR

	push ebp
	mov ebp, esp
	sub esp, 10h
	
	int 3
	mov eax, offset lpszOopsAV
	mov dword ptr [ebp-10h], eax
	;try{
	xor eax, eax
	push esp		; pPrevESP
	push ebp		;pPrevEBP
	push offset __exception_handler;	the master handler
	assume fs:nothing
	push dword ptr fs:[0]	;the current EXCEPTION_REGISTRATION struct pointer
	mov dword ptr fs:[0], esp	;the new EXCEPTION_REGISTRATION struct pointer
	
	lea eax, offset lpszOopsIDZ;
	mov dword ptr [ebp-8], eax
	;
	xor eax, eax
	push eax
	push eax
	mov ebp, 0DEADBEEFh;  <--this has changed the base pointer
	;
	mov dword ptr [eax], 0DEADDEADh ;<--- the access violation is provoked here!
	;I'll simulate the situation
	;push STATUS_INTEGER_DIVIDE_BY_ZERO; STATUS_ACCESS_VIOLATION
	;push esp
	;call __exception_handler;
	;
	jmp offset __continue0;
	
	
__exception_handler:
;EXCEPTION_DISPOSITION __cdecl _except_handler (
;    __in struct _EXCEPTION_RECORD *_ExceptionRecord,
;    __in void * _EstablisherFrame,
;   __inout struct _CONTEXT *_ContextRecord,
;    __inout void * _DispatcherContext
;    );
	;
	mov ebx, dword ptr [esp+4];  _EXCEPTION_RECORD *_ExceptionRecord
	mov ecx, dword ptr [ebx];  _ExceptionRecord->ExceptionCode 
	;
	mov ebx, dword ptr fs:[0]  ;pSEHEXREG->pPrevSEHEXREG;
	mov esp, dword ptr [ebx] ;<--- restore the stack pointer
	pop dword ptr fs:[0]		;<--- set back the prev SEH ER pointer		
	mov ebp, dword ptr [esp+4];the prev EBP
	mov esp, dword ptr [esp+8]; the prev ESP
	;
__access_violation_handler:
	mov eax, STATUS_ACCESS_VIOLATION ;the expr must have an exception code to use
	cmp eax, ecx	;is this exception AV as I expected?
	jnz offset __next_seh_handler0;
	;{
	push dword ptr [ebp-10h]
	call System_Debug_OutputString;	
	;}
	jmp offset __safer_place0;
__next_seh_handler0: ;no more seh handling
	mov eax, STATUS_INTEGER_DIVIDE_BY_ZERO ; 
	cmp eax, ecx
	jnz offset __next_seh_handler1;
	;{
	push dword ptr [ebp-8]
	call System_Debug_OutputString;	
	;}
	jmp offset __safer_place0;
__next_seh_handler1:
	jmp offset __safer_place0;
__continue0:
	mov esp, dword ptr fs:[0]  ;<--- restore the stack pointer
	pop fs:[0]		;<--- set back the prev ER pointer
	mov ebp, dword ptr [esp+4];the prev EBP
	mov esp, dword ptr [esp+8]; the prev ESP	
__safer_place0:
__end_func:
	leave
	ret
main ENDP


END main		;the Entry Point