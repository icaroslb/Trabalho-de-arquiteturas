org 0x7c00
bits 16

cli

int 0x13
	mov ah, 0x02
	mov al, 1
	mov cl, 2
	mov ch, 0
	mov dh, 0
	mov bx, 0x7E00
int 0x13


mov si, chave
mov ah, 0x0E

printar:
	mov dl, [si]
	mov al, [bx]

	cmp al, 0
	je final

	cmp dl, 10
	je reiniciar

	sub al, dl
	int 0x10

	inc si
	inc bx
	jmp printar

reiniciar:
	mov si, chave
	jmp printar

final: 	
	hlt

chave: 
	db 3, 9, 7, 6, 1, 3, 10 

times 510 - ($-$$) db 0
dw 0xaa55