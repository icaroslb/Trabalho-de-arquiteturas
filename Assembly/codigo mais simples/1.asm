org 0x7C00
bits 16

cli

int 0x13
	mov ah, 2
	mov al, 32
	mov cl, 2 
	mov ch, 0 
	mov dh, 0 
	mov bx, 0x7E00
int 0x13

mov ax, 0x13
int 0x10

mov cx, 0xA000
mov es, cx
mov di, 0

desenhar:
	mov ax, [bx]
	mov [es:di], ax
	inc bx
	inc di

	cmp bx, 16000
	je fim
	jmp desenhar

fim:
	jmp fim

times 510 - ($-$$) db 0
dw 0XAA55