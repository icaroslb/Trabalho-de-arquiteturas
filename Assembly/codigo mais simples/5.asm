org 0x7C00
bits 16

cli

mov ax, 0x13
int 0x10

mov cx, 0xA000
mov es, cx

letecla:
	mov ax, 0
	int 0x16

	mov cx, 0

	pinta:
		mov di, cx
		mov [es:di], al
		inc cx
		cmp cx, 64000
		je letecla
		jmp pinta

times 510 - ($-$$) db 0
dw 0xAA55