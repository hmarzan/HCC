
;	*****************************************************************************
;					H++ Include Library.
;
;		This file contains the assembly implementation of the H++ library
;		API set.
;
;		Harold L. Marzan
;		H++ Developer & Designer.
;		May 2006.
;
;	*****************************************************************************
.386P
.MODEL flat, stdcall
.STACK 1024 * 1024

INCLUDE SmallWin.inc

GetConsoleTitle EQU <GetConsoleTitleA>
MessageBox		EQU <MessageBoxA>

GetConsoleTitle PROTO, lpConsoleTitle: PTR BYTE, ; points to string
					   nSize: DWORD				 ; the size of buffer

GetFileSize PROTO, hFile:DWORD,					; file handle
				   lpFileSizeHigh:PTR DWORD		; high-dword of 64-bit files
				   
WinExec PROTO, lpCmdLine:PTR BYTE,				;the command line
			   uCmdShow:DWORD					;show flags				   
			   
OutputDebugString EQU <OutputDebugStringA>
OutputDebugString PROTO, lpOutputString : PTR BYTE	;points to a string

IsDebuggerPresent PROTO

FillConsoleOutputCharacter EQU <FillConsoleOutputCharacterA>
FillConsoleOutputCharacter PROTO, hConsoleOutput: DWORD,
								  cCharacter: DWORD,
								  nLength: DWORD,
								  dwWriteCoord: DWORD,
								  lpNumberOfCharsWritten: PTR DWORD
								  
FillConsoleOutputAttribute PROTO, hConsoleOutput: DWORD,
								  wAttribute: DWORD,
								  nLength: DWORD,
								  dwWriteCoord: DWORD,
								  lpNumberOfAttrsWritten: PTR DWORD

GetComputerName PROTO, 
					lpBuffer: PTR DWORD,
					lpnSize: PTR DWORD			;return BOOL


GetWindowsDirectory PROTO,
					  lpBuffer:PTR DWORD,
					  uSize:DWORD				;return UINT 


GetEnvironmentVariable PROTO,
						lpName:PTR DWORD,
						lpBuffer:PTR DWORD,
						nSize:DWORD				;return DWORD


GetCurrentDirectory PROTO,
						nBufferLength: DWORD,
						lpBuffer: PTR DWORD		;return DWORD 

CreateDirectory PROTO,
					lpPathName:PTR DWORD,
					lpSecurityAttributes: PTR DWORD		;return BOOL 

RemoveDirectory PROTO,
					lpPathName: PTR DWORD			;return BOOL
					
GetCommandLine		EQU <GetCommandLineW>
CommandLineToArgv	EQU <CommandLineToArgvW>
lstrlen				EQU <lstrlenW>

lstrlen	PROTO,
				 lpString: PTR WORD					;return int

GetCommandLine PROTO							;returns LPWSTR

CommandLineToArgv PROTO,
					lpCmdLine: PTR DWORD,
					pNumArgs: PTR DWORD			;returns LPWSTR*
					
WideCharToMultiByte PROTO,
				CodePage: DWORD,			; code page
				dwFlags: DWORD,				;performance and mapping flags
				lpWideCharStr:PTR WORD,		;wide-character string
				cchWideChar:DWORD,          ;number of chars in string
				lpMultiByteStr:PTR BYTE,    ;buffer for new string
				cbMultiByte:DWORD,          ;size of buffer
				lpDefaultChar: PTR BYTE,    ;default for unmappable chars
				lpUsedDefaultChar:DWORD		;set when default char used
	;return int 
	
GlobalFree PROTO,
				hMem: DWORD					;must return NULL

LocalFree PROTO, hMem: DWORD 				;must return NULL
				

GetProcessHeap PROTO					

HeapAlloc PROTO,
			hHeap: DWORD,
			dwFlags: DWORD,
			dwBytes:DWORD				;return LPVOID

HeapFree PROTO,
			hHeap: DWORD,
			dwFlags: DWORD,
			lpMem: DWORD				;return BOOL 

HeapSize PROTO,
			hHeap: DWORD,
			dwFlags: DWORD,
			lpMem: DWORD				;return SIZE_T 
			
HeapReAlloc PROTO,
			hHeap: DWORD,
			dwFlags: DWORD,
			lpMem: DWORD,
			dwBytes: DWORD				;return LPVOID			

MessageBox PROTO,
		   hWnd: DWORD,
		   lpText: PTR DWORD,
		   lpCaption: PTR DWORD,
		   uType: DWORD				;return int


PUBLIC ConsoleHandle;
PUBLIC ConsoleBuffer;
PUBLIC ConsoleTitle;

;symbolic constants 
NULL			EQU 0
MAX_READ_CHARS	EQU 050h

HEAP_ZERO_MEMORY	EQU	00000008
HEAP_NO_SERIALIZE	EQU	00000001      
MAX_DIGITS_COUNT	EQU	00000020h ;==32
MAX_EXPONENT_VALUE	EQU	00000308

INVALID_NUMBER EQU		0000000Ah	
INVALID_FRACTION EQU	0000000Ch
INVALID_EXPONENT EQU	0000000Fh

.DATA

__hargc		  DWORD 0
ConsoleHandle DWORD 0
ConsoleBuffer BYTE MAX_READ_CHARS DUP(?)
CRLF		  BYTE 0Dh, 0Ah
ConsoleTitle  BYTE 100h DUP(?)

__xtable BYTE "0123456789ABCDEF"
__virtual_abstract_call_msg BYTE "Call to a virtual abstract member function is illegal. Program aborted.", 0
;
;Floating point conversion error messages
__msg_enumber 			BYTE "Invalid Number.", 0
__msg_efraction			BYTE "Invalid Fraction.", 0
__msg_eexponent			BYTE "Invalid Exponent.",0
__msg_too_many_digits	BYTE "Too many digits.",0
__msg_fp_out_of_range	BYTE "Floating point value out of range.",0

__FP_MAX_RANGE			DQ		9.997e+1,	9.997e+3,	9.997e+5,	9.997e+7,	9.997e+9,	9.997e+11,	9.997e+13,	9.997e+15
__FP_MAX_POWER_10		DQ		1.0e+16,	1.0e+14,	1.0e+12,	1.0e+10,	1.0e+8,		1.0e+6,		1.0e+4,		1.0e+2
__FP_MAX_EXPONENT		BYTE	10h,		0Eh,		0Ch,		0Ah,		8,			6,			4,			2


_@StringBooleanTrue@@__  BYTE "True",0
_@StringBooleanFalse@@__ BYTE "False",0

.CODE

; These functions will be the APIs exposed in H++'s hcclib32.obj file 
; imported from the stdapi.hcc file.
; This apis are wrappers to the ones in Kernel32.dll, and other Windows libraries

;//C O N S O L E  A P I 

Console_ClearScreen PROC
	push ebp
	mov ebp, esp
	push edi
	push esi
	push ebx		
	xor	ebx,ebx
				;COORD coordScreen = { 0, 0 };    // home for the cursor; we will use 4 bytes
				;DWORD cCharsWritten;				
				;CONSOLE_SCREEN_BUFFER_INFO csbi; requires 012h bytes; we will use 014h				
				;DWORD dwConSize;				
	;the local variables
	sub esp, 20h
	;Get the console handle
	push STD_OUTPUT_HANDLE
	call GetStdHandle;
	mov esi, OFFSET ConsoleHandle
	mov dword ptr [esi], eax	; the console handle	
	;Get the number of character cells in the current buffer. 	
	lea eax, dword ptr [ebp-1Ch]	;csbi
	push eax
	push dword ptr [esi]			;hConsoleHandle
	call GetConsoleScreenBufferInfo;
	test eax, eax
	jz _EndClrScr
	;dwSize : contains the size of the console screen buffer, in character columns and rows. 
	xor eax,eax	
	mov ax, word ptr [ebp-1Ch]		;csbi.dwSize.X
	mov cx, word ptr [ebp-1Ah]		;csbi.dwSize.Y
	mul cx							;csbi.dwSize.X * csbi.dwSize.Y;	
	mov dword ptr [ebp-20h], eax	;dwConSize
	;
	; Fill the entire screen with blanks.
	lea edx, dword ptr [ebp-8]	;lpNumberOfCharsWritten == cCharsWritten
	push edx
	push ebx					;dwWriteCoord
	push dword ptr [ebp-20h]	;nLength
	push 00000020h				;cCharacter
	push dword ptr [esi]		;hConsoleOutput
	call FillConsoleOutputCharacter;
	test eax, eax
	jz _EndClrScr
	;
	; Get the current text attribute.
	lea eax, dword ptr [ebp-1Ch]	;csbi
	push eax
	push dword ptr [esi]			;hConsoleHandle
	call GetConsoleScreenBufferInfo;
	test eax, eax
	jz _EndClrScr
	;
    ; Set the buffer's attributes accordingly.	
	xor eax, eax
	mov ax, word ptr [ebp-14h]  ;csbi.wAttributes
	lea edx, dword ptr [ebp-8]	;lpNumberOfCharsWritten == cCharsWritten
	push edx
	push ebx					;dwWriteCoord
	push dword ptr [ebp-20h]	;nLength	
	push eax					;wAttribute
	push dword ptr [esi]		;hConsoleHandle
	call FillConsoleOutputAttribute;
	test eax, eax
	jz _EndClrScr;
	;Put the cursor at its home coordinates.   
	push ebx
	push dword ptr [esi]
	call SetConsoleCursorPosition;
	;
_EndClrScr:	
	pop ebx
	pop esi
	pop edi
	mov esp, ebp
	pop ebp
	ret
Console_ClearScreen ENDP

Console_ReadString PROC
	push ebp
	mov ebp, esp
	sub esp, 4
	push ebx
	push esi
	push edi
;
	push STD_INPUT_HANDLE
	call GetStdHandle;
	mov esi, OFFSET ConsoleHandle
	mov dword ptr [esi], eax	; the console handle	
	;
	lea eax, dword ptr [ebp-4]
	lea edx, OFFSET ConsoleBuffer
	push NULL					; reserved
	push eax					; bytes read
	push MAX_READ_CHARS			; max chars to read
	push edx					;the buffer
	push dword ptr [esi]		;the console handle
	call ReadConsole
	;	
	lea edx, OFFSET ConsoleBuffer	;the buffer
	mov eax, dword ptr [ebp-4]		;bytes read
	lea edx, [edx+eax-2]			
	mov dword ptr [edx], 0			;deletes the 0dh, 0ah from result	
	lea eax, OFFSET ConsoleBuffer	;the buffer
;
	pop edi
	pop esi
	pop ebx
	mov esp, ebp
	pop ebp
	ret
Console_ReadString ENDP

Console_ReadChar PROC
	push ebp
	mov ebp, esp
	sub esp, 0Ch
	push ebx
	push esi
	push edi
;
	push STD_INPUT_HANDLE
	call GetStdHandle;
	mov esi, OFFSET ConsoleHandle
	mov dword ptr [esi], eax	; the console handle	
;
	lea eax, dword ptr [ebp-4]	; to save the current input mode
	push eax
	push dword ptr [esi]
	call GetConsoleMode;
;	
	mov esi, OFFSET ConsoleHandle
	push 0						;read one char at a time	
	push dword ptr [esi]
	call SetConsoleMode;	
;
	lea eax, dword ptr [ebp-8]
	lea edx, OFFSET ConsoleBuffer
	push NULL					; reserved
	push eax					; bytes read
	push 1						; max chars to read
	push edx					;the buffer
	push dword ptr [esi]		;the console handle
	call ReadConsole
		
	;
	mov eax, dword ptr [ebp-4]	;the saved input mode
	mov esi, OFFSET ConsoleHandle
	push eax
	push dword ptr [esi]
	call SetConsoleMode;
	
	lea edx, OFFSET ConsoleBuffer	
	mov ax, word ptr [edx]		; the byte that was read
	pop edi
	pop esi
	pop ebx
	mov esp, ebp
	pop ebp
	ret
Console_ReadChar ENDP

Console_WriteString PROC
	push ebp
	mov ebp, esp		
	push ebx
	push esi
	push edi
	
	push ecx
	
;
	push STD_OUTPUT_HANDLE
	call GetStdHandle;
	mov esi, OFFSET ConsoleHandle
	mov dword ptr [esi], eax	; the console handle	
;	
	lea eax, dword ptr [esp]
	mov edx, dword ptr [ebp+8]	; ptr to buffer
	push 0						;reserved
	push eax					; number of chars written		
	mov ecx, dword ptr [ebp+0Ch]; number of chars to write	
	test ecx, ecx				; if zero, calculate it by the string length
	jnz __use_char_size;
	push edx
	call StringHandling_StringLength;
	mov ecx, eax
	mov edx, dword ptr [ebp+8]	; ptr to buffer
__use_char_size:
	push ecx
	push edx					; source buffer
	push dword ptr [esi]		; the output handle
	call WriteConsole;
	;returns the number of bytes read
	mov eax, dword ptr [esp]	
	add esp, 4
	
	pop edi
	pop esi
	pop ebx
		
	mov esp, ebp
	pop ebp
	retn 8
Console_WriteString ENDP

Console_WriteCrlf PROC
	push ebx
	push esi
	push edi
	
	mov eax, OFFSET CRLF
	push 2
	push eax
	call Console_WriteString;	

	pop edi
	pop esi
	pop ebx
	ret	
Console_WriteCrlf ENDP

Console_WriteChar PROC
	push ebx
	push esi
	push edi	
	
	lea eax, dword ptr [esp+4*4]
	push 1
	push eax
	call Console_WriteString;

	pop edi
	pop esi
	pop ebx
	ret	4
Console_WriteChar ENDP

Console_WriteBoolean PROC
	push ebx
	push esi
	push edi	
	
	mov al, byte ptr [esp+4*4]
	test al, al
	push 0
	jz offset __WriteFalse;
	push offset _@StringBooleanTrue@@__
	jmp offset __WriteBooleanString;
__WriteFalse:
	push offset _@StringBooleanFalse@@__
__WriteBooleanString:
	call Console_WriteString;

	pop edi
	pop esi
	pop ebx
	ret	4
Console_WriteBoolean ENDP


Console_ReadInteger PROC
	call Console_ReadString;	
	push eax
	call Integer_fromString;
	retn
Console_ReadInteger ENDP

Console_WriteIntegerEx PROC
     mov ecx, dword ptr [esp+8]	;radix
     mov eax, dword ptr [esp+4] ;integer value
     push ebp
     mov ebp, esp     
     sub esp, 20h
     lea ebx, [esp]
     push	ebx					; buffer
     push 	ecx					; radix
     push	eax					; the number
     call	Integer_toStringEx	; do conversion
     lea ebx, [esp]
     push eax					; number of chars to print
     push ebx     
     call Console_WriteString;     
     leave
     ret 8
Console_WriteIntegerEx ENDP

Console_WriteInteger PROC
	mov eax, dword ptr [esp+4]
	push 0Ah ;the radix 10
	push eax
	call Console_WriteIntegerEx
	retn 4
Console_WriteInteger ENDP

Console_WriteBinary PROC
	mov eax, dword ptr [esp+4]
	push 02h ;the radix 2
	push eax
	call Console_WriteIntegerEx
	retn 4
Console_WriteBinary ENDP

Console_WriteHex PROC
	mov eax, dword ptr [esp+4]
	push 10h ;the radix 16
	push eax
	call Console_WriteIntegerEx
	retn 4
Console_WriteHex ENDP

Console_WriteOctal PROC
	mov eax, dword ptr [esp+4]
	push 8h ;the radix 8
	push eax
	call Console_WriteIntegerEx
	retn 4
Console_WriteOctal ENDP

Console_ReadDouble PROC	;double expr
	push edi
	push esi
	call Console_ReadString;
	push eax
	call FloatingPoint_fromString;
	pop esi
	pop edi
	retn
Console_ReadDouble ENDP

Console_WriteDouble PROC
	mov edx, dword ptr [esp+4]
	push dword ptr [esp+8]
	push edx
	call FloatingPoint_toString
	push edi
	mov edi, eax
	push 0
	push eax
	call Console_WriteString;
	push edi
	call System_Memory_Destroy;
	pop edi
	retn 8
Console_WriteDouble ENDP

Console_WriteInteger64 PROC
	mov edx, dword ptr [esp+4]
	mov eax, dword ptr [esp+4*2]
	push eax
	push edx
	fclex
	fild qword ptr [esp]	; st = m64int
	fstp qword ptr [esp]	; m64 = st
	call Console_WriteDouble;
	retn 8
Console_WriteInteger64 ENDP

Console_SetCursorPos PROC	; short X, short Y
	push ebp
	mov ebp, esp
	push ebx
	push esi
	push edi
	
	push STD_OUTPUT_HANDLE
	call GetStdHandle;
	mov esi, OFFSET ConsoleHandle
	mov dword ptr [esi], eax	; the console handle	
		
	sub esp, 4
	xor eax,eax
	mov ax, word ptr [ebp+0Ch] ; Y
	shl eax,010h
	mov ax, word ptr [ebp+8]   ; X		
	mov dword ptr [esp], eax
	push dword ptr [esi]
	call SetConsoleCursorPosition;
	;
	pop edi
	pop esi
	pop ebx
	
	mov esp, ebp
	pop ebp
	retn 8
Console_SetCursorPos ENDP

Console_SetTextColor PROC
	push ebp
	mov ebp, esp
	push ebx
	push esi
	push edi
	
	push STD_OUTPUT_HANDLE
	call GetStdHandle;
	mov esi, OFFSET ConsoleHandle
	mov dword ptr [esi], eax	; the console handle	
	;
	mov eax, dword ptr [ebp+8]
	push eax
	push dword ptr [esi]
	call SetConsoleTextAttribute;
	;
	pop edi
	pop esi
	pop ebx
	
	mov esp, ebp
	pop ebp
	retn 4	
Console_SetTextColor ENDP

Console_SetConsoleSize PROC
	push ebp
	mov ebp, esp
	push ebx
	push esi
	push edi
	
	push ecx
	
	push STD_OUTPUT_HANDLE
	call GetStdHandle;
	mov esi, OFFSET ConsoleHandle
	mov dword ptr [esi], eax	; the console handle	

	mov eax, dword ptr [ebp+0Ch] ; Y
	shl eax, 10h
	mov ax, word ptr [ebp+8]	 ; X
	mov dword ptr [esp]	, eax
	push dword ptr [esi]		;output handle
	call SetConsoleScreenBufferSize
	;
	pop edi
	pop esi
	pop ebx
	
	mov esp, ebp
	pop ebp
	retn 8
Console_SetConsoleSize ENDP

Console_SetTitle PROC
	push ebp
	mov ebp, esp
	push ebx
	push esi
	push edi
	
	push dword ptr [ebp+8]
	call SetConsoleTitle;
	;
	pop edi
	pop esi
	pop ebx
	
	mov esp, ebp
	pop ebp
	retn 4
Console_SetTitle ENDP

Console_GetTitle PROC
	push ebp
	mov ebp, esp
	push ebx
	push esi
	push edi
	
	mov edx, SIZEOF ConsoleTitle	
	mov eax, OFFSET ConsoleTitle
	push edx
	push eax
	call GetConsoleTitle;
	mov eax, OFFSET ConsoleTitle
	;
	pop edi
	pop esi
	pop ebx
	
	mov esp, ebp
	pop ebp
	ret
Console_GetTitle ENDP

;BOOL SetConsoleWindowInfo(
;  HANDLE hConsoleOutput,
;  BOOL bAbsolute,
;  const SMALL_RECT* lpConsoleWindow
;);

Console_MoveWindow PROC	
						;Left,	+8
						;Top,	+0Ch
						;Right,	+10h	
						;Bottom	+14h
	push ebp
	mov ebp, esp
	push ebx
	push esi
	push edi
	;
	push STD_OUTPUT_HANDLE
	call GetStdHandle;
	mov esi, OFFSET ConsoleHandle
	mov dword ptr [esi], eax	; the console handle	
		
	xor eax,eax
	xor edx, edx
	sub esp, 8
	mov ax, word ptr [ebp+0Ch]	;Top
	shl eax, 10h
	add ax, word ptr [ebp+8]	;Left
	
	mov dx, word ptr [ebp+14h]	;Bottom
	shl edx, 10h
	add dx, word ptr [ebp+10h]	;Right
		
	mov dword ptr [esp], eax
	mov dword ptr [esp+4], edx
	lea eax, dword ptr [esp]
	push eax
	push 1	;bAbsolute
	push dword ptr [esi]	
	call SetConsoleWindowInfo
	pop ecx
	pop ecx
	;	
	pop edi
	pop esi
	pop ebx
	
	mov esp, ebp
	pop ebp
	ret 10h
Console_MoveWindow ENDP

;// F I L E   H A N D L I N G   A P I 

;HANDLE CreateFile(
;  LPCTSTR lpFileName,
;  DWORD dwDesiredAccess,
;  DWORD dwShareMode,
;  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
;  DWORD dwCreationDisposition,
;  DWORD dwFlagsAndAttributes,
;  HANDLE hTemplateFile
;);

File_Open PROC
				;string filePath,		+8
				;FileMode mode,			+0Ch
				;FileAccess access,		+10h
				;FileSharing sharing	+14h

	push ebp
	mov ebp, esp
	push ebx
	push esi
	push edi
	;
	
	push 0						;hTemplateFile
	push FILE_ATTRIBUTE_NORMAL	;dwFlagsAndAttributes
	push dword ptr [ebp+0Ch]	;dwCreationDisposition
	push 0						;lpSecurityAttributes
	push dword ptr [ebp+14h]	;dwShareMode
	push dword ptr [ebp+10h]	;dwDesiredAccess
	push dword ptr [ebp+8]		;lpFileName
	call CreateFile;
	;
	pop edi
	pop esi
	pop ebx
	mov esp, ebp
	pop ebp
	retn	010h
File_Open ENDP

File_Close PROC
	mov eax, dword ptr [esp+4]
	push eax
	call CloseHandle;
	retn 4
File_Close ENDP

;BOOL ReadFile(
;  HANDLE hFile,
;  LPVOID lpBuffer,
;  DWORD nNumberOfBytesToRead,
;  LPDWORD lpNumberOfBytesRead,
;  LPOVERLAPPED lpOverlapped
;);

File_Read PROC
			;FileHandle handle,		+8
			;char buffer[],			+0Ch
			;long nBytesToRead		+010h
	;
	push ebp
	mov ebp, esp	
	push 0
	push ebx
	push esi
	push edi
	;	
	lea ecx, dword ptr [ebp-4]
	push 0						;lpOverlapped
	push ecx					;lpNumberOfBytesRead
	push dword ptr [ebp+010h]	;nNumberOfBytesToRead
	push dword ptr [ebp+0Ch]	;lpBuffer
	push dword ptr [ebp+8]		;hFile
	call ReadFile;
	;
	mov eax, dword ptr [ebp-4]	
	pop edi
	pop esi
	pop ebx
	mov esp, ebp
	pop ebp
	retn	0Ch			
File_Read ENDP

;BOOL WriteFile(
;  HANDLE hFile,
;  LPCVOID lpBuffer,
;  DWORD nNumberOfBytesToWrite,
;  LPDWORD lpNumberOfBytesWritten,
;  LPOVERLAPPED lpOverlapped
;);

File_Write PROC
			;FileHandle handle,		+8
			;char buffer[],			+0Ch
			;long nBytesToWrite		+10h
	push ebp
	mov ebp, esp	
	push 0
	push ebx
	push esi
	push edi
	;
	lea eax, dword ptr [ebp-4]
	push 0						;lpOverlapped
	push eax					;lpNumberOfBytesWritten
	push dword ptr [ebp+10h]	;nNumberOfBytesToWrite
	push dword ptr [ebp+0Ch]	;lpBuffer
	push dword ptr [ebp+8]		;hFile
	call WriteFile;
	;
	mov eax, dword ptr [ebp-4]				
	pop edi
	pop esi
	pop ebx
	mov esp, ebp
	pop ebp
	retn	0Ch			
File_Write ENDP

;DWORD SetFilePointer(
;  HANDLE hFile,
;  LONG lDistanceToMove,
;  PLONG lpDistanceToMoveHigh,
;  DWORD dwMoveMethod
;);

File_Seek PROC
			;FileHandle handle,		+8
			;SeekType moveType,		+0Ch
			;long offset			+10h
	push ebp
	mov ebp, esp	
	push 0
	push ebx
	push esi
	push edi		
	
	push dword ptr [ebp+0Ch]	;dwMoveMethod
	push 0						;lpDistanceToMoveHigh (0 for the first version)
	push dword ptr [ebp+10h]	;lDistanceToMove
	push dword ptr [ebp+8]		;hFile
	call SetFilePointer;
	;	
	pop edi
	pop esi
	pop ebx
	mov esp, ebp
	pop ebp
	retn	0Ch;	
File_Seek ENDP

;DWORD GetFileSize(
;  HANDLE hFile,
;  LPDWORD lpFileSizeHigh
;);

File_FileSize PROC
	push 0
	lea eax, dword ptr [esp]	
	mov edx, dword ptr [esp+8]	;hFile
	push eax				;lpFileSizeHigh
	push edx				;hFile
	call GetFileSize;
	pop ecx	
	retn 4
File_FileSize ENDP


System_Exit PROC
	mov eax, dword ptr [esp+4]
	push eax
	call ExitProcess;
	retn 4
System_Exit ENDP

System_LastError PROC
	call GetLastError;
	ret
System_LastError ENDP

System_CommandLineArgs PROC
	call GetCommandLine;
	ret
System_CommandLineArgs ENDP

System_Execute PROC
	mov ecx, dword ptr [esp+8]	;show how?
	mov eax, dword ptr [esp+4]	;cmd	
	push ecx
	push eax
	call WinExec;
	ret 8
System_Execute ENDP

System_GetTickCount PROC
	call GetTickCount;
	ret;
System_GetTickCount ENDP

System_Sleep PROC
	mov edx, dword ptr [esp+4]
	push edx
	call Sleep
	ret 4
System_Sleep ENDP

System_Debug_OutputString PROC
	push dword ptr [esp+4]
	call OutputDebugString;
	retn 4
System_Debug_OutputString ENDP

System_BreakPoint PROC
	ret	;the compiler will emit int 3 when a call to this function is made
System_BreakPoint ENDP

System_IsDebuggerPresent PROC
	call IsDebuggerPresent;
	ret
System_IsDebuggerPresent ENDP

Math_Init PROC
	fninit
	ret
Math_Init ENDP

Math_Abs PROC
	fclex
		;EDX:EAX --> high:low
	fld qword ptr [esp+4]
	fabs
	sub esp, 8
	fstp qword ptr [esp]
	pop edx		;high
	pop eax		;low
	retn 8	
Math_Abs ENDP

Math_ChangeSign PROC
	fclex
	fld qword ptr [esp+4]
	fchs
	sub esp, 8
	fstp qword ptr [esp]
	pop edx		;high
	pop eax		;low
	retn 8
Math_ChangeSign ENDP

Math_Sqrt PROC	
	fclex
	fld qword ptr [esp+4]
	fsqrt			; y = sqrt(x);
	sub esp, 8
	fstp qword ptr [esp]
	pop edx		;high
	pop eax		;low
	retn 8
Math_Sqrt ENDP

Math_Sqr PROC
	fclex
	fld qword ptr [esp+4]
	fmul qword ptr [esp+4]	; y = x * x;
	sub esp, 8
	fstp qword ptr [esp]
	pop edx		;high
	pop eax		;low
	retn 8
Math_Sqr ENDP

Math_Sin PROC		;source operand must be given in radians and must be within the range -2^63 to +2^63
	fclex
	fld qword ptr [esp+4]
	fsin		;sin(x);
	sub esp, 8
	fstp qword ptr [esp]
	pop edx		;high
	pop eax		;low
	retn 8	
Math_Sin ENDP

Math_Cos PROC		;source operand must be given in radians and must be within the range -2^63 to +2^63
	fclex
	fld qword ptr [esp+4]
	fcos		;cos(x);
	sub esp, 8
	fstp qword ptr [esp]
	pop edx		;high
	pop eax		;low
	retn 8	
Math_Cos ENDP

Math_Tan PROC		;source operand must be given in radians and must be within the range -2^63 to +2^63
	fclex
	fld qword ptr [esp+4]
	fptan		;tan(x);
	sub esp, 8
	fstp qword ptr [esp]	;take out the +1.0
	fstp qword ptr [esp]	;tan(x)
	pop edx		;high
	pop eax		;low
	retn 8	
Math_Tan ENDP

Math_ArcTan PROC	;ArcTan(double X, double Y);	
	fclex
	fld qword ptr [esp+0Ch] ; Y	-->st(1)
	fld qword ptr [esp+4]	; X	-->st
	fpatan			;ArcTan(X, Y); //arctan(Y/X) | arctan(st(1)/st);
	sub esp, 8	
	fstp qword ptr [esp]
	pop edx		;high
	pop eax		;low
	retn 010h
Math_ArcTan ENDP

Math_Round PROC	;FRNDINT—Round to Integer if(EAX==0 && EDX !=0) result is Int32 else Int64
	fclex
	fld qword ptr [esp+4]
	frndint
	sub esp, 8
	fistp qword ptr [esp]
	pop edx		;high
	pop eax		;low
	test eax,eax
	jz __UpdateLowPart;	
	jmp __EndConv64;	
__UpdateLowPart:
	mov eax, edx
	xor edx, edx
__EndConv64:	
	retn 8
Math_Round ENDP

Math_Round64 PROC
	mov edx, dword ptr [esp+4]
	push dword ptr [esp+8]
	push edx
	call Math_Round;	
Math_Round64 ENDP

Math_pi PROC
	fldpi
	sub esp, 8
	fstp qword ptr [esp]
	pop edx		;high
	pop eax		;low
	ret	
Math_pi ENDP

Math_log_10_base2 PROC
	fldl2t
	sub esp, 8
	fstp qword ptr [esp]
	pop edx		;high
	pop eax		;low
	ret	
Math_log_10_base2 ENDP

Math_log_e_base2 PROC
	fldl2e
	sub esp, 8
	fstp qword ptr [esp]
	pop edx		;high
	pop eax		;low
	ret	
Math_log_e_base2 ENDP

Math_log_2_base10 PROC
	fldlg2
	sub esp, 8
	fstp qword ptr [esp]
	pop edx		;high
	pop eax		;low
	ret	
Math_log_2_base10 ENDP

Math_log_2_base_e PROC
	fldln2
	sub esp, 8
	fstp qword ptr [esp]
	pop edx		;high
	pop eax		;low
	ret	
Math_log_2_base_e ENDP

Math_Modulus PROC		;FPREM—Partial Remainder
	fclex
	fld qword ptr [esp+0Ch]	;Divisor
	fld qword ptr [esp+4]	;Dividend
	fprem	
	sub esp, 8
	fstp qword ptr [esp]
	pop edx		;high
	pop eax		;low
	retn 010h
Math_Modulus ENDP

Math_IEEEModulus PROC	;FPREM—Partial Remainder : computes the remainder specified in IEEE Standard 754.
	fclex
	fld qword ptr [esp+0Ch]	;Divisor
	fld qword ptr [esp+4]	;Dividend
	fprem1	
	sub esp, 8
	fstp qword ptr [esp]
	pop edx		;high
	pop eax		;low
	retn 010h	
Math_IEEEModulus ENDP

Math_ModulusTruncateDivisor PROC	; is more clear, but get an overhead in performance!
	push ebp
	mov ebp, esp
	push dword ptr [ebp+014h]
	push dword ptr [ebp+010h]	
	push dword ptr [ebp+0Ch]
	push dword ptr [ebp+8]
	call Math_IEEEModulus
	pop ebp
	retn 010h
Math_ModulusTruncateDivisor ENDP

Math_Ln PROC
	fclex			;log_b(x) <-- (log_2(b))^–1 * log_2(x)
	fldl2e			;log_2(e);
	fld1
	fld qword ptr [esp+4]
	fyl2x
	fdiv st, st(1)
	sub esp, 8
	fstp qword ptr [esp]
	pop edx		;high
	pop eax		;low
	retn 8
Math_Ln ENDP

Math_log2 PROC
	fclex
					;log_b(x) <-- (log_2(b))^–1 * log_2(x)
	fld1
	fld qword ptr [esp+4]
	fyl2x			;log_2(x);
	sub esp, 8
	fstp qword ptr [esp]
	pop edx		;high
	pop eax		;low
	retn 8
Math_log2 ENDP

Math_log10 PROC
	fclex			;log_b(x) <-- (log_2(b))^–1 * log_2(x)	
	fldl2t			;log_2(10);
	fld1
	fld qword ptr [esp+4]
	fyl2x
	fdiv st, st(1)
	sub esp, 8
	fstp qword ptr [esp]
	pop edx		;high
	pop eax		;low
	retn 8	
Math_log10 ENDP

Math_Pow PROC	;double x, double y	E: power = x^y
	finit						; STAT = 0000
	fld qword ptr [esp+4*3]		;y
	fld qword ptr [esp+4]		;x
	fyl2x						; st = y * log_2(x)
	fld st						; st(1) = st = y * log_2(x)
	frndint						; st = int(st) | int(y * log_2(x)),  st(1) = y * log_2(x)
	fsub st(1), st				; st(1) = fraction(y * log_2(x))
	fxch						; swap st, st(1); now st = fraction(y * log_2(x)), st(1) = int(y * log_2(x))
	f2xm1						; st = 2^fraction(y * log_2(x)) - 1, st(1) = int(y * log_2(x))
	fld1
	faddp st(1), st				; st = 2^fraction(y * log_2(x)), st(1) = int(y * log_2(x))
	fscale						; st = 2^fraction(y * log_2(x)) * 2^int(y * log_2(x)) ; x^y = mantissa * 2^exp
	ffree st(1)					; 
	push ecx
	push ecx
	fstp qword ptr [esp]
	pop edx
	pop eax
	retn 010h
Math_Pow ENDP

Math_Exp PROC
	fclex
	mov edx, dword ptr [esp+4]
	push dword ptr [esp+8]
	push edx		
	;'e' constant	== 2.718282
	push        4005BF0Ah
	push        0A21A719Bh	;64 bits binary form for 'e'
	call Math_Pow
	retn 8
Math_Exp ENDP

Math_MantissaOf PROC	;FXTRACT—Extract Exponent and Significand
	fclex
	fld qword ptr [esp+4]	;number
	push ebx
	fxtract
	sub esp, 8
	fstp qword ptr [esp]	; the mantissa
	mov edx, dword ptr [esp]
	mov eax, dword ptr [esp+4]
	fistp qword ptr [esp]	;the exponent	
	mov ecx, dword ptr [esp] ; in the low part
	add esp, 8
	mov ebx, dword ptr [esp+0Ch+4]
	mov dword ptr [ebx], ecx
	pop ebx
	retn 0Ch
Math_MantissaOf ENDP

Math_GetNumberFrom PROC
	fclex
	fild dword ptr [esp+0Ch]	;exponent
	fld qword ptr [esp+4]		;mantissa
	fscale
	fstp st(1)
	sub esp, 8
	fstp qword ptr [esp]
	pop edx		;high
	pop eax		;low
	retn 0Ch
Math_GetNumberFrom ENDP

Math_LogN_base PROC
	fclex
	mov edx, dword ptr [esp+4]
	push dword ptr [esp+8]
	push edx						;X
	call Math_Ln;					; Ln(X)	
	;
	mov ecx, dword ptr [esp+0Ch]	;b
	sub esp, 8
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax
	push ecx
	fild dword ptr [esp]
	push ecx
	fstp qword ptr [esp]			; b floating-point
	call Math_Ln;					; Ln(b)
	;
	fld qword ptr [esp]				; st == Ln(X);
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax	
	;	
	fld qword ptr [esp]				; st == Ln(b);
									; st(1) == Ln(X);
	fdivp st(1), st
	fstp qword ptr [esp]			; LogN_base(X);	
	pop edx		;high
	pop eax		;low
	retn 0Ch
Math_LogN_base ENDP

;BEGIN - N O N - I N T R I N S I C   M A T H   F U N C T I O N S 

Math_Cotan PROC		;source operand must be given in radians and must be within the range -2^63 to +2^63
	finit
	fld qword ptr [esp+4]
	fptan		;tan(x);
	fdivrp	st(1), st	; y = 1/tan(x);
	sub esp, 8	
	fstp qword ptr [esp]
	pop edx		;high
	pop eax		;low
	retn 8	
Math_Cotan ENDP

Math_Sec PROC	;Sec(X) = 1 / Cos(X) 
	finit
	mov edx, dword ptr [esp+4]
	push dword ptr [esp+8]
	push edx
	call Math_Cos
	push eax
	push edx
	fld qword ptr [esp]
	fld1
	fdiv st, st(1)
	fstp qword ptr [esp]
	pop edx		;high
	pop eax		;low
	retn 8
Math_Sec ENDP

Math_Cosec PROC	;Cosec(X) = 1 / Sin(X)
	finit
	mov edx, dword ptr [esp+4]
	push dword ptr [esp+8]
	push edx
	call Math_Sin
	push eax
	push edx
	fld qword ptr [esp]
	fld1
	fdiv st, st(1)
	fstp qword ptr [esp]
	pop edx		;high
	pop eax		;low
	retn 8
Math_Cosec ENDP

Math_Inverse_Arcsin PROC ;Arcsin(X) = Atn(X / Sqr(-X * X + 1))
	finit
	fld qword ptr [esp+4]	; X
	fmul qword ptr [esp+4]	; X * X
	fld1
	fchs					;-1		
	fmulp st(1),st			;-1 * (X^2)	
	fld1
	fadd st, st(1)			; -X^2 + 1
	fabs					; |-X'| == |X'| == X'
	fsqrt					; sqrt(-X * X + 1)	
	fld qword ptr [esp+4]	; X	--> st(0)
	fdiv st, st(1)
	sub esp, 8
	fstp qword ptr [esp]	; X / sqrt(-X * X + 1)
	push ecx
	push ecx
	fld1
	fstp qword ptr [esp]
	call Math_ArcTan;
	retn 8
Math_Inverse_Arcsin ENDP

Math_Inverse_Arccos PROC ;Arccos(X) = Atn(-X / Sqr(-X * X + 1)) + 2 * Atn(1) 
	finit
	fld qword ptr [esp+4]	; X
	fmul qword ptr [esp+4]	; X * X
	fld1
	fchs					;-1		
	fmulp st(1),st			;-1 * (X^2)	
	fld1
	fadd st, st(1)			; -X^2 + 1
	fabs					; |-X'| == |X'| == X'
	fsqrt					; sqrt(-X * X + 1)	
	fld qword ptr [esp+4]	; X	--> st(0)
	fchs					; -X
	fdiv st, st(1)
	sub esp, 8
	fstp qword ptr [esp]	; Y = -X / sqrt(-X * X + 1)
	push ecx
	push ecx
	fld1
	fstp qword ptr [esp]	; X	--> ArcTan(Y/X) == ArcTan(Y)
	call Math_ArcTan;
	push eax
	push edx				;save the result on the CPU stack
	;
	fld1
	fld1
	fpatan		;ArcTan(1);
	push		40000000h	
	push		0		
	fmul qword ptr [esp]		; 2.0 * ArcTan(1)
	add esp, 8
	fadd qword ptr [esp]		; Atn(Y) + 2 * Atn(1)
	;
	fstp qword ptr [esp]		; result
	pop edx		;high
	pop eax		;low
	retn 8
Math_Inverse_Arccos ENDP

Math_Inverse_Arcsec PROC ;Arcsec(X) = Atn(X / Sqr(X * X - 1)) + Sgn((X) -1) * (2 * Atn(1)) 
	finit
	fld1
	fld qword ptr [esp+4]	; X
	fmul qword ptr [esp+4]	; X * X	
	fsub st, st(1)			; st = X^2 - 1
	fabs					; |-X'| == |X'| == X'
	fsqrt					; sqrt(X * X - 1)	
	fld qword ptr [esp+4]	; X	--> st(0)
	fdiv st, st(1)
	sub esp, 8
	fstp qword ptr [esp]	; Y = X / sqrt(X * X - 1)
	push ecx
	push ecx
	fld1
	fstp qword ptr [esp]	; X	--> ArcTan(Y/X) == ArcTan(Y)
	call Math_ArcTan;		
	push eax
	push edx				; esp = Atn(*);
	
	fld1	
	fld qword ptr [esp+4*3] ; X
	fsub st, st(1)			; X-1
	sub esp, 8
	fstp qword ptr [esp]
	call Math_Sign;			;Sign(X - 1)
	push eax
	push edx				; esp = Sign(*)
	
	fld1
	fld1
	fpatan		;ArcTan(1);
	push		40000000h	
	push		0		
	fmul qword ptr [esp]		; st = 2.0 * ArcTan(1)
	add esp, 8
	
	fmul qword ptr [esp]		; st = st * Sign(*)
	pop ecx
	pop ecx
		
	fadd qword ptr [esp]		; st = st + Atn(*);
	fstp qword ptr [esp]	; result = Atn(*) + Sign(*) * (2 * Atn(1));	
	pop edx
	pop eax	
	
	retn 8
Math_Inverse_Arcsec ENDP

Math_Inverse_Arccosec PROC ;Arccosec(X) = Atn(X / Sqr(X * X - 1)) + (Sgn(X) - 1) * (2 * Atn(1)) 
	finit
	fld1
	fld qword ptr [esp+4]	; X
	fmul qword ptr [esp+4]	; X * X	
	fsub st, st(1)			; X^2 - 1
	fabs					; |-X'| == |X'| == X'
	fsqrt					; sqrt(X * X - 1)	
	fld qword ptr [esp+4]	; X	--> st(0)
	fdiv st, st(1)
	sub esp, 8
	fstp qword ptr [esp]	; Y = X / sqrt(X * X - 1)
	push ecx
	push ecx
	fld1
	fstp qword ptr [esp]	; X	--> ArcTan(Y/X) == ArcTan(Y)
	call Math_ArcTan;		
	push eax
	push edx				; esp = Atn(*);
		
	fld qword ptr [esp+4*3] ; X
	push ecx
	push ecx
	fstp qword ptr [esp]
	call Math_Sign;			;Sign(X)
	push eax
	push edx				
	fld1
	fld qword ptr [esp]		; st = Sign(X)
	fsub st, st(1)			; st = Sign(X) - 1
	fstp qword ptr [esp]
	
	fld1
	fld1
	fpatan		;ArcTan(1);
	push		40000000h	
	push		0		
	fmul qword ptr [esp]		; st = 2.0 * ArcTan(1)
	add esp, 8
	fmul qword ptr [esp]		; st = st * (Sign(X)- 1)
	pop ecx
	pop ecx
		
	fadd qword ptr [esp]		; st = st + Atn(*);
	fstp qword ptr [esp]	; result = Atn(*) + (Sign(X) - 1) * (2 * Atn(1));	
	pop edx
	pop eax	
	
	retn 8
Math_Inverse_Arccosec ENDP


Math_Inverse_Arccotan PROC ;Arccotan(X) = Atn(X) + 2 * Atn(1) 
	finit
	mov edx, dword ptr [esp+4]
	push dword ptr [esp+8]		;Y==X
	push edx
	sub esp, 8
	fld1
	fstp qword ptr [esp]		;X==X'==1	
	call Math_ArcTan;	;arctan(Y/X)
	sub esp, 8
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax
	fld1
	fld1
	fpatan		;ArcTan(1)
	push		40000000h	
	push		0		
	fmul qword ptr [esp]		; 2.0 * ArcTan(1)
	add esp, 8
	fadd qword ptr [esp]		; Atn(X) + 2 * Atn(1)
	fstp qword ptr [esp]		; result
	pop edx		;high
	pop eax		;low
	retn 8	
Math_Inverse_Arccotan ENDP


;END - N O N - I N T R I N S I C   M A T H   F U N C T I O N S 

FloatingPoint_Compare PROC
	finit
	fld qword ptr [esp+4] ;st==X
	fcomp qword ptr [esp+0Ch]; comp(x,y)
	fstsw ax
	sahf	; ah now in EFLAGS
	jp __Compare_Error;	PF==1
	jc __Less;	
	jz __Equals;
	;is greater otherwise!
	mov eax, 1
	jmp __End_Compare;
__Less:
	mov eax, 0FFFFFFFFh; -1 when X < Y
	jmp __End_Compare;
__Equals:
	xor eax, eax
	jmp __End_Compare;
__Compare_Error:
	mov eax, 0FFFFFFFBh; -5, flags an error for a param of type NaNs
__End_Compare:		
	retn 010h
FloatingPoint_Compare ENDP

Math_Max PROC
	push ebp
	mov ebp, esp
	;
	push dword ptr [ebp+014h]	;Y
	push dword ptr [ebp+010h]
	push dword ptr [ebp+0Ch]	;X
	push dword ptr [ebp+8]
	call FloatingPoint_Compare;
	test eax, eax
	jz __Max_Or_Equals;
	js __Greater;
	jmp __Max_Or_Equals;
	;
__Greater:		;return the Greater
	mov edx, dword ptr [ebp+010h]
	mov eax, dword ptr [ebp+014h]	
	jmp __End;
__Max_Or_Equals:
	mov edx, dword ptr [ebp+8]
	mov eax, dword ptr [ebp+0Ch]
	;
__End:
	mov esp, ebp
	pop ebp
	retn 010h
Math_Max ENDP

Math_Min PROC
	push ebp
	mov ebp, esp
	;
	push dword ptr [ebp+014h]	;Y
	push dword ptr [ebp+010h]
	push dword ptr [ebp+0Ch]	;X
	push dword ptr [ebp+8]
	call FloatingPoint_Compare;
	test eax, eax
	jz __Min_Or_Equals;
	jns __Less;
	jmp __Min_Or_Equals;
	;
__Less:		;return the min/lesser one
	mov edx, dword ptr [ebp+010h]
	mov eax, dword ptr [ebp+014h]	
	jmp __End;
__Min_Or_Equals:
	mov edx, dword ptr [ebp+8]
	mov eax, dword ptr [ebp+0Ch]
	;
__End:
	mov esp, ebp
	pop ebp
	retn 010h
Math_Min ENDP

Math_Sign PROC
	fclex
	fld qword ptr [esp+4]
	ftst
	fstsw ax
	sahf	; ah now in EFLAGS
	jp __Compare_Error;	PF==1
	jc __Less;	
	jz __Equals;
	;is greater otherwise!
	mov eax, 1
	jmp __End_Compare;
__Less:
	mov eax, 0FFFFFFFFh; -1 when X < Y
	jmp __End_Compare;
__Equals:
	xor eax, eax
	jmp __End_Compare;
__Compare_Error:
	mov eax, 0FFFFFFFBh; -5, flags an error for a param of type NaNs
__End_Compare:			
	retn 8
Math_Sign ENDP

FloatingPoint_CompareToInteger PROC
	fclex
	fld qword ptr [esp+4]		; double X
	ficomp dword ptr [esp+0Ch]	; int Y
	fstsw ax
	sahf	; ah now in EFLAGS
	jp __Compare_Error;	PF==1
	jc __Less;	
	jz __Equals;
	;is greater otherwise!
	mov eax, 1
	jmp __End_Compare;
__Less:
	mov eax, 0FFFFFFFFh; -1 when X < Y
	jmp __End_Compare;
__Equals:
	xor eax, eax
	jmp __End_Compare;
__Compare_Error:
	mov eax, 0FFFFFFFBh; -5, flags an error for a param of type NaNs
__End_Compare:			
	retn 0Ch;
FloatingPoint_CompareToInteger ENDP

FloatingPoint_ToInt PROC ;
	fld qword ptr [esp+4]
	push ecx
	fistp dword ptr [esp]	;as int32
	pop eax
	retn 8
FloatingPoint_ToInt ENDP

FloatingPoint_ToInt64 PROC ;
	fld qword ptr [esp+4]
	push ecx
	push ecx
	fistp qword ptr [esp]	;as int64
	pop eax		;low
	pop edx		;high	
;	test eax,eax
;	jz __UpdateLowPart;	
;	jmp __EndConv64;	
;__UpdateLowPart:
;	mov eax, edx
;	xor edx, edx
;__EndConv64:
	retn 8
FloatingPoint_ToInt64 ENDP

FloatingPoint_FromInt PROC
	fild dword ptr [esp+4]
	sub esp, 8
	fstp qword ptr [esp]
	pop edx		;high
	pop eax		;low
	retn 4;
FloatingPoint_FromInt ENDP

FloatingPoint_FromInt64 PROC
	fild qword ptr [esp+4]
	sub esp, 8
	fstp qword ptr [esp]
	pop edx		;high
	pop eax		;low
	retn 8;
FloatingPoint_FromInt64 ENDP

Math_Pow2 PROC	; Recursive Power with Integer Exponent
	;double expr, int n = exp
	; following this equation:
	;---------------------------------------
	;E:	x^n == ...
	; { 1					n = 0,
	; { x^2		[n%2==0]	n > 0, n is even
	; { x(x^2)	[n%2!=0]	n > 0, n is odd.
	;---------------------------------------
	cmp dword ptr [esp+0Ch], 0
	jz __ret1;
	;
	xor edx, edx	
	mov eax, dword ptr [esp+0Ch]
	mov ecx, 2
	idiv ecx
	push eax				; n / 2
	test edx, edx
	jnz __odd_block;
	;even
	fld qword ptr [esp+4*2]	
	fmul qword ptr [esp+4*2] ; pow2(x*x, n/2)
	push ecx
	push ecx
	fstp qword ptr [esp]
	call Math_Pow2;
	retn 0Ch
__odd_block:
	fld qword ptr [esp+4*2]	
	fmul qword ptr [esp+4*2] ; x*x
	push ecx
	push ecx
	fstp qword ptr [esp]
	call Math_Pow2;
	fld qword ptr [esp+4]
	push eax
	push edx
	fmul qword ptr [esp]	; x * pow2(x*x, n/2)
	fstp qword ptr [esp]
	jmp __ret_Pow
__ret1:
	fld1
	push ecx
	push ecx
	fstp qword ptr [esp]
__ret_Pow:
	pop edx
	pop eax	
	retn 0Ch
Math_Pow2 ENDP

Math_Odd	PROC    
	mov ecx, 2
	xor edx, edx
	mov eax, dword ptr [esp+4]
	idiv ecx
	test edx, edx
	setnz al
	retn 4
Math_Odd	ENDP

Math_Even	PROC    
	mov ecx, 2
	xor edx, edx
	mov eax, dword ptr [esp+4]
	idiv ecx
	test edx, edx
	setz al
	retn 4
Math_Even	ENDP

Math_Trunc PROC
	fld qword ptr [esp+4]
	frndint
	sub esp, 8
	fstp qword ptr [esp]
	pop edx
	pop eax
	retn 8
Math_Trunc ENDP


StringHandling_StringLength2 PROC
	push ebp
	mov ebp, esp
	push 0
	push ebx
	push esi
	push edi
	xor eax, eax
	mov edx, dword ptr [ebp+8]
_Calc_Length:	
	cmp byte ptr [edx], 0
	jz _Return;
	inc eax
	inc edx
	jmp _Calc_Length;
_Return:	
	pop edi
	pop esi
	pop ebx
	pop ecx
	pop ebp
	ret 4
StringHandling_StringLength2 ENDP

StringHandling_StringLength PROC
	push esi
	push edi
	push ebx
	;
	xor eax, eax
	mov edi, dword ptr [esp+4+3*4]
	mov ecx, 0FFFFFFFFh		;max number of chars we can check
	repne scasb				;while ecx > 0 or found
	test ecx, ecx			;is not a null terminated C string
	je __NotNullTerminated;
	;
	not ecx					;turn from negative to positive
	dec ecx					;(length + 1 for null char) - 1 to skip the null char in the count
	mov eax, ecx			;return the count
	jmp _StringLengthOk;
__NotNullTerminated:
	mov eax, 0FFFFFFFFh		; -1 for not currently determine the string length	
_StringLengthOk:
	pop ebx
	pop edi
	pop esi
	retn 4	
StringHandling_StringLength ENDP

StringHandling_StringCopy PROC	;string ref target, string source, int length
	push ebp
	mov ebp, esp
	push esi
	push edi
	push ebx
	;
	mov esi, dword ptr [ebp+0Ch]
	test esi, esi
	je __EndCopy;
	mov edi, dword ptr [ebp+8]
	test edi, edi
	je __EndCopy;
	mov ecx, dword ptr [ebp+010h]
	test ecx, ecx
	je __EndCopy;
	;do copy
	rep movsb	;copy(dest, src);
	mov byte ptr [edi], 0 ; the null character finishes this string copy
	;
__EndCopy:
	pop ebx
	pop edi
	pop esi
	pop ebp
	retn 8
StringHandling_StringCopy ENDP

StringHandling_StringCompare PROC	; string target, string source, int length
	push esi
	push edi
	push ebx
	;
	mov esi, dword ptr [esp+8+3*4]
	mov edi, dword ptr [esp+4+3*4]
	mov ecx, dword ptr [esp+0Ch+3*4]	;length
	repe cmpsb		;compare(dest, src);
	;now, we can use EFLAGS to determine which is less, greater or both equals
	;  a <= b == !(b < a)
	jl __SrcIsLess;		Dest > Src
	jg __SrcIsGreater;
	xor eax, eax
	jmp __EndCompare;
__SrcIsLess:
	mov eax, 1
	jmp __EndCompare;
__SrcIsGreater:
	mov eax, 0FFFFFFFFh
__EndCompare:
	pop ebx
	pop edi
	pop esi
	retn 0Ch	
StringHandling_StringCompare ENDP

StringHandling_StringUCase PROC	;string ref source, int length
	push esi
	push edi
	push ebx
	;
	mov edi, dword ptr [esp+4+3*4]
	mov ecx, dword ptr [esp+8+3*4]
	test edi, edi
	jz __EndUCase;
	test ecx, ecx
	jz __EndUCase;
__LoopChangeCase:
	and dword ptr [edi], 20h
	jz offset __skip_upper_char;
	xor dword ptr [edi], 20h
__skip_upper_char:
	inc edi
	dec ecx		
	jne __LoopChangeCase;
	;
__EndUCase:
	pop ebx
	pop edi
	pop esi
	retn 4
StringHandling_StringUCase ENDP

StringHandling_StringLCase PROC	;string ref source, int length
	push esi
	push edi
	push ebx
	;
	mov edi, dword ptr [esp+4+3*4]
	mov ecx, dword ptr [esp+8+3*4]
	test edi, edi
	jz __EndUCase;
	test ecx, ecx
	jz __EndUCase;
__LoopChangeCase:
	or dword ptr [edi], 20h
	inc edi
	dec ecx		
	jne __LoopChangeCase;
	;
__EndUCase:
	pop ebx
	pop edi
	pop esi
	retn 4
StringHandling_StringLCase ENDP

Math_Gcd PROC
	push ebp
	mov ebp, esp
	sub esp, 8
	push ebx
	push esi
	push edi
	mov dword ptr [ebp-8], edx ; b
	mov dword ptr [ebp-4], ecx ; a
	xor eax, eax
	;
	cmp edx, 0
	jz _ret_a
	;
	xor edx, edx  ; a%b
	mov eax, dword ptr [ebp-4] 
	mov ecx, dword ptr [ebp-8] ; b
	div ecx
	call Math_Gcd		;call it recursively
	jmp _ret_end
_ret_a:
	mov eax, ecx
_ret_end:
	pop edi
	pop esi
	pop ebx
	mov esp, ebp
	pop ebp
	retn 8
Math_Gcd ENDP

Integer_toStringEx PROC ;toStringEx(Int32 expr, short radix, char [] result)
     push	edi
     push	esi
     push	ebx
     mov ebx, dword ptr [esp+8+3*4]		;radix
     mov eax, dword ptr [esp+4+3*4]		;integer value     
     mov edi, dword ptr [esp+0Ch+3*4]	;we expect a minimum buffer with at least 20h bytes
     xor	ecx, ecx				;digit counter
     test eax, eax
     jnl __setup;
     cmp ebx, 0Ah			;compare to decimal radix
     jnz __setup;
     neg eax				;we make it positive for the conversion
     mov ecx, 1
     __setup:
     cmp   	ebx,2			;EBX must be between
     jb		__unsupported_radix           ; 2 and 16.
     cmp   	ebx, 010h
     ja		__unsupported_radix		 
     sub esp, 20h			;use a local array
     lea esi, [esp]
     add	esi, 1Fh				;we expect a minimum buffer with at least 20h bytes
     and dword ptr [esi], 0			;set the null char          
	 push	ecx						;to keep track of sign
__get_char_digit:  
     xor 	edx, edx         		; clear dividend to zero
     div 	ebx           			; divide EAX by the radix (EAX / EBX)

     xchg  	eax, edx       			; exchange quotient, remainder
     push  	ebx  	  				;save the radix            
     mov   	ebx, offset __xtable	;translate table
     xlat							;look up ASCII digit
     pop   	ebx                   
     dec	esi              		; back up in buffer; the first time, we save the null character to terminate the string
     mov   	byte ptr [esi], al		; move digit into buffer
     xchg  	eax,edx           		; swap quotient into EAX

     inc   	ecx              		; increment digit count
     test	eax, eax           		; quotient = 0?
     jnz   	__get_char_digit		; no: divide again

	 pop	edx						;ask for the sign
	 test	dl, dl
	 jz __return_now;
	 dec	esi
	 inc	ecx
	 mov   	byte ptr [esi], '-'
__return_now:
     ; returns the number of digits from ECX
     mov eax, ecx
     ;now, copy the result to user buffer
     rep movsb
     add esp, 20h					;destroy locals
__unsupported_radix:
     pop ebx
     pop esi
     pop edi
     ret 0Ch
Integer_toStringEx ENDP

Integer_toString PROC
    push edi
    push esi
    mov esi, dword ptr [esp+4*3]
	push 20h
	call System_Memory_New;
	mov edi, eax
	push edi
	push 0Ah
	push esi
	call Integer_toStringEx;
__end_conversion:
	mov eax, edi
	pop esi
	pop edi
	retn 4
Integer_toString ENDP

Integer_fromString PROC		;fromString(string value); ; ecx = ch - '0' + ecx * 0Ah    
	push esi
	mov esi, dword ptr [esp+8]
	xor ecx, ecx	
	push ecx
__remove_blanks:
	mov al, byte ptr [esi]
	cmp al, 20h		;' '
	jnz __if_sign;
	inc esi
	jmp __remove_blanks;
__if_sign:
	test cl, cl
	jnz __add_byte;
	mov al, byte ptr [esi]
	cmp al, 2Dh		;'-'
	jnz __add_byte;
	mov cl, 1
	inc esi					;skip the sign
	mov al, byte ptr [esi]
	cmp al, 20h		;' '
	jz __remove_blanks;
__add_byte:
	xor eax, eax
	mov al, byte ptr [esi]
	cmp al, 30h
	jl __end_read;
	cmp al, 39h
	jg __end_read;
	sub al, 30h
	mov ebx, eax
	mov eax, 0Ah
	imul dword ptr [esp]
	add eax, ebx
	mov dword ptr [esp], eax
	inc esi
	jmp __add_byte
__end_read:
	pop eax
    test cl, cl
    jz __return;    
    neg eax;
    __return:	
	pop esi
	retn 4
Integer_fromString ENDP

FloatingPoint_toString PROC	
	push edi
	push 40h
	call System_Memory_New
	mov edi, eax
	push eax
	mov edx, dword ptr [esp+4+4*2]
	push dword ptr [esp+8+4*2]
	push edx
	call FloatingPoint_toStringEx;	
	pop edi
	retn 8
FloatingPoint_toString ENDP


FloatingPoint_toStringEx PROC ;toStringEx(double expr, char [] result)
	push ebp
	mov ebp, esp
	sub esp, 14h			;to hold the fraction part
	xor ecx, ecx
	push ecx				;flag for the sign
	push ecx				;flag for fp with integer part only											
	push edi
	push esi		
	mov edi, dword ptr [ebp+10h]
	finit
_number_analysis0:	
	fld qword ptr [ebp+8]
	ftst				; is zero ?
	fstsw ax
	sahf
	jz __is_zero;
	jnc _number_analysis; st > 0 ?
	fabs				; st = |st|
	mov byte ptr [ebp-18h], 1	;st < 0 ? save sign info	
_number_analysis:	
	fld st(0)			;duplicate the number
	frndint				; round it		
	fclex						
	fcomp 				;now, compare between numbers : st == st(1) ? compare and pop st
	fnstsw ax
	sahf	
	jp __end_analysis_NaN	;for NaNs
	jz __bcd_conversion;
_determine_max:
	fclex	
	ffree st(1)
	ffree st
	;
	lea edx, [__FP_MAX_RANGE+ecx*8]
	fld qword ptr [edx]
	fld qword ptr [ebp+8]	;st = theNumber
	fabs					; st = fabs(st)
	fcom
	fstsw ax	
	sahf					; st <= st(1) ?	
	jp __end_analysis_NaN	;for NaNs
	jc __raise_power_10;	
	jz __raise_power_10;
	inc ecx;
	cmp ecx, 8
	jz __too_big;
	jmp _determine_max;
__too_big:
	xor ecx, ecx
	jmp __bcd_conversion;
__raise_power_10:	
	lea ebx, [__FP_MAX_POWER_10+ecx*8]
	fld qword ptr [ebx]
	fmulp st(1), st			; st = st(1) * st
	frndint				; round it	
	mov cl, byte ptr [__FP_MAX_EXPONENT+ecx]
__bcd_conversion:
	sub esp, 0Ah		; up to 18-digits
	fbstp [esp]			; convert to packed BCD		
	xor ebx, ebx		; offset to the next BCD byte
	;
	test ecx, ecx		; ecx == 0? this is an integer value; so process it as is
	jnz __fraction_part0;
	mov dword ptr [ebp-1Ch], 1	;has integer part only
	mov ecx, 9			; we mean, read the 18 digits
	jmp __first_not_zero;
__fraction_part0:
	mov eax, ecx
	xor edx, edx
	mov ecx, 2
	idiv ecx	
	mov ecx, eax		; set counter		
	xor esi, esi		
__remove_zeroes:
	mov al, byte ptr [esp+ebx]
	test al, al
	jnz __fraction_part;
	inc ebx
	loop __remove_zeroes;
	test ecx, ecx			;ecx==0 ? the number is too small
	jz __failed_conv_then_zero;
__fraction_part:	
	mov al, byte ptr [esp+ebx]
	and al, 0Fh			;low nibble
	add al, 030h		;ASCII char	
	mov byte ptr [ebp-14h+esi], al		; from right-to-left every digit
	mov al, byte ptr [esp+ebx]	
	shr al, 4			; high nibble	
	add al, 030h		;ASCII char
	inc esi
	mov byte ptr [ebp-14h+esi], al
	inc esi
	inc ebx				;number of bytes of fraction read
	loop __fraction_part;
	;
	;if edx % 2 ==1 then we must read the current byte and set the period '.' and then the next digit
	test edx, edx
	jz __integer_part0;
	mov al, byte ptr [esp+ebx]
	and al, 0Fh			;low nibble
	add al, 030h		;ASCII char
	mov byte ptr [ebp-14h+esi], al	; write fraction from end-to-start
	inc esi
	mov byte ptr [ebp-14h+esi], 02Eh	; '.fraction'	
	mov al, byte ptr [esp+ebx]
	shr al, 4			; high nibble
	add al, 030h		;ASCII char	
	inc esi
	inc ebx				;add to number of bytes of fraction read
	mov byte ptr [ebp-14h+esi], al		; 'd.fraction'		
	inc esi
	jmp __integer_part;
__integer_part0:
	mov byte ptr [ebp-14h+esi], 02Eh	; '.fraction'
	inc esi
__integer_part:
	mov eax, 9							; 9 bytes = 18 digits
	sub eax, ebx						; x = (9-ebx) * 2 == explore up to x digits
	mov ecx, eax						; used in the next loop
	xor ebx, ebx			
	test ecx, ecx						;if ecx is zero, just copy the fraction part
	jz __assign_zero;
__first_not_zero:						
	;determine the first byte with a non-zero nibble
	lea eax, [ebx-8]
	neg eax
	mov al, byte ptr [esp+eax]
	test al, al
	jnz __read_int_part0;	
	inc ebx
	loop __first_not_zero;
	;	
	mov edi, dword ptr [ebp+10h]		;if we got here, we just set the sequence to | '-', | '0', '.'
	cmp byte ptr [ebp-18h], 1
	jnz __assign_zero;					;first, determine the sign flag	
	mov byte ptr [edi], 02Dh			; '-'
	inc edi	
__assign_zero:
	test edx, edx
	jnz __complete_number;
	mov byte ptr [edi], 030h
	inc edi
	jmp __complete_number;
	;
__read_int_part0:
	mov edi, dword ptr [ebp+10h]	;target buffer
	cmp byte ptr [ebp-18h], 1
	jnz __no_sign;
	mov byte ptr [edi], 02Dh	; '-'
	inc edi		
__no_sign:
	lea eax, [ebx-8]
	neg eax
	mov al, byte ptr [esp+eax]		;decompose the current byte
	shr al, 4			; high nibble first
	test al, al
	jz __read_int_low_part;
__read_int_high_part:
	lea eax, [ebx-8]
	neg eax
	mov al, byte ptr [esp+eax]		;decompose the current byte
	shr al, 4			; high nibble first
	add al, 30h
	mov byte ptr [edi], al
	inc edi
__read_int_low_part:
	lea eax, [ebx-8]
	neg eax
	mov al, byte ptr [esp+eax]		;decompose the current byte
	and al, 0Fh			; low nibble	
	add al, 30h
	mov byte ptr [edi], al
	inc edi	
	inc ebx
	loop __read_int_high_part;
	;
	cmp dword ptr [ebp-1Ch], 1
	jz __end_conversion;			;if has int part only, just end
__complete_number:
	lea edx, [ebp-14h]
	lea eax, [edx+esi-1]			;to begin reading from the correct byte and others
	mov al, byte ptr [eax]
	mov byte ptr [edi], al
	inc edi
	dec esi
	test esi, esi
	jnz __complete_number;
	jmp __end_conversion;
	;
__end_conversion:
	add esp, 0Ah		;restore stack
	cmp dword ptr [ebp-1Ch], 1
	jz __dot_zero;			;if has int part only, just end	
	jmp __end_analysis;
__failed_conv_then_zero:
	add esp, 0Ah			; if we get here, we couldn't convert this number; the reason: too long, or too small
__is_zero:					; "0.0"	
	mov byte ptr [edi], 30h
	inc edi
__dot_zero:
	mov byte ptr [edi], 2Eh
	inc edi
	mov byte ptr [edi], 30h
	inc edi
	jmp __end_analysis;
__end_analysis_NaN:
	mov byte ptr [edi], 4Eh
	inc edi
	mov byte ptr [edi], 61h
	inc edi
	mov byte ptr [edi], 4Eh
	inc edi	
__end_analysis:
	fclex
	mov byte ptr [edi], 0
	pop esi
	pop edi
	mov eax, dword ptr [ebp+10h]
	leave
	retn 0Ch
FloatingPoint_toStringEx ENDP

;
;The function __AccumValue_Number, is used exclusively by : FloatingPoint_fromString
;
__AccumValue_Number PROC	; (char [] buffer, double ref value, Int32 ref digitCount, ErrorCode error, bool ref errTooManyDigits);
	push ebp
	mov ebp, esp
	push 0Ah				; to raise the number to a power of base 10^iterations
	;
	push edi
	mov esi, dword ptr [ebp+8]	
	mov edi, dword ptr [ebp+0Ch]
	;
	mov al, byte ptr [esi]
	cmp al, 30h
	jl __number_error;
	cmp al, 39h
	jg __number_error
	;	
__accumulate:	;Accumulate the value as long as the total allowable number of digits has not been exceeded.
	mov edx, dword ptr [ebp+10h]
	add dword ptr [edx], 1		; ++digitCount
	mov eax, dword ptr [edx]	; digitCount
	cmp eax, MAX_DIGITS_COUNT
	jg __eTooManyDigits;
	;
	fclex
	mov edx, dword ptr [ebp+0Ch]	;value
	fld qword ptr [edx]
	fimul dword ptr [ebp-4]			; st = st * 0Ah
	xor ebx, ebx
	mov bl, byte ptr [esi]
	sub bl, 030h
	push ebx
	fild dword ptr [esp]
	faddp st(1), st
	fstp qword ptr [edx]			; v = v * 0Ah + digit
	pop ebx
	;
__next_char:
	inc esi
	mov al, byte ptr [esi]
	cmp al, 30h
	jl __end_accum_true;
	cmp al, 39h
	jg __end_accum_true;	
	jmp __accumulate;
	;	
__end_accum_true:
	mov eax, 1			;true	
	jmp __end_accumulate;
__eTooManyDigits:
	mov edx, dword ptr [ebp+18h]
	mov byte ptr [edx], 1
	jmp __next_char;
__number_error:
	xor eax, eax		;false
	cmp dword ptr [ebp+14h]	, INVALID_NUMBER	;Invalid Number
	jz __enumber;
	cmp dword ptr [ebp+14h]	, INVALID_FRACTION	;Invalid Fraction
	jz __efraction;
	cmp dword ptr [ebp+14h]	, INVALID_EXPONENT	;Invalid Exponent
	jz __eexponent;
__enumber:
	push SIZEOF __msg_enumber
	push OFFSET __msg_enumber;
	jmp __error;
__efraction:
	push SIZEOF __msg_efraction
	push OFFSET __msg_efraction;
	jmp __error;
__eexponent:
	push SIZEOF __msg_eexponent
	push OFFSET __msg_eexponent;
	jmp __error;
__error:
	call Console_WriteString;
	call Console_WriteCrLf;
__end_accumulate:
	pop edi	
	leave
	retn 14h
__AccumValue_Number ENDP


FloatingPoint_fromString PROC ;fromString(string value);
	push ebp
	mov ebp, esp
	sub esp, 2Ch
	push edi
	push esi	
	xor eax, eax
	mov ecx, 0Bh
	lea edi, dword ptr [ebp-2Ch]
	rep stosd	
	mov esi, dword ptr [ebp+8]
__remove_blanks:
	mov al, byte ptr [esi]
	cmp al, 20h		;' '
	jnz __if_sign;
	inc esi
	jmp __remove_blanks;
__if_sign:
	mov al, byte ptr [esi]
	cmp al, 2Dh		;'-'
	jnz __accumulate;
	mov dword ptr [ebp-2Ch], 1		; this number has sign
	inc esi
__accumulate:
						;__AccumValue_Number(char [] buffer, double ref value, Int32 ref digitCount, ErrorCode error, bool ref errTooManyDigits);
	lea edx, dword ptr [ebp-20h]	;flag errTooManyDigits
	push edx
	push INVALID_NUMBER
	lea eax, dword ptr [ebp-4]		; digitCount
	push eax
	lea ebx, dword ptr [ebp-0Ch]	; theNumber
	push ebx
	push esi						; buffer
	call __AccumValue_Number;
	xor edx, edx
	test eax, eax	
	jz __end_convert				; this may be an error
	;
	mov eax, dword ptr [ebp-4]
	mov dword ptr [ebp-10h], eax	; wholePlaces = digitCount	
	mov bl, byte ptr [esi]
	cmp bl, 2Eh						; found a fraction?;
	jnz __process_exponent0;
	inc esi							;skip the '.'
	;we have a fraction part; so let's acumulate it in the double variable
						;__AccumValue_Number(char [] buffer, double ref value, Int32 ref digitCount, ErrorCode error, bool ref errTooManyDigits);
	lea edx, dword ptr [ebp-20h]	;flag errTooManyDigits
	push edx
	push INVALID_FRACTION
	lea eax, dword ptr [ebp-4]		; digitCount
	push eax
	lea ebx, dword ptr [ebp-0Ch]	; theNumber
	push ebx
	push esi						; buffer
	call __AccumValue_Number;		
	xor edx, edx
	test eax, eax	
	jz __end_convert				; this may be an error
	;keep track of the decimal places from the point
	mov eax, dword ptr [ebp-4]		;digitCount	
	sub eax, dword ptr [ebp-10h]	
	mov dword ptr [ebp-14h], eax	; decimals = digitCount - wholePlaces	
	;
__process_exponent0:
	mov bl, byte ptr [esi]
	cmp bl, 45h						; 'E'
	jz __process_exponent;
	cmp bl, 65h						; 'e'
	jz __process_exponent;
	jmp __complete_number;
__process_exponent:
	mov byte ptr [ebp-18h],	2Bh		; '+' the default sign for the exponent
	inc esi;						; skip it
	;compare the current sign
	mov bl, byte ptr [esi]
	cmp bl, 2Bh						; '+'
	jz __skip_exp_sign
	cmp bl, 2Dh						; '-'
	jz __skip_exp_sign
	jmp __getExponent;
__skip_exp_sign:
	mov byte ptr [ebp-18h],	bl;
	inc esi
__getExponent:
	and dword ptr [ebp-4], 0
						;__AccumValue_Number(char [] buffer, double ref value, Int32 ref digitCount, ErrorCode error, bool ref errTooManyDigits);
	lea edx, dword ptr [ebp-20h]	;flag errTooManyDigits
	push edx
	push INVALID_EXPONENT;
	lea eax, dword ptr [ebp-4]		; digitCount
	push eax	
	lea edx, dword ptr [ebp-28h]	; exp_value
	push edx
	push esi
	call __AccumValue_Number;		
	xor edx, edx
	test eax, eax	
	jz __end_convert				; this may be an error
	cmp byte ptr [ebp-18h], 2Dh		; if exp is negative: i.e: 999.999e-999
	jnz __complete_number
	fld qword ptr [ebp-28h]			; st = the exponent
	fchs	
	fstp qword ptr [ebp-28h]		; exp = -exp;
__complete_number:
	;
	cmp byte ptr [ebp-20h], 1		;flag errTooManyDigits 
	jz __err_too_many_digits;
	;
	fld qword ptr [ebp-28h]			; st = the exponent
	push ecx						; now, determine the final exponent
	fistp dword ptr [esp]			; exp = -exp;
	mov eax, dword ptr [esp]
	sub eax, dword ptr [ebp-14h]	;
	mov dword ptr [esp], eax		; esp = exp - decimalPlaces
	add eax, dword ptr [ebp-10h]	
	cmp eax, MAX_EXPONENT_VALUE		; exp + wholePlaces > MAX_EXPONENT_VALUE
	jg __err_out_of_range;
	cmp eax, -MAX_EXPONENT_VALUE	; exp + wholePlaces < -MAX_EXPONENT_VALUE
	jl __err_out_of_range;
	;
	mov eax, dword ptr [esp]
	test eax, eax
	jz __finish_number000;
	fclex
	fild dword ptr [esp]			; st = to_float(int_exp)
	sub esp, 0Ch
	fstp qword ptr [esp+4]			; esp = st
	push 0Ah
	fild dword ptr [esp]			; st = 10.0
	fstp qword ptr [esp]			; esp = st
	call Math_Pow;
	push eax
	push edx
	fld qword ptr [esp]	
	fld qword ptr [ebp-0Ch]			; theNumber
	frndint							; st = round(theNumber)
	fmulp st(1), st
	cmp dword ptr [ebp-2Ch], 1
	jnz __get_mult_result;	
	fchs							; -result = -(theNumber *= pow(10, exp))
__get_mult_result:
	fstp qword ptr [ebp-0Ch]		; +result = theNumber *= pow(10, exp);
	pop ecx
	pop ecx	
	jmp __finish_number;
__finish_number000:
	cmp dword ptr [ebp-2Ch], 1
	jnz __finish_number;
	fclex
	fld qword ptr [ebp-0Ch]
	fchs
	fstp qword ptr [ebp-0Ch]		; theNumber = -theNumber
__finish_number:
	pop ecx	
	mov edx, dword ptr [ebp-0Ch]	;high part
	mov eax, dword ptr [ebp-8]		;low part	
	jmp __end_convert;
__err_out_of_range:
	pop ecx
	push SIZEOF __msg_fp_out_of_range;
	push OFFSET __msg_fp_out_of_range;
	jmp __error;
__err_too_many_digits:
	push SIZEOF __msg_too_many_digits;
	push OFFSET __msg_too_many_digits;
__error:
	call Console_WriteString
	call Console_WriteCrLf;
	xor eax, eax
	xor edx, edx
__end_convert:
	pop esi
	pop edi
	leave
	retn 4
FloatingPoint_fromString ENDP

; MEMORY SPECIFICS

System_Memory_GetProcessHeap PROC
	call GetProcessHeap;
	retn
System_Memory_GetProcessHeap ENDP

System_Memory_Memset PROC	;Pointer mem_ptr, int nVal, int nBytes
	push esi
	push edi
	push ebx
	mov eax, dword ptr [esp+8+3*4]	;nVal
	mov edi, dword ptr [esp+4+3*4]
	mov ecx, dword ptr [esp+0Ch+3*4] ; nBytes	
	test edi, edi
	je __EndMemset;
	rep stosb
__EndMemset:
	pop ebx
	pop edi
	pop esi
	retn 0Ch
System_Memory_Memset ENDP

System_Memory_Alloc PROC ; Alloc(Handle hHeap, long nBytes);
	mov eax, dword ptr [esp+8]
	mov edx, dword ptr [esp+4]	
	mov ecx, HEAP_ZERO_MEMORY
	push eax	;nBytes
	push ecx   ;dwFlags
	push edx	;hHeap
	call HeapAlloc;
	retn 8
System_Memory_Alloc ENDP

System_Memory_Free PROC	; Free(Handle hHeap, Pointer mem_ptr);
	mov eax, dword ptr [esp+8]
	mov edx, dword ptr [esp+4]	
	xor ecx, ecx
	push eax	;mem_ptr
	push ecx	;dwFlags
	push edx	;hHeap	
	call HeapFree
	retn 8
System_Memory_Free ENDP

System_Memory_Size PROC ;Size(Handle hHeap, Pointer mem_ptr);
	mov eax, dword ptr [esp+8]
	mov edx, dword ptr [esp+4]	
	xor ecx, ecx
	push eax	;mem_ptr
	push ecx	;dwFlags
	push edx	;hHeap	
	call HeapSize;
	retn 8
System_Memory_Size ENDP

System_Memory_New PROC ;New(long nBytes);
	call GetProcessHeap;
	mov edx, dword ptr [esp+4]	
	push edx ; nBytes
	push eax ; hHeap
	call System_Memory_Alloc;
	retn 4
System_Memory_New ENDP

System_Memory_Destroy PROC ;Destroy(Pointer mem_ptr);
	call GetProcessHeap;
	mov edx, dword ptr [esp+4]	
	push edx ; mem_ptr
	push eax ; hHeap
	call System_Memory_Free;
	retn 4;
System_Memory_Destroy ENDP

System_Memory_SizeFor PROC ;SizeFor(Pointer mem_ptr);
	call GetProcessHeap;
	mov edx, dword ptr [esp+4]
	push edx
	push eax
	call System_Memory_Size;
	retn 4
System_Memory_SizeFor ENDP

System_ShowMessage PROC	;string message, string title, MessageType type
	mov ecx, dword ptr [esp+4*3] ; type
	mov edx, dword ptr [esp+4*2] ; title
	push ebx
	xor ebx, ebx
	push ecx
	push edx
	push dword ptr [esp+010h]	; message
	push ebx
	call MessageBox;
	pop ebx
	retn 0Ch
System_ShowMessage ENDP

StringHandling_StringSet PROC	;Pointer mem_ptr, int nVal, int nBytes
	push edi
	mov edi, dword ptr [esp+8]; mem_ptr
	mov ecx, dword ptr [esp+0Ch]; nVal
	push dword ptr [esp+10h]; nBytes
	push ecx;
	push edi
	call System_Memory_Memset;
	mov eax, dword ptr [esp+8];
	pop edi
	ret 0Ch
StringHandling_StringSet ENDP

__purecall PROC    
	push LENGTHOF __virtual_abstract_call_msg
	push OFFSET __virtual_abstract_call_msg
	call Console_WriteString;
	push 0FFFFFFFFh
	call ExitProcess;
	retn
__purecall ENDP

System_GetCommandLineArgv PROC ;string[] System::GetCommandLineArgv(int ref argc)
	push ebp
	mov ebp, esp
	sub esp, 10h
	push edi
	push esi
	push ebx
	xor eax, eax
	mov ebx, dword ptr [ebp+8]	; Int32 ref argc
	test ebx, ebx
	jz offset __end_argv_proc;
	call GetCommandLine
	mov dword ptr [ebp-4], eax ; the full command line
	push eax
	call lstrlen;
	add eax, 10h ;for the extra null chars that will be necessary later
	mov dword ptr [ebp-8], eax ; length in chars of the full command line
	lea edx, OFFSET __hargc
	push edx
	push dword ptr [ebp-4]
	call CommandLineToArgv;	 ;now, we have an UNICODE array of the command line arguments
	mov esi, eax
	mov dword ptr [ebp-10h], esi ;later, we must destroy this array calling LocalFree
	mov ecx, __hargc
	mov dword ptr [ebx], ecx	; the number of arguments
	;
	mov eax, dword ptr [ebp-8];
	lea ecx, [ecx*4+4];
	add ecx, eax;    addr1, addr2, addr3,...,NULL, str1\0, str2\0, str3\0,....str(n)\0\0
	;
	push ecx ;the total chars supported in this heap array
	call System_Memory_GetProcessHeap;
	push eax
	call System_Memory_Alloc;
	mov edi, eax
	mov dword ptr [ebp-0Ch], eax
	;
	mov ebx, __hargc;
	lea ecx, [ebx*4]
	mov dword ptr [edi+ecx], 0 ;the null to the pointer array
	lea eax, [edi+ecx+4]
	mov dword ptr [ebp-8h], eax; pointer to the first string argument
	mov dword ptr [ebp-4h], 0; last length
__while_loop:
	;process the BinaryHeap structure, by copying its content to another BinaryHeap (FROM UNICODE TO ASCII)
	mov edx, dword ptr [esi];   szwArg = szwArgList[index];
	push edx
	call lstrlen;
	mov dword ptr [ebp-4h], eax ; last string length
	;
	push 0
	push 0
	push eax	; size in buffer
	mov ecx, dword ptr [ebp-8h]; the output buffer in the binary heap...
	push ecx	;LPSTR
	push eax
	mov edx, dword ptr [esi]
	push edx	;LPWSTR
	push 0
	push 0		;CP_ACP  default to ANSI code page
	call WideCharToMultiByte;
	;TODO: check success for this previous call
	;
	mov ecx, dword ptr [ebp-4h];
	mov eax, dword ptr [ebp-8h];
	;lea edx, [eax+ecx]
	;mov dword ptr [edx], 0 ; a null char between strings
	;inc edx;
	lea edx, [eax+ecx+1] ;the string is already null-terminated (the heap memory was zeroed when allocated)
	mov dword ptr [ebp-8], edx; next string buffer available...
	mov dword ptr [edi], eax;			buff[index] = WtoA(szArgList[index]);  the start of the new string
	;
	add esi, 4;
	add edi, 4;
	;
	dec ebx;
	jnz offset __while_loop;
	;
	mov ebx, dword ptr [ebp-10h];
	push ebx;
	call LocalFree;
	mov eax, dword ptr [ebp-0Ch];the array of null-terminated command arguments
__end_argv_proc:
	pop ebx
	pop esi	
	pop edi	
	leave
	ret 4
System_GetCommandLineArgv ENDP

END