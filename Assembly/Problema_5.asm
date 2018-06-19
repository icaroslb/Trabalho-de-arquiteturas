	org 0x7c00
	bits 16

MudarCor:
	;Limpa os registradores
	cli
	
	;Incializa a interrupção de vídeo
	mov ax, 0
	mov al, 0x13
	int 0x10

	;Guarda a posição da VRAM
	mov ax, 0xA000
	mov es, ax

.lerTeclado:
	;Lê o teclado
	mov ax, 0
	int 0x16
	jmp .mudarCor



.mudarCor:
	;Guarda o valor lido
	mov dl, al
	
	;Inicializa pra poder percorrer a tela
	mov ax, 0
	mov cx, 64000

.loop:
	mov di, ax
	mov [es:di], dl
	inc ax
	dec cx
	jz .lerTeclado	;Se pintou toda a tela
	jmp .loop

	
	times 510 - ($-$$) db 0
	dw 0xaa55
