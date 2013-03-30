
.386P
.MODEL flat, stdcall
.STACK 1024 * 1024

INCLUDE hcclib32.INC

.DATA

;symbolic constants 
NULL			EQU 0
MAX_READ_CHARS	EQU 050h

ConsoleBuffer BYTE MAX_READ_CHARS DUP(?)
ConsoleTitle  BYTE 100h DUP(?)


;this globals are for test purposes
hello		  BYTE 72,69,76,76,79, 0Dh, 0Ah
hello_len	  DWORD ($ - hello)
hcclib_title  BYTE "H++ Simple Library!", 0Dh, 0Ah, 0
test_file	  BYTE "c:\devicetable.log",0
file_buffer	  BYTE 1024 * 10 DUP(?)
note_pad	  BYTE "notepad.exe",0
test_output	  BYTE "Harold Marzan, Software Engineer & Compiler Writer", 0Dh, 0Ah
test_write_int BYTE "The value in radix 10 of hex 0100h is = ", 0
title_test	  BYTE "Testing H++ Library",0
message_test  BYTE "This Console screen will be cleared before continue tests.",0
__ask_fp_numbers BYTE "Write floating point numbers (10 times each line)",0
__now_write		BYTE "Enter your number",0
__you_wrote		BYTE "You wrote the number: ",0

.CODE


test_fp PROC
	push esi
	
	push 0
	push 0	; 0.0	
	call Console_WriteDouble;	
	call Console_WriteCrLf	
	
	push 03ff00000h
	push 000000000h; 1.0
	call Console_WriteDouble;	
	call Console_WriteCrLf	

	push 040000000h
	push 000000000h; 2.0
	call Console_WriteDouble;	
	call Console_WriteCrLf	
	
	push 0408f4800h
	push 000000000h; 1001.0
	call Console_WriteDouble;	
	call Console_WriteCrLf	

	push 03f57e77dh
	push 0523b3637h; 0.001459
	call Console_WriteDouble;	
	call Console_WriteCrLf	
	
	push 040623ffeh
	push 032a0663ch; 146.0
	call Console_WriteDouble;	
	call Console_WriteCrLf	

	push 0400921fbh
	push 05a7ed197h; 3.14159
	call Console_WriteDouble;	
	call Console_WriteCrLf	


	push 04045ca11h
	push 0dbca9692h; 43.5787
	call Console_WriteDouble;	
	call Console_WriteCrLf	


	push 04068be1eh
	push 09d0e9920h; 197.941
	call Console_WriteDouble;	
	call Console_WriteCrLf	


	push 04023fae1h
	push 047ae147bh; 9.99
	call Console_WriteDouble;	
	call Console_WriteCrLf	


	push 04033ff3bh
	push 0645a1cach; 19.997
	call Console_WriteDouble;	
	call Console_WriteCrLf	


	push 040100095h
	push 06c0d6f54h; 4.00057
	call Console_WriteDouble;	
	call Console_WriteCrLf	


	push 0c007d3c3h
	push 06113404fh; -2.9784
	call Console_WriteDouble;	
	call Console_WriteCrLf	


	push 0c088aba6h
	push 0d9be4cd7h; -789.456
	call Console_WriteDouble;	
	call Console_WriteCrLf	


	push 0c023fae1h
	push 047ae147bh; -9.99
	call Console_WriteDouble;	
	call Console_WriteCrLf	


	push 0c044c46ch
	push 0b10342abh; -41.5346
	call Console_WriteDouble;	
	call Console_WriteCrLf	


	push 040590000h
	push 000000000h; 100
	call Console_WriteDouble;	
	call Console_WriteCrLf	


	push 0408f403dh
	push 070a3d70ah; 1000.03
	call Console_WriteDouble;	
	call Console_WriteCrLf	

	push 0bf235775h
	push 0b2b49dach; -0.000147565
	call Console_WriteDouble;	
	call Console_WriteCrLf	


	push 03f847ae1h
	push 047ae147bh; 0.01
	call Console_WriteDouble;	
	call Console_WriteCrLf	


	push 03f50624dh
	push 0d2f1a9fch; 0.001
	call Console_WriteDouble;	
	call Console_WriteCrLf	


	push 03f1a36e2h
	push 0eb1c432dh; 0.0001
	call Console_WriteDouble;	
	call Console_WriteCrLf	

	push 03fefae14h
	push 07ae147aeh; 0.99
	call Console_WriteDouble;	
	call Console_WriteCrLf	


	push 03fb95810h
	push 0624dd2f2h; 0.099
	call Console_WriteDouble;	
	call Console_WriteCrLf	


	push 03f844673h
	push 081d7dbf5h; 0.0099
	call Console_WriteDouble;	
	call Console_WriteCrLf	

	pop esi
	retn
test_fp ENDP
	
test_fp_from_string PROC
	push SIZEOF __ask_fp_numbers
	push OFFSET __ask_fp_numbers
	call Console_WriteString;
	call Console_WriteCrLf;
	mov esi, 20h
__loopx:
	push SIZEOF __now_write;
	push OFFSET __now_write;
	call Console_WriteString;
	call Console_ReadString;
	push eax;
	call FloatingPoint_fromString;
	push eax
	push edx
	;int 3
		
	push SIZEOF __you_wrote	
	push OFFSET __you_wrote
	call Console_WriteString;
	
	call Console_WriteDouble;
	call Console_WriteCrLf;
	call Console_WriteCrLf;
	
	dec esi
	jns __loopx;
	
	retn
test_fp_from_string ENDP 

main PROC
;
	push ebp
	mov ebp, esp	
	push ebx
	push esi
	push edi
	
	;int 3
	push 040h ;icon information
	push OFFSET title_test
	push OFFSET message_test
	call System_ShowMessage
	
	int 3
	call Console_ClearScreen;
	
	call test_fp;
	
	call test_fp_from_string;
	
	;call Console_ReadString;
	;push eax;
	;call FloatingPoint_fromString;
	;push eax
	;push edx
	;add esp, 8
	
	
	push 6667
	call Integer_toString;
	mov esi, eax
	push 0
	push eax
	call Console_WriteString;
	call Console_WriteCrLf;
	push esi
	call System_Memory_Destroy;
	
	call Console_ReadInteger
	push eax
	call Console_WriteInteger;
	
	;call __purecall;
	
	mov ecx, LENGTHOF test_write_int
	push ecx
	mov eax, OFFSET test_write_int
	push eax
	call Console_WriteString;
	
	push 0100h
	call Console_WriteInteger;
	
	mov eax, OFFSET test_output
	push eax
	call System_OutputString;
	
	call System_IsDebuggerPresent;
	

	call Math_Init;	
	int 3	; breakpoint
	;|-13.1415927| == -13.1415927
	push        0C02A487Eh	;low part
	push        0D69FB466h	;high part
	call Math_Abs
	sub esp, 8
	mov dword ptr [esp], edx	;high of 13.1415927
	mov dword ptr [esp+4], eax	;low of 13.1415927
	
	call Math_ChangeSign	;-13.1415927
	sub esp, 8
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax
	
	push        409F3C00h	; 1999.0
	push        0	
	call Math_Sqrt			; 44.710177812216314199613423002048
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax
	
	call Math_Sqr			; 1999.0
	sub esp, 8
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax		
	
	;int 3	; breakpoint
	push        403E0000h		; 30.0
	push        0
	call Math_Sin				;-0.98803162409286178998774890729446
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax		
	
	push        403E0000h		; 30.0
	push        0
	call Math_Cos				;0.15425144988758405071866214661421
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax		
	
	push        403E0000h		; 30.0
	push        0
	call Math_Tan				;-6.405331196646275784896075505668
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax		

	push        403E0000h		; 30.0
	push        0	
	call Math_Cotan				; 1/-6.405331196646275784896075505668 == -0.15611995216165922287132050523869
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax			
	
	;int 3	; breakpoint
	
	push        403E0000h	; 30.0
	push        0	
	push        402E0000h	;15.0
	push        0
	call Math_ArcTan		;
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax			
	
	push        40059999h	; 2.7
	push        9999999Ah
	call Math_Round
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax				
	
	;int 3	; breakpoint
	
	call Math_pi
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax					
	
	call Math_log_10_base2
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax					

	call Math_log_e_base2
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax					

	call Math_log_2_base10
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax					

	call Math_log_2_base_e
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax						
	
	;int 3	; breakpoint

	push        402E0000h	;15.0	;Divisor
	push        0	
	push        40404C1Ah	;32.594546	;Dividend
	push        1554FBDBh
	call Math_Modulus		; partial modulus ==
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax						

	push        402E0000h	;15.0	;Divisor
	push        0	
	push        40404C1Ah	;32.594546	;Dividend
	push        1554FBDBh
	call Math_ModulusTruncateDivisor	; partial modulus ==
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax						

	push        402E0000h	;15.0	;Divisor
	push        0	
	push        40404C1Ah	;32.594546	;Dividend
	push        1554FBDBh	
	call Math_IEEEModulus
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax							
	
	int 3
	
	push        40404C1Ah	;32.594546	;Dividend
	push        1554FBDBh
	push        40404C1Ah	;32.594546	;Dividend
	push        1554FBDBh	
	call FloatingPoint_Compare	;result ==0
	
	push        40404C1Ah	;32.594546	;Dividend
	push        1554FBDBh
	push        402E0000h	;15.0	;Divisor
	push        0	
	call FloatingPoint_Compare	;result ==-1
	
	push        402E0000h	;15.0	;Divisor
	push        0	
	push        40404C1Ah	;32.594546	;Dividend
	push        1554FBDBh
	call FloatingPoint_Compare	;result ==1	

	int 3
	
	push        402E0000h	;15.0	;Y
	push        0	
	push        40490000h	; 50.0	;X
	push        0		
	call Math_Max;	;50.0
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax	


	push        40404C1Ah	;32.594546	;Y
	push        1554FBDBh
	push        402E0000h	;15.0		;X
	push        0		
	call Math_Max;
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax	

	push        40404C1Ah	;32.594546	;Y
	push        1554FBDBh
	push        40404C1Ah	;32.594546	;X
	push        1554FBDBh
	call Math_Max;
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax	
	
	push        402E0000h	;15.0	;Y
	push        0	
	push        40490000h	; 50.0	;X
	push        0		
	call Math_Min;	;15.0
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax	

	push        40404C1Ah	;32.594546	;Y
	push        1554FBDBh
	push        402E0000h	;15.0		;X
	push        0		
	call Math_Min;
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax	

	push        40404C1Ah	;32.594546	;Y
	push        1554FBDBh
	push        40404C1Ah	;32.594546	;X
	push        1554FBDBh
	call Math_Min;
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax	
	
	push        402E0000h	;15.0	
	push        0		
	call Math_Sign;	
	
	push        0C02E0000h	;-15.0
	push        0	
	call Math_Sign;		
	
	push 0
	push 0
	call Math_Sign;		
	
	;int 3;
	
	push		0Fh			; 15  int	;Y
	push        402E0000h	;15.0 double;X
	push        0			
	call FloatingPoint_CompareToInteger;	
	
	push		0Ah			; 10 int	;Y
	push        40404C1Ah	;32.594546	;X
	push        1554FBDBh
	call FloatingPoint_CompareToInteger;		
	
	push		0100h		; 256 int	;Y
	push        40490000h	; 50.0		;X
	push        0		
	call FloatingPoint_CompareToInteger;		
	
	;int 3;
		
	push        40404C1Ah	;32.594546	;X
	push        1554FBDBh	
	call FloatingPoint_ToInt
	
	push        42541864h		;345,234,234,523.0
	push        1726C000h
	call FloatingPoint_ToInt64
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax	
	
		
	push        40490000h	; 50.0
	push        0	
	call Math_Ln;			;3.9120230054281460586187507879106
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax	
	
	push        40490000h	; 50.0
	push        0	
	call Math_log2;			;5.6438561897747246957406388589788
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax								

	push        40490000h	; 50.0
	push        0		
	call Math_log10;		;1.6989700043360188047862611052755
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax								
	
	int 3
	;Recursive version	
	push		6
	push		40000000h	; 2.0
	push		0	
	call Math_Pow2;			; 64.0

	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax
	
	push		5
	push		40180000h	; 6.0
	push		0
	call Math_Pow2;			; 7776.0
		
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax
	
	;scientific power
	push		40180000h	; 6.0
	push		0
	push		40000000h	; 2.0
	push		0	
	call Math_Pow;			; 64.0

	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax
	
	
	push        40404C1Ah	;32.594546	;Y
	push        1554FBDBh
	push		40000000h	; 2.0
	push		0	
	call Math_Pow;			; x^y = 2^32.594546 == 6485389155.8816977702934985503259
	
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax	
	
	
	push		40000000h	; 2.0
	push		0	
	call Math_Exp			;7.389057031524
	
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax	
	
	;int 3
	
	push eax
	push edx
	call Math_Trunc	; 7.0
	
	push eax
	push edx
	add esp, 8	
	
	push 5
	call Math_Odd	;true
	push 5
	call Math_Even	; false
	
	push 4
	call Math_Even	; true
	push 4
	call Math_Odd	; false
		
	int 3
	
	push ecx
	lea eax, dword ptr [esp]
	push eax
	push        403E0000h		; 30.0
	push        0		
	call Math_MantissaOf;	
	pop ecx	
	
	push ecx
	push eax
	push edx
	call Math_GetNumberFrom;

	push        403E0000h		; 30.0
	push        0	
	call Math_Sec;				;6.4829212349626777884063491596252
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax

	push        403E0000h		; 30.0
	push        0		
	call Math_Cosec				;-1.0121133530701779868975132834767
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax	
	
	int 3
	
	push        403E0000h		; 30.0
	push        0		
	call Math_Inverse_Arcsin;	;0.78567609559630802
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax		
		
	push        403E0000h		; 30.0
	push        0		
	call Math_Inverse_Arccos;	;
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax	
	
	push        403E0000h		; 30.0
	push        0			
	call Math_Inverse_Arccotan;	
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax		

	push        403E0000h		; 30.0
	push        0			
	call Math_Inverse_Arcsec
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax		

	push        403E0000h		; 30.0
	push        0			
	call Math_Inverse_Arccosec
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax	
	
	
;	push		40240000h	; 10.0
;	push		0				
	
	int 3
	push		0Ah	
	push        408F4000h	; 1000.0
	push        0
	call Math_LogN_base;	; 3.0
	mov dword ptr [esp], edx
	mov dword ptr [esp+4], eax		
	
	pop edx	;high
	pop eax	;low		
	
	
	mov eax, OFFSET hcclib_title
	push eax
	call Console_SetTitle;
	
	;temporally get away while testing the math part...
	;jmp __IsTheEnd;
	
	
	call Console_GetTitle;	
	
	mov edx, SIZEOF ConsoleTitle
	push edx
	push eax
	call Console_WriteString;
	
	push 0Ah
	push 0
	call Console_SetCursorPos;
	
	;	
	call Console_ReadChar;
	call Console_ReadString;
	
	;int 3	; breakpoint
	mov edx, OFFSET ConsoleBuffer
	mov eax, SIZEOF ConsoleBuffer
	push eax
	push edx
	call Console_WriteString;	
	
	call Console_WriteCrlf;	
	
	push 000Ch	; Y
	push 0028h	; X
	call Console_SetCursorPos;	
	
	;write chars
	mov eax, OFFSET hello
	push eax
	mov ecx, hello_len
	push ecx
_WriteAllChars:
	mov dword ptr [esp], ecx
	mov dword ptr [esp+4], eax
	;
	push dword ptr [eax]
	call Console_WriteChar;
	mov ecx, dword ptr [esp]
	mov eax, dword ptr [esp+4]
	inc eax	;next char
	loop _WriteAllChars;
			
	call Console_GetTitle;	
	
	mov edx, SIZEOF ConsoleTitle
	push edx
	push eax
	call Console_WriteString;	
	
	mov eax, OFFSET test_file;
	mov edx, GENERIC_READ
	or edx, GENERIC_WRITE	
	push FILE_SHARE_READ
	push edx
	push OPEN_EXISTING
	push eax
	call File_Open;
	mov esi, eax
		
	push esi
	call File_FileSize	
	push eax
	
	mov edi, OFFSET file_buffer
	mov ecx, SIZEOF file_buffer	;could be eax if < 10kb
	push ecx	;nBytesToRead
	push edi	;Buffer
	push esi	;hFile
	call File_Read
	
	;already on the stack	
	;mov ecx, SIZEOF file_buffer	
	;push ecx
	push edi
	call Console_WriteString;			
	
	push esi
	call File_FileSize;
	mov edi, eax
	
	push 200h	;512 bytes from origin
	push 0
	push esi
	call File_Seek;
	
	push edi
	push 0
	push esi
	call File_Seek;
		
	;Write some data...
	mov ecx, SIZEOF hcclib_title
	mov eax, OFFSET hcclib_title
	push ecx
	push eax	
	push esi
	call File_Write;
	
	push esi
	call File_Close;
	
	;int 3 ; breakpoint
	
	push 40h		;X
	push 0Ah		;Y
	call Console_SetConsoleSize
	
	push 4
	call Console_SetTextColor;
	
	call System_CommandLineArgs
	mov esi, eax
	push eax
	call StringHandling_StringLength
	
	push eax
	push esi
	call Console_WriteString;			
	
	push 7
	call Console_SetTextColor;
	
	call System_CommandLineArgs
	mov esi, eax
	push eax
	call StringHandling_StringLength
	
	push eax
	push esi
	call Console_WriteString;
	
	push 18h
	push 4Fh	
	push 5
	push 5	
	call Console_MoveWindow;
	
	mov eax, OFFSET note_pad
	push 1	;show
	push eax
	call System_Execute	
	
	call System_GetTickCount;
	push eax
	
	push 2000
	call System_Sleep;
	
	;2000 milisecs have passed
	call System_GetTickCount;
	;int 3 ; breakpoint
	pop esi
	sub eax, esi
	
__IsTheEnd:
	call System_Exit;
	pop edi
	pop esi
	pop ebx	
	mov esp, ebp
	pop ebp
	ret
;
main ENDP


END main ; the Entry Point 