	org 0x7c00
	bits 16
	
	cli
	
fibo:
	mov ah, 0x0E
	mov al, 49
	int 0x10

	mov ax, 1
	mov bx, 0
	mov cx, 21
	
	hlt
	
	times 510 - ($-$$) db 0
	dw 0xaa55
