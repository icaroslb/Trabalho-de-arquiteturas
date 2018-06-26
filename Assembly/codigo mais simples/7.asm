org 0x7C00
bits 16

cli

eprimoounao:
mov cx, 0

loop:
	mov ax, 0
	int 0x16

	cmp al, 13
	je fim

	mov ah, 0x0E
	int 0x10

	sub al, 48
	imul cx, 10

	add cl, al

	jmp loop

fim:
	cmp cx, 1
	je fimnprimo
	push cx
	mov ax, cx

loop2:
	mov dx, 0
	dec cx
	cmp cx, 1
	je fimprimo

	div cx

	cmp dx, 0
	je fimnprimo

	pop ax
	push ax
	jmp loop2


fimprimo:
	mov bx, primo
	mov ah, 0x0E
	jmp printar

fimnprimo:
	mov bx, nprimo
	mov ah, 0x0E
	jmp printar

printar:
	mov al, [bx]
	int 0x10
	cmp al, '!'
	je final
	inc bx
	jmp printar

final:
	mov al, 10
	int 0x10
	mov al, 13
	int 0x10

	jmp eprimoounao

primo: db " ", 144, " primo!"
nprimo: db " nao ", 144, " primo!"

times 510 - ($-$$) db 0
dw 0xAA55