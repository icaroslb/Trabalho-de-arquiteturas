	org 0x7c00
	bits 16

cores:
	cli

	mov ax, 0
	mov al, 0x13
	int 0x10

	mov ax, 0xA000
	mov es, ax

	mov dl, 0
	push dx

;inicialização de loop
.loop:
	mov ax, 0
	mov cx, 64000

;pintar as telas
.percorrer:
	mov di, ax
	mov [es:di], dl
	inc ax
	dec cx
	jz .proxloop
	inc dl
	jmp .percorrer

;inicializa o proximo loop
.proxloop:
	pop dx
	inc dl
	push dx
	jmp .loop


	times 510 - ($-$$) db 0
	dw 0xaa55
