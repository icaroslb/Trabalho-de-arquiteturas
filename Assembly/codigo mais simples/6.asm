org 0x7c00
bits 16

cli

mov cx, 1
mov ax, 1
mov bx, 0

calcular:
	push ax
	push bx
	push cx
	call printar
	pop cx
	pop bx
	pop ax

	mov dx, ax
	add ax, bx
	mov bx, dx

	cmp cx, 22
	je fim
	inc cx
	jmp calcular

fim:
	hlt

printar:
	mov cx, 0

	dividir:
		mov dx, 0
		mov bx, 10
		idiv bx

		mov dh, 0x0E
		add dl, 48
		push dx

		cmp ax, 0
		je exibir
		inc cx
		
		jmp dividir

	exibir:
		pop ax
		int 0x10
		cmp cx, 0
		je retorno
		dec cx
		jmp exibir

	retorno:
		mov al, ' '
		int 0x10
		ret

times 510 - ($-$$) db 0
dw 0xAA55