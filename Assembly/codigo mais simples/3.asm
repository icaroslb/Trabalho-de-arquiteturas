org 0x7C00
bits 16

cli

mov cx, 0

leTecla:
	mov ah, 0
	int 0x16

	mov ah, 0x0E
	int 0x10
	push ax
	inc cx

	cmp al, 13
	je printar

	jmp leTecla

printar:
	mov al, 10
	int 0x10

	contrario:
		pop ax
		int 0x10
		dec cx
		cmp cx, 0
		je fim
		jmp contrario

fim:
	mov al, 10
	int 0x10
	mov al, 13
	int 0x10
	jmp leTecla		

times 510 - ($-$$) db 0
dw 0xAA55