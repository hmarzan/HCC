

.386
.MODEL flat, c ; external "C" linkage
.STACK 1024 * 1024

.DATA

;no global variables where defined

.CODE

Factorial PROC
	push ebp
	mov ebp, esp
	push ebx
	push esi
	push edi
	xor eax, eax
	;
	cmp dword ptr [ebp+8], 0
	jnz offset _calc_fact;
	mov eax, 1
	jmp offset _end_calc;
_calc_fact:
	mov ebx, dword ptr [ebp+8]
	dec ebx;	calc the fact(n-1);
	push ebx
	call Factorial
	mul dword ptr [ebp+8]	; return n * factorial(n-1);
	;
_end_calc:
	pop edi
	pop esi
	pop ebx
	mov esp, ebp
	pop ebp
	ret
Factorial ENDP
END