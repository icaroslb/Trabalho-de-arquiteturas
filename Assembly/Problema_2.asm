	org 0x7c00
	bits 16

cripitado:	
	cli
	
	int 0x13
	
	mov ah, 0x02
	mov al, 1
	mov cl, 2
	mov ch, 0
	mov dh, 0
	mov bx, 0x7E00
	int 0x13
	
	mov si, matricula

.descripitar:
	mov al, [bx]
	or al, al
	jz .fim
	
	mov dl, [si]
	cmp dl, 10
	je .retorno
	
.calcula:
	sub al, dl
	mov ah, 0x0E
	int 0x10
	inc si
	inc bx
	jmp .descripitar
	
	
.retorno:
	mov si, matricula
	mov dl, [si]
	jmp .calcula
	
.fim:
	hlt
	
matricula: db 3, 9, 9, 0, 0, 2, 10
	
	times 510 - ($-$$) db 0
	dw 0xaa55
