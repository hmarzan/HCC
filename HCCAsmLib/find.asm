
.386P
.MODEL flat, c
.STACK 1024 * 1024 ;1MB of stack space will suffice!

.DATA	; the data segment

.CODE	; the code segment

;this procedure uses the "C" calling convetion or _cdecl
FindArray PROC
				; FindArray(long value_to_search, long *array | long array[], long count) 
	push ebp
	mov ebp, esp
	push ebx
	push esi
	push edi
	;
	mov ecx, dword ptr [ebp+10h] ; the array length
	mov eax, dword ptr [ebp+8]	 ; the value to search
	mov edi, dword ptr [ebp+0Ch] ; ptr to the first array item
	;
	repne scasd			; do repeat search while ZF = 0	
	;repne scas dword ptr [edi]
	jz _found_value		; if ZF = 1, found	
	xor al,al
	jmp _finish;
	;
_found_value:
	mov al, 1
	;
_finish:	
	pop edi
	pop esi
	pop ebx
	mov esp, ebp
	pop ebp
	ret
FindArray ENDP
END

