	org 0x7C00
	bits 16
	
	cli
	
	mov bx, 0
.leitura:
	;Ler teclado
	mov ax, 0
	int 0x16
	
	cmp al, 13
	je .verifica
	
	sub al, 48
	imul bx, 10
	add bx, al
	
	jmp .leitura
	
	
	
.verifica:
	mov ax, bx
	push ax
	
	cmp ax, 1
	je .nPrimo
	
.divide:
	mov dx, 0
	dec bx
	cmp bx, 1
	je .ePrimo
	
	idiv bx
	or dx, dx
	jz .nPrimo
	
	pop ax
	push ax
	jmp .divide
	
.nPrimo:
	
	
.ePrimo:
	
	
.fim:
	
	hlt
	
	
nP:	db "nao e primo!"
eP:	db "e primo!"
	
	times 510 - ($-$$) db 0
	dw 0xAA55
