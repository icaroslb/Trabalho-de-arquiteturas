org 0x7c00
bits 16

cli

mov ax, 0x13
int 0x10

mov cx, 0
mov dx, 0

push dx
mov bx, 0xA000
mov es, bx

inicio:
	pop dx
	inc dx
	push dx
	
	mov di, 0
	mov cx, 0

	pintar:
		mov [es:di], dx

		cmp cx, 64000
		je inicio

		inc cx
		inc di
		inc dx

		jmp pintar

times 510 - ($-$$) db 0
dw 0xaa55