	org 0x7c00
	bits 16

cripitado:	
	cli			;Limpo os registradores da bios
	
	int 0x13		;Preparo o disco para enviar um pedido de leitura
	
	mov ah, 0x02		;Informo que quero ler setores
	mov al, 1		;Informo quantos setores quero ler
	mov cl, 2		;Informo apartir de qual setor quero ler (os setores são enumerados a partir do 1)
	mov ch, 0		;Informo qual cilindro quero ler (os cilindros são enumerados a partir do 0)
	mov dh, 0		;Informo qual cabeçote quero usar (os cabeçotes são enumerados a partir do 0)
	mov bx, 0x7E00		;Informo onde quero guardar os dados do setor lido (0x7E00 é o primeiro posição depois do programa)
	
	int 0x13		;Requisito as informações passadas anteriormente
	
	mov si, matricula	;Guardo a posição da matrícula

.descripitar:
	mov al, [bx]		;Pego a letra criptografada atual
	or al, al	
	jz .fim			;Se a frase acabou
	
	mov dl, [si]		;Pego o número da matrícula atual
	cmp dl, 10	
	je .retorno		;Se a matrícula acabou
	
	.calcula:
		sub al, dl		;Decripto diminuindo pelo valor do número atual da matrícula
		mov ah, 0x0E
		int 0x10		;Escrevo a letra descriptografada
		inc si			;Vou para próximo número da matrícula
		inc bx			;Vou para a próxima letra da frase
		jmp .descripitar
	
	
.retorno:
	mov si, matricula	;Reinicio a posição do número atual da matrícula
	mov dl, [si]		;Pego o verdadeiro número atual da matrícula
	jmp .calcula		;Volto pra decriptar
	
.fim:
	hlt
	
;Número da matrícula do aluno cuja mensagem foi gerada terminando por 10 que indica o término da matrícula	
matricula: db 3, 9, 7, 6, 1, 3, 10
	
	times 510 - ($-$$) db 0
	dw 0xaa55
